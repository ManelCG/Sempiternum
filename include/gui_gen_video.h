#ifndef GUI_BLUEPRINTS_H
#define GUI_BLUEPRINTS_H

#include <gtk/gtk.h>

typedef struct{
  GtkWidget *menubar;

  //File menu
  GtkWidget *filemenu;
  GtkWidget *fileMi;
  GtkWidget *button_quit;
  GtkWidget *button_save_plot;
  GtkWidget *button_gen_video;
} mainMenu;

mainMenu *menu_main_new();
char *gui_gen_video_choose_folder(GtkWidget *w, gpointer data);
GtkWidget *gui_gen_video_generate_default_resolutions_combo_box(int *num);
void gui_gen_video_resolutions_combo_box_to_entries(GtkWidget *, gpointer entries);
void gui_gen_video_lock_span_ratio(GtkWidget *, gpointer);

#endif //GUI_BLUEPRINTS_H
