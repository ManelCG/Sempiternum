#include <opencl_funcs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <complex.h>

#include <time.h>
#include <stdbool.h>

#include <math.h>

#include <lodepng.h>


#include <file_io.h>
#include <draw_julia.h>

#define DEBUG_DRAW_JULIA_C



unsigned char *merge_sets(unsigned char *full, unsigned char *empty, int h, int w){
  unsigned char *r = malloc(w*h*3);

  struct OpenCL_Program *prog = get_opencl_info();
  FILE *fp;
  char *filename = "opencl/merge_sets.c";

  fp = fopen(filename, "r");
  if (!fp){
    fprintf(stderr, "Failed to load kernel.\n");
    exit(1);
  }

  prog->src = (char *) calloc(MAX_SOURCE_SIZE, 1);
  prog->src_size = fread(prog->src, 1, MAX_SOURCE_SIZE, fp);

  fclose(fp);

  #ifdef DEBUG_DRAW_JULIA_C
  printf("Executing:\n------------------------\n%s\n------------------------\n", prog->src);
  #endif

  prog->context = clCreateContext(NULL, 1, &(prog->device), NULL, NULL, &(prog->ret));

  #ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL context created. Return code: %d\n", prog->ret);
  #endif

  prog->command_queue = clCreateCommandQueue(prog->context, prog->device, 0, &(prog->ret));
  #ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL CommandQueue created. Return code: %d\n", prog->ret);
  #endif


  cl_mem mem_empty = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              w*h, NULL, &(prog->ret));
  cl_mem mem_full = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              w*h*3, NULL, &(prog->ret));
  cl_mem mem_h = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_w = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_ret = clCreateBuffer(prog->context, CL_MEM_WRITE_ONLY,
                              w*h*3, NULL, &(prog->ret));

  clEnqueueWriteBuffer(prog->command_queue, mem_empty, CL_TRUE, 0, w*h*sizeof(unsigned char), empty , 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_full, CL_TRUE, 0, w*h*3*sizeof(unsigned char), full , 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_h, CL_TRUE, 0, sizeof(int), &h, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_w, CL_TRUE, 0, sizeof(int), &w, 0, NULL, NULL);

  prog->program = clCreateProgramWithSource(prog->context,
                                            1,
                                            (const char **)  &(prog->src),
                                            (const size_t *) &(prog->src_size),
                                            &(prog->ret));
  #ifdef DEBUG_DRAW_JULIA_C
  printf("Building %s... ", filename);
  #endif
  fflush(stdout);
  clBuildProgram(prog->program, 1, &(prog->device), NULL, NULL, NULL);

  prog->kernel = clCreateKernel(prog->program, "merge", &(prog->ret));

  #ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL Kernel created. Return code: %d\n\n", prog->ret);
  #endif

  clSetKernelArg(prog->kernel, 0, sizeof(mem_ret),  (void *)&mem_ret);
  clSetKernelArg(prog->kernel, 1, sizeof(mem_empty),  (void *)&mem_empty);
  clSetKernelArg(prog->kernel, 2, sizeof(mem_full),  (void *)&mem_full);
  clSetKernelArg(prog->kernel, 3, sizeof(mem_h),  (void *)&mem_h);
  clSetKernelArg(prog->kernel, 4, sizeof(mem_w),  (void *)&mem_w);

  fflush(stdout);

  const size_t worksize[] = {h, w};

  cl_int status;

  status = clEnqueueNDRangeKernel(prog->command_queue,
                         prog->kernel, 2, NULL,
                         worksize,
                         NULL,
                         0, NULL, NULL);

  clEnqueueReadBuffer(prog->command_queue,
                      mem_ret,
                      CL_TRUE,
                      0,
                      w*h*3,
                      r,
                      0,
                      NULL, NULL);



  return r;
}

