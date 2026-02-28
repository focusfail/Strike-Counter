#include "common/file.h"

#include <stdio.h>

#include "common/log.h"

size_t file_read(const char *filename, char **dest) {
    *dest    = NULL;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        log_error("Failed to open \"%s\"", filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *buf = malloc(size + 1);
    buf[size] = '\0';

    size_t read = fread(buf, 1, size, fp);
    if (read != size) {
        log_error("Failed to read \"%s\"", filename);
        free(buf);
        fclose(fp);
        return 0;
    }

    *dest = buf;
    fclose(fp);
    return size;
}
