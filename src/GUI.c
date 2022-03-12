#include <gtk/gtk.h>
#include <stdbool.h>

#include <math.h>

#include <file_io.h>

#include <ComplexPlane.h>
#include <draw_julia.h>

// #define DEBUG_GUI

struct ComplexPlane *complex_plane = NULL;
struct ComplexPlane *thumbnail = NULL;

GtkWidget *input_N;
GtkWidget *input_N_line;
GtkWidget *input_z0;
GtkWidget *input_z1;
GtkWidget *input_spanx;
GtkWidget *input_spany;
GtkWidget *input_center0;
GtkWidget *input_center1;
GtkWidget *input_plot_type;

GtkWidget *input_thumbN;
GtkWidget *check_draw_thumb;
_Bool draw_thumbnails_bool = true;
GtkWidget *check_draw_lines;
_Bool draw_lines_bool = true;

GtkWidget *point_targeted;
GtkWidget *thumb_img;
GtkWidget *thumb_box;

void draw_main_window(GtkWidget *widget, gpointer data);
void gen_plot(GtkWidget *widget, _Bool gen_default);
void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data);
void draw_thumbnail_gui(GtkWidget *widget, double x, double y);

void change_thumbnail_state(GtkWidget *widget){
  draw_thumbnails_bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}
void change_lines_state(GtkWidget *widget){
  draw_lines_bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void apply_changes(GtkWidget *widget){
  int w, h;
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);
  int N = atoi(gtk_entry_get_text(GTK_ENTRY(input_N)));
  int N_line = atoi(gtk_entry_get_text(GTK_ENTRY(input_N_line)));

  double z[2] = {strtod(gtk_entry_get_text(GTK_ENTRY(input_z0)), NULL),
                 strtod(gtk_entry_get_text(GTK_ENTRY(input_z1)), NULL)};
  double center[2] = {strtod(gtk_entry_get_text(GTK_ENTRY(input_center0)), NULL),
                 strtod(gtk_entry_get_text(GTK_ENTRY(input_center1)), NULL)};
  complex_plane->plot_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(input_plot_type));

  double ratio, SpanYmin, SpanXmin;

  SpanXmin = strtod(gtk_entry_get_text(GTK_ENTRY(input_spanx)), NULL);
  SpanYmin = strtod(gtk_entry_get_text(GTK_ENTRY(input_spany)), NULL);

  complex_plane->w = w * 0.80, complex_plane->h = h;
  complex_plane->N = (N == 0)? 500: N;
  complex_plane->N_line = (N_line == 0)? 25: N_line;


  complex_plane->z[0] = z[0];
  complex_plane->z[1] = z[1];
  complex_plane->center[0] = center[0];
  complex_plane->center[1] = center[1];

  if (SpanXmin == 0 || SpanYmin == 0){
    if (w >= h){
      ratio = (double) complex_plane->w / (double) complex_plane->h;
      SpanYmin = 4;
      SpanXmin = SpanYmin*ratio;
    } else {
      ratio = (double) complex_plane->h / (double) complex_plane->w;
      SpanXmin = 4;
      SpanYmin = SpanXmin*ratio;
    }
  }
    complex_plane->Sx[0] = (-SpanXmin/2) + complex_plane->center[0];
    complex_plane->Sx[1] = (SpanXmin/2) + complex_plane->center[0];
    complex_plane->Sy[0] = (-SpanYmin/2) + complex_plane->center[1];
    complex_plane->Sy[1] = (SpanYmin/2) + complex_plane->center[1];
    complex_plane->SpanX = SpanXmin;
    complex_plane->SpanY = SpanYmin;

}

