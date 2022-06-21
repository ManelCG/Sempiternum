#include <math.h>
#include <stdbool.h>
#include <image_manipulation.h>
#include <stdlib.h>

#include <opencl_funcs.h>

// #define DEBUG_IMAGE_MANIPULATION_C

void cartesian_to_polar(double a, double b, double *r, double *t){
  const double PI = 3.141592;
  double theta;
  theta = atan(b/a);
  if (a < 0){
    theta += PI;
  } else if (b < 0){
    theta += 2*PI;
  }

  *r = sqrt(pow(a, 2) + pow(b, 2));
  *t = theta;
}

void color_radial(double x, double y, unsigned char *red, unsigned char *green, unsigned char *blue){
  const double PI = 3.141592;
  double r, theta;

  cartesian_to_polar(x, y, &r, &theta);
  theta = fmod(theta, 2*PI);

  double hp = theta / (PI/3);
  double C = 1;

  double X = C * (1 - fabs(fmod(hp, 2) - 1));
  X = floor(X*255);
  C = floor(X*255);

  int Xi = (int) X;
  int Ci = (int) C;

  if (0 <= hp && hp < 1){
    *red = Ci;
    *green = Xi;
    *blue = 0;
  } else if (1 <= hp && hp < 2){
    *red = Xi;
    *green = Ci;
    *blue = 0;
  } else if (2 <= hp && hp < 3){
    *red = 0;
    *green = Ci;
    *blue = Xi;
  } else if (3 <= hp && hp < 4){
    *red = 0;
    *green = Xi;
    *blue = Ci;
  } else if (4 <= hp && hp < 5){
    *red = Xi;
    *green = 0;
    *blue = Ci;
  } else if (5 <= hp && hp < 6){
    *red = Ci;
    *green = 0;
    *blue = Xi;
  }
}

void draw_line(unsigned char *m, int x0, int y0, int x1, int y1, int w, int h){
  void plot_px(unsigned char *m, int x, int y, double c, int w, int h){
    int br = (int) floor(c*255);
    if (x >= 0 && x < w && y >= 0 && y < h){
      m[(y*w + x)*3 + 0] = fmin(br + m[(y*w + x)*3 + 0], 255);
      m[(y*w + x)*3 + 1] = fmin(br + m[(y*w + x)*3 + 1], 255);
      m[(y*w + x)*3 + 2] = fmin(br + m[(y*w + x)*3 + 2], 255);
    }
  }
  int ipart(double x){return floor(x);}
  double round(double x){return ((double) ipart(x) + 0.5);}
  double fpart(double x){return (x - floor(x));}
  double rfpart(double x){return (1.0 - fpart(x));}

  _Bool draw_dots = false, draw_lines = true;

  //Draw circles
  if (draw_dots){
    for (int i = x0-1; i <= x0+1; i++){
      if (i >= 0 && i < w){
        for (int j = y0-1; j <= y0+1; j++){
          if (j >= 0 && j < h){
            m[(j*w +i)*3 +0] = 255;
            m[(j*w +i)*3 +1] = 255;
            m[(j*w +i)*3 +2] = 255;
          }
        }
      }
    }
    for (int i = x1-1; i <= x1+1; i++){
      if (i >= 0 && i < w){
        for (int j = y1-1; j <= y1+1; j++){
          if (j >= 0 && j < h){
            m[(j*w +i)*3 +0] = 255;
            m[(j*w +i)*3 +1] = 255;
            m[(j*w +i)*3 +2] = 255;
          }
        }
      }
    }
  }

  if (!draw_lines){
    return;
  }

  _Bool steep = (abs(y1-y0) > abs(x1-x0));
  int aux;
  double gradient, dx, dy;
  if (steep){
    aux = x0; x0 = y0; y0 = aux;
    aux = x1; x1 = y1; y1 = aux;
  }
  if (x0 > x1){
    aux = x0; x0 = x1; x1 = aux;
    aux = y0; y0 = y1; y1 = aux;
  }
  dx = x1 - x0; dy = y1 - y0;
  if (dx == 0){
    gradient = 1;
  } else {
    gradient = dy/dx;
  }

  //First endpoint
  double xend, yend, xgap, xpxl1, ypxl1;
  xend = round((double) x0);
  yend = (double) y0 + gradient * (xend - x0);
  xgap = rfpart((double)x0 + 0.5);
  xpxl1 = xend;
  ypxl1 = ipart(yend);

  if (steep){
    plot_px(m, (int)floor(ypxl1),   (int)floor(xpxl1), rfpart(yend)*xgap, w, h);
    plot_px(m, (int)floor(ypxl1)+1, (int)floor(xpxl1),  fpart(yend)*xgap, w, h);
  } else {
    plot_px(m, (int)floor(xpxl1), (int)floor(ypxl1),   rfpart(yend)*xgap, w, h);
    plot_px(m, (int)floor(xpxl1), (int)floor(ypxl1)+1,  fpart(yend)*xgap, w, h);
  }

  double intery = yend + gradient;
  double xpxl2, ypxl2;
  xend = round((double) x1);
  yend = (double) y1 + gradient * (xend - x1);
  xgap = fpart((double)x1 + 0.5);
  xpxl2 = xend;
  ypxl2 = ipart(yend);

  if (steep){
    plot_px(m, (int)floor(ypxl2),   (int)floor(xpxl2), rfpart(yend)*xgap, w, h);
    plot_px(m, (int)floor(ypxl2)+1, (int)floor(xpxl2),  fpart(yend)*xgap, w, h);
  } else {
    plot_px(m, (int)floor(xpxl2), (int)floor(ypxl2),   rfpart(yend)*xgap, w, h);
    plot_px(m, (int)floor(xpxl2), (int)floor(ypxl2)+1,  fpart(yend)*xgap, w, h);
  }

  if (steep){
    for (int i = (int)floor(xpxl1)+1; i < (int)floor(xpxl2)-1; i++){
      plot_px(m, ipart(intery), i, rfpart(intery), w, h);
      plot_px(m, ipart(intery)+1, i, fpart(intery), w, h);
      intery += gradient;
    }
  } else {
    for (int i = (int)floor(xpxl1)+1; i < (int)floor(xpxl2)-1; i++){
      plot_px(m, i, ipart(intery), rfpart(intery), w, h);
      plot_px(m, i, ipart(intery)+1, fpart(intery), w, h);
      intery += gradient;
    }
  }
}