unsigned char *draw_julia(int N, int h, int w, double c[2], double Sx[2], double Sy[2], char *plot_type){
  //Make h and w even
  h = (h%2 == 0)? h: h+1;
  w = (w%2 == 0)? w: w+1;

  unsigned char *m = calloc(h*w*3*sizeof(char), 1);

  //Perform OpenCL program


  struct OpenCL_Program *prog = get_opencl_info();

  FILE *fp;
  char *filename = "opencl/draw_julia.c";

  fp = fopen(filename, "r");
  if (!fp){
    fprintf(stderr, "Failed to load kernel.\n");
    exit(1);
  }

  prog->src = (char *) calloc(MAX_SOURCE_SIZE, 1);
  prog->src_size = fread(prog->src, 1, MAX_SOURCE_SIZE, fp);

  fclose(fp);

#ifdef DEBUG_DRAW_JULIA_C
  printf("Executing:\n------------------------\n%s\n------------------------\n", prog->src);
#endif

  prog->context = clCreateContext(NULL, 1, &(prog->device), NULL, NULL, &(prog->ret));
#ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL context created. Return code: %d\n", prog->ret);
#endif

  prog->command_queue = clCreateCommandQueue(prog->context, prog->device, 0, &(prog->ret));
#ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL CommandQueue created. Return code: %d\n", prog->ret);
#endif

  //Create all memory objects for Julia set Drawing
  //Memobjects for images and dmap
  cl_mem mem_m = clCreateBuffer(prog->context, CL_MEM_WRITE_ONLY,
                              w*h*3, NULL, &(prog->ret));
  cl_mem mem_N = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_h = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_w = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_c = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(double)*2, NULL, &(prog->ret));
  cl_mem mem_Sx = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(double)*2, NULL, &(prog->ret));
  cl_mem mem_Sy = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(double)*2, NULL, &(prog->ret));

  //Write data to mem objects
  clEnqueueWriteBuffer(prog->command_queue, mem_N, CL_TRUE, 0, sizeof(int), &N, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_h, CL_TRUE, 0, sizeof(int), &h, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_w, CL_TRUE, 0, sizeof(int), &w, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_c, CL_TRUE, 0, sizeof(double)*2, c, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_Sx, CL_TRUE, 0, sizeof(double)*2, Sx, 0, NULL, NULL);

  clEnqueueWriteBuffer(prog->command_queue, mem_Sy, CL_TRUE, 0, sizeof(double)*2, Sy, 0, NULL, NULL);

  prog->program = clCreateProgramWithSource(prog->context,
                                            1,
                                            (const char **)  &(prog->src),
                                            (const size_t *) &(prog->src_size),
                                            &(prog->ret));
#ifdef DEBUG_DRAW_JULIA_C
  printf("Building %s... ", filename);
#endif
  fflush(stdout);
  clBuildProgram(prog->program, 1, &(prog->device), NULL, NULL, NULL);

  prog->kernel = clCreateKernel(prog->program, plot_type, &(prog->ret));

#ifdef DEBUG_DRAW_JULIA_C
  printf("OpenCL Kernel created. Return code: %d\n\n", prog->ret);
#endif

  clSetKernelArg(prog->kernel, 0, sizeof(mem_m),  (void *)&mem_m);
  clSetKernelArg(prog->kernel, 1, sizeof(mem_N),  (void *)&mem_N);
  clSetKernelArg(prog->kernel, 2, sizeof(mem_h),  (void *)&mem_h);
  clSetKernelArg(prog->kernel, 3, sizeof(mem_w),  (void *)&mem_w);
  clSetKernelArg(prog->kernel, 4, sizeof(mem_c),  (void *)&mem_c);
  clSetKernelArg(prog->kernel, 5, sizeof(mem_Sx),  (void *)&mem_Sx);
  clSetKernelArg(prog->kernel, 6, sizeof(mem_Sy),  (void *)&mem_Sy);

  fflush(stdout);

  const size_t worksize[] = {h, w};

  cl_int status;

  status = clEnqueueNDRangeKernel(prog->command_queue,
                         prog->kernel, 2, NULL,
                         worksize,
                         NULL,
                         0, NULL, NULL);

  clEnqueueReadBuffer(prog->command_queue,
                      mem_m,
                      CL_TRUE,
                      0,
                      w*h*3,
                      m,
                      0,
                      NULL, NULL);

  clFlush(prog->command_queue);
  clFinish(prog->command_queue);
  clReleaseCommandQueue(prog->command_queue);
  clReleaseKernel(prog->kernel);
  clReleaseProgram(prog->program);
  clReleaseDevice(prog->device);
  clReleaseContext(prog->context);
  clReleaseMemObject(mem_m);
  clReleaseMemObject(mem_N);
  clReleaseMemObject(mem_h);
  clReleaseMemObject(mem_w);
  clReleaseMemObject(mem_c);
  clReleaseMemObject(mem_Sx);
  clReleaseMemObject(mem_Sy);

  free(prog->src);
  free(prog);

  return m;
}

