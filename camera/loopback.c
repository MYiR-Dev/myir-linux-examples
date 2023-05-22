#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <omap_drm.h>
#include <omap_drmif.h>
#include <xf86drmMode.h>
#include "jpeg.h"
#include "loopback.h"

#define ERROR(fmt, ...) \
	do { fprintf(stderr, "ERROR:%s:%d: " fmt "\n", __func__, __LINE__,\
##__VA_ARGS__); } while (0)

#define MSG(fmt, ...) \
	do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); } while (0)

/* Dynamic debug. */
#define DBG(fmt, ...) \
	do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); } while (0)

/* align x to next highest multiple of 2^n */
#define ALIGN2(x,n)   (((x) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
#define FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | \
	((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | \
	((uint32_t)(uint8_t)(d) << 24 ))
#define FOURCC_STR(str)    FOURCC(str[0], str[1], str[2], str[3])

#define PAGE_SHIFT 12
#define NBUF (3)
#define MAX_DRM_PLANES 5

control_t status;

struct dmabuf_buffer {
	uint32_t fourcc, width, height;
	int nbo;
	struct omap_bo *bo[4];
	uint32_t pitches[4];
	bool multiplanar;	/* True when Y and U/V are in separate buffers. */
	int fd[4];		/* dmabuf */
	unsigned fb_id;
};

struct connector_info {
	unsigned int id;
	char mode_str[64];
	drmModeModeInfo *mode;
	drmModeEncoder *encoder;
	int crtc;
	int pipe;
};

/*
* drm output device structure declaration
*/
struct drm_device_info
{
	int fd;
	int width;
	int height;
	unsigned int num_planes;
	char dev_name[9];
	char name[4];
	unsigned int bo_flags;
	struct dmabuf_buffer **buf[2];
	struct omap_device *dev;
	unsigned int crtc_id;
	unsigned int plane_id[2];
	unsigned int prop_id_trans_key_mode;
	unsigned int prop_id_alpha_blender;
	unsigned int prop_id_zorder;
	uint64_t prop_value_trans_key_mode;
	uint64_t prop_value_alpha_blender;
	uint32_t prop_value_zorder[MAX_DRM_PLANES];
	uint32_t drm_plane_id[MAX_DRM_PLANES];
	bool multiplanar;	/* True when Y and U/V are in separate buffers. */
} drm_device;

/*
* V4L2 capture device structure declaration
*/
struct v4l2_device_info {
	int type;
	int fd;
	enum v4l2_memory memory_mode;
	unsigned int num_buffers;
	int width;
	int height;
	char dev_name[12];
	char name[10];

	struct v4l2_buffer *buf;
	struct v4l2_format fmt;
	struct dmabuf_buffer **buffers;
	struct dmabuf_buffer *b[NBUF];

} cap0_device, cap1_device;

static struct omap_bo *alloc_bo(struct drm_device_info *device, unsigned int bpp, unsigned int width, unsigned int height,
								unsigned int *bo_handle, unsigned int *pitch)
{
	struct omap_bo *bo;
	unsigned int bo_flags = device->bo_flags;

	bo_flags |= OMAP_BO_WC;
	bo = omap_bo_new(device->dev, width * height * bpp / 8, bo_flags);

	if (bo) {
		*bo_handle = omap_bo_handle(bo);
		*pitch = width * bpp / 8;
		if (bo_flags & OMAP_BO_TILED)
			*pitch = ALIGN2(*pitch, PAGE_SHIFT);
	}

	return bo;
}


static unsigned int reserved_plane_ids[2];

static int find_reserved_plane(uint32_t plane_id)
{
	unsigned int i;

	for (i = 0; i < sizeof(reserved_plane_ids)/sizeof(unsigned int); ++i) {
		if (reserved_plane_ids[i] == plane_id)
			return i;
	}

	return -1;
}

//You can use DRM ioctl as well to allocate buffers (DRM_IOCTL_MODE_CREATE_DUMB) and drmPrimeHandleToFD() to get the buffer descriptors/ 
static struct dmabuf_buffer *alloc_buffer(struct drm_device_info *device, unsigned int fourcc, unsigned int w, unsigned int h)
{
	struct dmabuf_buffer *buf;
	unsigned int bo_handles[4] = {0}, offsets[4] = {0};
	int ret;

	buf = (struct dmabuf_buffer *) calloc(1, sizeof(struct dmabuf_buffer));
	if (!buf) {
		ERROR("allocation failed");
		return NULL;
	}

	buf->fourcc = fourcc;
	buf->width = w;
	buf->height = h;
	buf->multiplanar = false;

	buf->nbo = 1;

	if (!fourcc)
		fourcc = FOURCC('A','R','2','4');

	switch(fourcc) {
case FOURCC('A','R','2','4'):
	buf->nbo = 1;
	buf->bo[0] = alloc_bo(device, 32, buf->width, buf->height,
		&bo_handles[0], &buf->pitches[0]);
	break;
case FOURCC('U','Y','V','Y'):
case FOURCC('Y','U','Y','V'):
	buf->nbo = 1;
	buf->bo[0] = alloc_bo(device, 16, buf->width, buf->height,
		&bo_handles[0], &buf->pitches[0]);
	break;
case FOURCC('N','V','1','2'):
	if (device->multiplanar) {
		buf->nbo = 2;
		buf->bo[0] = alloc_bo(device, 8, buf->width, buf->height,
			&bo_handles[0], &buf->pitches[0]);
		buf->fd[0] = omap_bo_dmabuf(buf->bo[0]);
		buf->bo[1] = alloc_bo(device, 16, buf->width/2, buf->height/2,
			&bo_handles[1], &buf->pitches[1]);
		buf->fd[1] = omap_bo_dmabuf(buf->bo[1]);
	} else {
		buf->nbo = 1;
		buf->bo[0] = alloc_bo(device, 8, buf->width, buf->height * 3 / 2,
			&bo_handles[0], &buf->pitches[0]);
		buf->fd[0] = omap_bo_dmabuf(buf->bo[0]);
		bo_handles[1] = bo_handles[0];
		buf->pitches[1] = buf->pitches[0];
		offsets[1] = buf->width * buf->height;
		buf->multiplanar = false;
	}
	break;
case FOURCC('I','4','2','0'):
	buf->nbo = 3;
	buf->bo[0] = alloc_bo(device, 8, buf->width, buf->height,
		&bo_handles[0], &buf->pitches[0]);
	buf->bo[1] = alloc_bo(device, 8, buf->width/2, buf->height/2,
		&bo_handles[1], &buf->pitches[1]);
	buf->bo[2] = alloc_bo(device, 8, buf->width/2, buf->height/2,
		&bo_handles[2], &buf->pitches[2]);
	break;
default:
	ERROR("invalid format: 0x%08x", fourcc);
	goto fail;
	}

	ret = drmModeAddFB2(device->fd, buf->width, buf->height, fourcc,
		bo_handles, buf->pitches, offsets, &buf->fb_id, 0);

	if (ret) {
		ERROR("drmModeAddFB2 failed: %s (%d)", strerror(errno), ret);
		goto fail;
	}

	return buf;

fail:
	return NULL;
}

void free_vid_buffers(struct dmabuf_buffer **buf, unsigned int n, unsigned char multiplanar)
{
	unsigned int i;

        if (buf == NULL) return;
	for (i = 0; i < n; i++) {
		if (buf[i]) {
			close(buf[i]->fd[0]);
			omap_bo_del(buf[i]->bo[0]);
			if(multiplanar){
				close(buf[i]->fd[1]);
				omap_bo_del(buf[i]->bo[1]);
			}
			free(buf[i]);
		}
	}
	free(buf);
}


static struct dmabuf_buffer **get_vid_buffers(struct drm_device_info *device, unsigned int n,
											  unsigned int fourcc, unsigned int w, unsigned int h)
{
	struct dmabuf_buffer **bufs;
	unsigned int i = 0;

	bufs = (struct dmabuf_buffer **) calloc(n, sizeof(*bufs));
	if (!bufs) {
		ERROR("allocation failed");
		goto fail;
	}

	for (i = 0; i < n; i++) {
		bufs[i] = alloc_buffer(device, fourcc, w, h);
		if (!bufs[i]) {
			ERROR("allocation failed");
			goto fail;
		}
	}
	return bufs;

fail:
	return NULL;
}

static int v4l2_init_device(struct v4l2_device_info *device)
{
	int ret, i;
	struct v4l2_capability capability;

	/* Open the capture device */
	device->fd = open((const char *) device->dev_name, O_RDWR);
	if (device->fd <= 0) {
		printf("Cannot open %s device\n", device->dev_name);
		return -1;
	}

	MSG("\n%s: Opened Channel\n", device->name);

	/* Check if the device is capable of streaming */
	if (ioctl(device->fd, VIDIOC_QUERYCAP, &capability) < 0) {
		perror("VIDIOC_QUERYCAP");
		goto ERROR;
	}

	if (capability.capabilities & V4L2_CAP_STREAMING)
		MSG("%s: Capable of streaming\n", device->name);
	else {
		ERROR("%s: Not capable of streaming\n", device->name);
		goto ERROR;
	}

	{
		struct v4l2_streamparm streamparam;
		streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(device->fd, VIDIOC_G_PARM, &streamparam) < 0){
			perror("VIDIOC_G_PARM");
			goto ERROR;
		}
	}

	device->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(device->fd, VIDIOC_G_FMT, &device->fmt);
	if (ret < 0) {
		ERROR("VIDIOC_G_FMT failed: %s (%d)", strerror(errno), ret);
		goto ERROR;
	}

	device->fmt.fmt.pix.pixelformat = FOURCC_STR("YUYV");
	device->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	device->fmt.fmt.pix.width = device->width;
	device->fmt.fmt.pix.height = device->height;

	ret = ioctl(device->fd, VIDIOC_S_FMT, &device->fmt);
	if (ret < 0) {
		perror("VIDIOC_S_FMT");
		goto ERROR;
	}

	//Initialize the dma frame buffers to -1
	for(i = 0; i < NBUF; i++){
		device->b[i] = NULL;
	}

	MSG("%s: Init done successfully\n", device->name);
	return 0;

ERROR:
	close(device->fd);

	return -1;
}

