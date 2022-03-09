#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <math.h>

#include <lodepng.h>
#include <draw_julia.h>
#include <opencl_funcs.h>



int main(int argc, char *argv[]){

  //Profiling info vars
  clock_t start, end, start_g, end_g;
  start_g = clock();
  printf("Started whole program clock\n");

  int N = 1000000;   //Max iterations
  int h = 10000;
  int w = 10000;
  int A = h*w;
  double c[2] = {0.26, 0};   //If parameter space this is interpreted as Z
  double Sx[2] = {-2, 2};
  double Sy[2] = {-2, 2};

  // double p[2] = {-0.743039152, -0.14809243};
  double p[2] = {0, 0};

  printf("Drawing julia by backwards iteration...\n");
  start = clock();
  unsigned char *m = draw_julia_backwards_iteration(N, h, w, c, 100000);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);
  lodepng_encode_file("test.png", m, w, h, LCT_GREY, 8);

  ////rec_f or parameter_space
  //draw_julia_zoom(0, N, h, w, c, p, "rec_f");
  //end_g = clock();
  //printf("Finished whole program execution. Took %f seconds\n", ((double)(end_g - start_g))/CLOCKS_PER_SEC);

}
