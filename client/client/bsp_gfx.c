#include "client/bsp_gfx.h"
#include "client/bsp.h"
#include "client/gl_common.h"
#include "client/wad.h"
#include "common/log.h"

#include <stdlib.h>
#include <glad/gl.h>
#include <string.h>
#include <strings.h>
#include <stb_image.h>

#define DEFAULT_VERTEX_SHADER SC_ASSET_PATH "/shaders/bsp.vert"
#define DEFAULT_FRAGMENT_SHADER SC_ASSET_PATH "/shaders/bsp.frag"

typedef struct {
    uint32_t count;
    uint32_t instance_count;
    uint32_t first_index;
    uint32_t base_vertex;
    uint32_t base_instance;
} drawcmd_t;

void construct_element_buffer(bsp_t *bsp, bspgfx_t *gfx) {
    drawcmd_t *cmds        = malloc(sizeof(*cmds) * bsp->face_count);
    uint32_t max_elements  = bsp->surfedge_count * 3;
    uint32_t *elements     = malloc(sizeof(*elements) * max_elements);
    uint32_t element_count = 0;

    for (int i = 0; i < bsp->face_count; i++) {
        cmds[i] = (drawcmd_t){0};
    }

    for (int i = 0; i < bsp->face_count; i++) {
        bspface_t *face = &bsp->faces[i];

        uint32_t face_start_index = element_count;

        uint32_t face_verts[64];
        for (int j = 0; j < face->edge_count; j++) {
            int32_t surfedge = bsp->surfedges[face->edge_first + j];
            uint32_t vert;
            if (surfedge >= 0) {
                vert = bsp->edges[surfedge].v[0];
            } else {
                vert = bsp->edges[abs(surfedge)].v[1];
            }
            face_verts[j] = vert;
        }

        for (int j = 2; j < face->edge_count; j++) {
            if (element_count + 3 > max_elements) break;
            elements[element_count++] = face_verts[0];
            elements[element_count++] = face_verts[j - 1];
            elements[element_count++] = face_verts[j];
        }

        uint32_t index_count = element_count - face_start_index;
        cmds[i]              = (drawcmd_t){
                         .count          = index_count,
                         .instance_count = 1,
                         .first_index    = face_start_index,
                         .base_vertex    = 0,
                         .base_instance  = 0,
        };
    }

    if (element_count > 0) {
        glNamedBufferStorage(gfx->elems, sizeof(*elements) * element_count,
                             elements, GL_DYNAMIC_STORAGE_BIT);
    }

    glNamedBufferStorage(gfx->cmds, sizeof(*cmds) * bsp->face_count, cmds,
                         GL_DYNAMIC_STORAGE_BIT);

    gfx->elem_count = element_count;
    free(elements);
    free(cmds);
}

void construct_vertex_buffer(bsp_t *bsp, bspgfx_t *br) {
    vec4 *verts = malloc(sizeof(*verts) * bsp->vert_count);

    for (int i = 0; i < bsp->vert_count; i++) {
        verts[i][0] = bsp->verts[i][0];
        verts[i][1] = bsp->verts[i][1];
        verts[i][2] = bsp->verts[i][2];
        verts[i][3] = 1.0f;
    }

    glNamedBufferStorage(br->verts, sizeof(*verts) * bsp->vert_count, verts,
                         GL_DYNAMIC_STORAGE_BIT);

    free(verts);
}

bool load_wad_image(wad_t *wad, wadlumpinfo_t *lump, uint8_t **image, int *w,
                    int *h) {
    *image       = NULL;
    *w           = 0;
    *h           = 0;
    miptex_t *mt = (miptex_t *)(wad->file_buffer + lump->filepos);

    if (mt->offsets[0] < 1 || mt->w == 0 || mt->h == 0 || mt->w > 1024 ||
        mt->h > 1024) {
        return false;
    }

    uint32_t pixels  = mt->w * mt->h;
    uint8_t *indices = (uint8_t *)((char *)mt + mt->offsets[0]);
    uint8_t *rgba    = malloc(pixels * 4);

    int16_t *palette_indices =
        (int16_t *)((char *)mt + mt->offsets[3] + (pixels / 64));

    uint8_t *palette = (uint8_t *)palette_indices + sizeof(int16_t);

    for (int i = 0; i < pixels; i++) {
        uint8_t idx = indices[i];

        rgba[i * 4 + 0] = palette[idx * 3 + 0];
        rgba[i * 4 + 1] = palette[idx * 3 + 1];
        rgba[i * 4 + 2] = palette[idx * 3 + 2];
        rgba[i * 4 + 3] = 255;
    }

    *w     = mt->w;
    *h     = mt->h;
    *image = rgba;

    return true;
}

bool is_one_of(char c, const char *chars) {
    for (int i = 0; i < strlen(chars); i++) {
        if (chars[i] == c) return true;
    }

    return false;
}

const char *remove_effect(const char *src) {

    while (*src != '\0' && is_one_of(*src, "+~-{}")) {
        src++;
    }

    return src;
}

bool find_texture_in_wad(bsp_t *bsp, const char *name, wad_t **outwad,
                         wadlumpinfo_t **outlmp) {
    if (strncasecmp(name, "aaatrigger", 16) == 0) return false;
    if (strncasecmp(name, "invisible", 16) == 0) return false;
    if (strncasecmp(name, "clip", 16) == 0) return false;

    *outwad = NULL;
    *outlmp = NULL;

    for (int i = 0; i < bsp->wad_count; i++) {

        // name = remove_effect(name);

        wad_t *wad          = bsp->wads[i];
        wadlumpinfo_t *lump = NULL;
        if (wad_find_lump_by_name(wad, &lump, name)) {
            *outwad = wad;
            *outlmp = lump;
            return true;
        }
    }

    return false;
}

