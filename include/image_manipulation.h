#ifndef IMAGE_MANIPULATION_H
#define IMAGE_MANIPULATION_H

#include <stdbool.h>
#include <opencl_funcs.h>

void draw_line(unsigned char *m, int x0, int y0, int x1, int y1, int w, int h);
unsigned char *merge_sets(unsigned char *full, unsigned char *empty, int w, int h);
_Bool LiangBarsky(int, int, int, int, int, int, int, int, int *, int *, int *, int *);

unsigned char *image_manipulation_clone_image(unsigned char *source, int w, int h, struct OpenCL_Program **, _Bool);


#endif //IMAGE_MANIPULATION_H
