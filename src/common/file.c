#include "common/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/log.h"

const char *expand_user(const char *path) {
    const char *home = getenv("HOME");
    if (!home) {
        home = getenv("USERPROFILE");
    }

    if (path[0] == '~') {
        char *expanded = malloc(strlen(path) + strlen(home));
        strcpy(expanded, home);
        strcat(expanded, path + 1);
        return expanded;
    }

    return strdup(path);
}

size_t file_read(const char *filename, char **dest) {
    *dest = NULL;
    FILE *fp = fopen(expand_user(filename), "rb");
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
