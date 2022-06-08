#include <gtk/gtk.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#include <sys/time.h>

#include <math.h>

#include <file_io.h>

#include <ComplexPlane.h>
#include <complex_plane_colorschemes.h>
#include <image_manipulation.h>
#include <draw_julia.h>
#include <lodepng.h>

#include <pthread.h>

#include <gui_gen_video.h>
#include <gui_templates.h>

#define COMPLEX_PLANE_THUMBNAIL_ID 69
#define USE_THREADS 1

#define GUI_ACTION_RENDER_VIDEO 1
#define GUI_ACTION_GEN_FRAMES 2

// #define DEBUG_GUI

struct genVideoData{
  ComplexPlane *cp;
  GtkWidget **option_widgets;
  int num_resolutions;
  int action;
  _Bool stop;
  _Bool pause;
  _Bool rendering;
  pthread_t thread;
};

struct videoProgress{
  GtkWidget *preview_image;
  GtkWidget *progress_bar;
  const unsigned char *cp_plot;
  struct timeval start_time;
  int w;
  int h;
  int stride;
  int frame;
  int total_frames;
  int start_frame;
};

//Chose between quadratic, polynomial, etc
//TODO: Remove these global variables

GtkWidget *point_targeted;
GtkWidget *thumb_img = NULL;
GtkWidget *thumb_box;

//Function declarations. TODO: Move to header file
void draw_main_window(GtkWidget *widget, gpointer data);
void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data);
void draw_thumbnail_gui(GtkWidget *widget, double x, double y, gpointer data);
void draw_from_options(GtkWidget *widget, gpointer data);
void save_polynomial_member(GtkWidget *widget, gpointer data);
void gui_draw_zoom_box(GtkWidget *window, GdkEventButton *event, gpointer data);
void plot_zoom(GtkWidget *widget, double zoomratio, complex double p, gpointer data);

int calculate_number_of_frames(double maxx, double maxy, double minx, double miny, double zoomratio){
  int framesx = (int) floor((log((double) minx / (double) maxx))/log(zoomratio));
  int framesy = (int) floor((log((double) miny / (double) maxy))/log(zoomratio));
  return fmin(framesx, framesy);
}
void calculate_video_duration(GtkWidget *w, gpointer data){
  #ifdef DEBUG_GUI
    printf("\n#####\nCalculating video duration...\n");
  #endif //DEBUG_GUI
  GtkWidget **widgets = (GtkWidget **) data;
  double maxx, maxy, minx, miny, zoomratio, fps;
  int frames, seconds;
  char *text;
  maxx = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[0])), NULL);
  maxy = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[1])), NULL);
  minx = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[2])), NULL);
  miny = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[3])), NULL);
  zoomratio = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[4])), NULL);
  fps = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[5])), NULL);

  frames = calculate_number_of_frames(maxx, maxy, minx, miny, zoomratio);
  frames = fmax(frames, 0);

  seconds = (int) floor(frames / fps);

  int eta_h = seconds/3600; seconds -= eta_h * 3600;
  int eta_m = seconds/60; seconds -= eta_m * 60;
  int eta_s = floor(seconds);

  text = g_strdup_printf("Video duration: %02d:%02d:%02d", eta_h, eta_m, eta_s);
  gtk_label_set_text(GTK_LABEL(widgets[6]), text);
  #ifdef DEBUG_GUI
    printf("Video duration is %d seconds. %s.\n", seconds, text);
  #endif //DEBUG_GUI
  free(text);
  text = g_strdup_printf("Will render %d frames", frames);
  gtk_label_set_text(GTK_LABEL(widgets[7]), text);
  #ifdef DEBUG_GUI
    printf("%s.\n", text);
    printf("Done\n#####\n");
  #endif //DEBUG_GUI
  free(text);
}

void insert_text_event_int(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  #ifdef DEBUG_GUI
    printf("\n#####\ninsert_text_event_int called\n#####\n");
  #endif //DEBUG_GUI
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i])){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return;
    }
  }
}

void insert_text_event_float(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  #ifdef DEBUG_GUI
    printf("\n#####\ninsert_text_event_float called\n#####\n");
  #endif //DEBUG_GUI
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i]) && !(text[i] == '.') && !(text[i] == '-')){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return;
    }
  }
}

void save_polynomial_member(GtkWidget *widget, gpointer data){
  #ifdef DEBUG_GUI

    printf("\n#####\nsave_polynomial_member called\n");
  #endif //DEBUG_GUI
  ComplexPlane *cp = (ComplexPlane *) data;

  const char *name = gtk_widget_get_name(widget);
  char type = *name;
  int box = atoi(name + 1); //Name is 'r4' for example.

  switch(type){
    case 'r':
      double old_imag = cimag(complex_plane_get_polynomial_member(cp, box));
      complex_plane_set_polynomial_member(cp, strtod(gtk_entry_get_text(GTK_ENTRY(widget)), NULL) + old_imag * I, box);
      break;
    case 'i':
      double old_real = creal(complex_plane_get_polynomial_member(cp, box));
      complex_plane_set_polynomial_member(cp, old_real + strtod(gtk_entry_get_text(GTK_ENTRY(widget)), NULL) * I, box);
      break;
    case 'p':
      break;
    case 'c':
      complex_plane_set_polynomial_critical_point_member(cp, strtod(gtk_entry_get_text(GTK_ENTRY(widget)), NULL), box);
    default:
      break;
  }

  #ifdef DEBUG_GUI
  printf("Result:\n");
  complex_plane_print_all_polynomials(cp);
  printf("#####\n");
  #endif //DEBUG_GUI
}

void change_polynomial_order(GtkWidget *widget, GdkEventKey *event, gpointer data){
  if (strcmp(gdk_keyval_name (event->keyval), "Return") == 0){
    #ifdef DEBUG_GUI
      printf("\n#####\nchange_polynomial_order_called...\n");
    #endif //DEBUG_GUI
    ComplexPlane **planes = (ComplexPlane **) data;
    ComplexPlane *p = planes[0];
    ComplexPlane *t = planes[1];

    int order = atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    complex_plane_set_polynomial_order(p, order);
    complex_plane_set_polynomial_order(t, order);

    #ifdef DEBUG_GUI
      printf("Changed CPs %d and %d to %d\nCalling draw_main_window...\n#####\n", complex_plane_get_id(p), complex_plane_get_id(t), order);
    #endif //DEBUG_GUI

    draw_main_window(widget, data);
  }
}

void save_plot_as(GtkWidget *widget, gpointer data){
  #ifdef DEBUG_GUI
    printf("\n#####\nsave_plot_as called\n");
  #endif //DEBUG_GUI
  ComplexPlane *cp = (ComplexPlane *) data;
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new("Save File",
                                      NULL,
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      "_Cancel", GTK_RESPONSE_CANCEL,
                                      "_Open", GTK_RESPONSE_ACCEPT,
                                      NULL);
  gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), "Plot.png");
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
    char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    #ifdef DEBUG_GUI
      printf("File chosen\nGenerating plot with CP %d\n", complex_plane_get_id(cp));
    #endif //DEBUG_GUI

    complex_plane_gen_plot(cp);

    #ifdef DEBUG_GUI
      printf("Done. Saving image to %s\n", filename);
    #endif //DEBUG_GUI

    lodepng_encode24_file(filename,
                          complex_plane_get_plot(cp),
                          complex_plane_get_width(cp),
                          complex_plane_get_height(cp));

    #ifdef DEBUG_GUI
      printf("Done saving image\n");
    #endif //DEBUG_GUI
  }
  #ifdef DEBUG_GUI
  else {
    printf("Saving file cancelled.\n");
  }
  #endif //DEBUG_GUI
  gtk_widget_destroy(dialog);
  #ifdef DEBUG_GUI
    printf("#####\n");
  #endif //DEBUG_GUI
}

void generate_all_resolutions(GtkWidget *widget, gpointer data){
  struct genVideoData *imdata = (struct genVideoData *) data;
  ComplexPlane *cp = imdata->cp;
  GtkWidget *combo = imdata->option_widgets[6];
  int num_resolutions = imdata->num_resolutions;
  char *foldername = gui_gen_video_choose_folder(NULL, NULL);
  char *filename;
  char *filename_aux;
  char *progress;

  GtkWidget *label_progress = imdata->option_widgets[7];

  int previous = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

  for (int i = 0; i < num_resolutions; i++){
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
    filename_aux = gen_filename(foldername, gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo)));
    filename = g_strdup_printf("%s.png", filename_aux);

    int *dim = parse_dimensions(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo)));

    progress = g_strdup_printf("%d / %d: %dx%d\n", i+1, num_resolutions, dim[0], dim[1]);
    gtk_label_set_text(GTK_LABEL(label_progress), progress);

    complex_plane_set_dimensions(cp, dim[0], dim[1]);
    complex_plane_gen_plot(cp);

    lodepng_encode24_file(filename, complex_plane_get_plot(cp), dim[0], dim[1]);
    free(filename);
    free(filename_aux);
    free(dim);
    free(progress);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), previous);
}
void save_plot_with_name(GtkWidget *widget, ComplexPlane *cp, char *filename){
  complex_plane_gen_plot(cp);

  lodepng_encode24_file(filename,
                        complex_plane_get_plot(cp),
                        complex_plane_get_width(cp),
                        complex_plane_get_height(cp));
}

void *gui_set_entry_editable(gpointer w){
  GtkWidget *wid = (GtkWidget *) w;
  gtk_editable_set_editable(GTK_EDITABLE(wid), true);
  gtk_widget_set_can_focus(wid, true);
  gtk_widget_set_sensitive(wid, true);

  return NULL;
}
void *gui_set_entry_noneditable(gpointer w){
  GtkWidget *wid = (GtkWidget *) w;
  gtk_editable_set_editable(GTK_EDITABLE(wid), false);
  gtk_widget_set_can_focus(wid, false);
  gtk_widget_set_sensitive(wid, false);

  return NULL;
}
void *update_video_preview(gpointer data){
  struct videoProgress *p = (struct videoProgress *) data;

  double progress = ((double) p->frame / (double) p->total_frames);

  struct timeval actual_time;
  gettimeofday(&actual_time, NULL);
  double elapsed_time = (double) (actual_time.tv_usec - p->start_time.tv_usec) / 1000000 + (double) (actual_time.tv_sec - p->start_time.tv_sec);
  double fps = (double) (p->frame - p->start_frame) / elapsed_time;
  double eta = (elapsed_time / (double) (p->frame - p->start_frame)) * (p->total_frames - (p->frame - p->start_frame));
  int eta_h = eta/3600; eta -= eta_h * 3600;
  int eta_m = eta/60; eta -= eta_m * 60;
  int eta_s = floor(eta);

  char *progress_string = g_strdup_printf("%.2f%% (%d / %d). FPS: %.2f. ETA: %02d:%02d:%02d", progress*100, p->frame, p->total_frames, fps, eta_h, eta_m, eta_s);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(p->progress_bar), progress);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(p->progress_bar), progress_string);

  free(progress_string);

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(
      p->cp_plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      p->w, p->h,
      p->w * p->stride,
      NULL, NULL);

  double ratio = (double) p->w / (double) p->h;
  int preview_h = 175, preview_w = preview_h * ratio;

  pixbuf = gdk_pixbuf_scale_simple(pixbuf, preview_w, preview_h, GDK_INTERP_NEAREST);
  gtk_image_set_from_pixbuf(GTK_IMAGE(p->preview_image), pixbuf);


  return NULL;
}

