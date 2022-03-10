__kernel void merge(__global unsigned char *result,
                    __global unsigned char *empty,
                    __global unsigned char  *full,
                    __global int *hp,
                    __global int *wp) {

  int h = *hp, w = *wp;
  const int y = get_global_id(0);
  const int x = get_global_id(1);

  if (empty[y*w + x] == 255){
    result[(y*w + x)*3 + 0] = 255;
    result[(y*w + x)*3 + 1] = 255;
    result[(y*w + x)*3 + 2] = 255;
  } else {
    result[(y*w + x)*3 + 0] = full[(y*w + x)*3 + 0];
    result[(y*w + x)*3 + 1] = full[(y*w + x)*3 + 1];
    result[(y*w + x)*3 + 2] = full[(y*w + x)*3 + 2];
  }
}

