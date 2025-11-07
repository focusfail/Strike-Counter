#ifndef CLIENT_BSP_H
#define CLIENT_BSP_H

#include <stdint.h>
#include <cglm/cglm.h>

#include "client/wad.h"

#define LUMP_ENTITIES 0
#define LUMP_PLANES 1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES 5
#define LUMP_TEXINFO 6
#define LUMP_FACES 7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES 9
#define LUMP_LEAFS 10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14
#define LUMP_COUNT 15

#define MAX_LIGHTMAPS 4
#define MAX_MIPLEVELS 4
#define MAX_ENTITIES 1024

typedef struct {
    int32_t ofs;
    int32_t len;
} bsplump_t;

typedef struct {
    int32_t version;
    bsplump_t lumps[LUMP_COUNT];
} bspheader_t;

typedef vec3 bspvert_t;
typedef int32_t bspsurfedge_t;

typedef struct {
    int16_t plane_num;
    int16_t side;

    int32_t edge_first;
    int16_t edge_count;
    int16_t texinfo;

    uint8_t styles[MAX_LIGHTMAPS];
    int32_t lightofs;
} bspface_t;

typedef struct {
    float v_s[4];
    float v_t[4];
    int32_t miptex;
    int32_t flags;
} bsptexinfo_t;

typedef struct {
    char name[16];
    uint32_t w, h;
    uint32_t offsets[MAX_MIPLEVELS];
} miptex_t;

typedef struct {
    uint16_t v[2];
} bspedge_t;

typedef struct epair_s {
    struct epair_s *next;
    char *k;
    char *v;
    uint32_t k_len;
    uint32_t v_len;
} epair_t;

typedef struct {
    vec3 origin;
    int32_t brush_first;
    int32_t brush_count;
    epair_t *pairs;
} bspentity_t;

typedef struct {
    vec3 normal;
    float dist;
    int32_t type;
} bspplane_t;

typedef struct {
    uint32_t plane;
    int16_t children[2];
    int16_t box_min[3], box_max[3];
    uint16_t face_first;
    uint16_t face_count;
} bspnode_t;

typedef struct {
    int32_t contents;
    int32_t vis_offset;
    int16_t box_min[3], box_max[3];
    uint16_t marksurf_first;
    uint16_t marksurf_count;
    uint8_t ambient_levels[4];
} bspleaf_t;

typedef struct {
    void *file_buffer;

    wad_t **wads;
    bspleaf_t *leafs;
    bspnode_t *nodes;
    bspface_t *faces;
    bspvert_t *verts;
    bspedge_t *edges;
    miptex_t **miptex;
    bspplane_t *planes;
    bspentity_t entities[MAX_ENTITIES];
    bsptexinfo_t *texinfo;
    bspsurfedge_t *surfedges;

    uint32_t wad_count;
    uint32_t node_count;
    uint32_t leaf_count;
    uint32_t face_count;
    uint32_t vert_count;
    uint32_t edge_count;
    uint32_t plane_count;
    uint32_t miptex_count;
    uint32_t entity_count;
    uint32_t texinfo_count;
    uint32_t surfedge_count;
} bsp_t;

bsp_t *bsp_create(const char *filename);
void bsp_load(bsp_t *bsp, const char *filename);
void bsp_free(bsp_t *bsp);
void bsp_dump(bsp_t *bsp);
epair_t *epair_from_key(bspentity_t *ent, const char *key);

#endif // CLIENT_BSP_H