void *render_video(void *data){
  // w[0]  = entry_choose_resolution[0];
  // w[1]  = entry_choose_resolution[1];
  // w[2]  = entry_choose_spans[0];
  // w[3]  = entry_choose_spans[1];
  // w[4]  = entry_choose_spans[2];
  // w[5]  = entry_choose_spans[3];
  // w[6]  = entry_choose_center_point[0];
  // w[7]  = entry_choose_center_point[1];
  // w[8]  = folder_input;
  // w[9]  = file_input;
  // w[10] = progress_bar;
  // w[11] = video_preview;
  // w[12] = entry_fps;
  // w[13] = entry_zoomratio;
  struct genVideoData *videodata = (struct genVideoData *) data;
  int action = videodata->action;
  videodata->pause = false;
  videodata->stop = false;
  videodata->rendering = true;

  int pidFFMPEG;
  int pipeFFMPEG[2];

  int frames;
  ComplexPlane *cp_original = videodata->cp;
  ComplexPlane *cp = complex_plane_copy(NULL, cp_original);
  GtkWidget **widgets = videodata->option_widgets;

  printf("Making non editable...\n");
  for (int i = 0; i <= 9; i++){
    g_main_context_invoke(NULL, G_SOURCE_FUNC(gui_set_entry_noneditable), (gpointer) widgets[i]);
  }
  for (int i = 12; i <= 13; i++){
    g_main_context_invoke(NULL, G_SOURCE_FUNC(gui_set_entry_noneditable), (gpointer) widgets[i]);
  }

  double zoomratio = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[13])), NULL);
  const char *fps_str = gtk_entry_get_text(GTK_ENTRY(widgets[12]));
  double fps = strtod(fps_str, NULL);

  if (fps == 0){
    fps_str = "60";
    fps = 60;
  }

  GtkWidget *progress_bar = widgets[10];
  GtkWidget *video_preview = widgets[11];

  const char *folder = gtk_entry_get_text(GTK_ENTRY(widgets[8]));
  const char *videofile = gtk_entry_get_text(GTK_ENTRY(widgets[9]));
  char *fullvideopath;
  if (strcmp(folder, "") == 0){
    folder = "./";
    #ifdef __linux__
    folder=g_strdup_printf("%s/Videos/", getenv("HOME"));
    #endif
    #ifdef _WIN32
    folder=g_strdup_printf("%s/Desktop/", getenv("HOMEPATH"));
    #endif

  }
  if (strcmp(videofile, "") == 0){
    videofile = "video.mp4";
  }

  fullvideopath = malloc(strlen(folder) + strlen(videofile) + 5);
  strcpy(fullvideopath, folder);
  strcat(fullvideopath, "/");
  strcat(fullvideopath, videofile);

  if (action == GUI_ACTION_RENDER_VIDEO){
    printf("Will render video to mp4 format with ffmpeg\n");
    if (pipe(pipeFFMPEG) == -1){
      perror("Couldnt pipe");
      return NULL;
    }
    if ((pidFFMPEG = fork()) == -1){
      perror("Couldnt fork");
      return NULL;
    }
    if (pidFFMPEG == 0){
      close(0);
      dup(pipeFFMPEG[0]);
      close(pipeFFMPEG[0]);
      close(pipeFFMPEG[1]);

      execlp("ffmpeg", "ffmpeg", "-y", "-framerate", fps_str, "-i", "-", "-c:v", "libx264", "-pix_fmt", "yuv420p", fullvideopath, NULL);
      perror("Couldnt execl");
      return NULL;
    }
    close(pipeFFMPEG[0]);
  }


  //Get CP data
  int w = complex_plane_get_width(cp), h = complex_plane_get_height(cp), stride = complex_plane_get_stride(cp);
  double maxspanx = complex_plane_get_spanx(cp);
  double maxspany = complex_plane_get_spany(cp);

  double minspanx = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[4])), NULL);
  double minspany = strtod(gtk_entry_get_text(GTK_ENTRY(widgets[5])), NULL);

  printf("%f %f %f %f\n", maxspanx, maxspany, minspanx, minspany);

  printf("Saving data in %s. Saving video file in %s/%s\n", folder, folder, videofile);


  frames = calculate_number_of_frames(maxspanx, maxspany, minspanx, minspany, zoomratio);

  printf("Will generate %d frames.\n", frames);
  printf("At %f fps that is %f seconds.\n", fps, (double) frames / fps);
  printf("At ~~3 seconds per plot that is %f seconds to render\n", ((double)frames * 3.0)/ fps);

  struct videoProgress *video_progress = malloc(sizeof(struct videoProgress));
  video_progress->w = w; video_progress->h = h;
  video_progress->stride = stride;
  video_progress->progress_bar = progress_bar;
  video_progress->preview_image = video_preview;
  video_progress->start_frame = 0;
  video_progress->total_frames = frames;
  gettimeofday(&video_progress->start_time, NULL);

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigprocmask(SIG_BLOCK, &set, NULL);
  int sig;
  for (int i = 1; i <= frames; i++){
    while (videodata->pause){
      sigwait(&set, &sig);
      video_progress->start_frame = i-1;
      gettimeofday(&video_progress->start_time, NULL);

      printf("Exit pause\n");
    }
    if (videodata->stop){
      break;
    }
    complex_plane_set_spanx(cp, complex_plane_get_spanx(cp) * zoomratio);
    complex_plane_set_spany(cp, complex_plane_get_spany(cp) * zoomratio);


    if (action == GUI_ACTION_GEN_FRAMES){
      char framename[50];
      sprintf(framename, "%s/%010d.png", folder, i);

      printf("\33[2K\r"); //Clear line
      double progress = ((double) i / (double) frames);
      printf("Frame %d of %d... %.2f%% saving in %s", i, frames, progress * 100, framename);

      fflush(stdout);
      if (access(framename, F_OK) != 0){
        complex_plane_gen_plot(cp);

        video_progress->frame = i;
        video_progress->cp_plot = complex_plane_get_plot(cp);
        g_main_context_invoke(NULL, G_SOURCE_FUNC(update_video_preview), (gpointer) video_progress);

        lodepng_encode24_file(framename, complex_plane_get_plot(cp), w, h);
      } else {
        video_progress->start_frame = i;
      }
    } else if (action == GUI_ACTION_RENDER_VIDEO){
      complex_plane_gen_plot(cp);

      video_progress->frame = i;
      video_progress->cp_plot = complex_plane_get_plot(cp);
      g_main_context_invoke(NULL, G_SOURCE_FUNC(update_video_preview), (gpointer) video_progress);

      unsigned char *png;
      size_t pngsize;
      lodepng_encode24(&png, &pngsize, complex_plane_get_plot(cp), w, h);
      write(pipeFFMPEG[1], png, pngsize);
      free(png);
    }

  }
  complex_plane_free(cp);
  close(pipeFFMPEG[1]);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "Done!");

  printf("Making editable...\n");
  for (int i = 0; i <= 9; i++){
    g_main_context_invoke(NULL, G_SOURCE_FUNC(gui_set_entry_editable), (gpointer) widgets[i]);
  }
  for (int i = 12; i <= 13; i++){
    g_main_context_invoke(NULL, G_SOURCE_FUNC(gui_set_entry_editable), (gpointer) widgets[i]);
  }

  videodata->rendering = false;
  free(video_progress);
  printf("Done!\n");
  return NULL;
}

void destroy_cp_from_gpointer(GtkWidget *widget, gpointer gp){
  ComplexPlane *cp = (ComplexPlane *) gp;
  complex_plane_free(cp);
}
void render_video_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (d->rendering){
    printf("Already rendering a video!\n");
    return;
  }
  if (USE_THREADS == 1){
    pthread_t thread;
    pthread_create(&thread, NULL, render_video, (void *) data);
    d->thread = thread;
  } else {
    render_video((void *) data);
  }
}

void generate_video_input_handler(GtkWidget *wid, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  int w, h;
  switch(gtk_widget_get_name(wid)[0]){
    case '0':
      h = complex_plane_get_height(cp);
      complex_plane_set_dimensions(cp, atoi(gtk_entry_get_text(GTK_ENTRY(wid))), h);
      break;
    case '1':
      w = complex_plane_get_width(cp);
      complex_plane_set_dimensions(cp, w, atoi(gtk_entry_get_text(GTK_ENTRY(wid))));
      break;
    case '2':
      complex_plane_set_spanx(cp, strtod(gtk_entry_get_text(GTK_ENTRY(wid)), NULL));
      break;
    case '3':
      complex_plane_set_spany(cp, strtod(gtk_entry_get_text(GTK_ENTRY(wid)), NULL));
      break;
    case '6':
      complex_plane_set_center_real(cp, strtod(gtk_entry_get_text(GTK_ENTRY(wid)), NULL));
      break;
    case '7':
      complex_plane_set_center_imag(cp, strtod(gtk_entry_get_text(GTK_ENTRY(wid)), NULL));
      break;
  }
  #ifdef DEBUG_GUI
  printf("SECOND FUNCTION; %d %d\n", complex_plane_get_width(cp), complex_plane_get_height(cp));
  complex_plane_print(cp);
  #endif //DEBUG_GUI
}

void radio_render_video_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    d->action = GUI_ACTION_RENDER_VIDEO;
  }
}
void radio_gen_frames_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
    d->action = GUI_ACTION_GEN_FRAMES;
  }
}

void button_cancel_render_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (d->rendering){
    d->stop = true;
    if (d->pause){
      d->pause = false;
      pthread_kill(d->thread, SIGUSR1);
    }
    d->rendering = false;
  }
}
void button_pause_render_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (d->rendering){
    d->pause = true;
  }
}
void button_resume_render_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *d = (struct genVideoData *) data;
  if (d->rendering){
    d->pause = false;
    pthread_kill(d->thread, SIGUSR1);
  }
}

