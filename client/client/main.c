#include <stdio.h>
#include <stdlib.h>
#include <glad/gl.h>
#include <string.h>

#include "GLFW/glfw3.h"
#include "client/bsp_gfx.h"
#include "client/bsp.h"
#include "client/camera.h"
#include "client/window.h"
#include "common/log.h"

sccamera_t *cam;
scwin_t *win;
bsp_t *bsp;
bspgfx_t *gfx;
float mouse_sens = 0.1f;
double timeout   = 0.0;
char fps_buffer[32];

bool handle_input(double dt) {
    vec3 direction = GLM_VEC3_ZERO_INIT;
    float speed    = 200.0f;

    if (key_held(GLFW_KEY_ESCAPE)) {
        return false;
    }

    if (key_held(GLFW_KEY_LEFT_SHIFT)) {
        speed = 1000.0f;
    }

    if (key_once(GLFW_KEY_F1)) {
        w_toggle_wireframe();
    }

    if (key_once(GLFW_KEY_F2)) {
        gfx->debug++;
        if (gfx->debug > 2) {
            gfx->debug = 0;
        }
    }

    if (key_once(GLFW_KEY_F3)) {
        bsp_gfx_load(gfx, bsp);
    }

    // forward/back
    if (key_held(GLFW_KEY_W)) {
        glm_vec3_sub(direction, cam->front, direction);
    }
    if (key_held(GLFW_KEY_S)) {
        glm_vec3_add(direction, cam->front, direction);
    }

    // left/right
    if (key_held(GLFW_KEY_A)) {
        vec3 left;
        vec3 right;
        glm_vec3_cross(cam->up, cam->front, right);
        glm_vec3_negate_to(right, left);
        glm_normalize(left);
        glm_vec3_add(direction, left, direction);
    }
    if (key_held(GLFW_KEY_D)) {
        vec3 right;
        glm_vec3_cross(cam->up, cam->front, right);
        glm_normalize(right);
        glm_vec3_add(direction, right, direction);
    }

    if (!glm_vec3_eqv(direction, GLM_VEC3_ZERO)) {
        glm_vec3_normalize(direction);
        glm_vec3_scale(direction, speed * dt, direction);
        glm_vec3_add(cam->position, direction, cam->position);
    }

    vec2 mouse_delta;
    get_cursor_delta(mouse_delta);

    camera_handle_mouse(cam, mouse_delta, mouse_sens);
    camera_update(cam, 75.0f, w_get_aspect());

    return true;
}

int main(void) {
    win = w_create(1920, 1080, "strike counter");
    w_make_current(&win);
    w_hide_cursor();
    // glfwSwapInterval(0);

    int w, h;
    w_get_fb_size(&w, &h);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.2);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, w, h);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // bsp = bsp_load(SC_ASSET_PATH "/de_dust2.bsp");
    bsp = bsp_load(
        "/home/focus/Downloads/fy_pool_day_60bca/maps/fy_pool_day.bsp");
    gfx = bsp_gfx_create(bsp);
    bsp_dump(bsp);

    cam = malloc(sizeof(*cam));
    camera_init(cam);

    vec3 start;
    for (int i = 0; i < bsp->entity_count; i++) {
        epair_t *pair = bsp->entities[i].pairs;
        while (pair != NULL) {
            if (strcmp(pair->k, "origin") == 0) {
                sscanf(pair->v, "%f %f %f", &start[0], &start[2], &start[1]);
            }

            if (strcmp(pair->v, "info_player_start") == 0) {
                log_info("Found player spawn %s at %s", pair->k, pair->v);
                goto found_player_start;
            }

            pair = pair->next;
        }
    }

found_player_start:

    glm_vec3_copy(start, cam->position);

    while (!w_should_close()) {
        double dt  = w_get_deltatime();
        double fps = w_get_fps();

        if (!handle_input(dt)) break;

        nk_glfw3_new_frame(&win->glfw);

        if (nk_begin(win->nk, "Perf", nk_rect(0, 0, 300, 300),
                     NK_WINDOW_NOT_INTERACTIVE | NK_WINDOW_BACKGROUND)) {

            win->nk->style.window.background = nk_rgba(0, 0, 0, 0);
            win->nk->style.window.fixed_background =
                nk_style_item_color(nk_rgba(0, 0, 0, 0));
            win->nk->style.window.padding = nk_vec2(0, 0);
            win->nk->style.window.spacing = nk_vec2(0, 0);
            win->nk->style.text.color     = nk_rgba(0, 0, 0, 255);

            if (timeout > 0.5) {

                snprintf(fps_buffer, 32, "FPS: %.1f", fps);
                timeout = 0.0;
            }
            timeout += dt;

            nk_layout_row_static(win->nk, 30, 80, 1);
            nk_label(win->nk, fps_buffer, NK_TEXT_LEFT);
        }

        nk_end(win->nk);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        bsp_gfx_render(gfx, cam);
        nk_glfw3_render(&win->glfw, NK_ANTI_ALIASING_ON, 512 * 1024,
                        128 * 1024);
        glEnable(GL_DEPTH_TEST);
    }

    bsp_gfx_free(gfx);
    bsp_free(bsp);
    w_free(win);

    return EXIT_SUCCESS;
}
