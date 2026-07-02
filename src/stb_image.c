#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"
#include <stdlib.h>
#include <string.h>

STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp) {
    return 0;
}

STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp) {
    return 0;
}

STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp) {
    return NULL;
}

STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp) {
    return NULL;
}

#ifndef STBI_NO_STDIO
STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp) {
    return 0;
}

STBIDEF int stbi_info_from_file(FILE *f, int *x, int *y, int *comp) {
    return 0;
}

STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp) {
    return NULL;
}

STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp) {
    return NULL;
}
#endif

STBIDEF void stbi_image_free(void *retval_from_stbi_load) {
    free(retval_from_stbi_load);
}

STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply) {}
STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert) {}
STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip) {}
STBIDEF void stbi_set_unpremultiply_on_load_thread(int flag_true_if_should_unpremultiply) {}
STBIDEF int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len) { return 0; }
STBIDEF int stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user) { return 0; }
#ifndef STBI_NO_STDIO
STBIDEF int stbi_is_hdr(char const *filename) { return 0; }
STBIDEF int stbi_is_hdr_from_file(FILE *f) { return 0; }
#endif
STBIDEF float *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp) { return NULL; }
STBIDEF float *stbi_loadf_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp) { return NULL; }
#ifndef STBI_NO_STDIO
STBIDEF float *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp) { return NULL; }
STBIDEF float *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp) { return NULL; }
#endif
STBIDEF void stbi_hdr_to_ldr_gamma(float gamma) {}
STBIDEF void stbi_ldr_to_hdr_scale(float scale) {}

STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp) { return 0; }
STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp) { return 0; }
STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp) { return 0; }