void save_plot_handler(GtkWidget *widget, gpointer data){
  struct genVideoData *videodata = malloc(sizeof(struct genVideoData));
  ComplexPlane *cp_old = (ComplexPlane *) data;

  int num_resolutions;

  ComplexPlane *cp;
  complex_plane_copy(&cp, cp_old);

  GtkWindow *zoom_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(GTK_WINDOW(zoom_window), "Save new plot");
  gtk_window_set_resizable(GTK_WINDOW(zoom_window), false);
  // gtk_window_set_default_size(GTK_WINDOW(zoom_window), 720, 480);
  gtk_container_set_border_width(GTK_CONTAINER(zoom_window), 15);
  g_signal_connect(zoom_window, "destroy", G_CALLBACK(destroy), (gpointer) zoom_window);

  int default_resolution_x = 1920, default_resolution_y = 1080;
  int default_label_size = 160, default_entry_size = 15;
  int default_buttons_size = 125;

  char *strbuf;

  GtkAccelGroup *accel_group;

  //Boxes
  GtkWidget *main_vbox;
  GtkWidget *file_input_hbox;

  //Config
  GtkWidget *config_vbox;
  GtkWidget *config_resolution_hbox;
  GtkWidget *config_spans_start_hbox;

  //Entries
  GtkWidget *label_config_window = gtk_label_new("Options for saving plot:");
  GtkWidget **entry_choose_resolution   = malloc(sizeof(GtkWidget *) * 2);
  GtkWidget **entry_choose_spans        = malloc(sizeof(GtkWidget *) * 4);
  GtkWidget **video_input_widgets       = malloc(sizeof(GtkWidget *) * 10);

  GtkWidget *label_progress;

  //Config input resolution
  GtkWidget *label_choose_resolution = gtk_label_new("Choose image resolution:");
  gtk_widget_set_size_request(label_choose_resolution, default_label_size, 0);
  entry_choose_resolution[0] =  gtk_entry_new();
  entry_choose_resolution[1] =  gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_resolution[0]), default_entry_size);
  gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_resolution[1]), default_entry_size);
  strbuf = g_strdup_printf("%d", default_resolution_x);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_choose_resolution[0]), strbuf); free(strbuf);
  strbuf = g_strdup_printf("%d", default_resolution_y);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_choose_resolution[1]), strbuf); free(strbuf);
  gtk_widget_set_name(entry_choose_resolution[0], "0");
  gtk_widget_set_name(entry_choose_resolution[1], "1");

  #ifdef DEBUG_GUI
  printf("FIRST FUNCTION; %d %d\n", complex_plane_get_width(cp), complex_plane_get_height(cp));
  #endif //DEBUG_GUI

  GtkWidget *combo_default_resolutions = gui_gen_video_generate_default_resolutions_combo_box(&num_resolutions);
  gtk_widget_set_size_request(combo_default_resolutions, default_buttons_size, 0);
  g_signal_connect(combo_default_resolutions, "changed", G_CALLBACK(gui_gen_video_resolutions_combo_box_to_entries), (gpointer) entry_choose_resolution);

  #ifdef DEBUG_GUI
  complex_plane_print(cp);
  #endif //DEBUG_GUI

  g_signal_connect(GTK_ENTRY(entry_choose_resolution[0]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(entry_choose_resolution[1]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);

  gui_gen_video_resolutions_combo_box_to_entries(combo_default_resolutions, (gpointer) entry_choose_resolution);

  config_resolution_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), label_choose_resolution, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), entry_choose_resolution[0], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), entry_choose_resolution[1], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), combo_default_resolutions, false, false, 0);

  //Config zoom span
  GtkWidget *label_choose_start_span = gtk_label_new("Plot span:");
  gtk_widget_set_size_request(label_choose_start_span, default_label_size, 0);
  GtkWidget *button_config_span_set_ratio;
  for (int i = 0; i < 4; i++){
    entry_choose_spans[i] = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_spans[i]), default_entry_size);
  }
  strbuf = g_strdup_printf("%.16g", complex_plane_get_spanx(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[0]), strbuf); free(strbuf);
  strbuf = g_strdup_printf("%.16g", complex_plane_get_spany(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[1]), strbuf); free(strbuf);
  gtk_widget_set_name(entry_choose_spans[0], "2");
  gtk_widget_set_name(entry_choose_spans[1], "3");
  button_config_span_set_ratio = gtk_button_new_with_label("Lock ratio");
  gtk_widget_set_size_request(button_config_span_set_ratio, default_buttons_size, 0);

  g_signal_connect(GTK_ENTRY(entry_choose_spans[0]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[1]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);

  config_spans_start_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), label_choose_start_span, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), entry_choose_spans[0], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), entry_choose_spans[1], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), button_config_span_set_ratio, false, false, 0);

  //Config vbox
  config_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(config_vbox), label_config_window, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_resolution_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_spans_start_hbox, true, true, 0);

  //Filename/Accept/Cancel bar
  GtkWidget *button_cancel;
  GtkWidget *button_begin_render;
  GtkWidget *button_all_resolutions;

  label_progress = gtk_label_new(NULL);

  video_input_widgets[0] = entry_choose_resolution[0];
  video_input_widgets[1] = entry_choose_resolution[1];
  video_input_widgets[2] = entry_choose_spans[0];
  video_input_widgets[3] = entry_choose_spans[1];
  video_input_widgets[4] = entry_choose_spans[2];
  video_input_widgets[5] = entry_choose_spans[3];
  video_input_widgets[6] = combo_default_resolutions;
  video_input_widgets[7] = label_progress;

  videodata->cp = cp;
  videodata->option_widgets = video_input_widgets;
  videodata->num_resolutions = num_resolutions;

  button_cancel = gtk_button_new_with_label("Cancel");
  button_begin_render = gtk_button_new_with_label("  Save");
  button_all_resolutions = gtk_button_new_with_label("Render all");
  gtk_widget_set_size_request(button_cancel, default_buttons_size, 20);
  gtk_widget_set_size_request(button_begin_render, default_buttons_size + 13, 20);
  gtk_widget_set_size_request(button_all_resolutions, default_buttons_size + 13, 20);

  file_input_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(file_input_hbox), label_progress, true, false, 0);
  gtk_box_pack_end(GTK_BOX(file_input_hbox), button_cancel, false, false, 0);
  gtk_box_pack_end(GTK_BOX(file_input_hbox), button_all_resolutions, false, false, 0);
  gtk_box_pack_end(GTK_BOX(file_input_hbox), button_begin_render, false, false, 0);

  g_signal_connect(button_begin_render, "clicked", G_CALLBACK(save_plot_as), (gpointer) cp);
  g_signal_connect(button_cancel, "clicked", G_CALLBACK(destroy_cp_from_gpointer), (gpointer) cp);
  g_signal_connect(button_cancel, "clicked", G_CALLBACK(destroy), (gpointer) zoom_window);
  g_signal_connect(button_all_resolutions, "clicked", G_CALLBACK(generate_all_resolutions), (gpointer) videodata);

  g_signal_connect(button_config_span_set_ratio, "clicked", G_CALLBACK(gui_gen_video_lock_span_ratio), (gpointer) video_input_widgets);
  gui_gen_video_lock_span_ratio(NULL, (gpointer) video_input_widgets);

  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(zoom_window), accel_group);
  gtk_widget_add_accelerator(button_cancel, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);


  //Main vbox
  main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_end(GTK_BOX(main_vbox), file_input_hbox, false, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), config_vbox, false, false, 0);

  gtk_container_add(GTK_CONTAINER(zoom_window), main_vbox);

  gtk_window_set_keep_above(GTK_WINDOW(zoom_window), true);
  gtk_widget_show_all(GTK_WIDGET(zoom_window));
}


