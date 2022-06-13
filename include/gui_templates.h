#ifndef __GUI_TEMPLATES_H_
#define __GUI_TEMPLATES_H_

#include <gtk/gtk.h>
void gui_templates_show_help_window(GtkWidget *w, gpointer data);
void gui_templates_show_about_window(GtkWidget *w, gpointer data);

void gui_templates_configure_roots(GtkWidget *w, gpointer data);

void gui_templates_show_preferences_window(GtkWidget *w, gpointer data);

void clear_container(GtkWidget *window);
void destroy(GtkWidget *w, gpointer data);

void insert_text_event_int(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data);
void insert_text_event_float(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data);


#endif //__GUI_TEMPLATES_H_
