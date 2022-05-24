#include <opencl_funcs.h>

#define DEBUG_OPENCL_FUNCS_C

struct OpenCL_Program *get_opencl_info(){
  struct OpenCL_Program *prog = malloc(sizeof (struct OpenCL_Program));

  prog->ret = false;
  prog->src = NULL;

  #ifdef DEBUG_OPENCL_FUNCS_C
  printf("Getting OpenCL info...\n");
  #endif //DEBUG_OPENCL_FUNCS_C
  cl_int err_num;
  char str_buffer[1024];
  cl_uint num_platforms_available;

  cl_device_id cl_devices[1];
  err_num = clGetPlatformIDs(0, NULL, &num_platforms_available);

  if (err_num != CL_SUCCESS){
    perror("");
    exit(EXIT_FAILURE);
  }

  cl_platform_id cl_platforms[num_platforms_available];
  err_num = clGetPlatformIDs(num_platforms_available, cl_platforms, NULL);

  if (err_num != CL_SUCCESS){
    perror("");
    exit(EXIT_FAILURE);
  }

  printf("Found %d platforms\n", num_platforms_available);

  // for (int i = 0; i < num_platforms_available -1; i++){
  int i = 0;
    #ifdef DEBUG_OPENCL_FUNCS_C
    printf("Platform %d\n", i);
    #endif //DEBUG_OPENCL_FUNCS_C

    clGetPlatformInfo(cl_platforms[i], CL_PLATFORM_NAME, sizeof(str_buffer), &str_buffer, NULL);
    #ifdef DEBUG_OPENCL_FUNCS_C
    printf("Platform name: %s\n", str_buffer);


    clGetPlatformInfo(cl_platforms[i], CL_PLATFORM_VENDOR, sizeof(str_buffer), &str_buffer, NULL);
    printf("Platform vendor: %s\n", str_buffer);
    #endif //DEBUG_OPENCL_FUNCS_C

    cl_uint num_devices_available;
    clGetDeviceIDs(cl_platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices_available);
    #ifdef DEBUG_OPENCL_FUNCS_C
    printf("Num devices available: %d\n", num_devices_available);
    #endif //DEBUG_OPENCL_FUNCS_C

    clGetDeviceIDs(cl_platforms[i], CL_DEVICE_TYPE_ALL, num_devices_available, cl_devices, NULL);
    //Get info of device

    #ifdef DEBUG_OPENCL_FUNCS_C
    for (int j = 0; j < num_devices_available; j++){
      printf("Getting info for device %d\n", j);

      clGetDeviceInfo(cl_devices[j], CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
      printf("DEVICE NAME: %s\n", str_buffer);

      clGetDeviceInfo(cl_devices[j], CL_DEVICE_VERSION, sizeof(str_buffer), &str_buffer, NULL);
      printf("DEVICE VERSION: %s\n", str_buffer);

      clGetDeviceInfo(cl_devices[j], CL_DRIVER_VERSION, sizeof(str_buffer), &str_buffer, NULL);
      printf("DRIVER VERSION: %s\n", str_buffer);

      cl_uint max_units;
      clGetDeviceInfo(cl_devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_units), &max_units, NULL);
      printf("PARALLEL COMPUTE UNITS: %d\n", max_units);

      cl_uint max_dim;
      clGetDeviceInfo(cl_devices[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(max_dim), &max_dim, NULL);
      printf("MAX WORK ITEM DIMENSIONS: %d\n", max_dim);

      printf("\n");
    }
    #endif //DEBUG_OPENCL_FUNCS_C

    prog->device = cl_devices[0];

  // }

  return prog;
}

void opencl_free(struct OpenCL_Program *cl){
  clReleaseProgram(cl->program);
  clReleaseDevice(cl->device);
  clReleaseContext(cl->context);

  free(cl->src);
  free(cl);
  printf("Post cl free\n");
}