void generate_video_zoom(GtkWidget *widget, gpointer data){
  struct genVideoData *videodata = malloc(sizeof(struct genVideoData));
  videodata->action = GUI_ACTION_RENDER_VIDEO;
  videodata->rendering = false;

  ComplexPlane *cp_old = (ComplexPlane *) data;
  // ComplexPlane *cp = (ComplexPlane *) data;

  ComplexPlane *cp;
  complex_plane_copy(&cp, cp_old);

  complex_plane_free_plot(cp);

  GtkWindow *zoom_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(GTK_WINDOW(zoom_window), "Generate zoom video");
  gtk_window_set_resizable(GTK_WINDOW(zoom_window), false);
  // gtk_window_set_default_size(GTK_WINDOW(zoom_window), 720, 480);
  gtk_container_set_border_width(GTK_CONTAINER(zoom_window), 15);
  g_signal_connect(zoom_window, "destroy", G_CALLBACK(destroy), (gpointer) zoom_window);

  int default_resolution_x = 1920, default_resolution_y = 1080;
  int default_label_size = 150, default_entry_size = 15;
  int default_buttons_size = 125;
  int prs_buttons_size = 100;

  const char *default_fps = "60";
  const char *default_zoomratio = "0.99";

  char *strbuf;

  GtkAccelGroup *accel_group;

  //Boxes
  GtkWidget *main_vbox;
  GtkWidget *main_hbox;
  GtkWidget *folder_input_hbox;
  GtkWidget *file_input_hbox;

  GtkWidget *pause_resume_stop_hbox;
  GtkWidget *preview_vbox;

  //Config
  GtkWidget *config_vbox;
  GtkWidget *config_resolution_hbox;
  GtkWidget *config_spans_start_hbox;
  GtkWidget *config_spans_final_hbox;
  GtkWidget *config_center_hbox;
  GtkWidget *config_frames_hbox;
  GtkWidget *config_zoomratio_hbox;

  //Entries
  GtkWidget *label_config_window = gtk_label_new("Options for generating video:");
  GtkWidget **entry_choose_resolution   = malloc(sizeof(GtkWidget *) * 2);
  GtkWidget **entry_choose_spans        = malloc(sizeof(GtkWidget *) * 4);
  GtkWidget **entry_choose_center_point = malloc(sizeof(GtkWidget *) * 2);
  GtkWidget **video_input_widgets       = malloc(sizeof(GtkWidget *) * 14);
  GtkWidget **duration_calc_widgets     = malloc(sizeof(GtkWidget *) * 8);
  GtkWidget *entry_fps; GtkWidget *entry_zoomratio;

  //Toggleables
  GtkWidget *radio_render_video;
  GtkWidget *radio_gen_frames;

  //Preview and progress
  // GtkWidget *progress_bar_separator;
  GtkWidget *vertical_separator;
  GtkWidget *progress_bar;
  GtkWidget *video_preview;

  GtkWidget *button_pause;
  GtkWidget *button_resume;
  GtkWidget *button_stop;

  //Config input resolution
  GtkWidget *label_choose_resolution = gtk_label_new("Choose video resolution:");
  gtk_label_set_justify(GTK_LABEL(label_choose_resolution), GTK_JUSTIFY_RIGHT);
  gtk_widget_set_size_request(label_choose_resolution, default_label_size, 0);
  entry_choose_resolution[0] =  gtk_entry_new();
  entry_choose_resolution[1] =  gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_resolution[0]), default_entry_size);
  gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_resolution[1]), default_entry_size);

  strbuf = g_strdup_printf("%d", default_resolution_x);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_choose_resolution[0]), strbuf); free(strbuf);
  strbuf = g_strdup_printf("%d", default_resolution_y);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_choose_resolution[1]), strbuf); free(strbuf);

  gtk_widget_set_name(entry_choose_resolution[0], "0");
  gtk_widget_set_name(entry_choose_resolution[1], "1");

  #ifdef DEBUG_GUI
  printf("FIRST FUNCTION; %d %d\n", complex_plane_get_width(cp), complex_plane_get_height(cp));
  #endif

  GtkWidget *combo_default_resolutions = gui_gen_video_generate_default_resolutions_combo_box(NULL);
  gtk_widget_set_size_request(combo_default_resolutions, default_buttons_size, 0);
  g_signal_connect(combo_default_resolutions, "changed", G_CALLBACK(gui_gen_video_resolutions_combo_box_to_entries), (gpointer) entry_choose_resolution);

  #ifdef DEBUG_GUI
  complex_plane_print(cp);
  #endif //DEBUG_GUI

  g_signal_connect(GTK_ENTRY(entry_choose_resolution[0]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(entry_choose_resolution[1]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);

  gui_gen_video_resolutions_combo_box_to_entries(combo_default_resolutions, (gpointer) entry_choose_resolution);

  config_resolution_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), label_choose_resolution, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), entry_choose_resolution[0], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), entry_choose_resolution[1], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_resolution_hbox), combo_default_resolutions, false, false, 0);

  //Config zoom span
  GtkWidget *label_choose_start_span = gtk_label_new("Starting span:");
  gtk_label_set_justify(GTK_LABEL(label_choose_start_span), GTK_JUSTIFY_RIGHT);
  GtkWidget *label_choose_final_span = gtk_label_new("Final span:");
  gtk_label_set_justify(GTK_LABEL(label_choose_final_span), GTK_JUSTIFY_RIGHT);
  gtk_widget_set_size_request(label_choose_start_span, default_label_size, 0);
  gtk_widget_set_size_request(label_choose_final_span, default_label_size, 0);
  GtkWidget *button_config_span_set_ratio;
  for (int i = 0; i < 4; i++){
    entry_choose_spans[i] = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_spans[i]), default_entry_size);
  }
  gtk_widget_set_name(entry_choose_spans[0], "2");
  gtk_widget_set_name(entry_choose_spans[1], "3");
  button_config_span_set_ratio = gtk_button_new_with_label("Lock ratio");
  gtk_widget_set_size_request(button_config_span_set_ratio, default_buttons_size, 0);

  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[0]), "5.333");
  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[1]), "3");

  strbuf = g_strdup_printf("%.16g", complex_plane_get_spanx(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[2]), strbuf); free(strbuf);
  strbuf = g_strdup_printf("%.16g", complex_plane_get_spany(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_spans[3]), strbuf); free(strbuf);

  generate_video_input_handler(entry_choose_spans[0], (gpointer) cp);
  generate_video_input_handler(entry_choose_spans[1], (gpointer) cp);

  //Render video or gen frames
  radio_render_video = gtk_radio_button_new_with_label(NULL, "Render video");
  radio_gen_frames = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_render_video), "Gen frames");
  g_signal_connect(GTK_RADIO_BUTTON(radio_render_video), "toggled", G_CALLBACK(radio_render_video_handler), (gpointer) videodata);
  g_signal_connect(GTK_RADIO_BUTTON(radio_gen_frames), "toggled", G_CALLBACK(radio_gen_frames_handler), (gpointer) videodata);

  g_signal_connect(GTK_ENTRY(entry_choose_spans[0]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[1]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);

  config_spans_start_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  config_spans_final_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), label_choose_start_span, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), entry_choose_spans[0], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), entry_choose_spans[1], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_start_hbox), button_config_span_set_ratio, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_final_hbox), label_choose_final_span, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_final_hbox), entry_choose_spans[2], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_final_hbox), entry_choose_spans[3], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_spans_final_hbox), radio_render_video, false, false, 0);


  //Config center point
  GtkWidget *label_choose_center_point = gtk_label_new("Choose center point:");
  gtk_label_set_justify(GTK_LABEL(label_choose_center_point), GTK_JUSTIFY_RIGHT);
  gtk_widget_set_size_request(label_choose_center_point, default_label_size, 0);
  entry_choose_center_point[0] = gtk_entry_new(); gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_center_point[0]), default_entry_size);
  entry_choose_center_point[1] = gtk_entry_new(); gtk_entry_set_width_chars(GTK_ENTRY(entry_choose_center_point[1]), default_entry_size);

  strbuf = g_strdup_printf("%.16g", complex_plane_get_center_real(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_center_point[0]), strbuf); free(strbuf);
  strbuf = g_strdup_printf("%.16g", complex_plane_get_center_imag(cp));
  gtk_entry_set_text(GTK_ENTRY(entry_choose_center_point[1]), strbuf); free(strbuf);

  gtk_widget_set_name(entry_choose_center_point[0], "6");
  gtk_widget_set_name(entry_choose_center_point[1], "7");
  g_signal_connect(GTK_ENTRY(entry_choose_center_point[0]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(entry_choose_center_point[1]), "changed", G_CALLBACK(generate_video_input_handler), (gpointer) cp);

  config_center_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_center_hbox), label_choose_center_point, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_center_hbox), entry_choose_center_point[0], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_center_hbox), entry_choose_center_point[1], false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_center_hbox), radio_gen_frames, false, false, 0);

  //Config frames fps and speed hbox
  GtkWidget *label_fps = gtk_label_new("FPS:");
  GtkWidget *label_video_duration = gtk_label_new(NULL);
  GtkWidget *label_video_frames = gtk_label_new(NULL);
  gtk_label_set_justify(GTK_LABEL(label_fps), GTK_JUSTIFY_RIGHT);
  gtk_widget_set_size_request(label_fps, default_label_size, 0);

  entry_fps = gtk_entry_new(); gtk_entry_set_width_chars(GTK_ENTRY(entry_fps), default_entry_size);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_fps), "Final video FPS");
  gtk_widget_set_tooltip_text(entry_fps, "Final video frames per second");
  gtk_entry_set_text(GTK_ENTRY(entry_fps), default_fps);

  config_frames_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_frames_hbox), label_fps, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_frames_hbox), entry_fps, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_frames_hbox), label_video_duration, true, false, 0);


  //Zoomratio hbox
  GtkWidget *label_zoomratio = gtk_label_new("Zoom ratio:");
  gtk_label_set_justify(GTK_LABEL(label_zoomratio), GTK_JUSTIFY_RIGHT);
  gtk_widget_set_size_request(label_zoomratio, default_label_size, 0);

  entry_zoomratio = gtk_entry_new(); gtk_entry_set_width_chars(GTK_ENTRY(entry_zoomratio), default_entry_size);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_zoomratio), "Zoom ratio");
  gtk_widget_set_tooltip_text(entry_zoomratio, "Zoom ratio in each frame");
  gtk_entry_set_text(GTK_ENTRY(entry_zoomratio), default_zoomratio);

  config_zoomratio_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(config_zoomratio_hbox), label_zoomratio, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_zoomratio_hbox), entry_zoomratio, false, false, 0);
  gtk_box_pack_start(GTK_BOX(config_zoomratio_hbox), label_video_frames, true, false, 0);

  //Config vbox
  config_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(config_vbox), label_config_window, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_resolution_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_spans_start_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_spans_final_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_center_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_frames_hbox, true, true, 0);
  gtk_box_pack_start(GTK_BOX(config_vbox), config_zoomratio_hbox, true, true, 0);


  //Folder input
  GtkWidget *folder_input;
  GtkWidget *button_choose_folder;

  folder_input = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(folder_input), "Output folder");
  button_choose_folder = gtk_button_new_with_label("Explore");
  { GtkWidget *icon = gtk_image_new_from_icon_name("folder", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_choose_folder), icon); }
  g_signal_connect(button_choose_folder, "clicked", G_CALLBACK(gui_gen_video_choose_folder), (gpointer) folder_input);
  gtk_widget_set_size_request(button_choose_folder, default_buttons_size, 20);

  folder_input_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(folder_input_hbox), folder_input, true, true, 0);
  gtk_box_pack_start(GTK_BOX(folder_input_hbox), button_choose_folder, false, false, 0);


  //Filename/Accept/Cancel bar
  GtkWidget *file_input;
  GtkWidget *button_cancel;
  GtkWidget *button_begin_render;


  file_input = gtk_entry_new();

  // progress_bar_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  vertical_separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
  progress_bar = gtk_progress_bar_new();
  gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), true);
  // gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.5);
  // gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "Render video progress");


  unsigned char *empty = calloc(175 * 311 * 3 * sizeof(char), 1);
  GdkPixbuf *empty_pixbuf = gdk_pixbuf_new_from_data(
      empty,
      GDK_COLORSPACE_RGB,
      0,
      8,
      311, 175,
      311 * 3,
      NULL, NULL);
  video_preview = gtk_image_new_from_pixbuf(empty_pixbuf);

  video_input_widgets[0]  = entry_choose_resolution[0];
  video_input_widgets[1]  = entry_choose_resolution[1];
  video_input_widgets[2]  = entry_choose_spans[0];
  video_input_widgets[3]  = entry_choose_spans[1];
  video_input_widgets[4]  = entry_choose_spans[2];
  video_input_widgets[5]  = entry_choose_spans[3];
  video_input_widgets[6]  = entry_choose_center_point[0];
  video_input_widgets[7]  = entry_choose_center_point[1];
  video_input_widgets[8]  = folder_input;
  video_input_widgets[9]  = file_input;
  video_input_widgets[10] = progress_bar;
  video_input_widgets[11] = video_preview;
  video_input_widgets[12] = entry_fps;
  video_input_widgets[13] = entry_zoomratio;


  duration_calc_widgets[0] = entry_choose_spans[0];
  duration_calc_widgets[1] = entry_choose_spans[1];
  duration_calc_widgets[2] = entry_choose_spans[2];
  duration_calc_widgets[3] = entry_choose_spans[3];
  duration_calc_widgets[4] = entry_zoomratio;
  duration_calc_widgets[5] = entry_fps;
  duration_calc_widgets[6] = label_video_duration;
  duration_calc_widgets[7] = label_video_frames;

  videodata->option_widgets = video_input_widgets;
  videodata->cp = cp;
  videodata->stop = false;

  gtk_entry_set_placeholder_text(GTK_ENTRY(file_input), "Output file name");
  button_cancel = gtk_button_new_with_label("Close");
  { GtkWidget *icon = gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_cancel), icon); }

  button_begin_render = gtk_button_new_with_label("Render");
  { GtkWidget *icon = gtk_image_new_from_icon_name("media-record", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_begin_render), icon); }

  g_signal_connect(button_cancel, "clicked", G_CALLBACK(button_cancel_render_handler), (gpointer) videodata);
  g_signal_connect(button_cancel, "clicked", G_CALLBACK(destroy_cp_from_gpointer), (gpointer) cp);
  g_signal_connect(button_cancel, "clicked", G_CALLBACK(destroy), (gpointer) zoom_window);
  gtk_widget_set_size_request(button_cancel, default_buttons_size, 20);
  gtk_widget_set_size_request(button_begin_render, default_buttons_size, 20);

  file_input_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(file_input_hbox), file_input, true, true, 0);
  gtk_box_pack_start(GTK_BOX(file_input_hbox), button_begin_render, false, false, 0);
  gtk_box_pack_start(GTK_BOX(file_input_hbox), button_cancel, false, false, 0);

  g_signal_connect(button_begin_render, "clicked", G_CALLBACK(render_video_handler), (gpointer) videodata);

  g_signal_connect(button_config_span_set_ratio, "clicked", G_CALLBACK(gui_gen_video_lock_span_ratio), (gpointer) video_input_widgets);
  gui_gen_video_lock_span_ratio(NULL, (gpointer) video_input_widgets);

  //Main vbox
  main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_end(GTK_BOX(main_vbox), file_input_hbox, false, false, 0);
  gtk_box_pack_end(GTK_BOX(main_vbox), folder_input_hbox, false, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), config_vbox, false, false, 0);

  main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(main_hbox), main_vbox, true, true, 0);


  //pause resume stop
  button_pause =  gtk_button_new_with_label("Pause");
  { GtkWidget *icon = gtk_image_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_pause), icon); }
  button_resume = gtk_button_new_with_label("Resume");
  { GtkWidget *icon = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_resume), icon); }
  button_stop =   gtk_button_new_with_label("Stop");
  { GtkWidget *icon = gtk_image_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_MENU);
  gtk_button_set_image(GTK_BUTTON(button_stop), icon); }

  gtk_widget_set_tooltip_text(button_pause, "Pauses the render process.");
  gtk_widget_set_tooltip_text(button_resume, "If paused, resumes the render process.");
  gtk_widget_set_tooltip_text(button_stop, "Stops the render process and saves video at the current state");

  gtk_widget_set_size_request(button_pause, prs_buttons_size, 0);
  gtk_widget_set_size_request(button_resume, prs_buttons_size, 0);
  gtk_widget_set_size_request(button_stop, prs_buttons_size, 0);

  g_signal_connect(button_pause, "clicked", G_CALLBACK(button_pause_render_handler), (gpointer) videodata);
  g_signal_connect(button_resume, "clicked", G_CALLBACK(button_resume_render_handler), (gpointer) videodata);
  g_signal_connect(button_stop, "clicked", G_CALLBACK(button_cancel_render_handler), (gpointer) videodata);

  pause_resume_stop_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(pause_resume_stop_hbox), button_pause, true, false, 0);
  gtk_box_pack_start(GTK_BOX(pause_resume_stop_hbox), button_resume, true, false, 0);
  gtk_box_pack_start(GTK_BOX(pause_resume_stop_hbox), button_stop, true, false, 0);

  //Video preview
  GtkWidget *preview_label = gtk_label_new("Video preview:");
  preview_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(preview_vbox), preview_label, false, false, 0);
  gtk_box_pack_start(GTK_BOX(preview_vbox), video_preview, true, true, 0);

  gtk_box_pack_end(GTK_BOX(preview_vbox), pause_resume_stop_hbox, false, false, 0);
  gtk_box_pack_end(GTK_BOX(preview_vbox), progress_bar, true, true, 0);
  // gtk_box_pack_end(GTK_BOX(preview_vbox), progress_bar_separator, true, true, 3);


  gtk_box_pack_start(GTK_BOX(main_hbox), vertical_separator, true, true, 3);
  gtk_box_pack_start(GTK_BOX(main_hbox), preview_vbox, true, true, 0);

  gtk_container_add(GTK_CONTAINER(zoom_window), main_hbox);

  //Accel group //shortcuts
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(zoom_window), accel_group);
  gtk_widget_add_accelerator(button_cancel, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);

  calculate_video_duration(NULL, (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_fps), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_zoomratio), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[0]), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[1]), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[2]), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);
  g_signal_connect(GTK_ENTRY(entry_choose_spans[3]), "changed", G_CALLBACK(calculate_video_duration), (gpointer) duration_calc_widgets);

  gtk_window_set_keep_above(GTK_WINDOW(zoom_window), true);
  gtk_widget_show_all(GTK_WIDGET(zoom_window));
}

