#include "client/window.h"
#include "common/log.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include "nuklear.h"
#include "demo/glfw_opengl3/nuklear_glfw_gl3.h"

#include <string.h>

#define KEY_NAME_ESCAPE "escape"
#define KEY_NAME_ENTER "enter"
#define KEY_NAME_SPACE "space"
#define KEY_NAME_SHIFT "shift"
#define KEY_NAME_TAB "tab"

scwin_t *g_win = NULL;

const char *get_key_name(int kc, int sc) {
    switch (kc) {
    case GLFW_KEY_ESCAPE:
        return KEY_NAME_ESCAPE;
    case GLFW_KEY_ENTER:
        return KEY_NAME_ENTER;
    case GLFW_KEY_SPACE:
        return KEY_NAME_SPACE;
    case GLFW_KEY_TAB:
        return KEY_NAME_TAB;
    case GLFW_KEY_RIGHT_SHIFT:
    case GLFW_KEY_LEFT_SHIFT:
        return KEY_NAME_SHIFT;
    default:
        return glfwGetKeyName(kc, sc);
    }
}

void keyboard_callback(GLFWwindow *ptr, int kc, int sc, int ac, int mod) {
    nk_glfw3_key_callback(ptr, kc, sc, ac, mod);
    scwin_t *win = glfwGetWindowUserPointer(ptr);

    if (ac == GLFW_PRESS) {
        win->curr_keys[kc] = true;
        // log_debug("Key {'%s', kc=%d, down=%d}", get_key_name(kc, sc), kc,
        // win->curr_keys[kc]);
    } else if (ac == GLFW_RELEASE) {
        win->curr_keys[kc] = false;
    }
}

void mouse_button_callback(GLFWwindow *ptr, int button, int ac, int mods) {
    nk_glfw3_mouse_button_callback(ptr, button, ac, mods);
}

void scroll_callback(GLFWwindow *ptr, double xoff, double yoff) {}

void mouse_callback(GLFWwindow *ptr, double x, double y) {
    scwin_t *win        = glfwGetWindowUserPointer(ptr);
    win->curr_cursor[0] = (float)x;
    win->curr_cursor[1] = (float)y;
}

scwin_t *w_create(int w, int h, const char *title) {
    if (!glfwInit()) {
        log_fatal("Failed to initialize GLFW.");
    }

    scwin_t *win = malloc(sizeof(*win));

    glfwWindowHint(GLFW_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 8);
    win->ptr = glfwCreateWindow(w, h, title, NULL, NULL);
    if (!win->ptr) {
        free(win);
        glfwTerminate();
        log_fatal("Failed to create window.");
    }

    glfwMakeContextCurrent(win->ptr);
    if (!gladLoaderLoadGL()) {
        glfwDestroyWindow(win->ptr);
        glfwTerminate();
        free(win);
        log_fatal("Failed to load opengl functions.");
    }

    win->nk = nk_glfw3_init(&win->glfw, win->ptr, NK_GLFW3_DEFAULT);
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&win->glfw, &atlas);
        nk_glfw3_font_stash_end(&win->glfw);
    }

    glfwSetWindowUserPointer(win->ptr, win);
    glfwSetKeyCallback(win->ptr, (GLFWkeyfun)keyboard_callback);
    glfwSetCursorPosCallback(win->ptr, mouse_callback);
    glfwSetMouseButtonCallback(win->ptr, mouse_button_callback);

    win->wireframe = false;

    memset(win->curr_keys, 0, sizeof(win->curr_keys));
    memset(win->prev_keys, 0, sizeof(win->prev_keys));
    memset(win->curr_mouse_keys, 0, sizeof(win->curr_mouse_keys));
    memset(win->prev_mouse_keys, 0, sizeof(win->prev_mouse_keys));

    return win;
}

void w_make_current(scwin_t **win) { g_win = *win; }

void w_free(scwin_t *win) {
    if (win && win->ptr) {
        glfwDestroyWindow(win->ptr);
        glfwTerminate();
        nk_glfw3_shutdown(&win->glfw);
        if (win == g_win) {
            g_win = NULL;
        }
    }
}

void w_swap() { glfwSwapBuffers(g_win->ptr); }
void w_poll_events() {
    memcpy(g_win->prev_keys, g_win->curr_keys, sizeof(g_win->curr_keys));
    glm_vec2_copy(g_win->curr_cursor, g_win->prev_cursor);
    glfwPollEvents();

    double new_time  = glfwGetTime();
    g_win->deltatime = new_time - g_win->frametime;
    g_win->frametime = new_time;
}

void w_get_size(int32_t *w, int32_t *h) { glfwGetWindowSize(g_win->ptr, w, h); }

double w_get_deltatime() { return g_win->deltatime; }
double w_get_fps() { return 1.0 / g_win->deltatime; }

void w_get_fb_size(int32_t *w, int32_t *h) {
    glfwGetFramebufferSize(g_win->ptr, w, h);
}

void w_hide_cursor() {
    glfwSetInputMode(g_win->ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void w_show_cursor() {
    glfwSetInputMode(g_win->ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool w_should_close() {
    w_swap();
    w_poll_events();

    return glfwWindowShouldClose(g_win->ptr);
}

bool key_once(uint32_t kc) {
    return !g_win->prev_keys[kc] && g_win->curr_keys[kc];
}

bool key_held(uint32_t kc) {
    return g_win->prev_keys[kc] && g_win->curr_keys[kc];
}

void get_cursor_pos(vec2 pos) { glm_vec2_copy(g_win->curr_cursor, pos); }

void get_cursor_delta(vec2 pos) {
    glm_vec2_sub(g_win->curr_cursor, g_win->prev_cursor, pos);
}

float w_get_aspect() {
    int w, h;
    w_get_fb_size(&w, &h);
    return (float)w / h;
}

void w_toggle_wireframe() {
    if (g_win->wireframe) {
        g_win->wireframe = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        g_win->wireframe = true;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}
