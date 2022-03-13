#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <math.h>
#include <stdbool.h>

#include <lodepng.h>
#include <file_io.h>
#include <file_io.h>
#include <draw_julia.h>
#include <opencl_funcs.h>

int main(int argc, char *argv[]){

  //Profiling info vars
  clock_t start, end, start_g, end_g;
  start_g = clock();
  printf("Started whole program clock\n");

  int N = 1000;   //Max iterations
                 // 4K = 3840 × 2160
  int w = 3840;
  int h = 2160;
  char *plot_type = "rec_f";
  int A = h*w;
  double c[2] = {0.36, 0.34};   //If parameter space this is interpreted as Z
  double Sx[2] = {-2, 2};
  double Sy[2] = {-2, 2};

  //Zooming point for zoom function
  double p[2] = {0.36, 0.34};

  char *out_folder = gen_dir_name(c, "rec_f");
  char *empty_julia_f = gen_filename(out_folder, "Empty_Julia.png");
  char *full_julia_f = gen_filename((char *) out_folder, "Thumbnail.png");
  char *merged = gen_filename(out_folder, "Merged.png");

  printf("Saving results in: %s\n", out_folder);

  printf("Drawing julia by BI...      "); fflush(stdout);
  start = clock();
  unsigned char *empty_julia = draw_julia_backwards_iteration(N, h, w, c, 100000, true);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);
  printf("Saving empty Julia set...   "); fflush(stdout);
  start = clock();
  lodepng_encode_file(empty_julia_f, empty_julia, w, h, LCT_GREY, 8);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);


  printf("Drawing Full Julia set...   "); fflush(stdout);
  start = clock();
  unsigned char *full_julia = draw_thumbnail(N, h, w, c, plot_type, NULL, true);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);

  printf("Saving result...            "); fflush(stdout);
  start = clock();
  lodepng_encode24_file(full_julia_f, full_julia, w, h);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);

  printf("Merging result...           "); fflush(stdout);
  start = clock();
  unsigned char *merg = merge_sets(full_julia, empty_julia, h, w);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);

  printf("Saving result...            "); fflush(stdout);
  start = clock();
  lodepng_encode24_file(merged, merg, w, h);
  end = clock();
  printf("Done. Took %f seconds\n", ((double)(end - start))/CLOCKS_PER_SEC);

  ////rec_f or parameter_space
  //draw_julia_zoom(10, N, h, w, c, p, 0.05, "rec_f");

  end_g = clock();
  printf("Finished whole program execution. Took %f seconds\n", ((double)(end_g - start_g))/CLOCKS_PER_SEC);

}
