#ifndef CLIENT_CONSOLE_H
#define CLIENT_CONSOLE_H

#include "client/bsp_gfx.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <stdbool.h>
#include <stdint.h>
#include <nuklear.h>

typedef struct {
    bool visible;
    bool initial_focus;
    uint8_t open_delay;
    char command[256];
    char history[256][256];
    uint32_t history_len;
    int32_t command_len;
} console_t;

console_t *console_create();

void console_hide(console_t *con);
void console_show(console_t *con);

bool console_should_render(console_t *con);
void console_render(struct nk_context *nk, console_t *con, bsp_t *bsp,
                    bspgfx_t *gfx);
void console_destroy(console_t *con);

#endif // CLIENT_CONSOLE_H
