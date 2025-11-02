#ifndef CLIENT_GL_COMMON_H
#define CLIENT_GL_COMMON_H

#include <stdint.h>
#define IMARRAY_MAX 256
#define IMARRAY_MAX_SIZE 256

typedef struct {
    unsigned char *data;
    int32_t w, h;
} image_t;

typedef struct {
    image_t images[IMARRAY_MAX];
    int32_t image_count;
} imarr_t;

uint32_t gl_program_load_file(const char *vs_filename, const char *fs_filename);
uint32_t gl_program_load(const char *vs_source, const char *fs_source);
uint32_t gl_texture_load(const char *filename, int *w, int *h);

uint32_t gl_texture_array_create(int w, int h, int layers);
void gl_texture_array_append(uint32_t texture, int w, int h,
                             const uint8_t *data);

imarr_t *image_array_create();
image_t *image_array_add(imarr_t *arr, const char *filename);
uint32_t image_array_to_texture2darray(imarr_t *arr);
void image_array_free(imarr_t *arr);

#endif // CLIENT_GL_COMMON_H
