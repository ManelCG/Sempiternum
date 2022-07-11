#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string.h>


#include <file_io.h>

char *gen_dir_name(double c[2], const char *plot_type){
  const char *rootd = "/home/hrad/Universidad/4o/TFG/sketch_code/beta1";
  char *out_folder = calloc(300, 1);
  snprintf(out_folder, 300, "%s/out/%s_%f+%fi_VIDEO_FRAMES", rootd, plot_type, c[0], c[1]);

  DIR *d = opendir(out_folder);
  if (d){
    closedir(d);
  } else {
    #ifdef __unix__
    mkdir(out_folder, 0755);
    #elif defined(_WIN32) || defined (WIN32)
    mkdir(out_folder);
    #endif
  }

  return out_folder;
}

char *gen_filename(const char *dirname, const char *filename){
  char *f, c;
  int i = 0, j = 0;

  do {
    c = dirname[i];
    i++;
  } while (c != '\0');
  do {
    c = filename[j];
    j++;
  } while (c != '\0');

  f = malloc(i+j+1);
  strcpy(f, dirname);
  strcat(f, "/");
  strcat(f, filename);

  return f;
}

int *parse_dimensions(char *dimensions){
  int *dim = malloc(sizeof(int) * 2);
  char *s = "x";
  char *token;
  token = strtok(dimensions, s);
  for (int i = 0; i < 2; i++){
    dim[i] = atoi(token);
    token = strtok(NULL, s);
  }

  return dim;
}

char *get_root_folder(const char *exec_path){
  #ifdef __unix__
  char *p = realpath(exec_path, NULL);
  #elif defined(_WIN32) || defined (WIN32)
  char *p = _fullpath(NULL, exec_path, 1024);
  #endif
  int n_slashes = 0;

  #ifdef __unix__
  for (int i = strlen(p); i >= 0; i--){
    if (p[i] == '/'){
      n_slashes++;
    }
    if (n_slashes == 2){
      break;
    }
    p[i] = '\0';
  }
  #elif defined(_WIN32) || defined (WIN32)
  for (int i = strlen(p); i >= 0; i--){
    if (p[i] == '\\'){
      n_slashes++;
    }
    if (n_slashes == 2){
      break;
    }
    p[i] = '\0';
  }
  #endif
  return p;
}

char **file_io_folder_get_file_list(char *folder, char **extensions, int extension_n, _Bool leave_extension){
  int nfiles = file_io_folder_get_file_n(folder, extensions, extension_n);
  char **file_vector = malloc(sizeof(char *) * nfiles);

  DIR *d;
  struct dirent *dir;

  d = opendir(folder);
  int findex = 0;

  if (d){
    while ((dir = readdir(d)) != NULL){
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
        for (int i = 0; i < extension_n; i++){
          if (strlen(dir->d_name) > strlen(extensions[i]) && !strcmp(dir->d_name + strlen(dir->d_name) - strlen(extensions[i]), extensions[i])){
            file_vector[findex] = dir->d_name;
            if (!leave_extension){
              dir->d_name[strlen(dir->d_name) - strlen(extensions[i])] = '\0';
            }
            findex++;
          }
        }
      }
    }
    closedir(d);
  }

  return file_vector;
}

int file_io_folder_get_file_n(char *folder, char **extensions, int extension_n){
  int nfiles = 0;

  DIR *d;
  struct dirent *dir;
  d = opendir(folder);

  if (d){
    while ((dir = readdir(d)) != NULL){
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
        for (int i = 0; i < extension_n; i++){
          if (strlen(dir->d_name) > strlen(extensions[i]) && !strcmp(dir->d_name + strlen(dir->d_name) - strlen(extensions[i]), extensions[i])){
            nfiles++;
            break;
          }
        }
      }
    }
    closedir(d);
  }

  return nfiles;
}
