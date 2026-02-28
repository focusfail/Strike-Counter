#ifndef CLIENT_WINDOW_H
#define CLIENT_WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <cglm/cglm.h>
#include <glad/gl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "demo/glfw_opengl3/nuklear_glfw_gl3.h"

typedef struct {
    struct nk_glfw glfw;
    struct nk_context *nk;

    GLFWwindow *ptr;
    vec2 prev_cursor;
    vec2 curr_cursor;
    bool curr_keys[GLFW_KEY_LAST];
    bool prev_keys[GLFW_KEY_LAST];
    bool prev_mouse_keys[GLFW_MOUSE_BUTTON_LAST];
    bool curr_mouse_keys[GLFW_MOUSE_BUTTON_LAST];
    bool wireframe;
    bool cursor_visible;

    double frametime;
    double deltatime;
} scwin_t;

extern scwin_t *g_win;

const char *get_key_name(int kc, int sc);

scwin_t *w_create(int w, int h, const char *title);
void w_make_current(scwin_t *win);
void w_free(scwin_t *win);

void w_poll_events();
void w_swap();

void w_get_size(int32_t *w, int32_t *h);
void w_get_fb_size(int32_t *w, int32_t *h);
float w_get_aspect();
bool w_should_close();

double w_get_deltatime();
double w_get_fps();

void w_hide_cursor();
void w_show_cursor();
void w_toggle_wireframe();
void w_toggle_cursor();

void get_cursor_pos(vec2 pos);
void get_cursor_delta(vec2 pos);

bool key_once(uint32_t kc);
bool key_held(uint32_t kc);
bool mouse_once(uint32_t kc);
bool mouse_held(uint32_t kc);

#endif // CLIENT_WINDOW_H
