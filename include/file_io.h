char *gen_dir_name(double c[2], const char *plot_type);
char *gen_filename(const char *dirname, const char *filename);
char *get_root_folder(const char *exec_path);
int *parse_dimensions(char *dim);

int file_io_folder_get_file_n(char *folder, char **extensions, int extension_n);
char **file_io_folder_get_file_list(char *folder, char **ext, int extn, _Bool leave_ext);