unsigned char *draw_thumbnail(int N, int h, int w, double c[2], char *plot_type){
  double c_abs = pow(pow(c[0], 2) + pow(c[1], 2), 0.5);
  double SpanThumbnail = c_abs > 2? c_abs : 2;
  // double Sdx[2] = {-SpanThumbnail, SpanThumbnail}, Sdy[2] = {-SpanThumbnail, SpanThumbnail};

  double Sdx[2], Sdy[2];
  if (w >= h){
    double ratio = (double) w / (double) h;
    Sdy[0] = -SpanThumbnail;
    Sdy[1] = SpanThumbnail;
    Sdx[0] = -SpanThumbnail * ratio;
    Sdx[1] =  SpanThumbnail * ratio;
  } else {
    double ratio = (double) h / (double) w;
    Sdx[0] = -SpanThumbnail;
    Sdx[1] = SpanThumbnail;
    Sdy[0] = -SpanThumbnail * ratio;
    Sdy[1] =  SpanThumbnail * ratio;
  }

  unsigned char *Julia;

  Julia = draw_julia(N, h, w, c, Sdx, Sdy, plot_type);
  return Julia;
}

void draw_julia_zoom(int frames, int N, int h, int w, double c[2], double p[2], double zoom_ratio, char *plot_type){
  const char *out_folder = gen_dir_name(c, plot_type);

  int i = 0;

  double ratio;
  if (w >= h){
    ratio = (double) w / (double) h;
  } else {
    ratio = (double) h / (double) w;
  }

  double c_abs = pow(pow(c[0], 2) + pow(c[1], 2), 0.5);
  double SpanOriginal = 4;
  printf("%f\n", SpanOriginal);
  double Sx[2], Sy[2];
  double SpanX, SpanY;

  unsigned char *Julia;
  char *julia_out;
  char *thumb_f = gen_filename((char *) out_folder, "Thumbnail.png");

  printf("%s\n", thumb_f);

  time_t start, end;

  while (i < frames){
    if (w >= h){
      SpanY = SpanOriginal;
      SpanX = SpanY * ratio;
    } else {
      SpanX = SpanOriginal;
      SpanY = SpanX * ratio;
    }

    printf("%f, %f\n", SpanX, SpanY);

    Sx[0] = p[0] - SpanX/2;
    Sx[1] = p[0] + SpanX/2;
    Sy[0] = p[1] - SpanY/2;
    Sy[1] = p[1] + SpanY/2;

    julia_out = malloc(1024);
    snprintf(julia_out, 1024, "%s/%03d.png", out_folder, i);

    printf("%s\n", julia_out);


    printf("%f  %f\n", Sx[0], Sx[1]);
    printf("%f  %f\n", Sy[0], Sy[1]);

    printf("Drawing Julia...\n");
    start = clock();
    Julia = draw_julia(N, h, w, c, Sx, Sy, plot_type);
    printf("Saving Julia...\n");
    lodepng_encode24_file(julia_out, Julia, w, h);
    end = clock();
    printf("Done. Took %f seconds\n\n", ((double)(end - start))/CLOCKS_PER_SEC);

    free(julia_out);
    i++;
    SpanOriginal *= zoom_ratio;
  }
}

