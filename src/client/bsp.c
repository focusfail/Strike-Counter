#include "client/bsp.h"
#include "client/wad.h"
#include "common/file.h"
#include "common/log.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

static const char *wad_search_paths[] = {
    "/home/focus/.local/share/Steam/steamapps/common/Half-Life/cstrike",
    "/home/focus/.local/share/Steam/steamapps/common/Half-Life/valve"};
static const uint32_t wad_search_path_count =
    sizeof(wad_search_paths) / sizeof(*wad_search_paths);

void bsp_load_textures(bsp_t *bsp, bsplump_t lump) {
    if (lump.len < sizeof(int32_t)) {
        log_warn("No textures in texture lump.");
        return;
    }

    char *tex_start = (char *)bsp->file_buffer + lump.ofs;
    int32_t tex_count = *(int32_t *)tex_start;

    if (tex_count < 0 || tex_count > 2048) {
        log_error("Malformed or invalid texture count.");
        return;
    }

    bsp->miptex_count = tex_count;
    bsp->miptex = malloc(sizeof(bsp->miptex) * tex_count);

    // After the texture count 'n' follows 'n' offsets
    int32_t *tex_offsets = (int32_t *)(tex_start + sizeof(int32_t));

    for (int i = 0; i < tex_count; i++) {
        int32_t ofs = tex_offsets[i];

        if (ofs < 0 || ofs >= lump.len) {
            log_error("Invalid texture offset.");
            continue;
        }

        bsp->miptex[i] = (miptex_t *)(tex_start + ofs);
    }
}

static const char *skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p))
        p++;
    return p;
}

static const char *parse_string(const char *p, const char **out) {
    if (*p != '"') return NULL;
    p++;

    *out = p;

    while (*p && *p != '"') {
        p++;
    }

    if (*p != '"') return NULL;

    return p + 1;
}

void bsp_load_entities(bsp_t *bsp, bsplump_t lump) {
    char *entities_string = (char *)bsp->file_buffer + lump.ofs;
    const char *p = entities_string;

    bsp->entity_count = 0;

    while (*p) {
        p = skip_ws(p);
        if (!*p) break;

        if (*p != '{') {
            fprintf(stderr, "Expected '{' at start of entity\n");
            return;
        }
        p = skip_ws(p + 1);

        if (bsp->entity_count >= MAX_ENTITIES) {
            fprintf(stderr, "Too many entities (max %d)\n", MAX_ENTITIES);
            return;
        }

        bspentity_t *ent = &bsp->entities[bsp->entity_count];
        memset(ent->origin, 0, sizeof(ent->origin));
        ent->brush_first = -1;
        ent->brush_count = 0;
        ent->pairs = NULL;

        epair_t **pair_tail = &ent->pairs;

        while (*p && *p != '}') {
            p = skip_ws(p);
            if (*p == '}') break;

            const char *key_str, *val_str;
            p = parse_string(p, &key_str);
            if (!p) {
                fprintf(stderr, "Failed to parse key string\n");
                return;
            }

            p = skip_ws(p);
            p = parse_string(p, &val_str);
            if (!p) {
                fprintf(stderr, "Failed to parse value string\n");
                return;
            }

            size_t key_len = 0, val_len = 0;
            {
                const char *tmp = key_str;
                while (*tmp && *tmp != '"') {
                    tmp++;
                    key_len++;
                }
                tmp = val_str;
                while (*tmp && *tmp != '"') {
                    tmp++;
                    val_len++;
                }
            }
            // Allocate epair
            epair_t *ep = malloc(sizeof(epair_t));
            if (!ep) {
                fprintf(stderr, "Out of memory allocating epair\n");
                return;
            }

            ep->k = malloc(key_len + 1);
            ep->v = malloc(val_len + 1);
            if (!ep->k || !ep->v) {
                free(ep->k);
                free(ep->v);
                free(ep);
                fprintf(stderr, "Out of memory allocating key/value strings\n");
                return;
            }

            ep->k_len = (uint32_t)key_len;
            ep->v_len = (uint32_t)val_len;

            memcpy(ep->k, key_str, key_len);
            ep->k[key_len] = '\0';

            memcpy(ep->v, val_str, val_len);
            ep->v[val_len] = '\0';

            ep->k = ep->k;
            ep->v = ep->v;
            ep->k_len = (uint32_t)key_len;
            ep->v_len = (uint32_t)val_len;

            ep->next = NULL;
            *pair_tail = ep;
            pair_tail = &ep->next;

            p = skip_ws(p);
        }

        if (*p != '}') {
            fprintf(stderr, "Expected '}' to end entity\n");
            return;
        }
        p = skip_ws(p + 1);

        bsp->entity_count++;
    }
}

