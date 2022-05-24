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

char *get_root_folder(const char *exec_path){
  char *p = realpath(exec_path, NULL);
  int n_slashes = 0;

  for (int i = strlen(p); i >= 0; i--){
    if (p[i] == '/'){
      n_slashes++;
    }
    if (n_slashes == 2){
      break;
    }
    p[i] = '\0';
  }
  return p;
}