static void v4l2_exit_device(struct v4l2_device_info *device)
{

	free(device->buf);
	close(device->fd);

	return;
}


/*
* Enable streaming for V4L2 capture device
*/
static int v4l2_stream_on(struct v4l2_device_info *device)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret = 0;

	ret = ioctl(device->fd, VIDIOC_STREAMON, &type);

	if (ret) {
		ERROR("VIDIOC_STREAMON failed: %s (%d)", strerror(errno), ret);
	}

	return ret;
}

/*
* Disable streaming for V4L2 capture device
*/
static int v4l2_stream_off(struct v4l2_device_info *device)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret = -1;

        if (device->fd <= 0) {
                return ret;
        }

	ret = ioctl(device->fd, VIDIOC_STREAMOFF, &type);

	if (ret) {
		ERROR("VIDIOC_STREAMOFF failed: %s (%d)", strerror(errno), ret);
	}

	return ret;
}

static int v4l2_request_buffer(struct v4l2_device_info *device, struct dmabuf_buffer **bufs)
{
	struct v4l2_requestbuffers reqbuf;
	unsigned int i;
	int ret,dmafd;

	if (device->buf) {
		// maybe eventually need to support this?
		ERROR("already reqbuf'd");
		return -1;
	}

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = device->memory_mode;
	reqbuf.count = device->num_buffers;

	ret = ioctl(device->fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		ERROR("VIDIOC_REQBUFS failed: %s (%d)", strerror(errno), ret);
		return ret;
	}

	if ((reqbuf.count != device->num_buffers) ||
		(reqbuf.type != V4L2_BUF_TYPE_VIDEO_CAPTURE) ||
		(reqbuf.memory != V4L2_MEMORY_DMABUF)) {
			ERROR("unsupported..");
			return -1;
	}

	device->num_buffers = reqbuf.count;
	device->buffers = bufs;
	device->buf = (struct v4l2_buffer *) calloc(device->num_buffers, sizeof(struct v4l2_buffer));
	if (!device->buf) {
		ERROR("allocation failed");
		return -1;
	}

	for (i = 0; i < device->num_buffers; i++) {
		if (bufs[i]->nbo != 1){
			ERROR("Number of buffers not right");
		};

		/* Call omap_bo_dmabuf only once, to export only once
		* Otherwise, each call will return duplicated fds
		* This way, every call to omap_bo_dmabuf will return a new fd
		* Which won't match with any previously exported fds
		* Instead, store dma fd in buf->fd[] */
		dmafd = omap_bo_dmabuf(bufs[i]->bo[0]);
		bufs[i]->fd[0] = dmafd;

		device->buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		device->buf[i].memory = V4L2_MEMORY_DMABUF;
		device->buf[i].index = i;

		MSG("Exported buffer fd = %d\n", dmafd);
		ret = ioctl(device->fd, VIDIOC_QUERYBUF, &device->buf[i]);
		device->buf[i].m.fd = dmafd;

		if (ret) {
			ERROR("VIDIOC_QUERYBUF failed: %s (%d)", strerror(errno), ret);
			return ret;
		}
	}

	return 0;
}

