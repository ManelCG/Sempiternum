#include <gtk/gtk.h>
#include <stdbool.h>
#include <ctype.h>

#include <math.h>

#include <file_io.h>

#include <ComplexPlane.h>
#include <image_manipulation.h>
#include <draw_julia.h>

// #define DEBUG_GUI

struct mainWindowData{
  GtkWidget *window;

  GtkWidget *input_N;
  GtkWidget *input_N_line;
  GtkWidget *input_z0;
  GtkWidget *input_z1;
  GtkWidget *input_spanx;
  GtkWidget *input_spany;
  GtkWidget *input_center0;
  GtkWidget *input_center1;
  GtkWidget *input_plot_type;

  GtkWidget *combo_function_type;
  GtkWidget *input_thumbN;
  GtkWidget *check_draw_thumb;

  GtkWidget *check_draw_lines;
  GtkWidget **input_polynomial_settings_vector_real;
  GtkWidget **input_polynomial_settings_vector_imag;
  GtkWidget *combo_polynomial_parameter;
  GtkWidget *combo_polynomial_parameter_thumb;

  double *zoom_point1[2];
  double *zoom_point2[2];

  struct ComplexPlane *complex_plane;
  struct ComplexPlane *thumbnail;
  struct OpenCL_Program *opencl;
};


//Chose between quadratic, polynomial, etc
//TODO: Remove these global variables
int function_type;     //0 = quadratic, 1 = polynomial
_Bool draw_thumbnails_bool = true;
_Bool draw_lines_bool = true;
complex double *polynomial = NULL;
int polynomial_order = -1;

GtkWidget *point_targeted;
GtkWidget *thumb_img;
GtkWidget *thumb_box;

//Function declarations. TODO: Move to header file
void draw_main_window(GtkWidget *widget, gpointer data);
void gen_plot(GtkWidget *widget, _Bool gen_default, gpointer data);
void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data);
void draw_thumbnail_gui(GtkWidget *widget, double x, double y, gpointer data);
void apply_changes(GtkWidget *widget, gpointer data);
void draw_from_options(GtkWidget *widget, gpointer data);
void save_polynomial(GtkWidget *widget, GdkEventKey *event, gpointer data);
void draw_box(GtkWidget *window, GdkEventButton *event, gpointer data);

void change_thumbnail_state(GtkWidget *widget){
  draw_thumbnails_bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}
void change_lines_state(GtkWidget *widget){
  draw_lines_bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}
void combo_function_type_handler(GtkWidget *widget, gpointer data){
  struct mainWindowData *widgets = (struct mainWindowData *) data;
  function_type = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->combo_function_type));
  if (function_type == 0){
    polynomial_order = -1;
    polynomial = NULL;
  }
  // apply_changes(widget, data);
  ////TODO: Fix being able to draw main widnow from here
  //draw_main_window(widget, data);
}

void change_polynomial_parameter(GtkWidget *widget, gpointer data){
  struct ComplexPlane *p = (struct ComplexPlane *) data;
  p->polynomial_parameter = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

void insert_text_event_int(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  int i;
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i])){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return ;
    }
  }
}

void insert_text_event_float(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  int i;
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i]) && !text[i] == '.'){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return ;
    }
  }
}

void save_polynomial(GtkWidget *widget, GdkEventKey *event, gpointer data){
  struct mainWindowData *widgets = (struct mainWindowData *) data;
  for (int i = 0; i < polynomial_order + 2; i++){
    polynomial[i] = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_real[i])), NULL) +
                    strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_imag[i])), NULL) * I;
  }
  #ifdef DEBUG_GUI
  for (int i = 0; i < polynomial_order + 2; i++){
    printf("%d: %f + %f I\n", i, strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_real[i])), NULL),
                                 strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_imag[i])), NULL));
  }
  printf("\n");
  #endif

  if (event != NULL && strcmp(gdk_keyval_name (event->keyval), "Return") == 0){
    draw_from_options(widget, data);
  }
}

void change_polynomial_order(GtkWidget *widget, GdkEventKey *event, gpointer data){
  struct ComplexPlane *p = ((struct mainWindowData *) data)->complex_plane;
  struct ComplexPlane *t = ((struct mainWindowData *) data)->thumbnail;
  if (strcmp(gdk_keyval_name (event->keyval), "Return") == 0){
    int order = atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if (polynomial != NULL){free(polynomial);}
    polynomial = malloc(sizeof(complex double) * (order + 2));
    polynomial_order = order;
    p->polynomial_parameter = -1;
    t->polynomial_parameter = -1;
    for (int i = 0; i <= order + 1; i++){
      polynomial[i] = 0;
    }
    draw_main_window(widget, data);
  }
}

void apply_changes(GtkWidget *widget, gpointer data){
  int w, h;
  struct mainWindowData *widgets = (struct mainWindowData*) data;
  GtkWindow *window  = GTK_WINDOW(widgets->window);

  // GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);
  int N = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->input_N)));
  int N_line = atoi(gtk_entry_get_text(GTK_ENTRY(widgets->input_N_line)));

  double z[2] = {strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_z0)), NULL),
                 strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_z1)), NULL)};
  double center[2] = {strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_center0)), NULL),
                 strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_center1)), NULL)};
  widgets->complex_plane->plot_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets->input_plot_type));

  double ratio, SpanYmin, SpanXmin;

  SpanXmin = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_spanx)), NULL);
  SpanYmin = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_spany)), NULL);

  widgets->complex_plane->w = fmax(w - 385, 0);
  widgets->complex_plane->h = h;
  widgets->complex_plane->N = (N == 0)? 500: N;
  widgets->complex_plane->N_line = (N_line == 0)? 25: N_line;


  widgets->complex_plane->z[0] = z[0];
  widgets->complex_plane->z[1] = z[1];
  widgets->complex_plane->center[0] = center[0];
  widgets->complex_plane->center[1] = center[1];

  if (SpanXmin == 0 || SpanYmin == 0){
    if (w >= h){
      ratio = (double) widgets->complex_plane->w / (double) widgets->complex_plane->h;
      SpanYmin = 4;
      SpanXmin = SpanYmin*ratio;
    } else {
      ratio = (double) widgets->complex_plane->h / (double) widgets->complex_plane->w;
      SpanXmin = 4;
      SpanYmin = SpanXmin*ratio;
    }
  }
  widgets->complex_plane->Sx[0] = (-SpanXmin/2) + widgets->complex_plane->center[0];
  widgets->complex_plane->Sx[1] = (SpanXmin/2) + widgets->complex_plane->center[0];
  widgets->complex_plane->Sy[0] = (-SpanYmin/2) + widgets->complex_plane->center[1];
  widgets->complex_plane->Sy[1] = (SpanYmin/2) + widgets->complex_plane->center[1];
  widgets->complex_plane->SpanX = SpanXmin;
  widgets->complex_plane->SpanY = SpanYmin;
}