void construct_face_buffer(bsp_t *bsp, bspgfx_t *gfx) {
    rface_t *faces = malloc(sizeof(*faces) * bsp->face_count);

    uint8_t missing[256 * 256 * 4];
    for (int i = 0; i < 256 * 256; i++) {
        missing[i * 4 + 0] = 255;
        missing[i * 4 + 1] = 0;
        missing[i * 4 + 2] = 255;
        missing[i * 4 + 3] = 0;
    }

    uint32_t tex;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex);
    glTextureStorage3D(tex, 1, GL_RGBA8, 256, 256, bsp->miptex_count);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTextureParameteri(tex, GL_TEXTURE_WRAP_R, GL_REPEAT);
    uint32_t textures_found = 0;

    // create a gpu texture for every miptex
    for (int i = 0; i < bsp->miptex_count; i++) {
        miptex_t *mt = bsp->miptex[i];

        wad_t *wad;
        wadlumpinfo_t *lump;
        uint8_t *image = NULL;
        int w, h;

        if (!find_texture_in_wad(bsp, mt->name, &wad, &lump)) {
            log_warn("Unable to locate texture '%s'", mt->name);
            goto missing_texture;
        };

        if (!load_wad_image(wad, lump, &image, &w, &h)) {
            goto missing_texture;
        }

        log_debug("Texture %s at index %d", lump->name, i);

        glTextureSubImage3D(tex, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            image);
        textures_found += 1;
        free(image);
        continue;

    missing_texture:
        log_warn("Applying missing texture at index %d", i);
        glTextureSubImage3D(tex, 0, 0, 0, i, 256, 256, 1, GL_RGBA,
                            GL_UNSIGNED_BYTE, missing);
        continue;
    }

    log_info("Successfully loaded %d/%d textures", textures_found,
             bsp->miptex_count);

    gfx->face_count = bsp->face_count;
    for (int i = 0; i < gfx->face_count; i++) {
        bspface_t *face  = &bsp->faces[i];
        bsptexinfo_t *ti = &bsp->texinfo[face->texinfo];
        miptex_t *mt     = bsp->miptex[ti->miptex];

        faces[i] = (rface_t){
            .texture = ti->miptex,
            .size    = {mt->w, mt->h},
            .s       = {ti->v_s[0], ti->v_s[1], ti->v_s[2], ti->v_s[3]},
            .t       = {ti->v_t[0], ti->v_t[1], ti->v_t[2], ti->v_t[3]},
            .pad0    = 1337,
        };
    }

    glNamedBufferStorage(gfx->faces, sizeof(*faces) * gfx->face_count, faces,
                         GL_DYNAMIC_STORAGE_BIT);

    gfx->texture = tex;
    free(faces);
}

void bsp_gfx_setup_buffers(bsp_t *bsp) {}

bspgfx_t *bsp_gfx_create(bsp_t *bsp) {
    bspgfx_t *gfx = malloc(sizeof(*gfx));
    gfx->program =
        gl_program_load_file(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
    glCreateBuffers(1, &gfx->verts);
    glCreateBuffers(1, &gfx->faces);
    glCreateBuffers(1, &gfx->elems);
    glCreateBuffers(1, &gfx->cmds);

    construct_element_buffer(bsp, gfx);
    construct_vertex_buffer(bsp, gfx);
    construct_face_buffer(bsp, gfx);

    gfx->face_count = bsp->face_count;
    gfx->vert_count = bsp->vert_count;
    gfx->debug      = 0;

    return gfx;
}

void bsp_gfx_render(bspgfx_t *gfx, sccamera_t *cam) {
    glUseProgram(gfx->program);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gfx->elems);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gfx->cmds);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gfx->verts);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gfx->faces);
    glBindTextureUnit(0, gfx->texture);
    glUniform1i(2, 0);
    glUniform1ui(3, (uint32_t)gfx->debug);

    glUniformMatrix4fv(0, 1, GL_FALSE, (void *)cam->view);
    glUniformMatrix4fv(1, 1, GL_FALSE, (void *)cam->proj);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL,
                                gfx->face_count, 0);
}

void bsp_gfx_unload_gpu(bspgfx_t *gfx) {

    if (gfx->elems) {
        glDeleteBuffers(1, &gfx->elems);
    }
    if (gfx->verts) {
        glDeleteBuffers(1, &gfx->verts);
    }
    if (gfx->faces) {
        glDeleteBuffers(1, &gfx->faces);
    }
    if (gfx->program) {
        glDeleteProgram(gfx->program);
    }
}

void bsp_gfx_free(bspgfx_t *gfx) {
    if (gfx) {
        bsp_gfx_unload_gpu(gfx);
        free(gfx);
        gfx = NULL;
    }
}

void bsp_gfx_load(bspgfx_t *gfx, bsp_t *bsp) {
    bsp_gfx_unload_gpu(gfx);

    gfx->program =
        gl_program_load_file(DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
    glCreateBuffers(1, &gfx->verts);
    glCreateBuffers(1, &gfx->faces);
    glCreateBuffers(1, &gfx->elems);
    glCreateBuffers(1, &gfx->cmds);

    construct_element_buffer(bsp, gfx);
    construct_vertex_buffer(bsp, gfx);
    construct_face_buffer(bsp, gfx);

    gfx->face_count = bsp->face_count;
    gfx->vert_count = bsp->vert_count;
}