//---Main window input handlers
void input_spanx_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_spanx(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void input_spany_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_spany(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void input_N_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_iterations(cp, atoi(gtk_entry_get_text(GTK_ENTRY(w))));
}
void input_N_line_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_line_iterations(cp, atoi(gtk_entry_get_text(GTK_ENTRY(w))));
}
void input_z0_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_quadratic_parameter_real(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void input_z1_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_quadratic_parameter_imag(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void input_center0_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_center_real(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void input_center1_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_center_imag(cp, strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL));
}
void reset_view_handler(GtkWidget *w, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_spany(cp, 3);
  complex_plane_set_center(cp, 0);
}
void combo_plot_type_handler(GtkWidget *w, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  if (complex_plane_get_id(cp) == COMPLEX_PLANE_THUMBNAIL_ID){  //This complex plane is our thumbnail which has oposite plot type
    if (strcmp(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(w)), "Parameter space") == 0){
      complex_plane_set_plot_type(cp, COMPLEX_PLANE_DYNAMIC_PLANE);
    } else {
      complex_plane_set_plot_type(cp, COMPLEX_PLANE_PARAMETER_SPACE);
    }
  } else {  //Main complex plane
    if (strcmp(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(w)), "Parameter space") == 0){
      complex_plane_set_plot_type(cp, COMPLEX_PLANE_PARAMETER_SPACE);
    } else {
      complex_plane_set_plot_type(cp, COMPLEX_PLANE_DYNAMIC_PLANE);
    }
  }
}
void combo_function_type_handler(GtkWidget *w, gpointer data){
  int function_type = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];
  ComplexPlane *th = planes[1];
  complex_plane_set_function_type(cp, function_type);
  complex_plane_set_function_type(th, function_type);

  ////TODO: Fix being able to draw main widnow from here
  // draw_main_window(w, data);
}
void combo_polynomial_parameter_handler(GtkWidget *widget, gpointer data){
  ComplexPlane *p = (ComplexPlane *) data;
  if (complex_plane_set_polynomial_parameter(p, gtk_combo_box_get_active(GTK_COMBO_BOX(widget))) == -1){
    printf("Error changing polynomial parameter!\n");
  }
}
void toggle_draw_complex_plane_handler(GtkWidget *widget, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_drawing_active(cp, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}
void toggle_draw_lines_handler(GtkWidget *widget, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  complex_plane_set_drawing_lines_active(cp, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}
void save_polynomial_handler(GtkWidget *widget, GdkEventKey *event, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;

  save_polynomial_member(widget, planes[0]);
  save_polynomial_member(widget, planes[1]);

  #ifdef DEBUG_GUI
  complex_plane_print(planes[0]);

  complex_plane_print_polynomial(planes[0]);
  complex_plane_print_polynomial_derivative(planes[0]);
  #endif //DEBUG_GUI

  if (event != NULL && strcmp(gdk_keyval_name (event->keyval), "Return") == 0){
    draw_from_options(widget, data);
  }
}
void button_mandelbrot_handler(GtkWidget *widget, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];
  ComplexPlane *thumb_cp = planes[1];
  complex_plane_set_plot_type(thumb_cp, COMPLEX_PLANE_DYNAMIC_PLANE);

  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  int w, h;
  gtk_window_get_size(window, &w, &h);

  complex_plane_set_dimensions(cp, fmax(w-385, 0), h);
  complex_plane_set_mandelbrot_parameters(cp);

  complex_plane_gen_plot(cp);

  draw_main_window(widget, data);
}
void input_parameters_vector_handler(GtkWidget *widget, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;
  int param = gtk_combo_box_get_active(GTK_COMBO_BOX(widget)) - 1;
  int index = atoi(gtk_widget_get_name(widget));

  complex_plane_set_parameters_vector_member(cp, param, index);
  #ifdef DEBUG_GUI
  complex_plane_print(cp);
  #endif //DEBUG_GUI
}


//Misc
void draw_from_options(GtkWidget *widget, gpointer data){
  int w, h;
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];

  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  gtk_window_get_size(window, &w, &h);

  complex_plane_set_dimensions(cp, fmax(w-385, 0), h);
  complex_plane_adjust_span_ratio(cp);
  complex_plane_gen_plot(cp);
  draw_main_window(widget, data);
}

void quit_app(GtkWidget *w, gpointer data){
  gtk_main_quit();
}

void plot_zoom(GtkWidget *widget, double zoomratio, complex double p, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];
  #ifdef DEBUG_GUI
  printf("Zooming in %f %f\n", creal(p), cimag(p));
  #endif

  complex_plane_set_center(cp, p);
  complex_plane_set_spanx(cp,  complex_plane_get_spanx(cp) * zoomratio);
  complex_plane_set_spany(cp,  complex_plane_get_spany(cp) * zoomratio);

  draw_from_options(widget, data);
}

void zoom_button_handler(GtkWidget *widget, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];

  // gchar *arg = g_strdup_printf("%s", gtk_button_get_label(GTK_BUTTON(widget)));
  const gchar *arg = gtk_widget_get_name(widget);
  double zoomratio = 1;
  if (strcmp(arg, "button_zoomin") == 0){
    zoomratio = 0.5;
  } else if (strcmp(arg, "button_zoomout") == 0){
    zoomratio = 2;
  } else {
    return;
  }
  plot_zoom(widget, zoomratio, complex_plane_get_center(cp), data);
}

void draw_thumbnail_gui(GtkWidget *widget, double x, double y, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));

  ComplexPlane *cp_thumb = ((ComplexPlane **) data)[1];

  gtk_widget_destroy(thumb_img);
  complex_plane_set_quadratic_parameter(cp_thumb, x + y*I);
  complex_plane_set_numerical_method_a(cp_thumb, x + y*I);

  // double span = complex_plane_thumbnail_get_span(cp_thumb);
  // complex_plane_set_spanx(cp_thumb, span);
  // complex_plane_set_spany(cp_thumb, span);
  // complex_plane_adjust_span_ratio(cp_thumb);

  complex_plane_gen_plot(cp_thumb);

  int w = complex_plane_get_width(cp_thumb), h = complex_plane_get_height(cp_thumb), stride = complex_plane_get_stride(cp_thumb);
  GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
      complex_plane_get_plot(cp_thumb),
      GDK_COLORSPACE_RGB,
      0,
      8,
      w, h,
      w * stride,
      NULL, NULL);
  thumb_img = gtk_image_new_from_pixbuf(thumbBuf);

  gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);

  // draw_main_window(widget, data);
  gtk_widget_show_all(GTK_WIDGET(window));
}

void cp_mouse_handler(GtkWidget *event_box, GdkEventButton *event, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(event_box));

  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];
  ComplexPlane *cp_thumb = planes[1];

  double h_ev = gtk_widget_get_allocated_height(event_box);
  int h = complex_plane_get_height(cp);

  event->y = event->y + (h - h_ev)/2;

  double x = (event->x-0)/(complex_plane_get_width(cp)-0)
           * complex_plane_get_spanx(cp)
           + complex_plane_get_spanx0(cp);

  double y = (-(event->y - complex_plane_get_height(cp))-0)
           / (complex_plane_get_height(cp)-0)
           * complex_plane_get_spany(cp)
           + complex_plane_get_spany0(cp);

  gchar *point = g_strdup_printf("%.16g %+.16gi", x, y);
  gtk_label_set_text(GTK_LABEL(point_targeted), point);
  g_free(point);


  if (complex_plane_is_drawing_active(cp_thumb)){
    if (!complex_plane_polynomial_is_null(cp_thumb)){
      complex_plane_set_polynomial_member(cp_thumb, x + y*I, complex_plane_get_polynomial_parameter(cp));
    }
    draw_thumbnail_gui(event_box, x, y, data);
  }

  switch (event->button){
    case 0:   //MOUSE MOVED -> Draw orbits / zoom box
      if (complex_plane_zoom_point1_is_null(cp)){
        if (complex_plane_is_drawing_lines_active(cp)){
          draw_sequence(event_box, event, data);
        }
      } else {
        gui_draw_zoom_box(event_box, event, (gpointer) cp);
      }
      break;
    case 1:   //LEFT BUTTON CLICKED -> Zoom Box
      if (complex_plane_zoom_point1_is_null(cp)){
        if (event->type != GDK_BUTTON_PRESS){
          break;
        }
        complex_plane_set_zoom_point1(cp, x, y);

        draw_main_window(GTK_WIDGET(window), data);


        #ifdef DEBUG_GUI
        printf("First zoom point: %.16g %.16g\n", x, y);
        #endif
      } else {
        if (event->type != GDK_BUTTON_RELEASE){
          break;
        }
        complex_plane_set_zoom_point2(cp, x, y);
        #ifdef DEBUG_GUI
        printf("Second zoom point: %.16g %.16g\n", x, y);
        #endif

        complex_plane_zoom_points_normalize(cp);

        double newspanx = complex_plane_zoom_point_get_spanx(cp);
        double newspany = complex_plane_zoom_point_get_spany(cp);

        if (newspanx == 0 && newspany == 0){
          complex_plane_free_zoom_point1(cp);
          complex_plane_free_zoom_point2(cp);
          break;
        }

        complex_plane_zoom_points_set_center(cp);

        complex_plane_set_spanx(cp, newspanx);
        complex_plane_set_spany(cp, newspany);
        complex_plane_adjust_span_ratio(cp);

        draw_from_options(event_box, data);

        complex_plane_free_zoom_point1(cp);
        complex_plane_free_zoom_point2(cp);
      }
      break;
    case 2:   //MIDDLE BUTTON CLICKED -> Change center
      if (event->type == GDK_BUTTON_RELEASE){
        break;
      }
      complex_plane_set_center(cp, x + y*I);
      draw_from_options(event_box, data);
      break;
    case 3:   //RIGHT MOUSE CLICKED -> Change parameters, redraw
      if (event->type == GDK_BUTTON_RELEASE){
        break;
      }
      complex_plane_set_default_spans(cp);
      complex_plane_set_center(cp, 0);
      switch (complex_plane_get_function_type(cp)){
        case 0:
          if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_PARAMETER_SPACE){
            complex_plane_set_quadratic_parameter(cp, x + y*I);

            complex_plane_set_plot_type(cp, COMPLEX_PLANE_DYNAMIC_PLANE);
            complex_plane_set_plot_type(cp_thumb, COMPLEX_PLANE_PARAMETER_SPACE);
          } else if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_DYNAMIC_PLANE){
            complex_plane_set_quadratic_parameter(cp, 0);

            complex_plane_set_plot_type(cp, COMPLEX_PLANE_PARAMETER_SPACE);
            complex_plane_set_plot_type(cp_thumb, COMPLEX_PLANE_DYNAMIC_PLANE);
          }

          draw_from_options(event_box, data);

          break;
        case 1:
        case 2:
          int cp_par = complex_plane_get_polynomial_parameter(cp);
          int th_par = complex_plane_get_polynomial_parameter(cp_thumb);
          complex_plane_set_polynomial_member(cp, x + y*I, cp_par);
          complex_plane_set_polynomial_parameter(cp, th_par);
          complex_plane_set_polynomial_parameter(cp_thumb, cp_par);
          draw_from_options(event_box, data);
          break;
        case 3:
          if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_PARAMETER_SPACE){
            complex_plane_set_numerical_method_a(cp, x + y*I);

            complex_plane_set_plot_type(cp, COMPLEX_PLANE_DYNAMIC_PLANE);
            complex_plane_set_plot_type(cp_thumb, COMPLEX_PLANE_PARAMETER_SPACE);
          } else if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_DYNAMIC_PLANE){
            complex_plane_set_numerical_method_a(cp, 0);

            complex_plane_set_plot_type(cp, COMPLEX_PLANE_PARAMETER_SPACE);
            complex_plane_set_plot_type(cp_thumb, COMPLEX_PLANE_DYNAMIC_PLANE);
          }

          draw_from_options(event_box, data);

      }
      break;
  }
}

void gui_draw_zoom_box(GtkWidget *event_box, GdkEventButton *event, gpointer data){
  ComplexPlane *cp = (ComplexPlane *) data;

  int x0 = event->x;
  int y0 = event->y;
  int w = complex_plane_get_width(cp), h = complex_plane_get_height(cp);
  // double spanx = complex_plane_get_spanx(cp), spany = complex_plane_get_spany(cp);

  int x1 = complex_plane_zoom_point1_get_pixel_value_x(cp);
  int y1 = complex_plane_zoom_point1_get_pixel_value_y(cp);

  complex_plane_free_drawn_plot(cp);

  complex_plane_alloc_drawn_plot(cp);
  complex_plane_copy_plot(cp);

  unsigned char *drawn_plot = complex_plane_get_drawn_plot(cp);

  draw_line(drawn_plot, x0, y0, x0, y1, w, h);
  draw_line(drawn_plot, x0, y0, x1, y0, w, h);
  draw_line(drawn_plot, x0, y1, x1, y1, w, h);
  draw_line(drawn_plot, x1, y0, x1, y1, w, h);

  clear_container(event_box);

  GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
      drawn_plot,
      GDK_COLORSPACE_RGB,
      0,
      8,
      w, h,
      w * 3,
      NULL, NULL);
  GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);
  gtk_container_add(GTK_CONTAINER(event_box), image);
  gtk_widget_show_all(GTK_WIDGET(event_box));
}

