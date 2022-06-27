#include <gui_templates.h>
#include <stdbool.h>

#include <ctype.h>

#include <ComplexPlane.h>
#include <complex_plane_colorschemes.h>

#include <stdio.h>

#define GUI_TEMPLATES_BUTTON_WIDTH 85
//#define DEBUG_GUI_TEMPLATES

#ifdef MAKE_INSTALL
  #define ABOUT_PICTURE_PNG "/usr/share/sempiternum/assets/inapp_assets/about_picture.png"
#else
  #define ABOUT_PICTURE_PNG "assets/inapp_assets/about_picture.png"
#endif

//Internal function declarations
void gui_templates_configure_roots_internal(GtkWidget *window, gpointer data);

void clear_container(GtkWidget *window){
  GList *children, *iter;
  children = gtk_container_get_children(GTK_CONTAINER(window));
  for (iter = children; iter != NULL; iter = g_list_next(iter)){
    gtk_widget_destroy(GTK_WIDGET(iter->data));
  }
  g_list_free(children);
}

void destroy(GtkWidget *w, gpointer data){
  GtkWidget *window = (GtkWidget *) data;
  clear_container(window);
  gtk_widget_destroy(window);
}

void gui_templates_window_set_sempiternum_icon(GtkWindow *window){
  GdkPixbuf *window_icon = gdk_pixbuf_new_from_file(ABOUT_PICTURE_PNG, NULL);
  window_icon = gdk_pixbuf_scale_simple(window_icon, 16, 16, GDK_INTERP_NEAREST);
  gtk_window_set_icon(window, window_icon);
}

void insert_text_event_int(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  #ifdef DEBUG_GUI_TEMPLATES
    printf("\n#####\ninsert_text_event_int called\n#####\n");
  #endif //DEBUG_GUI_TEMPLATES
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i])){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return;
    }
  }
}



void insert_text_event_float(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data){
  #ifdef DEBUG_GUI_TEMPLATES
    printf("\n#####\ninsert_text_event_float called\n#####\n");
  #endif //DEBUG_GUI_TEMPLATES
  for (int i = 0; i < length; i++){
    if (!isdigit(text[i]) && !(text[i] == '.') && !(text[i] == '-')){
      g_signal_stop_emission_by_name(G_OBJECT(editable), "insert-text");
      return;
    }
  }
}


void input_root_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  ComplexPolynomial *root = (ComplexPolynomial *) data;
  int index = atoi(gtk_widget_get_name(w));
  complex double value = strtod(gtk_entry_get_text(GTK_ENTRY(w)), NULL);

  complex_polynomial_set_member(root, value, index);
}
void input_root_save_handler(GtkWidget *w, GdkEventKey *event, gpointer data){
  if (event != NULL && strcmp(gdk_keyval_name(event->keyval), "Return") == 0){
    void **pointers = (void *) data;
    ComplexPolynomial *cpoly = (ComplexPolynomial *) pointers[0];
    ComplexPlane *cp_main = (ComplexPlane *) pointers[1];
    ComplexPlane *cp_thumb = (ComplexPlane *) pointers[2];
    ComplexPlane **planes = malloc(sizeof(ComplexPlane *) * 2);
    planes[0] = cp_main;
    planes[1] = cp_thumb;

    RootArrayMember *ram1 = root_array_member_new(NULL, cpoly, complex_plane_get_polynomial_order(cp_main));
    RootArrayMember *ram2 = root_array_member_new(NULL, cpoly, complex_plane_get_polynomial_order(cp_main));

    complex_plane_root_array_member_add(cp_main, ram1);
    complex_plane_root_array_member_add(cp_thumb, ram2);

    GtkWidget *window = gtk_widget_get_toplevel(w);
    clear_container(window);
    gui_templates_configure_roots_internal(window, (gpointer) planes);
  }
}

void button_remove_root_handler(GtkWidget *button, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp_main = planes[0];
  ComplexPlane *cp_thumb = planes[1];

  int ID = atoi(gtk_widget_get_name(button));

  complex_plane_root_array_member_remove_by_index(cp_main, ID);
  complex_plane_root_array_member_remove_by_index(cp_thumb, ID);

  GtkWidget *window = gtk_widget_get_toplevel(button);
  clear_container(window);
  gui_templates_configure_roots_internal(window, (gpointer) planes);
}

