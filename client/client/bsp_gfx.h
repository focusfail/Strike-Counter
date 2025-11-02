#ifndef CLIENT_BSP_RENDERER_H
#define CLIENT_BSP_RENDERER_H

#include "client/bsp.h"
#include "client/camera.h"

#include <stdint.h>

typedef struct {
    float s[4]; // [xyz shift]
    float t[4]; // [xyz shift]
    vec2 size;
    uint32_t texture;
    uint32_t pad0;
} rface_t;

typedef struct {
    uint32_t program;
    uint32_t texture;

    uint32_t elems;
    uint32_t verts;
    uint32_t faces;
    uint32_t cmds; // One cmd per face

    uint32_t face_count;
    uint32_t elem_count;
    uint32_t vert_count;

    uint32_t debug;
} bspgfx_t;

bspgfx_t *bsp_gfx_create(bsp_t *bsp);
void bsp_gfx_load(bspgfx_t *gfx, bsp_t *bsp);
void bsp_gfx_render(bspgfx_t *gfx, sccamera_t *cam);
void bsp_gfx_free(bspgfx_t *gfx);

#endif // CLIENT_BSP_RENDERER_H
