#ifndef CLIENT_WAD_H
#define CLIENT_WAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char id[4];
    int32_t lump_count;
    int32_t info_ofs;
} wadinfo_t;

typedef struct {
    int32_t filepos;
    int32_t disksize;
    int32_t size;
    char type;
    char comp;
    char pad0, pad1;
    char name[16];
} wadlumpinfo_t;

typedef struct {
    char *name;
    char *file_buffer;
    size_t file_size;

    wadlumpinfo_t *lumps;
    uint32_t lump_count;
} wad_t;

wad_t *wad_open(const char *filename);
bool wad_find_lump_by_name(wad_t *wad, wadlumpinfo_t **lump, const char *name);
void wad_close(wad_t *wad);

#endif // CLIENT_WAD_H
