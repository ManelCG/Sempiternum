#ifndef __GUI_TEMPLATES_H_
#define __GUI_TEMPLATES_H_

#include <gtk/gtk.h>
void gui_templates_show_help_window(GtkWidget *w, gpointer data);
void gui_templates_show_about_window(GtkWidget *w, gpointer data);

void clear_container(GtkWidget *window);
void destroy(GtkWidget *w, gpointer data);



#endif //__GUI_TEMPLATES_H_
