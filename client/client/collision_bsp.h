#ifndef CLIENT_COLLISION_BSP_H
#define CLIENT_COLLISION_BSP_H

#include "cglm/cglm.h"
#include "client/bsp.h"
#include <stdint.h>

typedef struct {
    vec3 box[2];
    uint32_t plane; // plane index
    int16_t children[2];
    int16_t face_first;
    int16_t face_count;
} cnode_t;

typedef struct {
    vec3 normal;
    float dist;
    int32_t type;
} cplane_t;

typedef struct {
    vec3 box[2];
    int32_t contents;
    int32_t vis_offset;
} cleaf_t;

typedef struct {
    cleaf_t *leafs;
    cnode_t *nodes;
    cplane_t *planes;

    uint32_t leaf_count;
    uint32_t node_count;
    uint32_t plane_count;
} cbsp_t;

cbsp_t *cbsp_create();
void cbsp_load(cbsp_t *cbsp, bsp_t *bsp);
void cbsp_destroy(cbsp_t *cbsp);

#endif // CLIENT_COLLISION_BSP_H
