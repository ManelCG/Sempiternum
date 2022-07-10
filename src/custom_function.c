#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>

#include <file_io.h>

#include <custom_function.h>

int mkdir_p(const char *path){
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  char *p;

  errno = 0;

  if (len > sizeof(_path)-1){
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(_path, path);

  for (p = _path+1; *p; p++){
    if (*p == '/'){
      *p='\0';

      if (mkdir(_path, S_IRWXU) != 0){
        if (errno != EEXIST){
          return -1;
        }
      }

      *p = '/';
    }
  }

  if (mkdir(_path, S_IRWXU) != 0){
    if (errno != EEXIST){
      return -1;
    }
  }

  return 0;
}

char *custom_function_get_path(){
  char *path = malloc(1024);

  #ifndef MAKE_INSTALL
    sprintf(path, "./opencl/custom/src/");
  #else
    #ifdef __linux__
      sprintf(path, "%s/.local/sempiternum/custom_opencl/src/", getenv("HOME"));
    #endif
    #ifdef _WIN32
      sprintf(path, "%s/Desktop/sempiternum/custom_opencl/src/", getenv("HOMEPATH"));
    #endif
  #endif
  return path;
}

char *custom_function_get_headers_path(){
  char *path = malloc(1024);

  #ifndef MAKE_INSTALL
    sprintf(path, "./opencl/custom/headers/");
  #else
    #ifdef __linux__
      sprintf(path, "%s/.local/sempiternum/custom_opencl/headers/", getenv("HOME"));
    #endif
    #ifdef _WIN32
      sprintf(path, "%s/Desktop/sempiternum/custom_opencl/headers/", getenv("HOMEPATH"));
    #endif
  #endif
  return path;
}


char **custom_function_get_file_list(_Bool leave_extension){
  char *extensions[2] = {".cl", ".c"};
  char **file_vector = file_io_folder_get_file_list(custom_function_get_path(), extensions, 2, leave_extension);
  return file_vector;
}

int custom_function_get_n(){
  char *custom_function_path = custom_function_get_path();
  mkdir_p(custom_function_path);

  char *extensions[2];
  extensions[0] = ".c";
  extensions[1] = ".cl";

  int nfiles = file_io_folder_get_file_n(custom_function_path, extensions, 2);
  free(custom_function_path);
  return nfiles;
}