void gen_plot(GtkWidget *widget, _Bool gen_default, gpointer data){
  int w, h;
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);

  struct mainWindowData *widgets = (struct mainWindowData *) data;

  if (widgets->complex_plane == NULL){
    widgets->complex_plane = malloc(sizeof(struct ComplexPlane));
    widgets->complex_plane->plot = NULL;
    widgets->complex_plane->drawn_plot = NULL;
  }

  double SpanXmin;
  double SpanYmin;
  double ratio;
  if (gen_default){
    widgets->complex_plane->N = 500;
    widgets->complex_plane->N_line = 25;
    widgets->complex_plane->w = fmax(w - 385, 0), widgets->complex_plane->h = h;
    widgets->complex_plane->plot_type = "parameter_space";
    widgets->complex_plane->z[0] = 0;
    widgets->complex_plane->z[1] = 0;
    // complex_plane->center[0] = -0.5;
    widgets->complex_plane->center[0] = 0;
    widgets->complex_plane->center[1] = 0;

    if (w >= h){
      ratio = (double) widgets->complex_plane->w / (double) widgets->complex_plane->h;
      SpanYmin = 3;
      SpanXmin = SpanYmin*ratio;
    } else {
      ratio = (double) widgets->complex_plane->h / (double) widgets->complex_plane->w;
      SpanXmin = 3;
      SpanYmin = SpanXmin*ratio;
    }

    widgets->complex_plane->Sx[0] = (-SpanXmin/2) + widgets->complex_plane->center[0];
    widgets->complex_plane->Sx[1] = (SpanXmin/2) + widgets->complex_plane->center[0];
    widgets->complex_plane->Sy[0] = (-SpanYmin/2) + widgets->complex_plane->center[1];
    widgets->complex_plane->Sy[1] = (SpanYmin/2) + widgets->complex_plane->center[1];
    widgets->complex_plane->SpanX = SpanXmin;
    widgets->complex_plane->SpanY = SpanYmin;

  } else {
    apply_changes(widget, data);
  }

  if (widgets->complex_plane->plot != NULL){
    free(widgets->complex_plane->plot);
    widgets->complex_plane->plot = NULL;
  }
  if (widgets->complex_plane->drawn_plot != NULL){
    free(widgets->complex_plane->drawn_plot);
    widgets->complex_plane->drawn_plot = NULL;
  }

  #ifdef DEBUG_GUI
  printf("Spans: %f %f | %f %f\n", widgets->complex_plane->Sx[0],
                                   widgets->complex_plane->Sx[1],
                                   widgets->complex_plane->Sy[0],
                                   widgets->complex_plane->Sy[1]);
  #endif

  switch(function_type){
    case 0:
      widgets->complex_plane->plot = draw_julia(widgets->complex_plane->N,
                                    widgets->complex_plane->h,
                                    widgets->complex_plane->w,
                                    widgets->complex_plane->z,
                                    widgets->complex_plane->Sx,
                                    widgets->complex_plane->Sy,
                                    widgets->complex_plane->plot_type,
                                    &(widgets->opencl), !(widgets->opencl->init));
      break;
    case 1:
      if (polynomial_order != -1){
        widgets->complex_plane->polynomial = polynomial;
        widgets->complex_plane->polynomial_order = polynomial_order;
        widgets->complex_plane->plot = draw_julia_polynomial(widgets->complex_plane->N,
                                                    widgets->complex_plane->h,
                                                    widgets->complex_plane->w,
                                                    widgets->complex_plane->polynomial_order,
                                                    widgets->complex_plane->polynomial,
                                                    widgets->complex_plane->Sx,
                                                    widgets->complex_plane->Sy,
                                                    widgets->complex_plane->polynomial_parameter,
                                                    &(widgets->opencl), !widgets->opencl->init);
      }
      break;
    case 2:
      if (polynomial_order != -1){
        printf("WIP Newton's method\n");
      }
      break;
  }

}

void draw_from_options(GtkWidget *widget, gpointer data){
  gen_plot(widget, false, data);
  draw_main_window(widget, data);
}

