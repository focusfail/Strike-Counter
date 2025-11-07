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
    con->visible     = false;

    con->command[0] = '\0';

    return con;
}

void console_handle_command(console_t *con, bsp_t *bsp, bspgfx_t *gfx) {

    if (strncasecmp(con->command, "map", 3) == 0) {
        char path[256];
        snprintf(path, 256,
                 "/home/focus/.local/share/Steam/steamapps/common/Half-Life/"
                 "cstrike/maps/%s.bsp",
                 con->command + 4);

        bsp_load(bsp, path);
        bsp_gfx_load(gfx, bsp);
    }
}

void console_render(struct nk_context *nk, console_t *con, bsp_t *bsp,
                    bspgfx_t *gfx) {
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
        nk_flags e = nk_edit_string_zero_terminated(
            nk, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, con->command, 256,
            nk_filter_default);

        if (e & NK_EDIT_COMMITED) {
            log_debug("COMMAND: %s", con->command);
            strncpy(con->history[con->history_len++], con->command, 256);
            console_handle_command(con, bsp, gfx);
            con->command[0] = '\0';
        }
    }
    nk_end(nk);
}
