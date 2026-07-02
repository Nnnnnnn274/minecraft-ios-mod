#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_THREAD_LOCALS

#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp) {
    (void)buffer; (void)len; (void)x; (void)y; (void)comp; (void)req_comp;
    return NULL;
}

stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp) {
    (void)clbk; (void)user; (void)x; (void)y; (void)comp; (void)req_comp;
    return NULL;
}

int stbi_info(char const *filename, int *x, int *y, int *comp) {
    (void)filename; (void)x; (void)y; (void)comp;
    return 0;
}

int stbi_info_from_file(FILE *f, int *x, int *y, int *comp) {
    (void)f; (void)x; (void)y; (void)comp;
    return 0;
}

stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp) {
    (void)filename; (void)x; (void)y; (void)comp; (void)req_comp;
    return NULL;
}

stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp) {
    (void)f; (void)x; (void)y; (void)comp; (void)req_comp;
    return NULL;
}

int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp) {
    (void)buffer; (void)len; (void)x; (void)y; (void)comp;
    return 0;
}

int stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp) {
    (void)clbk; (void)user; (void)x; (void)y; (void)comp;
    return 0;
}

void stbi_image_free(void *retval_from_stbi_load) {
    (void)retval_from_stbi_load;
}

int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len) {
    (void)buffer; (void)len;
    return 0;
}

int stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user) {
    (void)clbk; (void)user;
    return 0;
}

int stbi_is_hdr(char const *filename) {
    (void)filename;
    return 0;
}

int stbi_is_hdr_from_file(FILE *f) {
    (void)f;
    return 0;
}

const char *stbi_failure_reason(void) {
    return "stb_image not fully implemented - stub only";
}

int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes) {
    (void)filename; (void)w; (void)h; (void)comp; (void)data; (void)stride_in_bytes;
    return 0;
}

int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data) {
    (void)filename; (void)w; (void)h; (void)comp; (void)data;
    return 0;
}

int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data) {
    (void)filename; (void)w; (void)h; (void)comp; (void)data;
    return 0;
}

int stbi_write_hdr(char const *filename, int w, int h, int comp, const void *data) {
    (void)filename; (void)w; (void)h; (void)comp; (void)data;
    return 0;
}