void gen_plot(GtkWidget *widget, _Bool gen_default){
  int w, h;
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);

  if (complex_plane == NULL){
    complex_plane = malloc(sizeof(struct ComplexPlane));
    complex_plane->plot = NULL;
    complex_plane->drawn_plot = NULL;
  }

  double SpanXmin;
  double SpanYmin;
  double ratio;
  if (gen_default){
    complex_plane->N = 500;
    complex_plane->N_line = 25;
    complex_plane->w = w * 0.80, complex_plane->h = h;
    complex_plane->plot_type = "parameter_space";
    complex_plane->z[0] = 0;
    complex_plane->z[1] = 0;
    // complex_plane->center[0] = -0.5;
    complex_plane->center[0] = 0;
    complex_plane->center[1] = 0;

    if (w >= h){
      ratio = (double) complex_plane->w / (double) complex_plane->h;
      SpanYmin = 3;
      SpanXmin = SpanYmin*ratio;
    } else {
      ratio = (double) complex_plane->h / (double) complex_plane->w;
      SpanXmin = 3;
      SpanYmin = SpanXmin*ratio;
    }

    complex_plane->Sx[0] = (-SpanXmin/2) + complex_plane->center[0];
    complex_plane->Sx[1] = (SpanXmin/2) + complex_plane->center[0];
    complex_plane->Sy[0] = (-SpanYmin/2) + complex_plane->center[1];
    complex_plane->Sy[1] = (SpanYmin/2) + complex_plane->center[1];
    complex_plane->SpanX = SpanXmin;
    complex_plane->SpanY = SpanYmin;

  } else {
    apply_changes(widget);
  }

  if (complex_plane->plot != NULL){
    free(complex_plane->plot);
    complex_plane->plot = NULL;
  }
  if (complex_plane->drawn_plot != NULL){
    free(complex_plane->drawn_plot);
    complex_plane->drawn_plot = NULL;
  }

  #ifdef DEBUG_GUI
  printf("Spans: %f %f | %f %f\n", complex_plane->Sx[0], complex_plane->Sx[1], complex_plane->Sy[0], complex_plane->Sy[1]);
  #endif

  complex_plane->plot = draw_julia(complex_plane->N,
                                complex_plane->h,
                                complex_plane->w,
                                complex_plane->z,
                                complex_plane->Sx,
                                complex_plane->Sy,
                                complex_plane->plot_type);
}

void draw_from_options(GtkWidget *widget){
  gen_plot(widget, false);
  draw_main_window(widget, NULL);
}

void draw_default_mandelbrot(GtkWidget *widget){
  gen_plot(widget, true);
  draw_main_window(widget, NULL);
}

void destroy(GtkWidget *w, gpointer data){
  gtk_main_quit();
}

void clear_container(GtkWidget *window){
  GList *children, *iter;
  children = gtk_container_get_children(GTK_CONTAINER(window));
  for (iter = children; iter != NULL; iter = g_list_next(iter)){
    gtk_widget_destroy(GTK_WIDGET(iter->data));
  }
  g_list_free(children);
}

void plot_zoom(GtkWidget *widget, double zoomratio, double p[2]){
  #ifdef DEBUG_GUI
  printf("Zooming in %f %f\n", p[0], p[1]);
  #endif
  char buffer[500];
  sprintf(buffer, "%f", p[0]);
  gtk_entry_set_text(GTK_ENTRY(input_center0), buffer);
  sprintf(buffer, "%f", p[1]);
  gtk_entry_set_text(GTK_ENTRY(input_center1), buffer);
  complex_plane->SpanX *= zoomratio;
  complex_plane->SpanY *= zoomratio;
  sprintf(buffer, "%f", complex_plane->SpanX);
  gtk_entry_set_text(GTK_ENTRY(input_spanx), buffer);
  sprintf(buffer, "%f", complex_plane->SpanY);
  gtk_entry_set_text(GTK_ENTRY(input_spany), buffer);
  draw_from_options(widget);
}

