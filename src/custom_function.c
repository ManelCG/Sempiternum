#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>

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
    sprintf(path, "./opencl/custom/");
  #else
    #ifdef __linux__
      sprintf(path, "%s/.local/sempiternum/custom_opencl/", getenv("HOME"));
    #endif
    #ifdef _WIN32
      sprintf(path, "%s/Desktop/sempiternum/custom_opencl/", getenv("HOMEPATH"));
    #endif
  #endif
  return path;
}

char **custom_function_get_file_list(_Bool leave_extension){
  int nfiles = custom_function_get_n();
  char **file_vector = malloc(sizeof(char *) * nfiles);

  DIR *d;
  struct dirent *dir;
  char *custom_function_path = custom_function_get_path();
  d = opendir(custom_function_path);
  int i = 0;

  if (d){
    while ((dir = readdir(d)) != NULL){
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
        if (strlen(dir->d_name) > 2 && !strcmp(dir->d_name + strlen(dir->d_name) - 2, ".c")){
          file_vector[i] = dir->d_name;
          if (!leave_extension){
            dir->d_name[strlen(dir->d_name)-2] = '\0';
          }
          i++;
        } else if (strlen(dir->d_name) > 3 && !strcmp(dir->d_name + strlen(dir->d_name) - 3, ".cl")){
          file_vector[i] = dir->d_name;
          if (!leave_extension){
            dir->d_name[strlen(dir->d_name)-3] = '\0';
          }
          i++;
        }
      }
    }
    closedir(d);
  }

  return file_vector;
}

int custom_function_get_n(){
  char *custom_function_path = custom_function_get_path();
  mkdir_p(custom_function_path);

  int nfiles = 0;

  DIR *d;
  struct dirent *dir;
  d = opendir(custom_function_path);

  if (d){
    while ((dir = readdir(d)) != NULL){
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
        if ((strlen(dir->d_name) > 2 && !strcmp(dir->d_name + strlen(dir->d_name) - 2, ".c")) ||
            (strlen(dir->d_name) > 3 && !strcmp(dir->d_name + strlen(dir->d_name) - 3, ".cl"))){
          nfiles++;
        }
      }
    }
    closedir(d);
  }

  free(custom_function_path);
  return nfiles;
}