/*
* Queue V4L2 buffer
*/
static int v4l2_queue_buffer(struct v4l2_device_info *device, struct dmabuf_buffer *buf)
{
	struct v4l2_buffer *v4l2buf = NULL;
	int  ret, fd;
	unsigned char i;

	if(buf->nbo != 1){
		ERROR("number of bufers not right\n");
		return -1;
	}

	fd = buf->fd[0];

	for (i = 0; i < device->num_buffers; i++) {
		if (device->buf[i].m.fd == fd) {
			v4l2buf = &device->buf[i];
		}
	}

	if (!v4l2buf) {
		ERROR("invalid buffer");
		return -1;
	}
	ret = ioctl(device->fd, VIDIOC_QBUF, v4l2buf);
	if (ret) {
		ERROR("VIDIOC_QBUF failed: %s (%d)", strerror(errno), ret);
	}

	return ret;
}

/*
* DeQueue V4L2 buffer
*/
struct dmabuf_buffer *v4l2_dequeue_buffer(struct v4l2_device_info *device)
{
	struct dmabuf_buffer *buf;
	struct v4l2_buffer v4l2buf;
	int ret;

	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_DMABUF;
	ret = ioctl(device->fd, VIDIOC_DQBUF, &v4l2buf);
	if (ret) {
		ERROR("VIDIOC_DQBUF failed: %s (%d)\n", strerror(errno), ret);
		return NULL;
	}