void replace(char *src, char find, char replace) {
    while (*src != '\0') {
        if (*src == find) {
            *src = replace;
        }
        src++;
    }
}

char **locate_wads(bsp_t *bsp, const char **base_paths, uint32_t path_count,
                   uint32_t *found_count) {
    char *wad_paths_str = strdup(bsp->entities[0].pairs->v);
    replace(wad_paths_str, '\\', '/');
    char complete_path[256];
    char **found_paths = malloc(sizeof(*found_paths) * 16);

    char *token = strtok(wad_paths_str, " ; ");
    while (token != NULL) {
        char *tmp_name = strdup(token);
        char *name = basename(tmp_name);

        bool found = false;
        for (int i = 0; i < path_count; i++) {
            snprintf(complete_path, 256, "%s/%s", base_paths[i], name);
            FILE *fp = fopen(complete_path, "r");
            if (fp) {
                fclose(fp);
                log_info("Located %s", name);
                found = true;
                break;
            }
        }

        if (!found) {
            log_error("Unable to locate \"%s\"", name);
            continue;
        }

        found_paths[(*found_count)++] = strdup(complete_path);
        // found_paths[(*found_count)++] = strndup(complete_path, 256);

        free(tmp_name);
        token = strtok(NULL, " ; ");
    }

    free(wad_paths_str);
    return found_paths;
}

void load_wads(bsp_t *bsp) {
    bspentity_t *ent = &bsp->entities[0];
    epair_t *pair = epair_from_key(ent, "wad");
    if (!pair) {
        log_error("Failed to retrieve WAD part from entities.");
        return;
    }

    uint32_t wad_path_count = 0;
    char **wad_paths = locate_wads(bsp, wad_search_paths, wad_search_path_count,
                                   &wad_path_count);

    bsp->wads = malloc(sizeof(*bsp->wads) * wad_path_count);
    bsp->wad_count = wad_path_count;
    for (int i = 0; i < wad_path_count; i++) {
        bsp->wads[i] = wad_open(wad_paths[i]);
        free(wad_paths[i]);
    }
    free(wad_paths);
}

bsp_t *bsp_create(const char *fn) {
    bsp_t *bsp = malloc(sizeof(*bsp));

    bsp->file_buffer = 0;
    bsp->faces = NULL;
    bsp->edges = NULL;
    bsp->surfedges = NULL;
    bsp->miptex = NULL;
    bsp->texinfo = NULL;

    bsp->face_count = 0;
    bsp->edge_count = 0;
    bsp->surfedge_count = 0;
    bsp->miptex_count = 0;
    bsp->texinfo_count = 0;

    bsp_load(bsp, fn);

    return bsp;
}

void bsp_load(bsp_t *bsp, const char *fn) {
    char *file_buf = NULL;
    size_t file_size = file_read(fn, &file_buf);

    if (file_size < sizeof(bspheader_t) || !file_buf) {
        return;
    }

    bspheader_t *hdr = (bspheader_t *)file_buf;
    bsp->file_buffer = file_buf;

    if (hdr->version != 30) {
        log_warn("Wrong .bsp version: %d instead of 30", hdr->version);
    }

    bsplump_t *lump;

    lump = &hdr->lumps[LUMP_FACES];
    bsp->faces = (bspface_t *)((char *)hdr + lump->ofs);
    bsp->face_count = lump->len / sizeof(*bsp->faces);

    lump = &hdr->lumps[LUMP_VERTEXES];
    bsp->verts = (bspvert_t *)((char *)hdr + lump->ofs);
    bsp->vert_count = lump->len / sizeof(*bsp->verts);

    lump = &hdr->lumps[LUMP_EDGES];
    bsp->edges = (bspedge_t *)((char *)hdr + lump->ofs);
    bsp->edge_count = lump->len / sizeof(*bsp->edges);

    lump = &hdr->lumps[LUMP_SURFEDGES];
    bsp->surfedges = (bspsurfedge_t *)((char *)hdr + lump->ofs);
    bsp->surfedge_count = lump->len / sizeof(*bsp->surfedges);

    lump = &hdr->lumps[LUMP_TEXINFO];
    bsp->texinfo = (bsptexinfo_t *)((char *)hdr + lump->ofs);
    bsp->texinfo_count = lump->len / sizeof(*bsp->texinfo);

    lump = &hdr->lumps[LUMP_PLANES];
    bsp->planes = (bspplane_t *)((char *)hdr + lump->ofs);
    bsp->plane_count = lump->len / sizeof(*bsp->planes);

    lump = &hdr->lumps[LUMP_LEAFS];
    bsp->leafs = (bspleaf_t *)((char *)hdr + lump->ofs);
    bsp->leaf_count = lump->len / sizeof(*bsp->leafs);

    lump = &hdr->lumps[LUMP_NODES];
    bsp->nodes = (bspnode_t *)((char *)hdr + lump->ofs);
    bsp->node_count = lump->len / sizeof(*bsp->nodes);

    lump = &hdr->lumps[LUMP_TEXTURES];
    bsp_load_textures(bsp, *lump);

    lump = &hdr->lumps[LUMP_ENTITIES];
    bsp_load_entities(bsp, *lump);

    load_wads(bsp);
    return;
}

