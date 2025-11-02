#include "client/wad.h"
#include "common/file.h"
#include "common/log.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

wad_t *wad_open(const char *filename) {
    wad_t *wad     = malloc(sizeof(*wad));
    wad->name      = strdup(filename);
    wad->file_size = file_read(wad->name, &wad->file_buffer);

    if (wad->file_size < sizeof(wadinfo_t)) {
        log_error("wad");
    }

    wadinfo_t header;
    memcpy(&header, wad->file_buffer, sizeof(header));

    if (strncmp(header.id, "WAD2", 4) && strncmp(header.id, "WAD3", 4)) {
        log_error("Unsupported WAD format %s", header.id);
        return NULL;
    }

    wad->lumps      = (wadlumpinfo_t *)(wad->file_buffer + header.info_ofs);
    wad->lump_count = header.lump_count;

    return wad;
}

void wad_close(wad_t *wad) {
    if (wad) {
        if (wad->file_buffer) {
            free(wad->file_buffer);
        }

        if (wad->name) {
            free(wad->name);
        }

        free(wad);
    }
}

bool wad_find_lump_by_name(wad_t *wad, wadlumpinfo_t **lump, const char *name) {
    if (!wad) return false;

    *lump = NULL;

    for (int i = 0; i < wad->lump_count; i++) {
        wadlumpinfo_t *l = &wad->lumps[i];

        if (strncasecmp(l->name, name, 16) == 0) {
            *lump = l;
            return true;
        }
    }

    return false;
}