void draw_thumbnail_gui(GtkWidget *widget, double x, double y){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  int N_thumb = strtod(gtk_entry_get_text(GTK_ENTRY(input_thumbN)), NULL);
  if (N_thumb == 0){
    N_thumb = 50;
  }
  thumbnail->N = N_thumb;
  gtk_widget_destroy(thumb_img);
  thumbnail->z[0] = x; thumbnail->z[1] = y;
  if (strcmp(complex_plane->plot_type, "parameter_space") == 0){
    thumbnail->plot = draw_thumbnail(thumbnail->N, thumbnail->h, thumbnail->w, thumbnail->z, "rec_f");
  } else if (strcmp(complex_plane->plot_type, "rec_f") == 0){
    thumbnail->plot = draw_thumbnail(thumbnail->N, thumbnail->h, thumbnail->w, thumbnail->z, "parameter_space");
  }
  GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
      thumbnail->plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      thumbnail->w, thumbnail->h,
      thumbnail->w * 3,
      NULL, NULL);
  thumb_img = gtk_image_new_from_pixbuf(thumbBuf);

  gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);

  gtk_widget_show_all(GTK_WIDGET(window));
}

void cp_mouse_handler(GtkWidget *widget, GdkEventButton *event, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  #ifdef DEBUG_GUI
  printf("%d\n", event->button);
  #endif
  double x = (event->x-0)/(complex_plane->w-0) * (complex_plane->Sx[1] - complex_plane->Sx[0]) + complex_plane->Sx[0];
  double y = (-(event->y-complex_plane->h)-0)/(complex_plane->h-0) * (complex_plane->Sy[1] - complex_plane->Sy[0]) + complex_plane->Sy[0];
  double point_clicked[2] = {x, y};
  gchar *point = g_strdup_printf("%f %+fi", x, y);
  gtk_label_set_text(GTK_LABEL(point_targeted), point);


  if (draw_thumbnails_bool){
    if (thumbnail->plot != NULL){
      free(thumbnail->plot);
      thumbnail->plot = NULL;
    }
    draw_thumbnail_gui(widget, x, y);
  }

  switch (event->button){
    case 0:
      if (draw_lines_bool){
        draw_sequence(widget, event, data);
      }
      break;
    case 1:
      plot_zoom(widget, 0.5, point_clicked);
      draw_lines_bool = false;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_draw_lines), draw_lines_bool);
      break;
    case 2:
      char buffer[500];
      sprintf(buffer, "%f", x);
      gtk_entry_set_text(GTK_ENTRY(input_z0), buffer);
      sprintf(buffer, "%f", y);
      gtk_entry_set_text(GTK_ENTRY(input_z1), buffer);
      gtk_entry_set_text(GTK_ENTRY(input_spanx), "");
      gtk_entry_set_text(GTK_ENTRY(input_spany), "");
      gtk_entry_set_text(GTK_ENTRY(input_center0), "");
      gtk_entry_set_text(GTK_ENTRY(input_center1), "");

      if (strcmp(complex_plane->plot_type, "parameter_space") == 0){
        gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 1);
      } else if (strcmp(complex_plane->plot_type, "rec_f") == 0){
        gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 0);
      }

      draw_from_options(widget);
      break;
  }
}

void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data){
  double w = gtk_widget_get_allocated_width(window);
  double h = gtk_widget_get_allocated_height(window);

  double x = (event->x-0)/(complex_plane->w-0) * (complex_plane->Sx[1] - complex_plane->Sx[0]) + complex_plane->Sx[0];
  double y = (-(event->y-complex_plane->h)-0)/(complex_plane->h-0) * (complex_plane->Sy[1] - complex_plane->Sy[0]) + complex_plane->Sy[0];

  #ifdef DEBUG_GUI
  printf("Plot %f + %fI\n", x, y);
  printf("Motion notify\n");
  printf("%f, %f\n", event->x, event->y);
  printf("Pointing at: %f, %f\n", x, y);
  #endif

  double p[2] = {x, y};

  complex_plane->drawn_plot = malloc(complex_plane->h*complex_plane->w*3);
  for (int i = 0; i < complex_plane->h*complex_plane->w*3; i++){
    complex_plane->drawn_plot[i] = complex_plane->plot[i];
  }

  complex_plane->N_line = strtod(gtk_entry_get_text(GTK_ENTRY(input_N_line)), NULL);
  draw_sequence_lines(complex_plane, p, w, h);

  clear_container(window);

  GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
      complex_plane->drawn_plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      complex_plane->w, complex_plane->h,
      complex_plane->w * 3,
      NULL, NULL);
  GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);
  gtk_container_add(GTK_CONTAINER(window), image);
  gtk_widget_show_all(GTK_WIDGET(window));
  free(complex_plane->drawn_plot);
  complex_plane->drawn_plot = NULL;
}