void draw_sequence(GtkWidget *window, GdkEventButton *event, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];

  int w = complex_plane_get_width(cp), h = complex_plane_get_height(cp);
  double x = (double) event->x/(double) w * (complex_plane_get_spanx(cp)) + complex_plane_get_spanx0(cp);
  double y = -((double) event->y - (double) h)/(double) h * (complex_plane_get_spany(cp)) + complex_plane_get_spany0(cp);

  #ifdef DEBUG_GUI
  printf("Plot %f + %fI\n", x, y);
  printf("Motion notify\n");
  printf("%f, %f\n", event->x, event->y);
  printf("Pointing at: %f, %f\n", x, y);
  #endif

  double p[2] = {x, y};

  complex_plane_free_drawn_plot(cp);

  complex_plane_alloc_drawn_plot(cp);
  complex_plane_copy_plot(cp);

  switch(complex_plane_get_function_type(cp)){
    case 0:
      draw_sequence_lines(cp, p, w, h);
      break;
    case 1:
      if (complex_plane_get_polynomial_order(cp) != -1){
        draw_sequence_lines_polynomial(cp, p, w, h);
      }
      break;
    case 2:
      if (complex_plane_get_polynomial_order(cp) != -1){
        draw_sequence_lines_newton(cp, p, w, h);
      }
  }

  clear_container(window);

  GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
      complex_plane_get_drawn_plot(cp),
      GDK_COLORSPACE_RGB,
      0,
      8,
      w, h,
      w * 3,
      NULL, NULL);
  GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);
  gtk_container_add(GTK_CONTAINER(window), image);
  gtk_widget_show_all(GTK_WIDGET(window));
}

