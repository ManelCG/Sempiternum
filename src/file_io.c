#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string.h>


#include <file_io.h>

char *gen_dir_name(double c[2], char *plot_type){
  const char *rootd = "/home/hrad/Universidad/4o/TFG/sketch_code/beta1";
  char *out_folder = calloc(300, 1);
  snprintf(out_folder, 300, "%s/out/%s_%f+%fi_VIDEO_FRAMES\0", rootd, plot_type, c[0], c[1]);

  DIR *d = opendir(out_folder);
  if (d){
    closedir(d);
  } else {
    mkdir(out_folder, 0755);
  }

  return out_folder;
}

char *gen_filename(char *dirname, char *filename){
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
