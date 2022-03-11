#ifndef IMAGE_MANIPULATION_H
#define IMAGE_MANIPULATION_H

void draw_line(unsigned char *m, int x0, int y0, int x1, int y1, int w, int h);
unsigned char *merge_sets(unsigned char *full, unsigned char *empty, int w, int h);

#endif //IMAGE_MANIPULATION_H
