#ifndef OPENCL_FUNCS_H
#define OPENCL_FUNCS_H

#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x001000)
#define CORES 16

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


struct OpenCL_Program{
  cl_uint ret;

  cl_device_id      device;
  cl_context        context;
  cl_command_queue  command_queue;
  cl_program        program;
  cl_kernel         kernel;

  char *src;
  size_t src_size;
};

struct OpenCL_Program *get_opencl_info();

#endif //OPENCL_FUNCS_H
