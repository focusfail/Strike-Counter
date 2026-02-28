#include "client/console.h"
#include "client/bsp.h"
#include "client/bsp_gfx.h"
#include "client/window.h"
#include "common/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

console_t *console_create() {
    console_t *con = malloc(sizeof(*con));

    con->history_len = 0;
    con->command_len = 0;
    con->visible = false;
    con->initial_focus = false;
    con->open_delay = 1; // delay console by one frame to ignore the console
                         // open bind as a command

    con->command[0] = '\0';

    return con;
}

void console_handle_command(console_t *con, bsp_t *bsp, bspgfx_t *gfx) {

    if (strncasecmp(con->command, "map", 3) == 0) {
        char path[256];
        snprintf(path, 256,
                 "~/.local/share/Steam/steamapps/common/Half-Life/"
                 "cstrike/maps/%s.bsp",
                 con->command + 4);

        bsp_load(bsp, path);
        bsp_gfx_load(gfx, bsp);
    }
}

bool console_should_render(console_t *con) {
    if (con->visible && con->open_delay < 1) {
        return true;
    }
    return false;
}

void console_hide(console_t *con) {
    con->open_delay = 1;
    con->visible = false;
}

void console_show(console_t *con) {
    con->initial_focus = true;
    con->visible = true;
    con->open_delay = 1;
}

void console_render(struct nk_context *nk, console_t *con, bsp_t *bsp,
                    bspgfx_t *gfx) {
    if (!console_should_render(con)) {
        con->open_delay--;
        return;
    }

    if (nk_begin(nk, "Console (close with \"`\")", nk_rect(10, 10, 500, 350),
                 NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR |
                     NK_WINDOW_MOVABLE)) {

        nk_layout_row_dynamic(nk, 275, 1);
        if (nk_group_begin(nk, "History", NK_WINDOW_BORDER)) {
            nk_layout_row_dynamic(nk, 20, 1);

            for (int i = 0; i < con->history_len; i++) {
                nk_label(nk, con->history[i], NK_TEXT_LEFT);
            }
            nk_group_end(nk);
        }

        nk_layout_row_dynamic(nk, 30, 1);

        if (con->initial_focus) {
            nk_edit_focus(nk, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER |
                                  NK_EDIT_GOTO_END_ON_ACTIVATE);
        }

        nk_flags e = nk_edit_string_zero_terminated(
            nk,
            NK_EDIT_FIELD | NK_EDIT_SIG_ENTER | NK_EDIT_GOTO_END_ON_ACTIVATE,
            con->command, 256, nk_filter_default);

        if (e & NK_EDIT_COMMITED) {
            log_debug("COMMAND: %s", con->command);
            strncpy(con->history[con->history_len++], con->command, 256);
            console_handle_command(con, bsp, gfx);
            con->command[0] = '\0';

            if (con->initial_focus) {
                con->initial_focus = false;
            }
        }
    }
    nk_end(nk);
}
