#include "client/app.h"
#include <stdlib.h>

app_t *app_create() {
    app_t *app = malloc(sizeof(*app));

    app->win = w_create(1920, 1080, "strike counter");
    w_make_current(app->win);
    w_hide_cursor();
    // glfwSwapInterval(0);

    int w, h;
    w_get_fb_size(&w, &h);
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    // glAlphaFunc(GL_GREATER, 0.2);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, w, h);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    app->bsp =
        bsp_load("/home/focus/.local/share/Steam/steamapps/common/Half-Life/"
                 "cstrike/maps/de_dust2.bsp");

    app->gfx = bsp_gfx_create(app->bsp);
    bsp_dump(app->bsp);

    app->cam = malloc(sizeof(*app->cam));
    camera_init(app->cam);

    app->con = console_create();
    return app;
}