	buf = device->buffers[v4l2buf.index];

	device->buf[v4l2buf.index].timestamp = v4l2buf.timestamp;
	if(buf->nbo != 1){
		ERROR("num buffers not proper\n");
		return NULL;
	}

	return buf;
}

/*
 * Reorder the zorder of DRM planes to the desired order for demo
 *      VID1: 1
 *      VID2: 2
 *      CRTC: 3
 */

int drm_init_plane_zorder(struct drm_device_info *device)
{
	int i,j;
	drmModePlaneRes *res;
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes *props_info;
	uint32_t zorder=1;
	int ret = 0;

	// This will explain the planes to include CRTC, but setPlane is not applicable to 
	// the CRTC plane
	drmSetClientCap(device->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

	res = drmModeGetPlaneResources(device->fd);

	if(res == NULL){
		ERROR("plane resources not found\n");
		ret = -1;
		goto drm_init_plane_zorder_error;
	}

	//DBG("drm_init_plane_zorder:no. of planes = %d\n", res->count_planes);

	if (res->count_planes > MAX_DRM_PLANES)
	{

		ERROR("There are two many DRM planes (%d > %d)\n", res->count_planes, MAX_DRM_PLANES);
		ret = -1;
		goto drm_init_plane_zorder_error;
	}

	device->num_planes = res->count_planes;
	for (i = 0; i < res->count_planes; i++) {
		uint32_t plane_id = res->planes[i];
		bool fCRTC = 0;
		uint32_t zorder_value = 3;

		drmModePlane *plane = drmModeGetPlane(device->fd, plane_id);
		if(plane == NULL){
			ERROR("plane (%d) not found\n", plane_id);
			continue;
		}

		// TBD: check for required plane attributes
		device->drm_plane_id[i] = plane->plane_id;

		props = drmModeObjectGetProperties(device->fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);

		if(props == NULL){
			ERROR("plane (%d) properties not found\n",  plane->plane_id);
			continue;
		}

		for (j = 0; j < props->count_props; j++)
		{
			props_info = drmModeGetProperty(device->fd, props->props[j]);

			if(props_info == NULL){
				ERROR("plane(%d) props[%d] not found\n", plane->plane_id, j);
				continue;
			}

			if (strcmp("type", props_info->name) == 0)
			{
				fCRTC = (props->prop_values[j] == 1)?1:0;
			}
			else if ((strcmp("zorder", props_info->name) == 0))
			{
				if (!device->prop_id_zorder)
				{
					device->prop_id_zorder = props_info->prop_id;
				}
				device->prop_value_zorder[i] = (uint32_t)props->prop_values[j];
			}

			drmModeFreeProperty(props_info);
		}

		/* Re-arrange zorder */
		if (!fCRTC)
		{
			zorder_value = zorder++;
		}

		j = drmModeObjectSetProperty(device->fd, plane->plane_id,
									 DRM_MODE_OBJECT_PLANE, device->prop_id_zorder,
									(uint64_t)zorder_value);

		if (j < 0) {
			ERROR("set z-order for plane id %d failed\n", plane->plane_id);
		}

		drmModeFreeObjectProperties(props);
		drmModeFreePlane(plane);

	}

	drmModeFreePlaneResources(res);

drm_init_plane_zorder_error:

	drmSetClientCap(device->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 0);

	return(ret);

}

uint32_t drm_reserve_plane(int fd)
{
	unsigned int i;
	drmModePlaneRes *res = drmModeGetPlaneResources(fd);
	if(res == NULL){
		ERROR("plane resources not found\n");
	}

	for (i = 0; i < res->count_planes; i++) {
		uint32_t plane_id = res->planes[i];

		int idx;

		idx = find_reserved_plane(plane_id);
		if (idx >= 0)
			continue;

		drmModePlane *plane = drmModeGetPlane(fd, plane_id);
		if(plane == NULL){
			ERROR("plane  not found\n");
		}

		// TODO: check for required plane attributes

		drmModeFreePlane(plane);

		idx = find_reserved_plane(0);
		if(res == NULL){
			ERROR("reserved plane index not valid\n");
		}

		reserved_plane_ids[idx] = plane_id;

		drmModeFreePlaneResources(res);

		return plane_id;
	}

	ERROR("plane couldn't be reserved\n");
	return -1;
}


/* Get crtc id and resolution. */
void drm_crtc_resolution(struct drm_device_info *device)
{
	drmModeCrtc *crtc;
	int i;

	drmModeResPtr res = drmModeGetResources(device->fd);

	if (res == NULL){
		ERROR("drmModeGetResources failed: %s\n", strerror(errno));
		exit(0);
	};

	for (i = 0; i < res->count_crtcs; i++) {
		unsigned int crtc_id = res->crtcs[i];

		crtc = drmModeGetCrtc(device->fd, crtc_id);
		if (!crtc) {
			DBG("could not get crtc %i: %s\n", res->crtcs[i], strerror(errno));
			continue;
		}
		if (!crtc->mode_valid) {
			drmModeFreeCrtc(crtc);
			continue;
		}

		printf("CRTCs size: %dx%d\n", crtc->width, crtc->height);
		device->crtc_id = crtc_id;
		device->width = crtc->width;
		device->height = crtc->height;

		drmModeFreeCrtc(crtc);
	}

	drmModeFreeResources(res);

}

/*
 * DRM restore properties
 */
static void drm_restore_props(struct drm_device_info *device)
{
	unsigned int i;

	/* restore the original zorder of DRM planes */

	drmSetClientCap(device->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

	for (i = 0; i < device->num_planes; i++) {
		if (device->drm_plane_id[i])
		{
			drmModeObjectSetProperty(device->fd, device->drm_plane_id[i],
									 DRM_MODE_OBJECT_PLANE, device->prop_id_zorder,
									 (uint64_t)device->prop_value_zorder[i]);
		}
	}

	drmSetClientCap(device->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 0);

	if(device->prop_id_trans_key_mode){
		drmModeObjectSetProperty(device->fd,  device->crtc_id,
			DRM_MODE_OBJECT_CRTC, device->prop_id_trans_key_mode, device->prop_value_trans_key_mode);
	}

	if(device->prop_id_alpha_blender){
		drmModeObjectSetProperty(device->fd,  device->crtc_id,
			DRM_MODE_OBJECT_CRTC, device->prop_id_alpha_blender, device->prop_value_alpha_blender);
	}
}

/*
* drm device init
*/
static int drm_init_device(struct drm_device_info *device)
{
	unsigned char j;

	if (!device->fd) {
		device->fd = drmOpen("omapdrm", NULL);
		if (device->fd < 0) {
			ERROR("could not open drm device: %s (%d)", strerror(errno), errno);
			return -1;
		}
		if (drm_init_plane_zorder(device))
			return -1;
	}

	device->dev = omap_device_new(device->fd);

	/* Get CRTC id and resolution. As well set the global display width and height */
	drm_crtc_resolution(device);

	/* Store display resolution so GUI can be configured */
	status.display_xres = device->width;
	status.display_yres = device->height;

	/* AM437x has two video planes and one graphics plane.  */
	/* Get one video plane  for each camera                 */
	for (j = 0; j < status.num_cams; j++) {

		device->plane_id[j] = drm_reserve_plane(device->fd);
	}

	return 0;
}

/* 
*Clean resources while exiting drm device 
*/
static void drm_exit_device(struct drm_device_info *device)
{

	drm_restore_props(device);
	omap_device_del(device->dev);
	device->dev = NULL;

	if (device->fd > 0) {
		close(device->fd);
	}

	return;
}

/*
* Set up the DSS for blending of video and graphics planes
*/
static int drm_init_dss(void)
{
	drmModeObjectProperties *props;
	drmModePropertyRes *propRes;
	unsigned int j;

	props = drmModeObjectGetProperties(drm_device.fd, drm_device.crtc_id,
		DRM_MODE_OBJECT_CRTC);

	for (j = 0; j < props->count_props; j++) {
		propRes = drmModeGetProperty(drm_device.fd, props->props[j]);

		if (propRes == NULL)
			continue;

		if (strcmp(propRes->name, "trans-key-mode") == 0) {
			drm_device.prop_id_trans_key_mode = props->props[j];
			drm_device.prop_value_trans_key_mode = props->prop_values[j];
		}

		if (strcmp(propRes->name, "alpha_blender") == 0) {
			drm_device.prop_id_alpha_blender = props->props[j];
			drm_device.prop_value_alpha_blender = props->prop_values[j];
		}

		if(drm_device.prop_id_trans_key_mode & drm_device.prop_id_alpha_blender){
			break;
		}
	}

	if(drm_device.prop_id_trans_key_mode == 0){
		ERROR("TRANSPARENCY PROPERTY NOT FOUND\n");
		return -1;
	}

	if(drm_device.prop_id_alpha_blender == 0){
		ERROR("ALPHA BLENDER PROPERTY NOT FOUND\n");
		return -1;
	}

	/* Enable the transparency color key */
	if (drmModeObjectSetProperty(drm_device.fd,  drm_device.crtc_id,
		DRM_MODE_OBJECT_CRTC, drm_device.prop_id_trans_key_mode, status.trans_key_mode) < 0) {
			ERROR("error setting drm property for transparency key mode\n");
			return -1;
	}

	/* Enable the LCD alpha blender */
	if (drmModeObjectSetProperty(drm_device.fd,  drm_device.crtc_id,
		DRM_MODE_OBJECT_CRTC, drm_device.prop_id_alpha_blender, 1) < 0){
			ERROR("error setting drm property for Alpha Blender\n");
			return -1;
	}
	return 0;
}


/*
* drm disable pip layer
*/
int drm_disable_pip(void)
{
	/* Set the  frame buffer id to 0 to disable the plane */
	drmModeSetPlane(drm_device.fd, drm_device.plane_id[1],
		0 /* fb_id */, 0, 0, 0,0,0,0,0,0,0,0);
	return 0;
}


/*
* Display v4l2 frame on drm device
*/
static int display_frame(struct v4l2_device_info *v4l2_device, 
struct drm_device_info *drm_device, bool pip)
{
	struct dmabuf_buffer *buf;
	int ret =0, i;

	/* Delay queuing the buffer back to capture driver for two display buffer timing */
	/* This is because drmSetModePlane() API is async and doesn't acknowldege when   */
	/* the buffer is done displaying. Assuming that two capture frame delays should  */
	/* be sufficient to ensure consumption of buffer by display and safe to be queued*/
	/* to capture driver pool                                                        */
	if(v4l2_device->b[0] != NULL){
		v4l2_queue_buffer(v4l2_device, v4l2_device->b[0]);
	}

	/* Request a capture buffer from the driver that can be copied to framebuffer */
	buf = v4l2_dequeue_buffer(v4l2_device);

	v4l2_device->b[NBUF-1] = buf;
	i = 0;
	do{
		v4l2_device->b[i] = v4l2_device->b[i+1];
		i++;
	}while(i != (NBUF-1));


	if(pip) {
		ret = drmModeSetPlane(drm_device->fd, drm_device->plane_id[pip],
			drm_device->crtc_id, buf->fb_id, 0,
			/* dst_x, dst_y, dst_w, dst_h */
			25, 25, drm_device->width/3, drm_device->height/3,
			/* src_x, src_y, src_w, src_h*/
			0, 0, v4l2_device->width << 16, v4l2_device->height << 16);
	} else {
		ret = drmModeSetPlane(drm_device->fd, drm_device->plane_id[pip],
			drm_device->crtc_id, buf->fb_id, 0,
			/* make video fullscreen: */
			/* dst_x, dst_y, dst_w, dst_h */
			0, 0, drm_device->width, drm_device->height,
			/* src_x, src_y, src_w, src_h*/
			0, 0 , v4l2_device->width << 16, v4l2_device->height << 16);
	}

	if (ret) {
		ERROR("failed to enable plane %d: %s",
			drm_device->plane_id[pip], strerror(errno));
	}

	return 0;
}

/*
* Capture v4l2 frame and save to jpeg
*/
static int capture_frame(struct v4l2_device_info *v4l2_device)
{
	struct dmabuf_buffer *buf;

	/* Request a capture buffer from the driver that can be copied to framebuffer */
	//buf = v4l2_dequeue_buffer(v4l2_device);

	// Since we are having delay of two frames submission between capture and  */
	/* drmModeSetPlane, no new buffer available in capture pool. Pull the last */
	/* buffer submitted to drmModeSetPlane.*/
	buf = v4l2_device->b[NBUF-1];

	jpegWrite(omap_bo_map(buf->bo[0]),
		status.num_jpeg, v4l2_device->width, v4l2_device->height);

	/* Give the buffer back to the driver so it can be filled again 
	v4l2_queue_buffer(v4l2_device, buf);*/

	return 0;
}

/*
* Initialize the app resources with default parameters
*/
void default_parameters(void)
{
	/* Main camera display */
	memset(&drm_device, 0, sizeof(drm_device));
	strcpy(drm_device.dev_name,"/dev/drm");
	strcpy(drm_device.name,"drm");
	drm_device.width=0;
	drm_device.height=0;
	drm_device.bo_flags = OMAP_BO_SCANOUT;
	drm_device.fd = 0;

	/* Main camera */
	cap0_device.memory_mode = V4L2_MEMORY_DMABUF;
	cap0_device.num_buffers = NBUF;
	strcpy(cap0_device.dev_name,"/dev/video1");
	strcpy(cap0_device.name,"Capture 0");
	cap0_device.buffers = NULL;
	cap0_device.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	cap0_device.width = 800;
	cap0_device.height = 600;

	/* PiP camera */
	cap1_device.memory_mode = V4L2_MEMORY_DMABUF;
	cap1_device.num_buffers = NBUF;
	strcpy(cap1_device.dev_name,"/dev/video0");
	strcpy(cap1_device.name,"Capture 1");
	cap1_device.buffers = NULL;
	cap1_device.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	cap1_device.width = 800;
	cap1_device.height = 600;

	/* Set the default parameters for device options */
	status.main_cam=0;
	status.num_cams=2;
	status.num_jpeg=0;
	if(status.num_cams == 1){
		status.pip=false;
	}
	else{
		status.pip=true;
	}
	status.jpeg=false;
	status.exit=false;

	/* Ensure that jpeg image save directory exists */
	mkdir("/usr/share/camera-images/", 0777);

	return;
}

/*
* Free resource and exit devices 
*/
void exit_devices(void)
{
	v4l2_exit_device(&cap0_device);

	if (status.num_cams==2) {
		v4l2_exit_device(&cap1_device);
	}
	free_vid_buffers(drm_device.buf[0], NBUF, drm_device.multiplanar);
	free_vid_buffers(drm_device.buf[1], NBUF, drm_device.multiplanar);
	drm_exit_device(&drm_device);
}

/* 
* End camera streaming
*/
void end_streaming(void)
{
	v4l2_stream_off(&cap0_device);
	if (status.num_cams==2) {
		v4l2_stream_off(&cap1_device);
	}
}

/*
* Initializes all drm and v4l2 devices for loopback
*/
int init_loopback(void)
{
	bool status_cam0 = 0;
	bool status_cam1 = 0;

	/* Declare properties for video and capture devices */
	default_parameters();

	/* Initialize the drm display devic */
	if (drm_init_device(&drm_device)) goto Error;

	/* Check to see if the display resolution is very small.  If so, the
	* camera capture resolution needs to be lowered so that the scaling
	* limits of the DSS are not reached */
	if (drm_device.width < 640) {
		/* Set capture 0 device resolution */
		cap0_device.width = 640;
		cap0_device.height = 480;

		/* Set capture 1 device resolution */
		cap1_device.width = 640;
		cap1_device.height = 480;
	}

	/* Initialize the v4l2 capture devices */
	if (v4l2_init_device(&cap0_device) < 0) {
		/* If there is not a second camera, program can still continue */
		status.num_cams=1;
		status.main_cam=1;
		status.pip=false;
	}
	else{
		unsigned int i;
		struct dmabuf_buffer **buffers = get_vid_buffers(&drm_device, cap0_device.num_buffers, 
			cap0_device.fmt.fmt.pix.pixelformat, cap0_device.width, cap0_device.height);

		if (!buffers) {
			goto Error;
		}

		/* Pass these buffers to the capture drivers */
		if (v4l2_request_buffer(&cap0_device, buffers) < 0) {
			goto Error;
		}

		for (i = 0; i < cap0_device.num_buffers; i++) {
			v4l2_queue_buffer(&cap0_device, buffers[i]);
		}

		status_cam0 = 1;
	}
	if (v4l2_init_device(&cap1_device) < 0) {
		/* If there is not a second camera, program can still continue */
		if(status.num_cams ==2){
			status.num_cams=1;
			status.pip=false;
			printf("Only one camera detected\n");
		}
		else{
			printf("No camera detected\n");
			goto Error;
		}
	}
	else{
		unsigned int i;
		struct dmabuf_buffer **buffers = get_vid_buffers(&drm_device, cap1_device.num_buffers, 
			cap1_device.fmt.fmt.pix.pixelformat, cap1_device.width, cap1_device.height);
		if (!buffers) {
			goto Error;
		}

		/* Pass these buffers to the capture drivers */
		if (v4l2_request_buffer(&cap1_device, buffers) < 0) {
			goto Error;
		}

		for (i = 0; i < cap1_device.num_buffers; i++) {
			v4l2_queue_buffer(&cap1_device, buffers[i]);
		}

		status_cam1 = 1;
	}

	/* Enable streaming for the v4l2 capture devices */
	if(status_cam0){
		if (v4l2_stream_on(&cap0_device) < 0) goto Error;
	}

	if (status_cam1) {
		if (v4l2_stream_on(&cap1_device) < 0) goto Error;
	}

	/* Configure the DSS to blend video and graphics layers */
	if (drm_init_dss() < 0 ) goto Error;

	return 0;

Error:
	exit_devices();
	status.exit = true;
	return -1;
}

/*
* Determines which camera feeds are being displayed and
* whether a jpeg image needs to be captured.
*/
void process_frame(void) {

	/* Display the main camera */
	if (status.main_cam==0)
		display_frame(&cap0_device, &drm_device, 0);
	else
		display_frame(&cap1_device, &drm_device, 0);

	/* Display PiP if enabled */
	if (status.pip==true) {
		if (status.main_cam==0)
			display_frame(&cap1_device, &drm_device, 1);
		else
			display_frame(&cap0_device, &drm_device, 1);
	}

	/* Save jpeg image if triggered */
	if (status.jpeg==true) {
		if (status.main_cam==0) {
			capture_frame(&cap0_device);
		}
		else {
			capture_frame(&cap1_device);
		}

		status.jpeg=false;
		status.num_jpeg++;
		if (status.num_jpeg==10)
			status.num_jpeg=0;
	}
}