void draw_default_mandelbrot(GtkWidget *widget, gpointer data){
  gen_plot(widget, true, data);
  draw_main_window(widget, data);
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

void plot_zoom(GtkWidget *widget, double zoomratio, double p[2], gpointer data){
  struct mainWindowData *widgets = (struct mainWindowData *) data;
  #ifdef DEBUG_GUI
  printf("Zooming in %f %f\n", p[0], p[1]);
  #endif
  char buffer[500];
  sprintf(buffer, "%.16g", p[0]);
  gtk_entry_set_text(GTK_ENTRY(widgets->input_center0), buffer);
  sprintf(buffer, "%.16g", p[1]);
  gtk_entry_set_text(GTK_ENTRY(widgets->input_center1), buffer);
  widgets->complex_plane->SpanX *= zoomratio;
  widgets->complex_plane->SpanY *= zoomratio;
  sprintf(buffer, "%.16g", widgets->complex_plane->SpanX);
  gtk_entry_set_text(GTK_ENTRY(widgets->input_spanx), buffer);
  sprintf(buffer, "%.16g", widgets->complex_plane->SpanY);
  gtk_entry_set_text(GTK_ENTRY(widgets->input_spany), buffer);
  draw_from_options(widget, data);
}

void zoom_button_handler(GtkWidget *widget, gpointer data){
  gchar *arg = g_strdup_printf("%s", gtk_button_get_label(GTK_BUTTON(widget)));
  struct mainWindowData *widgets = (struct mainWindowData *) data;
  if (strcmp(arg, " + ") == 0){
      plot_zoom(widget, 0.5, widgets->complex_plane->center, data);
  } else if (strcmp(arg, "  -  ") == 0){
      plot_zoom(widget, 2, widgets->complex_plane->center, data);
  }
}

void draw_thumbnail_gui(GtkWidget *widget, double x, double y, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  struct mainWindowData *widgets = (struct mainWindowData *) data;

  int N_thumb = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_thumbN)), NULL);
  if (N_thumb == 0){
    N_thumb = 50;
  }
  widgets->thumbnail->N = N_thumb;
  gtk_widget_destroy(thumb_img);
  widgets->thumbnail->z[0] = x; widgets->thumbnail->z[1] = y;
  switch(function_type){
    case 0:
      if (strcmp(widgets->complex_plane->plot_type, "parameter_space") == 0){
        widgets->thumbnail->plot = draw_thumbnail(widgets->thumbnail->N,
                                                  widgets->thumbnail->h,
                                                  widgets->thumbnail->w,
                                                  widgets->thumbnail->z,
                                                  "rec_f",
                                                  &(widgets->opencl), !(widgets->opencl->init));
      } else if (strcmp(widgets->complex_plane->plot_type, "rec_f") == 0){
        widgets->thumbnail->plot = draw_thumbnail(widgets->thumbnail->N,
                                                  widgets->thumbnail->h,
                                                  widgets->thumbnail->w,
                                                  widgets->thumbnail->z,
                                                  "parameter_space",
                                                  &(widgets->opencl),
                                                  !widgets->opencl->init);
      }
      break;
    case 1:
      if (polynomial_order != -1){
        complex double *polynomial_th = malloc(sizeof(complex double) * (polynomial_order + 2));
        for (int i = 0; i < polynomial_order + 2; i++){
          polynomial_th[i] = polynomial[i];
        }

        polynomial_th[widgets->complex_plane->polynomial_parameter] = x + y*I;
        widgets->thumbnail->plot = draw_thumbnail_polynomial(widgets->thumbnail->N,
                                                             widgets->thumbnail->h,
                                                             widgets->thumbnail->w,
                                                             polynomial_order,
                                                             polynomial_th,
                                                             widgets->thumbnail->polynomial_parameter,
                                                             &(widgets->opencl),
                                                             !widgets->opencl->init);
      } else {
        widgets->thumbnail->plot = calloc(widgets->thumbnail->w * widgets->thumbnail->h * 3, 1);
      }
  }

  GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
      widgets->thumbnail->plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      widgets->thumbnail->w, widgets->thumbnail->h,
      widgets->thumbnail->w * 3,
      NULL, NULL);
  thumb_img = gtk_image_new_from_pixbuf(thumbBuf);

  gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);

  gtk_widget_show_all(GTK_WIDGET(window));
}

