#include "client/camera.h"
#include "cglm/vec3.h"

#include <stdbool.h>

void camera_init(sccamera_t *cam) {
    glm_vec3_zero(cam->position);
    glm_vec3_copy(GLM_FORWARD, cam->front);
    glm_vec3_copy(GLM_YUP, cam->up);
    cam->pitch = 0.0f;
    cam->yaw   = 0.0f;
}

void camera_get_view(sccamera_t *cam, mat4 view) {}

void camera_get_projection(sccamera_t *cam, mat4 projection, float fov,
                           float aspect) {}

void camera_update(sccamera_t *cam, float fov, float aspect) {

    vec3 center;
    glm_vec3_sub(cam->position, cam->front, center);
    glm_lookat(cam->position, center, cam->up, cam->view);

    glm_perspective(glm_rad(fov), aspect, 0.01, 100000.0f, cam->proj);
}

void camera_handle_mouse(sccamera_t *cam, vec2 delta, float sens) {
    glm_vec2_scale(delta, sens, delta);

    cam->yaw += delta[0];
    cam->pitch += delta[1];

    if (cam->pitch > 89.0f)
        cam->pitch = 89.0f;
    else if (cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    vec3 front;
    front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch)); // x
    front[1] = sinf(glm_rad(cam->pitch));                           // y
    front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch)); // z
    glm_vec3_normalize_to(front, cam->front);
}