void draw_main_window(GtkWidget *widget, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(widget));
  clear_container(GTK_WIDGET(window));

  gint w, h;
  gtk_window_get_size(window, &w, &h);

  ComplexPlane **planes = (ComplexPlane **) data;

  ComplexPlane *cp = planes[0];
  ComplexPlane *cp_thumb = planes[1];

  char *strbuf;

  //Generate complex_plane
  if (cp == NULL){
    planes[0] = complex_plane_new(&cp);

    complex_plane_set_dimensions(cp, fmax(w-385, 0), h);
    complex_plane_set_stride(cp, 3);

    complex_plane_set_mandelbrot_parameters(cp);

    complex_plane_gen_plot(cp);
  }

  //Define all local widgets
  GtkAccelGroup *accel_group;

  //Input widgets
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

  GtkWidget *check_draw_lines;

  GtkWidget *button_reset_view;

  GtkWidget *hbox;
  GtkWidget *menu_vbox;
  GtkWidget *aux_hbox;
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
  GtkWidget *button_zoomin;
  GtkWidget *button_zoomout;

  //Arbitrary polynomial configuration
  GtkWidget *input_polynomial_order;
  GtkWidget *polynomial_config_vbox;
  GtkWidget *polynomial_scroll_box;
  GtkWidget *polynomial_config_hbox;
  GtkWidget *polynomial_order_box;

  GtkWidget *combo_polynomial_parameter;
  GtkWidget *combo_polynomial_parameter_thumb = gtk_combo_box_text_new();

  GtkWidget *plot_options_label;

  //Menu
  GtkWidget *menu_menubar;

  //File menu
  GtkWidget *menu_filemenu;
  GtkWidget *menu_fileMi;
  GtkWidget *menu_button_save_plot;
  GtkWidget *menu_button_render_video;
  GtkWidget *menu_separator_quit;
  GtkWidget *menu_button_quit;

  //Edit menu
  GtkWidget *menu_editmenu;
  GtkWidget *menu_editMi;
  GtkWidget *menu_button_preferences;

  //Help menu
  GtkWidget *menu_helpmenu;
  GtkWidget *menu_helpMi;
  GtkWidget *menu_button_help;
  GtkWidget *menu_button_about;

  //Initialize input widgets
  input_N = gtk_entry_new();
  input_N_line = gtk_entry_new();
  input_z0 = gtk_entry_new();
  input_z1 = gtk_entry_new();
  input_center0 = gtk_entry_new();
  input_center1 = gtk_entry_new();

  input_plot_type = gtk_combo_box_text_new();
  input_spanx = gtk_entry_new();
  input_spany = gtk_entry_new();

  combo_function_type = gtk_combo_box_text_new();

  point_targeted = gtk_label_new("0 + 0i");

  //Create boxes
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  menu_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  aux_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
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

  //Main menu
  menu_menubar = gtk_menu_bar_new();
  menu_filemenu = gtk_menu_new();
  menu_editmenu = gtk_menu_new();
  menu_helpmenu = gtk_menu_new();

  //File submenu
  menu_fileMi = gtk_menu_item_new_with_label("File");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_menubar), menu_fileMi);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_fileMi), menu_filemenu);

  {
    menu_button_save_plot = gtk_image_menu_item_new_with_label("Save plot as...");
    g_signal_connect(menu_button_save_plot, "activate", G_CALLBACK(save_plot_handler), (gpointer) cp);
    GtkWidget *icon = gtk_image_new_from_icon_name("document-save-as", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_save_plot), icon);
  }

  {
    menu_button_render_video = gtk_image_menu_item_new_with_label("Generate video zoom");
    g_signal_connect(menu_button_render_video, "activate", G_CALLBACK(generate_video_zoom), (gpointer) cp);
    GtkWidget *icon = gtk_image_new_from_icon_name("video-x-generic", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_render_video), icon);
  }

  menu_separator_quit = gtk_separator_menu_item_new();

  {
    menu_button_quit = gtk_image_menu_item_new_with_label("Quit");
    g_signal_connect(menu_button_quit, "activate", G_CALLBACK(quit_app), NULL);
    GtkWidget *icon = gtk_image_new_from_icon_name("window-close", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_quit), icon);
  }

  gtk_menu_shell_append(GTK_MENU_SHELL(menu_filemenu), menu_button_save_plot);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_filemenu), menu_button_render_video);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_filemenu), menu_separator_quit);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_filemenu), menu_button_quit);

  //Edit submenu
  menu_editMi = gtk_menu_item_new_with_label("Edit");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_menubar), menu_editMi);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_editMi), menu_editmenu);

  {
    menu_button_preferences = gtk_image_menu_item_new_with_label("Preferences...");
    g_signal_connect(menu_button_preferences, "activate", G_CALLBACK(gui_templates_show_preferences_window), (gpointer) planes);
    GtkWidget *icon = gtk_image_new_from_icon_name("applications-system", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_preferences), icon);
  }

  gtk_menu_shell_append(GTK_MENU_SHELL(menu_editmenu), menu_button_preferences);

  //Help submenu
  menu_helpMi = gtk_menu_item_new_with_label("Help");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_menubar), menu_helpMi);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_helpMi), menu_helpmenu);

  {
    menu_button_help = gtk_image_menu_item_new_with_label("Help");
    g_signal_connect(menu_button_help, "activate", G_CALLBACK(gui_templates_show_help_window), NULL);
    GtkWidget *icon = gtk_image_new_from_icon_name("help-contents", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_help), icon);
  }

  {
    menu_button_about = gtk_image_menu_item_new_with_label("About");
    GtkWidget *icon = gtk_image_new_from_icon_name("help-about", 16);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_button_about), icon);
    g_signal_connect(menu_button_about, "activate", G_CALLBACK(gui_templates_show_about_window), NULL);
  }

  gtk_menu_shell_append(GTK_MENU_SHELL(menu_helpmenu), menu_button_help);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_helpmenu), menu_button_about);



  //Iter box
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_N), "Iter N");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_N_line), "Number of lines");
  gtk_widget_set_tooltip_text(input_N, "Number of iterations to plot");
  gtk_widget_set_tooltip_text(input_N_line, "Number of iterations to plot");
  gtk_box_pack_start(GTK_BOX(iter_box), input_N, true, false, 0);
  gtk_box_pack_start(GTK_BOX(iter_box), input_N_line, true, false, 0);
  g_signal_connect(input_N, "key-release-event", G_CALLBACK(input_N_handler), (gpointer) cp);
  g_signal_connect(input_N_line, "key-release-event", G_CALLBACK(input_N_line_handler), (gpointer) cp);

  //Z box
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_z0), "z0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_z1), "z1");
  gtk_widget_set_tooltip_text(input_z0, "Real part of z");
  gtk_widget_set_tooltip_text(input_z1, "Imag part of z");
  gtk_box_pack_start(GTK_BOX(z_box), input_z0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(z_box), input_z1, true, false, 0);
  g_signal_connect(input_z0, "key-release-event", G_CALLBACK(input_z0_handler), (gpointer) cp);
  g_signal_connect(input_z1, "key-release-event", G_CALLBACK(input_z1_handler), (gpointer) cp);

  //Center box
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_center0), "Center0");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_center1), "Center1");
  gtk_widget_set_tooltip_text(input_center0, "Real part of center");
  gtk_widget_set_tooltip_text(input_center1, "Imag part of center");
  gtk_box_pack_start(GTK_BOX(center_box), input_center0, true, false, 0);
  gtk_box_pack_start(GTK_BOX(center_box), input_center1, true, false, 0);
  g_signal_connect(input_center0, "key-release-event", G_CALLBACK(input_center0_handler), (gpointer) cp);
  g_signal_connect(input_center1, "key-release-event", G_CALLBACK(input_center1_handler), (gpointer) cp);

  //Span box
  gtk_box_pack_start(GTK_BOX(span_box), input_spanx, true, false, 0);
  gtk_box_pack_start(GTK_BOX(span_box), input_spany, true, false, 0);
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_spanx), "Span X");
  gtk_entry_set_placeholder_text(GTK_ENTRY(input_spany), "Span Y");
  g_signal_connect(GTK_ENTRY(input_spanx), "key-release-event", G_CALLBACK(input_spanx_handler), (gpointer) cp);
  g_signal_connect(GTK_ENTRY(input_spany), "key-release-event", G_CALLBACK(input_spany_handler), (gpointer) cp);
  // g_signal_connect(G_OBJECT(main_data->input_spanx), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
  // g_signal_connect(G_OBJECT(main_data->input_spany), "insert-text", G_CALLBACK(insert_text_event_float), NULL);

  //Plot type box
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_function_type), NULL, "Quadratic");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_function_type), NULL, "Polynomial");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_function_type), NULL, "Newton's Fractal");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_function_type), NULL, "Newton's Method");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_function_type), complex_plane_get_function_type(cp));
  gtk_widget_set_tooltip_text(combo_function_type, "Set type of function to plot");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_plot_type), NULL, "Parameter space");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_plot_type), NULL, "Dynamic plane");
  gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), 0);
  gtk_widget_set_tooltip_text(input_plot_type, "Parameter space or Dynamic plane");
  gtk_box_pack_start(GTK_BOX(plot_type_box), combo_function_type, true, true, 0);
  gtk_box_pack_start(GTK_BOX(plot_type_box), input_plot_type, true, true, 0);
  gtk_widget_set_size_request(input_plot_type, 168, 20);
  gtk_widget_set_size_request(combo_function_type, 168, 20);
  g_signal_connect(combo_function_type, "changed", G_CALLBACK(combo_function_type_handler), (gpointer) planes);
  // g_signal_connect(combo_function_type, "changed", G_CALLBACK(draw_main_window), (gpointer) planes);
  g_signal_connect(input_plot_type, "changed", G_CALLBACK(combo_plot_type_handler), (gpointer) cp);
  g_signal_connect(input_plot_type, "changed", G_CALLBACK(combo_plot_type_handler), (gpointer) cp_thumb);

  //Draw/Clear box
  button_draw = gtk_button_new_with_label("Draw");
  button_clear = gtk_button_new_with_label("Clear");
  gtk_widget_set_tooltip_text(button_draw, "Draw plane from options specified");
  gtk_widget_set_tooltip_text(button_clear, "Redraw Complex Plane without drawings");
  g_signal_connect(button_draw, "clicked", G_CALLBACK(draw_from_options), (gpointer) planes);
  g_signal_connect(button_clear, "clicked", G_CALLBACK(draw_main_window), (gpointer) planes);
  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_draw, true, true, 0);
  gtk_box_pack_start(GTK_BOX(draw_apply_box), button_clear, true, true, 0);

  //Zoom box
  button_zoomin = gtk_button_new_with_label(" + ");
  button_zoomout = gtk_button_new_with_label("  -  ");
  gtk_widget_set_name(button_zoomin, "button_zoomin");
  gtk_widget_set_name(button_zoomout, "button_zoomout");
  gtk_widget_set_tooltip_text(button_zoomin, "Zoom in");
  gtk_widget_set_tooltip_text(button_zoomout, "Zoom out");
  g_signal_connect(button_zoomin, "clicked", G_CALLBACK(zoom_button_handler),  (gpointer) planes);
  g_signal_connect(button_zoomout, "clicked", G_CALLBACK(zoom_button_handler),  (gpointer) planes);
  gtk_box_pack_start(GTK_BOX(zoom_box), button_zoomin, false, false, 0);
  gtk_box_pack_start(GTK_BOX(zoom_box), button_zoomout, false, false, 0);

  //Draw lines
  check_draw_lines = gtk_check_button_new_with_mnemonic("Display _Orbits");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_draw_lines), complex_plane_is_drawing_lines_active(cp));
  g_signal_connect(check_draw_lines, "toggled", G_CALLBACK(toggle_draw_lines_handler), (gpointer) cp);

  //Reset viewport
  button_reset_view = gtk_button_new_with_label("Reset view");
  g_signal_connect(button_reset_view, "clicked", G_CALLBACK(reset_view_handler), (gpointer) cp);
  g_signal_connect(button_reset_view, "clicked", G_CALLBACK(draw_from_options), (gpointer) planes);

  //Zoom/Lines box
  gtk_box_pack_start(GTK_BOX(zoom_lines_box), zoom_box, false, false, 0);
  gtk_box_pack_start(GTK_BOX(zoom_lines_box), check_draw_lines, true, false, 0);
  gtk_box_pack_start(GTK_BOX(zoom_lines_box), button_reset_view, true, true, 0);

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

  //Accelerators
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
  gtk_widget_add_accelerator(menu_button_quit, "activate", accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(menu_button_save_plot, "activate", accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(menu_button_render_video, "activate", accel_group, GDK_KEY_s, GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

  gtk_widget_add_accelerator(menu_button_help, "activate", accel_group, GDK_KEY_F1, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(menu_button_about, "activate", accel_group, GDK_KEY_question, 0, GTK_ACCEL_VISIBLE);

  gtk_widget_add_accelerator(button_zoomin, "clicked", accel_group, GDK_KEY_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(button_zoomout, "clicked", accel_group, GDK_KEY_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);


 //configure scroll_box
 scroll_box = gtk_scrolled_window_new(NULL, NULL);
 gtk_container_add(GTK_CONTAINER(scroll_box), vbox);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_box), GTK_POLICY_NEVER, GTK_POLICY_EXTERNAL);
 gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(scroll_box), true);

 //Populate vbox
 gtk_box_pack_start(GTK_BOX(vbox), options_box, true, false, 0);

  //Gui depending on function type
  int ftype = complex_plane_get_function_type(cp);
  switch (ftype){
    case 0:   //Quadratic
      button_mandelbrot = gtk_button_new_with_label("Default Mandelbrot");
      gtk_widget_set_tooltip_text(button_mandelbrot, "Generate and plot Mandelbrot Set");
      g_signal_connect(button_mandelbrot, "clicked", G_CALLBACK(button_mandelbrot_handler), (gpointer) planes);

      gtk_box_pack_start(GTK_BOX(vbox), button_mandelbrot, true, false, 0);
      break;
    case 1:   //Polynomial
    case 2:   //Newton fractal
    case 3:   //Newton method
      int polynomial_order = complex_plane_get_polynomial_order(cp);

      polynomial_config_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
      polynomial_order_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);


      input_polynomial_order = gtk_entry_new();
      gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_order), "Order of polynomial");
      gtk_widget_set_tooltip_text(input_polynomial_order, "Order of polynomial to plot");
      g_signal_connect(G_OBJECT(input_polynomial_order), "insert-text", G_CALLBACK(insert_text_event_int), NULL);
      g_signal_connect(input_polynomial_order, "key-release-event", G_CALLBACK(change_polynomial_order), (gpointer) planes);

      if (polynomial_order > 0){
        GtkWidget **input_polynomial_settings_vector_real;
        GtkWidget **input_polynomial_settings_vector_imag;
        GtkWidget **input_polynomial_settings_vector_critical;
        GtkWidget **input_parameters_vector;
        if (ftype == 3){    //If newtons method we use array of parameters
          input_parameters_vector = malloc(sizeof(GtkWidget *) * (polynomial_order +2));
        }

        const complex double *polynomial = complex_plane_get_polynomial(cp);
        const complex double *critical   = complex_plane_get_critical(cp);

        strbuf = g_strdup_printf("%d", polynomial_order);
        gtk_entry_set_text(GTK_ENTRY(input_polynomial_order), strbuf); free(strbuf);

        //Create hbox for input values
        polynomial_config_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

        combo_polynomial_parameter = gtk_combo_box_text_new();
        // combo_polynomial_parameter_thumb = gtk_combo_box_text_new();

        input_polynomial_settings_vector_real = malloc(sizeof(GtkWidget *) * (polynomial_order + 2));
        input_polynomial_settings_vector_imag = malloc(sizeof(GtkWidget *) * (polynomial_order + 2));
        for (int i = 0; i < polynomial_order + 2; i++){
          GtkWidget *polynomial_settings_input_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
          input_polynomial_settings_vector_real[i] = gtk_entry_new();
          input_polynomial_settings_vector_imag[i] = gtk_entry_new();
          gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_settings_vector_real[i]), "Real");
          gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_settings_vector_imag[i]), "Imag");
          gtk_entry_set_width_chars(GTK_ENTRY(input_polynomial_settings_vector_real[i]), 5);
          gtk_entry_set_width_chars(GTK_ENTRY(input_polynomial_settings_vector_imag[i]), 5);
          gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), input_polynomial_settings_vector_real[i], false, false, 0);
          gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), input_polynomial_settings_vector_imag[i], false, false, 0);
          g_signal_connect(G_OBJECT(input_polynomial_settings_vector_real[i]), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
          g_signal_connect(G_OBJECT(input_polynomial_settings_vector_imag[i]), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
          g_signal_connect(G_OBJECT(input_polynomial_settings_vector_real[i]), "key_release_event", G_CALLBACK(save_polynomial_handler), (gpointer) planes);
          g_signal_connect(G_OBJECT(input_polynomial_settings_vector_imag[i]), "key_release_event", G_CALLBACK(save_polynomial_handler), (gpointer) planes);

          GtkWidget *polynomial_labels_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

          strbuf = g_strdup_printf("r%d", i);
          gtk_widget_set_name(input_polynomial_settings_vector_real[i], strbuf); free(strbuf);
          strbuf = g_strdup_printf("i%d", i);
          gtk_widget_set_name(input_polynomial_settings_vector_imag[i], strbuf); free(strbuf);

          if (ftype == 3){  //Newton's method -> Array of parameters
            input_polynomial_settings_vector_critical = malloc(sizeof(GtkWidget *) * (polynomial_order + 2));
            input_parameters_vector[i] = gtk_combo_box_text_new();
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_parameters_vector[i]), NULL, "-");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_parameters_vector[i]), NULL, "0");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(input_parameters_vector[i]), NULL, "+");

            strbuf = g_strdup_printf("%d", i);
            gtk_widget_set_name(input_parameters_vector[i], strbuf); free(strbuf);
            g_signal_connect(input_parameters_vector[i], "changed", G_CALLBACK(input_parameters_vector_handler), (gpointer) cp);
            g_signal_connect(input_parameters_vector[i], "changed", G_CALLBACK(input_parameters_vector_handler), (gpointer) cp_thumb);

            gtk_combo_box_set_active(GTK_COMBO_BOX(input_parameters_vector[i]), creal(complex_plane_get_parameters_vector_member(cp, i)) + 1);
            gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), input_parameters_vector[i], false, false, 0);

            input_polynomial_settings_vector_critical[i] = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_settings_vector_critical[i]), "Critical");
            gtk_entry_set_width_chars(GTK_ENTRY(input_polynomial_settings_vector_critical[i]), 5);
            gtk_box_pack_start(GTK_BOX(polynomial_settings_input_vbox), input_polynomial_settings_vector_critical[i], false, false, 0);
            g_signal_connect(G_OBJECT(input_polynomial_settings_vector_critical[i]), "insert-text", G_CALLBACK(insert_text_event_float), NULL);
            g_signal_connect(G_OBJECT(input_polynomial_settings_vector_critical[i]), "key_release_event", G_CALLBACK(save_polynomial_handler), (gpointer) planes);

            strbuf = g_strdup_printf("c%d", i);
            gtk_widget_set_name(input_polynomial_settings_vector_critical[i], strbuf); free(strbuf);

            char buffer[64];
            //Set polynomial z labels
            if (i < polynomial_order + 1){
              if (i < polynomial_order -1){
                sprintf(buffer, "a^%d + ", polynomial_order -i);
              } else if (i == polynomial_order -1){
                strcpy(buffer, "a + ");
              } else if (i == polynomial_order){
                strcpy(buffer, "");
              }

              GtkWidget *c_power_label = gtk_label_new(buffer);
              gtk_box_pack_end(GTK_BOX(polynomial_labels_vbox), c_power_label, false, false, 5);

              if (i < polynomial_order -1){
                sprintf(buffer, "az^%d + ", polynomial_order -i);
              } else if (i == polynomial_order -1){
                strcpy(buffer, "az + ");
              } else if (i == polynomial_order){
                strcpy(buffer, "");
              }

              GtkWidget *a_power_label = gtk_label_new(buffer);
              gtk_box_pack_end(GTK_BOX(polynomial_labels_vbox), a_power_label, false, false, 5);
            }


            if (critical != NULL){
              if (creal(critical[i]) != 0.0){
                strbuf = g_strdup_printf("%.16g", creal(critical[i]));
                gtk_entry_set_text(GTK_ENTRY(input_polynomial_settings_vector_critical[i]), strbuf); free(strbuf);
              }
            }
          }

          if (polynomial != NULL){
            if (creal(polynomial[i]) != 0.0){
              strbuf = g_strdup_printf("%.16g", creal(polynomial[i]));
              gtk_entry_set_text(GTK_ENTRY(input_polynomial_settings_vector_real[i]), strbuf); free(strbuf);
            }
            if (cimag(polynomial[i]) != 0.0){
              strbuf = g_strdup_printf("%.16g", cimag(polynomial[i]));
              gtk_entry_set_text(GTK_ENTRY(input_polynomial_settings_vector_imag[i]), strbuf); free(strbuf);
            }
          }

          gtk_box_pack_start(GTK_BOX(polynomial_config_hbox), polynomial_settings_input_vbox, true, false, 0);
          gtk_box_pack_start(GTK_BOX(polynomial_config_hbox), polynomial_labels_vbox, true, false, 0);


          char buffer[64];
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
            gtk_box_pack_start(GTK_BOX(polynomial_labels_vbox), z_power_label, true, false, 0);
          }

          if (i < polynomial_order + 1){
            if (i < polynomial_order -1){
              sprintf(buffer, "a * z^%d", polynomial_order-i);
            } else if (i == polynomial_order -1){
              sprintf(buffer, "a * z");
            } else if (i == polynomial_order){
              sprintf(buffer, "c");
            }
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_polynomial_parameter), NULL, buffer);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_polynomial_parameter_thumb), NULL, buffer);
          }

          //Populate parameter chosing combo box
        }

        //Create scrollbox for input values
        polynomial_scroll_box = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(polynomial_scroll_box), GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
        gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(polynomial_scroll_box), true);
        gtk_container_add(GTK_CONTAINER(polynomial_scroll_box), polynomial_config_hbox);

        gtk_box_pack_end(GTK_BOX(polynomial_config_vbox), polynomial_scroll_box, true, false, 0);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_polynomial_parameter), NULL, "z");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_polynomial_parameter_thumb), NULL, "z");
        gtk_entry_set_width_chars(GTK_ENTRY(input_polynomial_order), 5);
        gtk_box_pack_end(GTK_BOX(polynomial_order_box), combo_polynomial_parameter, true, true, 0);
        gtk_widget_set_tooltip_text(combo_polynomial_parameter, "Which parameter to plot");
        gtk_widget_set_tooltip_text(combo_polynomial_parameter_thumb, "Which parameter to plot in thumbnails");
        g_signal_connect(combo_polynomial_parameter, "changed", G_CALLBACK(combo_polynomial_parameter_handler), (gpointer) cp);
        g_signal_connect(combo_polynomial_parameter_thumb, "changed", G_CALLBACK(combo_polynomial_parameter_handler), (gpointer) cp_thumb);

        gtk_box_pack_end(GTK_BOX(thumb_options_box), combo_polynomial_parameter_thumb, true, false, 0);

        if (complex_plane_get_polynomial_parameter(cp) == -1){
          complex_plane_set_polynomial_parameter(cp, polynomial_order);
          if (complex_plane_get_function_type(cp) == 1){
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_polynomial_parameter), polynomial_order);
          } else {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo_polynomial_parameter), polynomial_order + 1);
          }
        } else {
          gtk_combo_box_set_active(GTK_COMBO_BOX(combo_polynomial_parameter), complex_plane_get_polynomial_parameter(cp));
        }

        if (complex_plane_get_function_type(cp) < 3){
          int parm = complex_plane_get_polynomial_parameter(cp);
          gtk_editable_set_editable(GTK_EDITABLE(input_polynomial_settings_vector_real[parm]), false);
          gtk_widget_set_can_focus(input_polynomial_settings_vector_real[parm], false);
          gtk_widget_set_sensitive(input_polynomial_settings_vector_real[parm], false);
          gtk_editable_set_editable(GTK_EDITABLE(input_polynomial_settings_vector_imag[parm]), false);
          gtk_widget_set_can_focus(input_polynomial_settings_vector_imag[parm], false);
          gtk_widget_set_sensitive(input_polynomial_settings_vector_imag[parm], false);

          gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_settings_vector_real[parm]), "");
          gtk_entry_set_placeholder_text(GTK_ENTRY(input_polynomial_settings_vector_imag[parm]), "");
          gtk_entry_set_text(GTK_ENTRY(input_polynomial_settings_vector_real[parm]), "");
          gtk_entry_set_text(GTK_ENTRY(input_polynomial_settings_vector_imag[parm]), "");
        }
      }

      GtkWidget *order_label = gtk_label_new("Order of polynomial: ");
      gtk_box_pack_start(GTK_BOX(polynomial_order_box), order_label, false, false, 0);
      gtk_box_pack_start(GTK_BOX(polynomial_order_box), input_polynomial_order, false, false, 0);
      gtk_box_pack_start(GTK_BOX(polynomial_config_vbox), polynomial_order_box, true, false, 0);

      //Populate vbox
      gtk_box_pack_start(GTK_BOX(vbox), polynomial_config_vbox, true, false, 0);
      break;
  }
  //Thumbnails
  gtk_box_pack_start(GTK_BOX(thumb_box), point_targeted, true, false, 0);
  if (complex_plane_get_plot(cp_thumb) != NULL){
    int w = complex_plane_get_width(cp_thumb), h = complex_plane_get_height(cp_thumb), stride = complex_plane_get_stride(cp_thumb);
    GdkPixbuf *thumbBuf = gdk_pixbuf_new_from_data(
        complex_plane_get_plot(cp_thumb),
        GDK_COLORSPACE_RGB,
        0,
        8,
        w, h,
        w * stride,
        NULL, NULL);
    thumb_img = gtk_image_new_from_pixbuf(thumbBuf);

    //Widget declarations
    GtkWidget *input_thumbN;
    GtkWidget *check_draw_thumb;

    //Input iter
    input_thumbN = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_thumbN), "Iter N");
    gtk_widget_set_tooltip_text(input_thumbN, "Number of iterations to plot for thumbnails");
    gchar *buffer = g_strdup_printf("%d", complex_plane_get_iterations(cp_thumb));
    gtk_entry_set_text(GTK_ENTRY(input_thumbN), buffer);
    free(buffer);
    g_signal_connect(GTK_ENTRY(input_thumbN), "key-release-event", G_CALLBACK(input_N_handler), (gpointer) cp_thumb);

    //Check draw_thumb
    check_draw_thumb = gtk_check_button_new_with_mnemonic("Draw _thumbnails");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_draw_thumb), complex_plane_is_drawing_active(cp_thumb));
    g_signal_connect(check_draw_thumb, "toggled", G_CALLBACK(toggle_draw_complex_plane_handler), (gpointer) cp_thumb);

    //Thumb options box
    gtk_box_pack_start(GTK_BOX(thumb_options_box), input_thumbN, true, false, 0);
    gtk_box_pack_start(GTK_BOX(thumb_options_box), check_draw_thumb, true, false, 0);

    //Thumb box
    gtk_box_pack_start(GTK_BOX(thumb_box), thumb_img, true, false, 0);
    gtk_box_pack_end(GTK_BOX(thumb_box), thumb_options_box, true, false, 0);

    if (complex_plane_get_polynomial_order(cp) > 0){
      // gtk_box_pack_start(GTK_BOX(thumb_options_box), combo_polynomial_parameter_thumb, true, false, 0);
      gtk_entry_set_width_chars(GTK_ENTRY(input_thumbN), 5);

      if (complex_plane_get_polynomial_parameter(cp_thumb) == -1){
        int order = complex_plane_get_polynomial_order(cp);
        switch(complex_plane_get_function_type(cp)){
          case 2:
            complex_plane_set_polynomial_parameter(cp_thumb, order);
            break;
          case 1:
          default:
            complex_plane_set_polynomial_parameter(cp_thumb, order + 1);
            break;
        }
      }
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo_polynomial_parameter_thumb), complex_plane_get_polynomial_parameter(cp_thumb));
    }
  }
 switch (complex_plane_get_function_type(cp)){
   case 0:
     break;
   case 1:
   case 2:
   case 3:
     break;
 }
 gtk_box_pack_start(GTK_BOX(vbox), thumb_box, true, false, 0);

 //If plane has been already generated
 if (complex_plane_get_plot(cp) != NULL){
   //Copy complex_plane values to input boxes
   char *buffer = malloc(500);
   sprintf(buffer, "%d", complex_plane_get_line_iterations(cp));
   gtk_entry_set_text(GTK_ENTRY(input_N_line), buffer);
   sprintf(buffer, "%d", complex_plane_get_iterations(cp));
   gtk_entry_set_text(GTK_ENTRY(input_N), buffer);
   sprintf(buffer, "%g", creal(complex_plane_get_quadratic_parameter(cp)));
   gtk_entry_set_text(GTK_ENTRY(input_z0), buffer);
   sprintf(buffer, "%g", cimag(complex_plane_get_quadratic_parameter(cp)));
   gtk_entry_set_text(GTK_ENTRY(input_z1), buffer);
   sprintf(buffer, "%g", complex_plane_get_center_real(cp));
   gtk_entry_set_text(GTK_ENTRY(input_center0), buffer);
   sprintf(buffer, "%g", complex_plane_get_center_imag(cp));
   gtk_entry_set_text(GTK_ENTRY(input_center1), buffer);
   sprintf(buffer, "%g", complex_plane_get_spanx(cp));
   gtk_entry_set_text(GTK_ENTRY(input_spanx), buffer);
   sprintf(buffer, "%g", complex_plane_get_spany(cp));
   gtk_entry_set_text(GTK_ENTRY(input_spany), buffer);
   free(buffer);

   if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_PARAMETER_SPACE){
     gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), COMPLEX_PLANE_PARAMETER_SPACE);
   } else if (complex_plane_get_plot_type(cp) == COMPLEX_PLANE_DYNAMIC_PLANE){
     gtk_combo_box_set_active(GTK_COMBO_BOX(input_plot_type), COMPLEX_PLANE_DYNAMIC_PLANE);
   }


   int w = complex_plane_get_width(cp), h = complex_plane_get_height(cp), stride = complex_plane_get_stride(cp);
   //Image (not generated by default, but when button is clicked).
   GdkPixbuf *mandelbrotBuf = gdk_pixbuf_new_from_data(
       complex_plane_get_plot(cp),
       GDK_COLORSPACE_RGB,
       0,
       8,
       w, h,
       w * stride,
       NULL, NULL);
   GtkWidget *image = gtk_image_new_from_pixbuf(mandelbrotBuf);

   //Event box
   GtkWidget *event_box = gtk_event_box_new();
   gtk_container_add(GTK_CONTAINER(event_box), image);
   g_signal_connect(G_OBJECT(event_box), "motion-notify-event", G_CALLBACK(cp_mouse_handler), (gpointer) planes);
   g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(cp_mouse_handler), (gpointer) planes);
   g_signal_connect(G_OBJECT(event_box), "button-release-event", G_CALLBACK(cp_mouse_handler), (gpointer) planes);
   gtk_widget_set_events(event_box, GDK_POINTER_MOTION_MASK);

   gtk_box_pack_start(GTK_BOX(hbox), event_box, false, true, 0);
 }

 gtk_box_pack_start(GTK_BOX(menu_vbox), menu_menubar, false, false, 0);
 gtk_box_pack_start(GTK_BOX(menu_vbox), aux_hbox, true, true, 0);
 gtk_box_pack_start(GTK_BOX(aux_hbox), scroll_box, true, true, 20);

 //Pack hbox (Other element is in if complex_plane != NULL)
 // gtk_box_pack_end(GTK_BOX(hbox), scroll_box, false, false, 20);
 gtk_box_pack_end(GTK_BOX(hbox), menu_vbox, true, true, 0);

 //Add hbox to window
 gtk_container_add(GTK_CONTAINER(window), hbox);

  gtk_widget_show_all(GTK_WIDGET(window));
}