void cp_mouse_handler(GtkWidget *widget, GdkEventButton *event, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));

  struct mainWindowData *widgets = (struct mainWindowData *) data;
  char buffer[500];
  #ifdef DEBUG_GUI
  printf("%d\n", event->button);
  #endif

  double w_ev = gtk_widget_get_allocated_width(widget);
  double h_ev = gtk_widget_get_allocated_height(widget);

  event->y = event->y + (widgets->complex_plane->h - h_ev)/2;

  double x = (event->x-0)/(widgets->complex_plane->w-0) * (widgets->complex_plane->Sx[1] - widgets->complex_plane->Sx[0]) + widgets->complex_plane->Sx[0];
  double y = (-(event->y-widgets->complex_plane->h)-0)/(widgets->complex_plane->h-0) * (widgets->complex_plane->Sy[1] - widgets->complex_plane->Sy[0]) + widgets->complex_plane->Sy[0];
  double point_clicked[2] = {x, y};
  gchar *point = g_strdup_printf("%.16g %+.16gi", x, y);
  gtk_label_set_text(GTK_LABEL(point_targeted), point);


  if (draw_thumbnails_bool){
    if (widgets->thumbnail->plot != NULL){
      free(widgets->thumbnail->plot);
      widgets->thumbnail->plot = NULL;
    }
    draw_thumbnail_gui(widget, x, y, data);
  }

  switch (event->button){
    case 0:
      if (widgets->zoom_point1[0] == NULL){
        if (draw_lines_bool){
          draw_sequence(widget, event, data);
        }
      } else {
        draw_box(widget, event, data);
      }
      break;
    case 1:
      if (widgets->zoom_point1[0] == NULL){
        if (event->type != GDK_BUTTON_PRESS){
          break;
        }
        widgets->zoom_point1[0] = malloc(sizeof(double));
        widgets->zoom_point1[1] = malloc(sizeof(double));
        *widgets->zoom_point1[0] = x; *widgets->zoom_point1[1] = y;

        draw_main_window(GTK_WIDGET(window), data);


        #ifdef DEBUG_GUI
        printf("First zoom point: %.16g %.16g\n", x, y);
        #endif
      } else {
        if (event->type != GDK_BUTTON_RELEASE){
          break;
        }
        widgets->zoom_point2[0] = malloc(sizeof(double));
        widgets->zoom_point2[1] = malloc(sizeof(double));
        *widgets->zoom_point2[0] = x; *widgets->zoom_point2[1] = y;
        #ifdef DEBUG_GUI
        printf("Second zoom point: %.16g %.16g\n", x, y);
        #endif

        double aux;
        if (*widgets->zoom_point1[0] > *widgets->zoom_point2[0]){
          aux = *widgets->zoom_point1[0];
          *widgets->zoom_point1[0] = *widgets->zoom_point2[0];
          *widgets->zoom_point2[0] = aux;
        }
        if (*widgets->zoom_point1[1] > *widgets->zoom_point2[1]){
          aux = *widgets->zoom_point1[1];
          *widgets->zoom_point1[1] = *widgets->zoom_point2[1];
          *widgets->zoom_point2[1] = aux;
        }

        widgets->complex_plane->center[0] = (*widgets->zoom_point2[0] + *widgets->zoom_point1[0])/2;
        widgets->complex_plane->center[1] = (*widgets->zoom_point2[1] + *widgets->zoom_point1[1])/2;

        sprintf(buffer, "%.16g", widgets->complex_plane->center[0]);
        gtk_entry_set_text(GTK_ENTRY(widgets->input_center0), buffer);
        sprintf(buffer, "%.16g", widgets->complex_plane->center[1]);
        gtk_entry_set_text(GTK_ENTRY(widgets->input_center1), buffer);

        widgets->complex_plane->SpanX = *widgets->zoom_point2[0] - *widgets->zoom_point1[0];
        widgets->complex_plane->SpanY = *widgets->zoom_point2[1] - *widgets->zoom_point1[1];

        if (widgets->complex_plane->w >= widgets->complex_plane->h){
          double ratio = (double) widgets->complex_plane->w / (double) widgets->complex_plane->h;
          widgets->complex_plane->SpanX = widgets->complex_plane->SpanY*ratio;
        } else {
          double ratio = (double) widgets->complex_plane->h / (double) widgets->complex_plane->w;
          widgets->complex_plane->SpanY = widgets->complex_plane->SpanX*ratio;
        }

        sprintf(buffer, "%.16g", widgets->complex_plane->SpanX);
        gtk_entry_set_text(GTK_ENTRY(widgets->input_spanx), buffer);
        sprintf(buffer, "%.16g", widgets->complex_plane->SpanY);
        gtk_entry_set_text(GTK_ENTRY(widgets->input_spany), buffer);


        apply_changes(widget, data);
        draw_from_options(widget, data);

        free(widgets->zoom_point1[0]);
        free(widgets->zoom_point1[1]);
        free(widgets->zoom_point2[0]);
        free(widgets->zoom_point2[1]);
        widgets->zoom_point1[0] = NULL;
        widgets->zoom_point1[1] = NULL;
        widgets->zoom_point2[0] = NULL;
        widgets->zoom_point2[1] = NULL;
      }
      break;
    case 2:   //Move center to point clicked with middle button
      sprintf(buffer, "%.16g", x);
      gtk_entry_set_text(GTK_ENTRY(widgets->input_center0), buffer);
      sprintf(buffer, "%.16g", y);
      gtk_entry_set_text(GTK_ENTRY(widgets->input_center1), buffer);
      draw_from_options(widget, data);
      break;
    case 3:   //Plot from point clicked with right button
      switch (function_type){
        case 0:
          sprintf(buffer, "%.16g", x);
          gtk_entry_set_text(GTK_ENTRY(widgets->input_z0), buffer);
          sprintf(buffer, "%.16g", y);
          gtk_entry_set_text(GTK_ENTRY(widgets->input_z1), buffer);
          gtk_entry_set_text(GTK_ENTRY(widgets->input_spanx), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_spany), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_center0), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_center1), "");

          if (strcmp(widgets->complex_plane->plot_type, "parameter_space") == 0){
            gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->input_plot_type), 1);
          } else if (strcmp(widgets->complex_plane->plot_type, "rec_f") == 0){
            gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->input_plot_type), 0);
          }

          draw_from_options(widget, data);
          break;
        case 1:
          sprintf(buffer, "%.16g", x);
          gtk_entry_set_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_real[widgets->complex_plane->polynomial_parameter]), buffer);
          sprintf(buffer, "%.16g", y);
          gtk_entry_set_text(GTK_ENTRY(widgets->input_polynomial_settings_vector_imag[widgets->complex_plane->polynomial_parameter]), buffer);
          widgets->complex_plane->polynomial[widgets->complex_plane->polynomial_parameter] = x + y*I;
          gtk_entry_set_text(GTK_ENTRY(widgets->input_spanx), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_spany), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_center0), "");
          gtk_entry_set_text(GTK_ENTRY(widgets->input_center1), "");
          gtk_combo_box_set_active(GTK_COMBO_BOX(widgets->combo_polynomial_parameter), gtk_combo_box_get_active(GTK_COMBO_BOX(widgets->combo_polynomial_parameter_thumb)));
          draw_from_options(widget, data);
      }
      break;
  }
}

void draw_box(GtkWidget *window, GdkEventButton *event, gpointer data){
  struct mainWindowData *widgets = (struct mainWindowData *) data;

  int x0 = event->x;
  int y0 = event->y;
  int x1 = (int) floor((*widgets->zoom_point1[0] - widgets->complex_plane->Sx[0])/(widgets->complex_plane->Sx[1]-widgets->complex_plane->Sx[0]) * widgets->complex_plane->w);
  int y1 = (int) floor((-(*widgets->zoom_point1[1]-widgets->complex_plane->center[1] - ((widgets->complex_plane->Sy[0] + widgets->complex_plane->Sy[1])/2)) - widgets->complex_plane->Sy[0])/(widgets->complex_plane->Sy[1] - widgets->complex_plane->Sy[0])*widgets->complex_plane->h);

  widgets->complex_plane->drawn_plot = malloc(widgets->complex_plane->h * widgets->complex_plane->w*3);
  for (int i = 0; i < widgets->complex_plane->h*widgets->complex_plane->w*3; i++){
    widgets->complex_plane->drawn_plot[i] = widgets->complex_plane->plot[i];
  }

  draw_line(widgets->complex_plane->drawn_plot, x0, y0, x0, y1, widgets->complex_plane->w, widgets->complex_plane->h);
  draw_line(widgets->complex_plane->drawn_plot, x0, y0, x1, y0, widgets->complex_plane->w, widgets->complex_plane->h);
  draw_line(widgets->complex_plane->drawn_plot, x0, y1, x1, y1, widgets->complex_plane->w, widgets->complex_plane->h);
  draw_line(widgets->complex_plane->drawn_plot, x1, y0, x1, y1, widgets->complex_plane->w, widgets->complex_plane->h);


  clear_container(window);

  GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
      widgets->complex_plane->drawn_plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      widgets->complex_plane->w, widgets->complex_plane->h,
      widgets->complex_plane->w * 3,
      NULL, NULL);
  GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);
  gtk_container_add(GTK_CONTAINER(window), image);
  gtk_widget_show_all(GTK_WIDGET(window));
  free(widgets->complex_plane->drawn_plot);
  widgets->complex_plane->drawn_plot = NULL;
}

