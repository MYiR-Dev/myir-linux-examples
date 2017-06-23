#include <malloc.h>
#include <sys/mman.h>
#include <jpeglib.h>

void jpegWrite(unsigned char* yuv, int index, int width, int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    unsigned char *img = (unsigned char *)malloc(width*height*3*sizeof(char));

    char jpegFilename[256];

    sprintf(jpegFilename, "/usr/share/camera-images/image%d.jpg", index);

    JSAMPROW row_pointer[1];
    FILE *outfile = fopen( jpegFilename, "wb" );

    // try to open file for saving
    if (!outfile) {
            printf("Can't open file!\n");
    }

    // create jpeg data
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // set image parameters
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;

    // set jpeg compression parameters to default
    jpeg_set_defaults(&cinfo);
    // and then adjust quality setting
    jpeg_set_quality(&cinfo, 90, TRUE);

    // start compress
    jpeg_start_compress(&cinfo, TRUE);

    // feed data
    row_pointer[0] = img;
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned i, j;
        unsigned offset = cinfo.next_scanline * cinfo.image_width * 2;
        for (i = 0, j = 0; i < cinfo.image_width*2; i += 4, j += 6) {
            img[j + 0] = yuv[offset + i + 0]; // Y
            img[j + 1] = yuv[offset + i + 1]; // U
            img[j + 2] = yuv[offset + i + 3]; // V
            img[j + 3] = yuv[offset + i + 2]; // Y
            img[j + 4] = yuv[offset + i + 1]; // U
            img[j + 5] = yuv[offset + i + 3]; // V
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // finish compression
    jpeg_finish_compress(&cinfo);

    // destroy jpeg data
    jpeg_destroy_compress(&cinfo);

    // close output file
    fclose(outfile);

    munmap(img, width*height*3*sizeof(char));

    return;
}
