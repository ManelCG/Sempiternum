#include <gui_gen_video.h>

mainMenu *menu_main_new(){
  mainMenu *menu = malloc(sizeof(mainMenu));

  return menu;
}

char *gui_gen_video_choose_folder(GtkWidget *w, gpointer data){
  GtkWidget *dialog;
  char *filename = "./";
  GtkWidget *entry = (GtkWidget *) data;

  dialog = gtk_file_chooser_dialog_new("Choose folder",
                                        NULL,
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Open", GTK_RESPONSE_ACCEPT,
                                        NULL);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT){
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (GTK_IS_ENTRY(entry)){
      gtk_entry_set_text(GTK_ENTRY(entry), filename);
    }
  }
  gtk_widget_destroy(dialog);

  return filename;

}

GtkWidget *gui_gen_video_generate_default_resolutions_combo_box(int *num){
  GtkWidget *combo = gtk_combo_box_text_new();
  int numr = 0;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "426x240");   numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "640x360");   numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "854x480");   numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "1280x720");  numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "1920x1080"); numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "1920x1200"); numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "2560x1440"); numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "3840x2160"); numr++;
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, "7680x4320"); numr++;

  if (num != NULL){
    *num = numr;
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 4);

  return combo;
}

void gui_gen_video_resolutions_combo_box_to_entries(GtkWidget *combo, gpointer e){
  GtkWidget **entries = (GtkWidget **) e;
  char *string = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
  char *delim = "x";
  char *token;

  token = strtok(string, delim);
  token = strtok(NULL, delim);
  gtk_entry_set_text(GTK_ENTRY(entries[0]), string);
  gtk_entry_set_text(GTK_ENTRY(entries[1]), token);
}

void gui_gen_video_lock_span_ratio(GtkWidget *widget, gpointer data){
  GtkWidget **wid = (GtkWidget **) data;
  int w = atoi(gtk_entry_get_text(GTK_ENTRY(wid[0])));
  int h = atoi(gtk_entry_get_text(GTK_ENTRY(wid[1])));
  double maxspany = strtod(gtk_entry_get_text(GTK_ENTRY(wid[3])), NULL);
  double minspany = strtod(gtk_entry_get_text(GTK_ENTRY(wid[5])), NULL);
  double minspanx;
  double maxspanx;

  double ratio = (double) w / (double) h;
  printf("RATIO %f\n", ratio);

  minspanx = minspany * ratio;
  maxspanx = maxspany * ratio;

  gtk_entry_set_text(GTK_ENTRY(wid[2]), g_strdup_printf("%.16g", maxspanx));
  gtk_entry_set_text(GTK_ENTRY(wid[4]), g_strdup_printf("%.16g", minspanx));
}