void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data){
  struct mainWindowData *widgets = (struct mainWindowData *) data;

  double w = (double) widgets->complex_plane->w, h = (double) widgets->complex_plane->h;
  double x = (event->x-0)/(widgets->complex_plane->w-0) * (widgets->complex_plane->Sx[1] - widgets->complex_plane->Sx[0]) + widgets->complex_plane->Sx[0];
  double y = (-(event->y-widgets->complex_plane->h)-0)/(widgets->complex_plane->h-0) * (widgets->complex_plane->Sy[1] - widgets->complex_plane->Sy[0]) + widgets->complex_plane->Sy[0];

  #ifdef DEBUG_GUI
  printf("Plot %f + %fI\n", x, y);
  printf("Motion notify\n");
  printf("%f, %f\n", event->x, event->y);
  printf("Pointing at: %f, %f\n", x, y);
  #endif

  double p[2] = {x, y};

  widgets->complex_plane->drawn_plot = malloc(widgets->complex_plane->h*widgets->complex_plane->w*3);
  for (int i = 0; i < widgets->complex_plane->h*widgets->complex_plane->w*3; i++){
    widgets->complex_plane->drawn_plot[i] = widgets->complex_plane->plot[i];
  }

  widgets->complex_plane->N_line = strtod(gtk_entry_get_text(GTK_ENTRY(widgets->input_N_line)), NULL);
  switch(function_type){
    case 0:
      draw_sequence_lines(widgets->complex_plane, p, w, h);
      break;
    case 1:
      if (polynomial_order != -1){
        draw_sequence_lines_polynomial(widgets->complex_plane, polynomial, polynomial_order, p, w, h);
      }
      break;
  }

  clear_container(window);

  GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
      widgets->complex_plane->drawn_plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      widgets->complex_plane->w, widgets->complex_plane->h,
      widgets->complex_plane->w * 3,
      NULL, NULL);
  GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);
  gtk_container_add(GTK_CONTAINER(window), image);
  gtk_widget_show_all(GTK_WIDGET(window));
  free(widgets->complex_plane->drawn_plot);
  widgets->complex_plane->drawn_plot = NULL;
}

