#include "common/logger.h"
#include "gui/gui.h"
#include <gtk/gtk.h>

static void on_activate(GtkApplication *app, gpointer user_data) {
  (void)user_data;

  CalcApp *calc_app = calc_app_create();

  // Set the application for the window
  gtk_window_set_application(GTK_WINDOW(calc_app->window), app);

  // Show the window
  gtk_window_present(GTK_WINDOW(calc_app->window));

  // Store app pointer for cleanup
  g_object_set_data_full(G_OBJECT(calc_app->window), "calc_app", calc_app,
                         (GDestroyNotify)calc_app_destroy);
}

int main(int argc, char **argv) {
  logger_init("calc42.log");

  GtkApplication *app =
      gtk_application_new("org.calc42.calculator", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);
  logger_shutdown();

  return status;
}