void gui_templates_configure_roots_internal(GtkWidget *window, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp_main = planes[0];
  ComplexPlane *cp_thumb = planes[1];

  int order = complex_plane_get_polynomial_order(cp_main);

  ComplexPolynomial *new_root = complex_polynomial_new(NULL, order);

  void **cp_pointers = malloc(sizeof(void *) * 3);
  cp_pointers[0] = new_root;
  cp_pointers[1] = cp_main;
  cp_pointers[2] = cp_thumb;

  //Boxes
  GtkWidget *main_vbox;
  GtkWidget *new_root_hbox;
  GtkWidget *lower_buttons_hbox;
  main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);


  //Buttons
  GtkWidget *button_close;

  new_root_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  for (int i = 0; i < order + 1; i++){
    GtkWidget *input_root = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(input_root), 5);
    char buffer[16];
    sprintf(buffer, "%d", i);
    gtk_widget_set_name(input_root, buffer);
    g_signal_connect(input_root, "insert-text", G_CALLBACK(insert_text_event_float), NULL);
    g_signal_connect(input_root, "key_release_event", G_CALLBACK(input_root_handler), (gpointer) new_root);
    g_signal_connect(input_root, "key_release_event", G_CALLBACK(input_root_save_handler), (gpointer) cp_pointers);

    gtk_box_pack_start(GTK_BOX(new_root_hbox), input_root, false, false, 5);

    if (order-i > 1){
      char buffer[64];
      sprintf(buffer, "a%s + ", complex_function_get_exponent_str(order -i));
      GtkWidget *label_root = gtk_label_new(buffer);
      gtk_box_pack_start(GTK_BOX(new_root_hbox), label_root, false, false, 5);
    } else if (order -i == 1){
      GtkWidget *label_root = gtk_label_new("a + ");
      gtk_box_pack_start(GTK_BOX(new_root_hbox), label_root, false, false, 5);
    }
  }

  gtk_box_pack_start(GTK_BOX(main_vbox), new_root_hbox, true, false, 0);

  int n_roots = complex_plane_root_array_member_get_n(cp_main);
  for (int i = 0; i < n_roots; i++){
    GtkWidget *root_hbox;
    GtkWidget *root_label;
    GtkWidget *button_remove_root;

    RootArrayMember *ram = complex_plane_root_array_member_get_by_index(cp_main, i);

    char buffer[1024];
    char *buffer1 = g_strdup_printf("Root %2d: ", i);
    char *buffer2 = complex_plane_polynomial_to_str(complex_polynomial_get_polynomial(root_array_member_get_root(ram)), NULL, order, 'a', 'x');

    strcpy(buffer, buffer1);
    strcat(buffer, buffer2);

    free(buffer1);
    free(buffer2);

    root_label = gtk_label_new(buffer);

    button_remove_root = gtk_button_new_with_label("");
    char *buffer_name = g_strdup_printf("%d", i);
    gtk_widget_set_name(button_remove_root, buffer_name);
    free(buffer_name);
    g_signal_connect(button_remove_root, "clicked", G_CALLBACK(button_remove_root_handler), (gpointer) planes);
    { GtkWidget *icon = gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
      gtk_button_set_image(GTK_BUTTON(button_remove_root), icon); }

    root_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    gtk_box_pack_start(GTK_BOX(root_hbox), root_label, false, false, 0);
    gtk_box_pack_end(GTK_BOX(root_hbox), button_remove_root, false, false, 0);

    gtk_box_pack_start(GTK_BOX(main_vbox), root_hbox, false, false, 0);
  }


  //Lower buttons
  lower_buttons_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

  button_close = gtk_button_new_with_label("Close");
  g_signal_connect(button_close, "clicked", G_CALLBACK(destroy), (gpointer) window);
  { GtkWidget *icon = gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_image(GTK_BUTTON(button_close), icon); }

  gtk_box_pack_end(GTK_BOX(lower_buttons_hbox), button_close, false, false, 0);

  //Main vbox
  gtk_box_pack_end(GTK_BOX(main_vbox), lower_buttons_hbox, false, false, 0);
  gtk_container_add(GTK_CONTAINER(window), main_vbox);

  gtk_widget_show_all(GTK_WIDGET(window));
  #ifdef DEBUG_GUI_TEMPLATES
  printf("Gui templates configure_roots window called succesfully\n");
  #endif
}