void draw_main_window(GtkWidget *widget, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  clear_container(GTK_WIDGET(window));

  struct mainWindowData *main_data = (struct mainWindowData *) data;
  main_data->input_polynomial_settings_vector_real = NULL;
  main_data->input_polynomial_settings_vector_imag = NULL;

  gint w, h;
  gtk_window_get_size(window, &w, &h);

  //Define all local widgets
  GtkWidget *hbox;
  GtkWidget *scroll_box;
  GtkWidget *vbox;
  GtkWidget *options_box;
  GtkWidget *iter_box;
  GtkWidget *z_box;
  GtkWidget *center_box;
  GtkWidget *draw_apply_box;
  GtkWidget *span_box;
  GtkWidget *plot_type_box;
  GtkWidget *zoom_lines_box;
  GtkWidget *zoom_box;

  GtkWidget *thumb_options_box;

  GtkWidget *button_clear;
  GtkWidget *button_draw;
  GtkWidget *button_mandelbrot;
  GtkWidget *button_quit;
  GtkWidget *button_zoomin;
  GtkWidget *button_zoomout;
  // GtkWidget *button_apply_changes;   //no longer used

  //Arbitrary polynomial configuration
  GtkWidget *input_polynomial_order;
  GtkWidget *polynomial_config_vbox;
  GtkWidget *polynomial_scroll_box;
  GtkWidget *polynomial_config_hbox;
  GtkWidget *polynomial_order_box;


// struct mainWindowData{
//   GtkWidget *input_N;
//   GtkWidget *input_N_line;
//   GtkWidget *input_z0;
//   GtkWidget *input_z1;
//   GtkWidget *input_spanx;
//   GtkWidget *input_spany;
//   GtkWidget *input_center0;
//   GtkWidget *input_center1;
//   GtkWidget *input_plot_type;
// }

  GtkWidget *plot_options_label;

  //Initialize input widgets
  main_data->input_N = gtk_entry_new();
  main_data->input_N_line = gtk_entry_new();
  main_data->input_z0 = gtk_entry_new();
  main_data->input_z1 = gtk_entry_new();
  main_data->input_center0 = gtk_entry_new();
  main_data->input_center1 = gtk_entry_new();

  main_data->input_plot_type = gtk_combo_box_text_new();
  main_data->input_spanx = gtk_entry_new();
  main_data->input_spany = gtk_entry_new();

  main_data->combo_function_type = gtk_combo_box_text_new();

  point_targeted = gtk_label_new("0 + 0i");


  //Create boxes
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
  plot_type_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  zoom_lines_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  zoom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);


  //Iter box
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_N), "Iter N");
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_N_line), "Number of lines");
  gtk_widget_set_tooltip_text(main_data->input_N, "Number of iterations to plot");
  gtk_widget_set_tooltip_text(main_data->input_N_line, "Number of iterations to plot");
  gtk_box_pack_start(GTK_BOX(iter_box), main_data->input_N, true, false, 0);
  gtk_box_pack_start(GTK_BOX(iter_box), main_data->input_N_line, true, false, 0);

  //Z box
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_z0), "z0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_z1), "z1");
  gtk_widget_set_tooltip_text(main_data->input_z0, "Real part of z");
  gtk_widget_set_tooltip_text(main_data->input_z1, "Imag part of z");
  gtk_box_pack_start(GTK_BOX(z_box), main_data->input_z0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(z_box), main_data->input_z1, true, false, 0);

  //Center box
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_center0), "Center0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_center1), "Center1");
  gtk_widget_set_tooltip_text(main_data->input_center0, "Real part of center");
  gtk_widget_set_tooltip_text(main_data->input_center1, "Imag part of center");
  gtk_box_pack_start(GTK_BOX(center_box), main_data->input_center0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(center_box), main_data->input_center1, true, false, 0);

  //Span box
  gtk_box_pack_start(GTK_BOX(span_box), main_data->input_spanx, true, false, 0);
  gtk_box_pack_start(GTK_BOX(span_box), main_data->input_spany, true, false, 0);
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_spanx), "Span X");
  gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_spany), "Span Y");

  //Plot type box
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_function_type), NULL, "Quadratic");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_function_type), NULL, "Polynomial");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_function_type), NULL, "Newton's Algorithm");
  gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->combo_function_type), function_type);
  g_signal_connect(main_data->combo_function_type, "changed", G_CALLBACK(combo_function_type_handler), main_data);
  gtk_widget_set_tooltip_text(main_data->combo_function_type, "Set type of function to plot");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->input_plot_type), NULL, "parameter_space");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->input_plot_type), NULL, "rec_f");
  gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->input_plot_type), 0);
  gtk_widget_set_tooltip_text(main_data->input_plot_type, "parameter_space or rec_f");
  gtk_box_pack_start(GTK_BOX(plot_type_box), main_data->combo_function_type, true, true, 0);
  gtk_box_pack_start(GTK_BOX(plot_type_box), main_data->input_plot_type, true, true, 0);
  gtk_widget_set_size_request(main_data->input_plot_type, 168, 20);
  gtk_widget_set_size_request(main_data->combo_function_type, 168, 20);


  //Draw/Clear box
  button_draw = gtk_button_new_with_label("Draw");
  button_clear = gtk_button_new_with_label("Clear");
  gtk_widget_set_tooltip_text(button_draw, "Draw plane from options specified");
  gtk_widget_set_tooltip_text(button_clear, "Redraw Complex Plane without drawings");
  g_signal_connect(button_draw, "clicked", G_CALLBACK(draw_from_options), (gpointer) main_data);
  g_signal_connect(button_clear, "clicked", G_CALLBACK(draw_main_window), (gpointer) main_data);
  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_draw, true, true, 0);
  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_clear, true, true, 0);

  //Zoom box
  button_zoomin = gtk_button_new_with_label(" + ");
  button_zoomout = gtk_button_new_with_label("  -  ");
  gtk_widget_set_tooltip_text(button_zoomin, "Zoom in");
  gtk_widget_set_tooltip_text(button_zoomout, "Zoom out");
  int in = 1, out = -1;
  g_signal_connect(button_zoomin, "clicked", G_CALLBACK(zoom_button_handler),  (gpointer) main_data);
  g_signal_connect(button_zoomout, "clicked", G_CALLBACK(zoom_button_handler),  (gpointer) main_data);
  gtk_box_pack_start(GTK_BOX(zoom_box), button_zoomin, false, false, 0);
  gtk_box_pack_start(GTK_BOX(zoom_box), button_zoomout, false, false, 0);

  //Draw lines
  main_data->check_draw_lines = gtk_check_button_new_with_mnemonic("Draw _lines");
  g_signal_connect(main_data->check_draw_lines, "toggled", G_CALLBACK(change_lines_state), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(main_data->check_draw_lines), draw_lines_bool);

  //Zoom/Lines box
  gtk_box_pack_start(GTK_BOX(zoom_lines_box), zoom_box, false, false, 0);
  gtk_box_pack_start(GTK_BOX(zoom_lines_box), main_data->check_draw_lines, false, false, 0);

  //Populate options box
  plot_options_label = gtk_label_new("Plot options");
  gtk_box_pack_start(GTK_BOX(options_box), plot_options_label, true, false, 5);
  gtk_box_pack_start(GTK_BOX(options_box), iter_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), z_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), center_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), span_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), plot_type_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), draw_apply_box, true, false, 0);
  gtk_box_pack_start(GTK_BOX(options_box), zoom_lines_box, true, false, 0);

  switch (function_type){
    case 0:   //Quadratic
      button_mandelbrot = gtk_button_new_with_label("Default Mandelbrot");
      gtk_widget_set_tooltip_text(button_mandelbrot, "Generate and plot Mandelbrot Set");
      g_signal_connect(button_mandelbrot, "clicked", G_CALLBACK(draw_default_mandelbrot), (gpointer) main_data);
      break;
    case 1:   //Polynomial
    case 2:
      polynomial_config_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
      polynomial_order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);


      input_polynomial_order = gtk_entry_new();
      gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_order), "Order of polynomial");
      gtk_widget_set_tooltip_text(input_polynomial_order, "Order of polynomial to plot");
      g_signal_connect(G_OBJECT(input_polynomial_order), "insert-text", G_CALLBACK(insert_text_event_int), NULL);
      g_signal_connect(input_polynomial_order, "key-release-event", G_CALLBACK(change_polynomial_order), (gpointer) main_data);

      if (polynomial_order > 0){
        gtk_entry_set_text(GTK_ENTRY(input_polynomial_order), g_strdup_printf("%d", polynomial_order));

        //Create hbox for input values
        polynomial_config_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

        main_data->combo_polynomial_parameter = gtk_combo_box_text_new();
        main_data->combo_polynomial_parameter_thumb = gtk_combo_box_text_new();

        main_data->input_polynomial_settings_vector_real = malloc(sizeof(GtkWidget) * (polynomial_order + 2));
        main_data->input_polynomial_settings_vector_imag = malloc(sizeof(GtkWidget) * (polynomial_order + 2));
        for (int i = 0; i < polynomial_order + 2; i++){
          GtkWidget *polynomial_settings_input_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
          main_data->input_polynomial_settings_vector_real[i] = gtk_entry_new();
          main_data->input_polynomial_settings_vector_imag[i] = gtk_entry_new();
          gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_polynomial_settings_vector_real[i]), "Real");
          gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_polynomial_settings_vector_imag[i]), "Imag");
          gtk_entry_set_width_chars(GTK_ENTRY(main_data->input_polynomial_settings_vector_real[i]), 5);
          gtk_entry_set_width_chars(GTK_ENTRY(main_data->input_polynomial_settings_vector_imag[i]), 5);
          gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), main_data->input_polynomial_settings_vector_real[i], false, false, 0);
          gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), main_data->input_polynomial_settings_vector_imag[i], false, false, 0);
          g_signal_connect(G_OBJECT(main_data->input_polynomial_settings_vector_real[i]), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
          g_signal_connect(G_OBJECT(main_data->input_polynomial_settings_vector_imag[i]), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
          g_signal_connect(G_OBJECT(main_data->input_polynomial_settings_vector_real[i]), "key_release_event", G_CALLBACK(save_polynomial), (gpointer) main_data);
          g_signal_connect(G_OBJECT(main_data->input_polynomial_settings_vector_imag[i]), "key_release_event", G_CALLBACK(save_polynomial), (gpointer) main_data);

          if (polynomial != NULL){
            if (creal(polynomial[i]) != 0.0){
              gtk_entry_set_text(GTK_ENTRY(main_data->input_polynomial_settings_vector_real[i]), g_strdup_printf("%.16g", creal(polynomial[i])));
            }
            if (cimag(polynomial[i]) != 0.0){
              gtk_entry_set_text(GTK_ENTRY(main_data->input_polynomial_settings_vector_imag[i]), g_strdup_printf("%.16g", cimag(polynomial[i])));
            }
            if (main_data->complex_plane->polynomial_parameter == i){
              // gtk_entry_can_focus(GTK_ENTRY(main_data->input_polynomial_settings_vector_real[i]), false);
              // gtk_entry_set_editable(GTK_ENTRY(main_data->input_polynomial_settings_vector_imag[i]), false);
            }
          }

          gtk_box_pack_start(GTK_BOX(polynomial_config_hbox), polynomial_settings_input_vbox, true, false, 0);

          char buffer[10];
          //Set polynomial z labels
          if (i < polynomial_order + 1){
            if (i < polynomial_order -1){
              sprintf(buffer, "z^%d + ", polynomial_order -i);
            } else if (i == polynomial_order -1){
              strcpy(buffer, "z + ");
            } else if (i == polynomial_order){
              strcpy(buffer, "z = ");
            }

            GtkWidget *z_power_label = gtk_label_new(buffer);
            gtk_box_pack_start(GTK_BOX(polynomial_config_hbox), z_power_label, true, false, 0);
          }

          if (i < polynomial_order + 1){
            if (i < polynomial_order -1){
              sprintf(buffer, "a * z^%d", polynomial_order-i);
            } else if (i == polynomial_order -1){
              sprintf(buffer, "a * z");
            } else if (i == polynomial_order){
              sprintf(buffer, "c");
            }
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_polynomial_parameter), NULL, buffer);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_polynomial_parameter_thumb), NULL, buffer);
          }

          //Populate parameter chosing combo box
        }

        //Create scrollbox for input values
        polynomial_scroll_box = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(polynomial_scroll_box), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
        gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(polynomial_scroll_box), true);
        gtk_container_add(GTK_CONTAINER(polynomial_scroll_box), polynomial_config_hbox);

        gtk_box_pack_end(GTK_BOX(polynomial_config_vbox), polynomial_scroll_box, true, false, 0);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_polynomial_parameter), NULL, "z");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(main_data->combo_polynomial_parameter_thumb), NULL, "z");
        gtk_entry_set_width_chars(GTK_ENTRY(input_polynomial_order), 5);
        gtk_box_pack_end(GTK_BOX(polynomial_order_box), main_data->combo_polynomial_parameter, true, true, 0);
        gtk_widget_set_tooltip_text(main_data->combo_polynomial_parameter, "Which parameter to plot");
        gtk_widget_set_tooltip_text(main_data->combo_polynomial_parameter_thumb, "Which parameter to plot in thumbnails");
        g_signal_connect(main_data->combo_polynomial_parameter, "changed", G_CALLBACK(change_polynomial_parameter), (gpointer) main_data->complex_plane);
        g_signal_connect(main_data->combo_polynomial_parameter_thumb, "changed", G_CALLBACK(change_polynomial_parameter), (gpointer) main_data->thumbnail);

        if (main_data->complex_plane->polynomial_parameter == -1){
          main_data->complex_plane->polynomial_parameter = polynomial_order;
          gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->combo_polynomial_parameter), polynomial_order);
        } else {
          gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->combo_polynomial_parameter), main_data->complex_plane->polynomial_parameter);
        }
      }

      GtkWidget *order_label = gtk_label_new("Order of polynomial: ");
      gtk_box_pack_start(GTK_BOX(polynomial_order_box), order_label, false, false, 0);
      gtk_box_pack_start(GTK_BOX(polynomial_order_box), input_polynomial_order, false, false, 0);
      gtk_box_pack_start(GTK_BOX(polynomial_config_vbox), polynomial_order_box, true, false, 0);
      break;
  }

  //Thumbnails
  gtk_box_pack_start(GTK_BOX(thumb_box), point_targeted, true, false, 0);
  if (main_data->thumbnail->plot != NULL){
    GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
        main_data->thumbnail->plot,
        GDK_COLORSPACE_RGB,
        0,
        8,
        main_data->thumbnail->w, main_data->thumbnail->h,
        main_data->thumbnail->w * 3,
        NULL, NULL);
    thumb_img = gtk_image_new_from_pixbuf(thumbBuf);


    //Input iter
    main_data->input_thumbN = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(main_data->input_thumbN), "Iter N");
    gtk_widget_set_tooltip_text(main_data->input_thumbN, "Number of iterations to plot for thumbnails");
    gchar *buffer = g_strdup_printf("%d", main_data->thumbnail->N);
    gtk_entry_set_text(GTK_ENTRY(main_data->input_thumbN), buffer);

    //Check draw_thumb
    main_data->check_draw_thumb = gtk_check_button_new_with_mnemonic("Draw _thumbnails");
    g_signal_connect(main_data->check_draw_thumb, "toggled", G_CALLBACK(change_thumbnail_state), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(main_data->check_draw_thumb), draw_thumbnails_bool);

    //Thumb options box
    gtk_box_pack_start(GTK_BOX(thumb_options_box), main_data->input_thumbN, true, false, 0);
    gtk_box_pack_start(GTK_BOX(thumb_options_box), main_data->check_draw_thumb, true, false, 0);

    //Thumb box
    gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);
    gtk_box_pack_end(GTK_BOX(thumb_box), thumb_options_box, true, false, 0);

    if (polynomial_order > 0){
      gtk_box_pack_start(GTK_BOX(thumb_options_box), main_data->combo_polynomial_parameter_thumb, true, false, 0);
      gtk_entry_set_width_chars(GTK_ENTRY(main_data->input_thumbN), 5);

      if (main_data->thumbnail->polynomial_parameter == -1){
        main_data->thumbnail->polynomial_parameter = polynomial_order + 1;
        gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->combo_polynomial_parameter_thumb), polynomial_order + 1);
      } else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->combo_polynomial_parameter_thumb), main_data->thumbnail->polynomial_parameter);
      }
    }
  }

 //Button quit
 button_quit = gtk_button_new_with_label("Quit");
 g_signal_connect(button_quit, "clicked", G_CALLBACK(destroy), NULL);

 //configure scroll_box
 scroll_box = gtk_scrolled_window_new(NULL, NULL);
 gtk_container_add(GTK_CONTAINER(scroll_box), vbox);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_box), GTK_POLICY_NEVER, GTK_POLICY_EXTERNAL);
 gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroll_box), true);

 //Populate vbox
 gtk_box_pack_start(GTK_BOX(vbox), options_box, true, false, 0);
 switch (function_type){
   case 0:
     gtk_box_pack_start(GTK_BOX(vbox), button_mandelbrot, true, false, 0);
     break;
   case 1:
   case 2:
     gtk_box_pack_start(GTK_BOX(vbox), polynomial_config_vbox, true, false, 0);
     break;
 }
 gtk_box_pack_start(GTK_BOX(vbox), thumb_box, true, false, 0);
 gtk_box_pack_end(GTK_BOX(vbox), button_quit, true, false, 0);

 //Generate complex_plane
 if (main_data->complex_plane == NULL){
   main_data->complex_plane = malloc(sizeof(struct ComplexPlane));
   main_data->complex_plane->plot = NULL;
   main_data->complex_plane->drawn_plot = NULL;
   main_data->complex_plane->polynomial_parameter = -1;
   gen_plot(widget, true, (gpointer) main_data);
 }

 //If plane has been already generated
 if (main_data->complex_plane->plot != NULL){
   //Copy complex_plane values to input boxes
   char buffer[500];
   sprintf(buffer, "%d", main_data->complex_plane->N_line);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_N_line), buffer);
   sprintf(buffer, "%d", main_data->complex_plane->N);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_N), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->z[0]);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_z0), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->z[1]);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_z1), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->center[0]);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_center0), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->center[1]);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_center1), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->SpanX);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_spanx), buffer);
   sprintf(buffer, "%.16g", main_data->complex_plane->SpanY);
   gtk_entry_set_text(GTK_ENTRY(main_data->input_spany), buffer);
   if (strcmp(main_data->complex_plane->plot_type, "parameter_space") == 0){
     gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->input_plot_type), 0);
   } else if (strcmp(main_data->complex_plane->plot_type, "rec_f") == 0){
     gtk_combo_box_set_active(GTK_COMBO_BOX(main_data->input_plot_type), 1);
   }

   //Image (not generated by default, but when button is clicked).
   GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
       main_data->complex_plane->plot,
       GDK_COLORSPACE_RGB,
       0,
       8,
       main_data->complex_plane->w, main_data->complex_plane->h,
       main_data->complex_plane->w * 3,
       NULL, NULL);
   GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);

   //Event box
   GtkWidget *event_box = gtk_event_box_new();
   gtk_container_add(GTK_CONTAINER(event_box), image);
   g_signal_connect(G_OBJECT(event_box), "motion-notify-event", G_CALLBACK(cp_mouse_handler), (gpointer) main_data);
   g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(cp_mouse_handler), (gpointer) main_data);
   g_signal_connect(G_OBJECT(event_box), "button-release-event", G_CALLBACK(cp_mouse_handler), (gpointer) main_data);
   gtk_widget_set_events(event_box, GDK_POINTER_MOTION_MASK);

   gtk_box_pack_start(GTK_BOX(hbox), event_box, false, true, 0);
 }
 //Pack hbox (Other element is in if complex_plane != NULL)
 gtk_box_pack_end(GTK_BOX(hbox), scroll_box, false, false, 20);

 //Add hbox to window
 gtk_container_add(GTK_CONTAINER(window), hbox);

  gtk_widget_show_all(GTK_WIDGET(window));
}

