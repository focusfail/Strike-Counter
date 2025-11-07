#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include "client/bsp.h"
#include "client/bsp_gfx.h"
#include "client/camera.h"
#include "client/console.h"
#include "client/window.h"
typedef struct {
    scwin_t *win;
    sccamera_t *cam;
    console_t *con;
    bsp_t *bsp;
    bspgfx_t *gfx;
} app_t;

app_t *app_create();
void app_destroy();

#endif // CLIENT_APP_H