int main (int argc, char *argv[]) {
  //cd to root folder
  chdir(get_root_folder(argv[0]));

  ComplexPlane **planes = malloc(sizeof(ComplexPlane *) * 2);

  gtk_init (&argc, &argv);

  // planes[1] = complex_plane_new(NULL);
  complex_plane_new(&(planes[1]));
  complex_plane_set_id(planes[1], COMPLEX_PLANE_THUMBNAIL_ID);
  complex_plane_set_dimensions(planes[1], 320, 180);
  complex_plane_set_stride(planes[1], 3);
  complex_plane_set_iterations(planes[1], 25);
  complex_plane_set_plot_type(planes[1], COMPLEX_PLANE_DYNAMIC_PLANE);
  complex_plane_set_spanx(planes[1], 6);
  complex_plane_set_spany(planes[1], 4);
  complex_plane_adjust_span_ratio(planes[1]);

  int w = 1600; int h = 850;

  // int w = 600, h = 400;

  planes[0] = NULL;

  // main_data->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget *window_root = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window_root), "Sempiternum");
  gtk_window_set_default_size(GTK_WINDOW(window_root), w, h);
  gtk_window_set_position(GTK_WINDOW(window_root), GTK_WIN_POS_CENTER);
  g_signal_connect(window_root, "destroy",
                   G_CALLBACK(quit_app),
                   (gpointer) window_root);


  gtk_widget_show_all(window_root);
  draw_main_window(window_root, (gpointer) planes);

  gtk_main();

  free(planes[0]);
  free(planes[1]);
  return 0;
}