void gui_templates_configure_roots(GtkWidget *w, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_resizable(window, false);
  gtk_window_set_title(window, "Configure function roots");
  gtk_window_set_default_size(GTK_WINDOW(window), 85, 0);
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), (gpointer) window);

  gui_templates_configure_roots_internal(GTK_WIDGET(window), data);
}


void gui_templates_show_help_window(GtkWidget *w, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gui_templates_window_set_sempiternum_icon(window);
  gtk_window_set_title(window, "Sempiternum documentation");
  gtk_window_set_resizable(window, false);
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);



  gtk_widget_show_all(GTK_WIDGET(window));

  #ifdef DEBUG_GUI_TEMPLATES
  printf("Gui templates show help window called succesfully\n");
  #endif
}

void combo_colorscheme_handler(GtkWidget *combo, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  ComplexPlane *cp = planes[0];
  ComplexPlane *th = planes[1];

  int colorscheme = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

  complex_plane_set_colorscheme(cp, colorscheme);
  complex_plane_set_colorscheme(th, colorscheme);
}

void gui_templates_show_preferences_window(GtkWidget *w, gpointer data){
  ComplexPlane **planes = (ComplexPlane **) data;
  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gui_templates_window_set_sempiternum_icon(window);
  gtk_window_set_resizable(window, false);
  gtk_window_set_title(window, "Sempiternum Preferences");
  gtk_window_set_default_size(GTK_WINDOW(window), 420, 600);
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);
  g_signal_connect(window, "destroy", G_CALLBACK(destroy), (gpointer) window);

  //boxes
  GtkWidget *main_vbox;
  GtkWidget *display_vbox;
  GtkWidget *gpu_vbox;

  GtkWidget *button_hbox;

  //Menu toolbar
  GtkWidget *toolbar;
  GtkToolItem *displayTb;
  GtkToolItem *gpuTb;

  //Buttons
  GtkWidget *button_help;
  GtkWidget *button_close;

  //Display
  displayTb = gtk_tool_button_new(NULL, "Display");
  { //Display vbox
    GtkWidget *colorscheme_hbox;
    { //Colorschemes
      colorscheme_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
      GtkWidget *label_colorscheme = gtk_label_new("Colorscheme:");
      GtkWidget *combo_colorscheme = gtk_combo_box_text_new();
      for (int i = 0; i < complex_plane_colorschemes_get_num(); i++){
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_colorscheme), NULL, complex_plane_colorschemes_get_name(i));
      }
      gtk_combo_box_set_active(GTK_COMBO_BOX(combo_colorscheme), complex_plane_get_colorscheme(planes[0]));

      g_signal_connect(combo_colorscheme, "changed", G_CALLBACK(combo_colorscheme_handler), (gpointer) planes);

      gtk_box_pack_start(GTK_BOX(colorscheme_hbox), label_colorscheme, false, false, 0);
      gtk_box_pack_end(GTK_BOX(colorscheme_hbox), combo_colorscheme, false, false, 0);
    }


    display_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(display_vbox), colorscheme_hbox, false, false, 0);
  }

  //GPU
  gpuTb = gtk_tool_button_new(NULL, "Device");

  toolbar = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), displayTb, -1);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gpuTb, -1);


  //Button hbox
  button_close = gtk_button_new_with_label("Close");
  g_signal_connect(button_close, "clicked", G_CALLBACK(destroy), (gpointer) window);
  { GtkWidget *icon = gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
    gtk_button_set_image(GTK_BUTTON(button_close), icon); }

  button_help = gtk_button_new_with_label("Help");
  { GtkWidget *icon = gtk_image_new_from_icon_name("help-contents", GTK_ICON_SIZE_MENU);
    gtk_button_set_image(GTK_BUTTON(button_help), icon); }

  button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(button_hbox), button_help, false, false, 0);
  gtk_box_pack_end(GTK_BOX(button_hbox), button_close, false, false, 0);



  //Vbox
  main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), toolbar, false, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), display_vbox, false, false, 0);
  gtk_box_pack_end(GTK_BOX(main_vbox), button_hbox, false, false, 0);
  gtk_container_add(GTK_CONTAINER(window), main_vbox);

  gtk_widget_show_all(GTK_WIDGET(window));

  #ifdef DEBUG_GUI_TEMPLATES
  printf("Gui templates show preferences window called succesfully\n");
  #endif
}

