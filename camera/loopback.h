#ifndef LOOPBACK_H
#define LOOPBACK_H
#include <stdbool.h>

struct control {
    int main_cam;
    int num_cams;
    int num_jpeg;
    int display_xres, display_yres;
    int trans_key_mode;
    bool pip;
    bool jpeg;
    bool exit;
};

typedef struct control control_t;

int init_loopback(void);
void process_frame(void);
void end_streaming(void);
void exit_devices(void);
int drm_disable_pip(void);

#endif // LOOPBACK_H
