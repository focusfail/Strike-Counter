#ifndef CLIENT_CAMERA_H
#define CLIENT_CAMERA_H

#include <cglm/cglm.h>

typedef struct {
    mat4 view;
    mat4 proj;
    vec3 position;
    vec3 front;
    vec3 up;
    float pitch;
    float yaw;
} sccamera_t;

void camera_init(sccamera_t *cam);
void camera_update(sccamera_t *cam, float fov, float aspec);
void camera_handle_mouse(sccamera_t *cam, vec2 delta, float sens);

#endif // CLIENT_CAMERA_H