void gui_templates_show_about_window(GtkWidget *w, gpointer data){
  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gui_templates_window_set_sempiternum_icon(window);

  gtk_window_set_title(window, "About Sempiternum");
  gtk_window_set_resizable(window, false);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_window_set_default_size(GTK_WINDOW(window), 420, 0);

  GtkAccelGroup *accel_group;

  GtkWidget *main_vbox;
  GtkWidget *final_hbox;

  GtkWidget *label_app_name;
  GtkWidget *label_version;
  GtkWidget *label_description;
  GtkWidget *label_git_repo;
  GtkWidget *label_copyright_manel;

  GtkWidget *button_credits;
  GtkWidget *button_license;
  GtkWidget *button_close;

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(ABOUT_PICTURE_PNG, NULL);
  pixbuf = gdk_pixbuf_scale_simple(pixbuf, 150, 150, GDK_INTERP_NEAREST);
  GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);

  label_app_name = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_app_name), "<b>Sempiternum</b>");

  label_version = gtk_label_new("Beta June '22");

  label_description = gtk_label_new("Sempiternum represents the Julia sets resulting on the iteration\nof different rational functions on the complex plane.");
  gtk_label_set_justify(GTK_LABEL(label_description),GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap(GTK_LABEL(label_description), true);
  gtk_label_set_max_width_chars(GTK_LABEL(label_description), 50);
  gtk_label_set_width_chars(GTK_LABEL(label_description), 50);

  const char *label_git_repo_text = "<a href=\"https://github.com/ManelCG/complex_function_dynamics_plotter\"> Github repository</a>";
  label_git_repo = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_git_repo), label_git_repo_text);
  gtk_widget_set_tooltip_text(label_git_repo, "Go to Sempiternum's Github repository");

  label_copyright_manel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_copyright_manel), "<small>Copyright © Manel Castillo Giménez</small>");

  button_close = gtk_button_new_with_label("Close");
  gtk_widget_set_size_request(button_close, GUI_TEMPLATES_BUTTON_WIDTH, 0);
  g_signal_connect(button_close, "clicked", G_CALLBACK(destroy), (gpointer) window);

  button_credits = gtk_button_new_with_label("Credits");
  gtk_widget_set_size_request(button_credits, GUI_TEMPLATES_BUTTON_WIDTH, 0);

  button_license = gtk_button_new_with_label("License");
  gtk_widget_set_size_request(button_license, GUI_TEMPLATES_BUTTON_WIDTH, 0);

  final_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(final_hbox), button_credits, false, false, 0);
  gtk_box_pack_start(GTK_BOX(final_hbox), button_license, false, false, 0);
  gtk_box_pack_end(GTK_BOX(final_hbox), button_close, false, false, 0);

  main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(main_vbox), image, true, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), label_app_name, true, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), label_version, true, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), label_description, false, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), label_git_repo, true, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), label_copyright_manel, true, false, 0);
  gtk_box_pack_start(GTK_BOX(main_vbox), final_hbox, true, false, 0);

  //accel group
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
  gtk_widget_add_accelerator(button_close, "clicked", accel_group, GDK_KEY_Escape, 0, GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(button_close, "clicked", accel_group, GDK_KEY_q, 0, GTK_ACCEL_VISIBLE);

  gtk_container_add(GTK_CONTAINER(window), main_vbox);
  gtk_widget_show_all(GTK_WIDGET(window));

  #ifdef DEBUG_GUI_TEMPLATES
  printf("Gui templates show about window called succesfully\n");
  #endif
}