int main (int argc, char *argv[]) {
  //cd to root folder
  chdir(get_root_folder(argv[0]));
  struct mainWindowData *main_data = malloc(sizeof(struct mainWindowData));

  main_data->opencl = malloc(sizeof(struct OpenCL_Program));
  main_data->opencl->init = false;

  gtk_init (&argc, &argv);

  main_data->thumbnail = malloc(sizeof(struct ComplexPlane));
  main_data->thumbnail->w = 320;
  main_data->thumbnail->h = 180;
  main_data->thumbnail->plot = malloc(main_data->thumbnail->w * main_data->thumbnail->h * 3);
  main_data->thumbnail->N = 25;
  main_data->thumbnail->polynomial_parameter = -1;
  for (int i = 0; i < main_data->thumbnail->w * main_data->thumbnail->h * 3; i++){
    main_data->thumbnail->plot[i] = 0;
  }

  function_type = 0;

  int w = 1500;
  int h = 780;

  main_data->complex_plane = NULL;

  main_data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget *window_root = main_data->window;

  main_data->zoom_point1[0] = NULL;
  main_data->zoom_point1[1] = NULL;
  main_data->zoom_point2[0] = NULL;
  main_data->zoom_point2[1] = NULL;

  gtk_window_set_title(GTK_WINDOW(window_root), "Julia Set Viewer");
  gtk_window_set_default_size(GTK_WINDOW(window_root), w, h);
  gtk_window_set_position(GTK_WINDOW(window_root), GTK_WIN_POS_CENTER);
  g_signal_connect(window_root, "destroy",
                   G_CALLBACK(destroy),
                   NULL);


  gtk_widget_show_all(window_root);
  draw_main_window(window_root, (gpointer) main_data);
  // gtk_window_fullscreen(GTK_WINDOW(window));
  // gtk_window_set_default_icon_from_file("assets/app_icon/256.png", NULL);
  gtk_main();

  free(main_data->complex_plane);
  free(main_data->thumbnail);
  return 0;
}
