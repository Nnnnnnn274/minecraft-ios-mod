/* stb_image.h - v2.28 - public domain image loader - header only stub */
/* Full implementation omitted for brevity — include the real stb_image.h from https://github.com/nothings/stb */

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t stbi_uc;
typedef uint16_t stbi_us;

typedef struct
{
   int (*read)  (void *user,char *data,int size);
   void (*skip) (void *user,int n);
   int (*eof)   (void *user);
} stbi_io_callbacks;

#ifdef __cplusplus
extern "C" {
#endif

extern int      stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
extern int      stbi_info_from_callbacks(void *clbk, void *user, int *x, int *y, int *comp);

extern stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_callbacks(void *clbk, void *user, int *x, int *y, int *comp, int req_comp);

#ifndef STBI_NO_STDIO
extern int      stbi_info(char const *filename, int *x, int *y, int *comp);
extern int      stbi_info_from_file(FILE *f, int *x, int *y, int *comp);
extern stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp);
#endif

extern void     stbi_image_free(void *retval_from_stbi_load);

extern int      stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
extern int      stbi_is_hdr_from_callbacks(void *clbk, void *user);
#ifndef STBI_NO_STDIO
extern int      stbi_is_hdr(char const *filename);
extern int      stbi_is_hdr_from_file(FILE *f);
#endif

extern float   *stbi_loadf_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);
extern float   *stbi_loadf_from_callbacks(void *clbk, void *user, int *x, int *y, int *comp, int req_comp);
#ifndef STBI_NO_STDIO
extern float   *stbi_loadf(char const *filename, int *x, int *y, int *comp, int req_comp);
extern float   *stbi_loadf_from_file(FILE *f, int *x, int *y, int *comp, int req_comp);
#endif

extern int      stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

#ifdef __cplusplus
}
#endif

#endif
