#include "client/gl_common.h"
#include "common/file.h"
#include "common/log.h"
#define STB_IMAGE_IMPLEMENTATION

#include <glad/gl.h>
#include <stdlib.h>
#include <stb_image.h>
#include <cglm/cglm.h>

uint32_t gl_program_load(const char *vs_source, const char *fs_source) {
    GLint success    = 0;
    GLuint vs_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader, 1, (const GLchar **)&vs_source, 0);
    glCompileShader(vs_shader);
    glGetShaderiv(vs_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint info_len = 0;
        glGetShaderiv(vs_shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            char *info_log = malloc(info_len);
            if (info_log) {
                glGetShaderInfoLog(vs_shader, info_len, &info_len, info_log);
                log_error("Vertex shader failed to compile: %s", info_log);
                free(info_log);
            }
        }
        glDeleteShader(vs_shader);
        return 0;
    }

    GLuint fs_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader, 1, (const GLchar **)&fs_source, 0);
    glCompileShader(fs_shader);
    glGetShaderiv(fs_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint info_len = 0;
        glGetShaderiv(fs_shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            char *info_log = malloc(info_len);
            if (info_log) {
                glGetShaderInfoLog(fs_shader, info_len, &info_len, info_log);
                log_error("Fragment shader failed to compile: %s", info_log);
                free(info_log);
            }
        }
        glDeleteShader(vs_shader);
        glDeleteShader(fs_shader);
        return 0;
    }

    uint32_t program = glCreateProgram();
    glAttachShader(program, vs_shader);
    glAttachShader(program, fs_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint info_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 0) {
            char *info_log = malloc(info_len);
            if (info_log) {
                glGetProgramInfoLog(program, info_len, &info_len, info_log);
                log_error("Program failed to link: %s", info_log);
                free(info_log);
            }
        }
        glDeleteProgram(program);
        glDeleteShader(vs_shader);
        glDeleteShader(fs_shader);
        return 0;
    }

    glDetachShader(program, vs_shader);
    glDetachShader(program, fs_shader);
    glDeleteShader(vs_shader);
    glDeleteShader(fs_shader);
    return program;
}

uint32_t gl_program_load_file(const char *vs_filename,
                              const char *fs_filename) {

    char *vs_source;
    char *fs_source;
    if (!file_read(vs_filename, &vs_source)) return 0;
    if (!file_read(fs_filename, &fs_source)) {
        free(vs_source);
        return 0;
    }
    uint32_t program = gl_program_load(vs_source, fs_source);

    free(vs_source);
    free(fs_source);

    return program;
    // return 0;
}

uint32_t gl_texture_load(const char *filename, int *w, int *h) {
    int32_t c;
    uint32_t texture;
    unsigned char *data = stbi_load(filename, w, h, &c, STBI_rgb_alpha);

    if (!data) return 0;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1 + floor(log2(glm_max(*w, *h))), GL_RGBA8, *w,
                       *h);
    glTextureSubImage2D(texture, 0, 0, 0, *w, *h, GL_RGBA, GL_UNSIGNED_BYTE,
                        data);
    glGenerateTextureMipmap(texture);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return texture;
}

imarr_t *image_array_create() {
    imarr_t *arr     = malloc(sizeof(*arr));
    arr->image_count = 0;
    return arr;
}

image_t *image_array_add(imarr_t *arr, const char *filename) {
    if (arr->image_count >= IMARRAY_MAX) return NULL;

    int w, h, c;
    unsigned char *data = stbi_load(filename, &w, &h, &c, STBI_rgb_alpha);

    if (!data) return NULL;

    image_t *im = &arr->images[arr->image_count++];
    im->w       = w;
    im->h       = h;
    im->data    = data;

    return im;
}

uint32_t image_array_to_texture2darray(imarr_t *arr) {
    uint32_t id = 0;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
    glTextureStorage3D(id, 1, GL_RGBA8, IMARRAY_MAX_SIZE, IMARRAY_MAX_SIZE,
                       IMARRAY_MAX);

    for (int i = 0; i < arr->image_count; i++) {
        glTextureSubImage3D(id, 0, 0, 0, i, arr->images[i].w, arr->images[i].h,
                            1, GL_RGBA, GL_UNSIGNED_BYTE, arr->images[i].data);
    }

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return id;
}

void image_array_free(imarr_t *arr) {
    for (int i = 0; i < arr->image_count; i++) {
        image_t *im = &arr->images[i];
        stbi_image_free(im->data);
    }
    free(arr);
}

uint32_t gl_texture_array_create(int w, int h, int layers) {
    uint32_t id = 0;

    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
    glTextureStorage3D(id, 1, GL_RGBA, w, h, layers);

    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return id;
}