epair_t *epair_from_key(bspentity_t *ent, const char *key) {
    epair_t *pair = ent->pairs;

    while (pair) {
        if (strncmp(pair->k, key, pair->k_len) == 0) {
            return pair;
        }
        pair = pair->next;
    }

    return NULL;
}

void bsp_dump(bsp_t *bsp) {
    if (!bsp || !bsp->file_buffer) return;

    FILE *fp = fopen(SC_ASSET_PATH "/dump.txt", "wb");

    bspheader_t *hdr = (bspheader_t *)bsp->file_buffer;
    fprintf(fp, "Version {%d}\n", hdr->version);

    for (int i = 0; i < LUMP_COUNT; i++) {
        fprintf(fp, "Lump {%d, ofs=%d, len=%d}\n", i, hdr->lumps[i].ofs,
                hdr->lumps[i].len);
    }

    for (int i = 0; i < bsp->miptex_count; i++) {
        fprintf(fp, "Texture {\"%s\", %dx%d, %d %d %d %d}\n",
                bsp->miptex[i]->name, bsp->miptex[i]->w, bsp->miptex[i]->h,
                bsp->miptex[i]->offsets[0], bsp->miptex[i]->offsets[1],
                bsp->miptex[i]->offsets[2], bsp->miptex[i]->offsets[3]);
    }

    for (int i = 0; i < bsp->texinfo_count; i++) {
        bsptexinfo_t *t = &bsp->texinfo[i];
        fprintf(fp,
                "Texinfo {id=%i, (%.3f %.3f %.3f %.3f) (%.3f %.3f %.3f %.3f) "
                "%dx%d}\n",
                t->miptex, t->v_s[0], t->v_s[1], t->v_s[2], t->v_s[3],
                t->v_t[0], t->v_t[1], t->v_t[2], t->v_t[3],
                bsp->miptex[t->miptex]->w, bsp->miptex[t->miptex]->h);
    }

    for (int i = 0; i < bsp->edge_count; i++) {
        fprintf(fp, "Edge {%u %u %u %u}\n", bsp->edges[i].v[0],
                bsp->edges[i].v[1], bsp->edges[i].v[2], bsp->edges[i].v[3]);
    }

    for (int i = 0; i < bsp->vert_count; i++) {
        fprintf(fp, "Vertex {(%.3f %.3f %.3f)}\n", bsp->verts[i][0],
                bsp->verts[i][1], bsp->verts[i][2]);
    }

    for (int i = 0; i < bsp->wad_count; i++) {
        wad_t *wad = bsp->wads[i];
        fprintf(fp, "Wad %s {\n", wad->name);

        for (int j = 0; j < wad->lump_count; j++) {
            wadlumpinfo_t *lmp = &wad->lumps[j];
            fprintf(fp, "\t%s pos=%d size=%d\n", lmp->name, lmp->filepos,
                    lmp->size);
        }
        fprintf(fp, "}\n");
    }

    for (int i = 0; i < bsp->entity_count; i++) {
        fprintf(fp, "Entity {\n\t");
        epair_t *pair = bsp->entities[i].pairs;
        while (pair != NULL) {
            fprintf(fp, "\t%s = %s\n", pair->k, pair->v);
            pair = pair->next;
        }

        fprintf(fp, "}\n");
    }
    fclose(fp);
}

void bsp_free_entities(bsp_t *bsp) {
    for (uint32_t i = 0; i < bsp->entity_count; i++) {
        epair_t *ep = bsp->entities[i].pairs;
        while (ep) {
            epair_t *next = ep->next;
            free(ep->k);
            free(ep->v);
            free(ep);
            ep = next;
        }
        bsp->entities[i].pairs = NULL;
    }
    bsp->entity_count = 0;
}

void bsp_free(bsp_t *bsp) {
    if (bsp) {
        if (bsp->file_buffer) {
            free(bsp->file_buffer);
            bsp->file_buffer = NULL;
        }

        if (bsp->miptex) {
            free(bsp->miptex);
        }

        for (int i = 0; i < bsp->wad_count; i++) {
            wad_close(bsp->wads[i]);
        }

        bsp_free_entities(bsp);
        free(bsp);
        bsp = NULL;
    }
}