_Bool LiangBarsky(int edgeL, int edgeR, int edgeB, int edgeT,
                  int x0, int y0, int x1, int y1,
                  int *x0r, int *y0r, int *x1r, int *y1r){
  double t0 = 0, t1 = 1;
  double xdelta = x1 - x0;
  double ydelta = y1 - y0;
  double p, q, r;

  for (int edge = 0; edge < 4; edge ++){
    switch (edge){
      case 0: p = -xdelta; q = -(edgeL - x0); break;
      case 1: p =  xdelta; q =  (edgeR - x0); break;
      case 2: p = -ydelta; q = -(edgeB - y0); break;
      case 3: p =  ydelta; q =  (edgeT - y0); break;
    }

    printf("p %f, q %f\n", p, q);
    if (p == 0 && q < 0) { return false; }
    r = q/p;
    if (p < 0){
      if (r > t1) { continue; }
      else if (r > t0) { t0 = r; }
    } else if (p > 0){
      if (r < t0) { continue; }
      else if (r < t1) { t1 = r; }
    }
  }

  *x0r = x0 + t0*xdelta;
  *y0r = y0 + t0*ydelta;
  *x1r = x0 + t1*xdelta;
  *y1r = y0 + t1*ydelta;
  return true;
}

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

  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("Executing:\n------------------------\n%s\n------------------------\n", prog->src);
  #endif

  prog->context = clCreateContext(NULL, 1, &(prog->device), NULL, NULL, &(prog->ret));

  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("OpenCL context created. Return code: %d\n", prog->ret);
  #endif

  prog->command_queue = clCreateCommandQueue(prog->context, prog->device, 0, &(prog->ret));
  #ifdef DEBUG_IMAGE_MANIPULATION_C
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
  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("Building %s... ", filename);
  #endif
  fflush(stdout);
  clBuildProgram(prog->program, 1, &(prog->device), NULL, NULL, NULL);

  prog->kernel = clCreateKernel(prog->program, "merge", &(prog->ret));

  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("OpenCL Kernel created. Return code: %d\n\n", prog->ret);
  #endif

  clSetKernelArg(prog->kernel, 0, sizeof(mem_ret),  (void *)&mem_ret);
  clSetKernelArg(prog->kernel, 1, sizeof(mem_empty),  (void *)&mem_empty);
  clSetKernelArg(prog->kernel, 2, sizeof(mem_full),  (void *)&mem_full);
  clSetKernelArg(prog->kernel, 3, sizeof(mem_h),  (void *)&mem_h);
  clSetKernelArg(prog->kernel, 4, sizeof(mem_w),  (void *)&mem_w);

  fflush(stdout);

  const size_t worksize[] = {h, w};

  clEnqueueNDRangeKernel(prog->command_queue,
                         prog->kernel, 2, NULL,
                         worksize,
                         NULL,
                         0, NULL, NULL);

  clFlush(prog->command_queue);
  clFinish(prog->command_queue);

  clEnqueueReadBuffer(prog->command_queue,
                      mem_ret,
                      CL_TRUE,
                      0,
                      w*h*3,
                      r,
                      0,
                      NULL, NULL);


  clReleaseCommandQueue(prog->command_queue);
  clReleaseKernel(prog->kernel);
  clReleaseProgram(prog->program);
  clReleaseDevice(prog->device);
  clReleaseContext(prog->context);
  clReleaseMemObject(mem_ret);
  clReleaseMemObject(mem_full);
  clReleaseMemObject(mem_empty);
  clReleaseMemObject(mem_w);
  clReleaseMemObject(mem_h);

  free(prog->src);
  free(prog);

  return r;
}

