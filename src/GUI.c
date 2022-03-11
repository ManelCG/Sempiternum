#include <gtk/gtk.h>
#include <stdbool.h>

#include <math.h>

#include <ComplexPlane.h>
#include <draw_julia.h>


struct ComplexPlane *complex_plane = NULL;

void draw_main_window(GtkWidget *widget, gpointer data);
void gen_default_mandelbrot(GtkWidget *widget);

void gen_default_mandelbrot(GtkWidget *widget){
  int w, h;
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);

  if (complex_plane == NULL){
    complex_plane = malloc(sizeof(struct ComplexPlane));
    complex_plane->plot = NULL;
    complex_plane->drawn_plot = NULL;
  }

  complex_plane->N = 500;
  complex_plane->N_line = 25;
  complex_plane->w = w * 0.85, complex_plane->h = h;
  complex_plane->plot_type = "parameter_space";
  complex_plane->z[0] = 0;
  complex_plane->z[1] = 0;
  complex_plane->center[0] = -0.25;
  complex_plane->center[1] = 0;
  double SpanXmin;
  double SpanYmin;
  double ratio;

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

  if (complex_plane->plot != NULL){
    free(complex_plane->plot);
    complex_plane->plot = NULL;
  }
  if (complex_plane->drawn_plot != NULL){
    free(complex_plane->drawn_plot);
    complex_plane->drawn_plot = NULL;
  }

  complex_plane->plot = draw_julia(complex_plane->N,
                                complex_plane->h,
                                complex_plane->w,
                                complex_plane->z,
                                complex_plane->Sx,
                                complex_plane->Sy,
                                complex_plane->plot_type);
}

void draw_default_mandelbrot(GtkWidget *widget){
  gen_default_mandelbrot(widget);
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

void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data){
  double w = gtk_widget_get_allocated_width(window);
  double h = gtk_widget_get_allocated_height(window);

  double x = (event->x-0)/(w-0) * (complex_plane->Sx[1] - complex_plane->Sx[0]) + complex_plane->Sx[0];
  double y = (-(event->y-h)-0)/(h-0) * (complex_plane->Sy[1] - complex_plane->Sy[0]) + complex_plane->Sy[0];

  printf("Plot %f + %fI\n", x, y);

  printf("Motion notify\n");
  printf("%f, %f\n", event->x, event->y);
  printf("Pointing at: %f, %f\n", x, y);

  double p[2] = {x, y};

  complex_plane->drawn_plot = malloc(complex_plane->h*complex_plane->w*3);
  for (int i = 0; i < complex_plane->h*complex_plane->w*3; i++){
    complex_plane->drawn_plot[i] = complex_plane->plot[i];
  }

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

  GtkWidget *hbox;
  GtkWidget *vbox;

  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);

  //Generate complex_plane set
  if (complex_plane == NULL){
    complex_plane = malloc(sizeof(struct ComplexPlane));
    complex_plane->plot = NULL;
    complex_plane->drawn_plot = NULL;
  }

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

    g_signal_connect(G_OBJECT(event_box), "button_press_event", G_CALLBACK(draw_sequence), NULL);
    g_signal_connect(G_OBJECT(event_box), "motion-notify-event", G_CALLBACK(draw_sequence), NULL);
  }

  button1 = gtk_button_new_with_label("Redraw plane");
  g_signal_connect(button1, "clicked", G_CALLBACK(draw_main_window), NULL);

  button2 = gtk_button_new_with_label("Button2");
  button3 = gtk_button_new_with_label("Default Mandelbrot");
  g_signal_connect(button3, "clicked", G_CALLBACK(draw_default_mandelbrot), NULL);

  gtk_box_pack_end(GTK_BOX(vbox), button3, true, false, 0);
  gtk_box_pack_end(GTK_BOX(vbox), button2, true, false, 0);
  gtk_box_pack_end(GTK_BOX(vbox), button1, true, false, 0);

  gtk_box_pack_end(GTK_BOX(hbox), vbox, true, false, 0);

  gtk_container_add(GTK_CONTAINER(window), hbox);

  gtk_widget_show_all(GTK_WIDGET(window));
}

int main (int argc, char *argv[]) {
  gtk_init (&argc, &argv);

  GtkWidget *window;
  // int w = 600;
  // int h = 480;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Julia Set Viewer");
  // gtk_window_set_default_size(GTK_WINDOW(window), w, h);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_signal_connect(window, "destroy",
                   G_CALLBACK(destroy),
                   NULL);


  gtk_widget_show_all(window);
  draw_main_window(window, NULL);
  gtk_main();

  free(complex_plane);
  return 0;
}