void draw_main_window(GtkWidget *widget, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  clear_container(GTK_WIDGET(window));

  gint w, h;
  gtk_window_get_size(window, &w, &h);

  GtkWidget *iter_box;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *options_box;
  GtkWidget *z_box;
  GtkWidget *center_box;
  GtkWidget *draw_apply_box;
  GtkWidget *thumb_options_box;
  GtkWidget *span_box;

  GtkWidget *button_clear;
  GtkWidget *button_draw;
  GtkWidget *button_mandelbrot;
  GtkWidget *button_quit;
  GtkWidget *button_apply_changes;

  GtkWidget *plot_options_label;
  GtkWidget *point_label_text;
  GtkWidget *center_label_text;
  GtkWidget *span_label_text;

  input_N = gtk_entry_new();
  input_N_line = gtk_entry_new();
  input_z0 = gtk_entry_new();
  input_z1 = gtk_entry_new();
  input_center0 = gtk_entry_new();
  input_center1 = gtk_entry_new();
  input_plot_type = gtk_combo_box_text_new();
  input_spanx = gtk_entry_new();
  input_spany = gtk_entry_new();

  point_targeted = gtk_label_new("0 + 0i");

  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_plot_type), NULL, "parameter_space");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_plot_type), NULL, "rec_f");
  gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 0);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  z_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  thumb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  iter_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  draw_apply_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  thumb_options_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  span_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

  plot_options_label = gtk_label_new("Plot options");
  point_label_text = gtk_label_new("c or z parameter");
  center_label_text = gtk_label_new("Central point of plot");
  span_label_text = gtk_label_new("Spanning points of plot");

  //Generate complex_plane set
  if (complex_plane == NULL){
    complex_plane = malloc(sizeof(struct ComplexPlane));
    complex_plane->plot = NULL;
    complex_plane->drawn_plot = NULL;
  }

  //If plane has been already generated
  if (complex_plane->plot != NULL){
    GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
        complex_plane->plot,
        GDK_COLORSPACE_RGB,
        0,
        8,
        complex_plane->w, complex_plane->h,
        complex_plane->w * 3,
        NULL, NULL);

    GtkWidget *event_box = gtk_event_box_new();
    GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);

    gtk_container_add(GTK_CONTAINER(event_box), image);
    gtk_box_pack_start(GTK_BOX(hbox), event_box, false, true, 0);

    g_signal_connect(G_OBJECT(event_box), "motion-notify-event", G_CALLBACK(cp_mouse_handler), NULL);
    g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(cp_mouse_handler), NULL);
    gtk_widget_set_events(event_box, GDK_POINTER_MOTION_MASK);
    gtk_widget_set_size_request(event_box, complex_plane->w, complex_plane->h);

    button_clear = gtk_button_new_with_label("Clear");
    gtk_widget_set_tooltip_text(button_clear, "Redraw Complex Plane without drawings");
    g_signal_connect(button_clear, "clicked", G_CALLBACK(draw_main_window), NULL);

    gtk_box_pack_end(GTK_BOX(options_box), button_clear, true, false, 0);

    char buffer[500];
    sprintf(buffer, "%d", complex_plane->N_line);
    gtk_entry_set_text(GTK_ENTRY(input_N_line), buffer);
    sprintf(buffer, "%d", complex_plane->N);
    gtk_entry_set_text(GTK_ENTRY(input_N), buffer);
    sprintf(buffer, "%f", complex_plane->z[0]);
    gtk_entry_set_text(GTK_ENTRY(input_z0), buffer);
    sprintf(buffer, "%f", complex_plane->z[1]);
    gtk_entry_set_text(GTK_ENTRY(input_z1), buffer);
    sprintf(buffer, "%f", complex_plane->center[0]);
    gtk_entry_set_text(GTK_ENTRY(input_center0), buffer);
    sprintf(buffer, "%f", complex_plane->center[1]);
    gtk_entry_set_text(GTK_ENTRY(input_center1), buffer);
    if (strcmp(complex_plane->plot_type, "parameter_space") == 0){
      gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 0);
    } else if (strcmp(complex_plane->plot_type, "rec_f") == 0){
      gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 1);
    }
    sprintf(buffer, "%f", complex_plane->SpanX);
    gtk_entry_set_text(GTK_ENTRY(input_spanx), buffer);
    sprintf(buffer, "%f", complex_plane->SpanY);
    gtk_entry_set_text(GTK_ENTRY(input_spany), buffer);
  }

  button_mandelbrot = gtk_button_new_with_label("Default Mandelbrot");
  gtk_widget_set_tooltip_text(button_mandelbrot, "Generate and plot Mandelbrot Set");
  g_signal_connect(button_mandelbrot, "clicked", G_CALLBACK(draw_default_mandelbrot), NULL);

  button_draw = gtk_button_new_with_label("Draw");
  button_apply_changes = gtk_button_new_with_label("Apply changes");


  button_quit = gtk_button_new_with_label("Quit");
  g_signal_connect(button_quit, "clicked", G_CALLBACK(destroy), NULL);

  gtk_box_pack_start(GTK_BOX(vbox), options_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(vbox), button_mandelbrot, true, false, 0);
  gtk_box_pack_end(GTK_BOX(vbox), button_quit, true, false, 0);

  gtk_box_pack_start(GTK_BOX(thumb_box), point_targeted, true, false, 0);
  if (thumbnail->plot != NULL){
    GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
        thumbnail->plot,
        GDK_COLORSPACE_RGB,
        0,
        8,
        thumbnail->w, thumbnail->h,
        thumbnail->w * 3,
        NULL, NULL);
    thumb_img = gtk_image_new_from_pixbuf(thumbBuf);
    gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);

    input_thumbN = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_thumbN), "Iter N");
    gtk_widget_set_tooltip_text(input_thumbN, "Number of iterations to plot for thumbnails");
    check_draw_thumb = gtk_check_button_new_with_mnemonic("Draw _thumbnails");
    g_signal_connect(check_draw_thumb, "toggled", G_CALLBACK(change_thumbnail_state), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_draw_thumb), draw_thumbnails_bool);

    gchar *buffer = g_strdup_printf("%d", thumbnail->N);
    gtk_entry_set_text(GTK_ENTRY(input_thumbN), buffer);

    check_draw_lines = gtk_check_button_new_with_mnemonic("Draw _lines");
    g_signal_connect(check_draw_lines, "toggled", G_CALLBACK(change_lines_state), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_draw_lines), draw_lines_bool);

    gtk_box_pack_end(GTK_BOX(thumb_box), thumb_options_box, true, false, 0);
    gtk_box_pack_start(GTK_BOX(thumb_options_box), input_thumbN, true, false, 0);
    gtk_box_pack_start(GTK_BOX(thumb_options_box), check_draw_thumb, true, false, 0);

  }

  gtk_box_pack_start(GTK_BOX(vbox), thumb_box, true, false, 0);

  gtk_entry_set_placeholder_text(GTK_ENTRY(input_N), "Iter N");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_N_line), "Number of lines");

  gtk_box_pack_start(GTK_BOX(options_box), plot_options_label, true, false, 5);


  gtk_box_pack_start(GTK_BOX(options_box), iter_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(iter_box), input_N, true, false, 0);
  gtk_box_pack_start(GTK_BOX(iter_box), input_N_line, true, false, 0);
  gtk_widget_set_tooltip_text(input_N, "Number of iterations to plot");
  gtk_widget_set_tooltip_text(input_N_line, "Number of iterations to plot");

  // gtk_box_pack_start(GTK_BOX(options_box), point_label_text, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), z_box, true, false, 0);
  // gtk_box_pack_start(GTK_BOX(options_box), center_label_text, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), center_box, true, false, 0);

  // gtk_box_pack_start(GTK_BOX(options_box), span_label_text, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), span_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(span_box), input_spanx, true, false, 0);
  gtk_box_pack_start(GTK_BOX(span_box), input_spany, true, false, 0);
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_spanx), "Span X");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_spany), "Span Y");


  gtk_box_pack_start(GTK_BOX(options_box), input_plot_type, true, false, 0);
  gtk_widget_set_tooltip_text(input_plot_type, "parameter_space or rec_f");

  gtk_box_pack_start(GTK_BOX(z_box), input_z0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(z_box), input_z1, true, false, 0);
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_z0), "z0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_z1), "z1");
  gtk_widget_set_tooltip_text(input_z0, "Real part of z");
  gtk_widget_set_tooltip_text(input_z1, "Imag part of z");

  gtk_box_pack_start(GTK_BOX(center_box), input_center0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(center_box), input_center1, true, false, 0);
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_center0), "Center0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_center1), "Center1");
  gtk_widget_set_tooltip_text(input_center0, "Real part of center");
  gtk_widget_set_tooltip_text(input_center1, "Imag part of center");

  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_draw, true, true, 0);
  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_apply_changes, true, true, 0);
  gtk_box_pack_start(GTK_BOX(options_box), draw_apply_box, true, false, 0);
  gtk_widget_set_tooltip_text(button_draw, "Draw plane from options specified");
  g_signal_connect(button_draw, "clicked", G_CALLBACK(draw_from_options), NULL);
  g_signal_connect(button_apply_changes, "clicked", G_CALLBACK(apply_changes), NULL);
  gtk_widget_set_tooltip_text(button_apply_changes, "Apply changes to backend without regenerating plot");

  gtk_box_pack_start(GTK_BOX(options_box), check_draw_lines, true, false, 0);

  gtk_box_pack_end(GTK_BOX(hbox), vbox, true, false, 20);

  gtk_container_add(GTK_CONTAINER(window), hbox);

  gtk_widget_show_all(GTK_WIDGET(window));
}

int main (int argc, char *argv[]) {
  //cd to root folder
  chdir(get_root_folder(argv[0]));

  gtk_init (&argc, &argv);

  thumbnail = malloc(sizeof(struct ComplexPlane));
  thumbnail->w = 320;
  thumbnail->h = 180;
  thumbnail->plot = malloc(thumbnail->w * thumbnail->h * 3);
  thumbnail->N = 25;
  for (int i = 0; i < thumbnail->w * thumbnail->h * 3; i++){
    thumbnail->plot[i] = 0;
  }

  GtkWidget *window;

  int w = 950;
  int h = 520;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Julia Set Viewer");
  gtk_window_set_default_size(GTK_WINDOW(window), w, h);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_signal_connect(window, "destroy",
                   G_CALLBACK(destroy),
                   NULL);


  gtk_widget_show_all(window);
  draw_main_window(window, NULL);
  // gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_default_icon_from_file("assets/app_icon/256.png", NULL);
  gtk_main();

  free(complex_plane);
  return 0;
}