unsigned char *image_manipulation_clone_image(unsigned char *source, int h, int w,
                                              struct OpenCL_Program **cl_prog, _Bool init_new_cl){
  unsigned char *r = malloc(w*h*3);

  struct OpenCL_Program *prog = cl_prog == NULL? NULL: *cl_prog;
  if (init_new_cl){
    if (cl_prog == NULL){
      prog = get_opencl_info();
    } else {
      *cl_prog = get_opencl_info();
      prog = *cl_prog;
    }


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

    #ifdef DEBUG_IMAGE_MANIPULATION_C
    printf("Executing:\n------------------------\n%s\n------------------------\n", prog->src);
    #endif

    prog->context = clCreateContext(NULL, 1, &(prog->device), NULL, NULL, &(prog->ret));

    #ifdef DEBUG_IMAGE_MANIPULATION_C
    printf("OpenCL context created. Return code: %d\n", prog->ret);
    #endif
  }

  prog->command_queue = clCreateCommandQueue(prog->context, prog->device, 0, &(prog->ret));
  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("OpenCL CommandQueue created. Return code: %d\n", prog->ret);
  #endif


  cl_mem mem_source = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              w*h*3, NULL, &(prog->ret));
  cl_mem mem_h = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_w = clCreateBuffer(prog->context, CL_MEM_READ_ONLY,
                              sizeof(int), NULL, &(prog->ret));
  cl_mem mem_ret = clCreateBuffer(prog->context, CL_MEM_WRITE_ONLY,
                              w*h*3, NULL, &(prog->ret));

  clEnqueueWriteBuffer(prog->command_queue, mem_source, CL_TRUE, 0, w*h*3*sizeof(unsigned char), source , 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_h, CL_TRUE, 0, sizeof(int), &h, 0, NULL, NULL);
  clEnqueueWriteBuffer(prog->command_queue, mem_w, CL_TRUE, 0, sizeof(int), &w, 0, NULL, NULL);

  if (init_new_cl){
    prog->program = clCreateProgramWithSource(prog->context,
                                              1,
                                              (const char **)  &(prog->src),
                                              (const size_t *) &(prog->src_size),
                                              &(prog->ret));
    #ifdef DEBUG_IMAGE_MANIPULATION_C
    // printf("Building %s... ", filename);
    #endif
    fflush(stdout);
    clBuildProgram(prog->program, 1, &(prog->device), NULL, NULL, NULL);
  }

  prog->kernel = clCreateKernel(prog->program, "clone", &(prog->ret));

  #ifdef DEBUG_IMAGE_MANIPULATION_C
  printf("OpenCL Kernel created. Return code: %d\n\n", prog->ret);
  #endif

  clSetKernelArg(prog->kernel, 0, sizeof(mem_ret),  (void *)&mem_ret);
  clSetKernelArg(prog->kernel, 1, sizeof(mem_source),  (void *)&mem_source);
  clSetKernelArg(prog->kernel, 2, sizeof(mem_h),  (void *)&mem_h);
  clSetKernelArg(prog->kernel, 3, sizeof(mem_w),  (void *)&mem_w);

  fflush(stdout);

  const size_t worksize[] = {h, w};

  clEnqueueNDRangeKernel(prog->command_queue,
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


  clFlush(prog->command_queue);
  clFinish(prog->command_queue);
  clReleaseCommandQueue(prog->command_queue);
  clReleaseKernel(prog->kernel);
  prog->init = true;
  clReleaseMemObject(mem_ret);
  clReleaseMemObject(mem_source);
  clReleaseMemObject(mem_w);
  clReleaseMemObject(mem_h);

  if (cl_prog == NULL){
    clReleaseProgram(prog->program);
    clReleaseDevice(prog->device);
    clReleaseContext(prog->context);

    free(prog->src);
    free(prog);
  }

  return r;
}

