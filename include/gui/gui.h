#ifndef GUI_H
#define GUI_H

#include "engine/engine.h"
#include <gtk/gtk.h>

// Main application structure
typedef struct {
  GtkWidget *window;
  GtkWidget *expression_entry;
  GtkWidget *result_label;
  GtkWidget *history_view;
  GtkWidget *mode_combo;
  GtkWidget *base_combo;
  GtkWidget *button_grid;
  GtkWidget *mode_panel_stack;

  GtkTextBuffer *history_buffer;
  engine_context_t *engine_ctx;

  // Thread for evaluation
  GThread *eval_thread;
  gboolean eval_running;
} CalcApp;

// Initialize the GUI
CalcApp *calc_app_create(void);

// Cleanup
void calc_app_destroy(CalcApp *app);

// Button callbacks
void on_number_clicked(GtkButton *button, gpointer user_data);
void on_operator_clicked(GtkButton *button, gpointer user_data);
void on_function_clicked(GtkButton *button, gpointer user_data);
void on_equals_clicked(GtkButton *button, gpointer user_data);
void on_clear_clicked(GtkButton *button, gpointer user_data);
void on_backspace_clicked(GtkButton *button, gpointer user_data);

// Mode switching
void on_mode_changed(GtkComboBox *combo, gpointer user_data);
void on_base_changed(GtkComboBox *combo, gpointer user_data);

#endif // GUI_H