unsigned char *draw_julia_backwards_iteration(int N, int h, int w, double c[2], long MAX_D, _Bool revisit){

  double complex sqrt_complex(double complex c){
    double l = sqrt(pow(creal(c), 2) + pow(cimag(c), 2));
    double u = sqrt((l + creal(c))/2);
    double v = sqrt((l - creal(c))/2);
    if (cimag(c) < 0){
      v *= -1;
    }
    double complex r = u + v*I;
    return r;
  }

  double scopex[2], scopey[2];
  if (w >= h){
    double ratio = (double) w / (double) h;
    scopey[0] = -2;
    scopey[1] = 2;
    scopex[0] = -2 * ratio;
    scopex[1] =  2 * ratio;
  } else {
    double ratio = (double) h / (double) w;
    scopex[0] = -2;
    scopex[1] = 2;
    scopey[0] = -2 * ratio;
    scopey[1] =  2 * ratio;
  }

  unsigned char *m = malloc(w*h*sizeof(unsigned char));
  //turn image white
  // for (int i = 0; i < w*h; i++){m[i] = 255;}

  complex C = c[0] + c[1]*I;

  //queue->n = z0 in first iteration
  struct complexBI *z0 = malloc(sizeof(struct complexBI));
  struct complexBI *queue = malloc(sizeof(struct complexBI));
  struct complexBI *new_queue = malloc(sizeof(struct complexBI));

  struct complexBI *queue_ptr = NULL;
  struct complexBI *new_queue_ptr = NULL;
  struct complexBI *aux_free = NULL;

  int mallocs = 0, frees = 0, npoints;

  z0->p = 0.5 * (1.0 + sqrt_complex(1.0 - 4*C));
  z0->der = sqrt(pow(creal(z0->p), 2) + pow(cimag(z0->p), 2));
  z0->n = NULL;
  z0->disregard = false;

  queue->n = z0;

  //N iters
  for (int i = 0; i < N; i++){
    int new_points = 0;
    #ifdef DEBUG_DRAW_JULIA_C
    printf("Iter number %d\n", i);
    #endif
    int j = 0;

    //Generate antiimages
    queue_ptr = queue->n;
    // free(queue);
    new_queue_ptr = new_queue;

    //Cycle through queue
    while (queue_ptr != NULL) {

      if (queue_ptr -> disregard != true){

        struct complexBI *ai1 = malloc(sizeof(struct complexBI));
        ai1->p = sqrt_complex(queue_ptr->p - C);
        ai1->der = queue_ptr->der * 2 * sqrt(pow(creal(ai1->p), 2) + pow(cimag(ai1->p), 2));
        ai1->j = j;
        ai1->disregard = false;
        j++;
        ai1->n = NULL;

        struct complexBI *ai2 = malloc(sizeof(struct complexBI));
        ai2->p = -ai1->p;
        ai2->der = ai1->der;
        ai2->j = j;
        ai2->disregard = false;
        j++;
        ai2->n = NULL;
        mallocs+=2;

        new_points += 2;
        //Draw points in m
        int x1 = (creal(ai1->p)-scopex[0]) / (scopex[1]-scopex[0]) * (w);
        int x2 = (creal(ai2->p)-scopex[0]) / (scopex[1]-scopex[0]) * (w);
        int y1 = (cimag(ai1->p)-scopey[0]) / (scopey[1]-scopey[0]) * (0-h) + h;
        int y2 = (cimag(ai2->p)-scopey[0]) / (scopey[1]-scopey[0]) * (0-h) + h;
        if (!revisit && m[(y1*w + x1)] == 255){
          ai1->disregard = true;
        } else {
          m[(y1*w + x1)] = 255;
        }
        if (!revisit && m[(y2*w + x2)] == 255){
          ai2->disregard = true;
        } else {
          m[(y2*w + x2)] = 255;
        }

        new_queue_ptr->n = ai1;
        new_queue_ptr = ai1;
        new_queue_ptr->n = ai2;
        new_queue_ptr = ai2;
        if (MAX_D != 0 && ai1->der > MAX_D){
          ai1->disregard = true;
          new_points --;
          ai2->disregard = true;
          new_points --;
        }
      }

      aux_free = queue_ptr;
      queue_ptr = queue_ptr->n;
      free(aux_free);
      frees++;
    }
    #ifdef DEBUG_DRAW_JULIA_C
    printf("%d mallocs y %d frees with %d new points\n", mallocs, frees, new_points);
    #endif

    if (new_points == 0){
      break;
    }

    queue = new_queue;
  }

  return m;
}
