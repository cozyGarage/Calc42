#include "common/logger.h"
#include "gui/gui.h"
#include <string.h>

// Structure to pass evaluation result to main thread
typedef struct {
  CalcApp *app;
  char *result_str;
  char *expression;
} EvalResult;

// Callback to update UI from main thread
static gboolean update_result_idle(gpointer data) {
  EvalResult *result = (EvalResult *)data;

  // Update result label
  gtk_label_set_text(GTK_LABEL(result->app->result_label), result->result_str);

  // Add to history if not an error
  if (strncmp(result->result_str, "Error:", 6) != 0) {
    char history_line[512];
    snprintf(history_line, sizeof(history_line), "%s = %s\n",
             result->expression, result->result_str);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(result->app->history_buffer, &iter);
    gtk_text_buffer_insert(result->app->history_buffer, &iter, history_line,
                           -1);
  }

  // Cleanup
  g_free(result->result_str);
  g_free(result->expression);
  g_free(result);

  return G_SOURCE_REMOVE;
}

// Evaluate expression in background thread
static gpointer eval_thread_func(gpointer data) {
  CalcApp *app = (CalcApp *)data;

  const char *expr = gtk_editable_get_text(GTK_EDITABLE(app->expression_entry));
  if (!expr || strlen(expr) == 0) {
    app->eval_running = FALSE;
    return NULL;
  }

  // Evaluate expression
  error_t error;
  value_t result = engine_eval(expr, app->engine_ctx, &error);

  // Prepare result
  EvalResult *eval_result = g_malloc(sizeof(EvalResult));
  eval_result->app = app;
  eval_result->expression = g_strdup(expr);

  if (error_is_ok(error)) {
    eval_result->result_str = value_to_string(&result, app->engine_ctx->base);
    value_free(&result);
  } else {
    eval_result->result_str = g_strdup_printf("Error: %s", error.message);
  }

  // Schedule UI update in main thread
  g_idle_add(update_result_idle, eval_result);

  app->eval_running = FALSE;
  return NULL;
}

// Button click handlers
void on_number_clicked(GtkButton *button, gpointer user_data) {
  CalcApp *app = (CalcApp *)user_data;
  const char *label = gtk_button_get_label(button);

  // Get current text
  const char *current =
      gtk_editable_get_text(GTK_EDITABLE(app->expression_entry));
  char *new_text = g_strdup_printf("%s%s", current, label);

  gtk_editable_set_text(GTK_EDITABLE(app->expression_entry), new_text);
  g_free(new_text);
}

void on_operator_clicked(GtkButton *button, gpointer user_data) {
  CalcApp *app = (CalcApp *)user_data;
  const char *label = gtk_button_get_label(button);

  const char *current =
      gtk_editable_get_text(GTK_EDITABLE(app->expression_entry));

  // Add spaces around operators for readability
  char *new_text = g_strdup_printf("%s %s ", current, label);

  gtk_editable_set_text(GTK_EDITABLE(app->expression_entry), new_text);
  g_free(new_text);
}

void on_function_clicked(GtkButton *button, gpointer user_data) {
  CalcApp *app = (CalcApp *)user_data;
  const char *label = gtk_button_get_label(button);

  const char *current =
      gtk_editable_get_text(GTK_EDITABLE(app->expression_entry));
  char *new_text = g_strdup_printf("%s%s(", current, label);

  gtk_editable_set_text(GTK_EDITABLE(app->expression_entry), new_text);
  g_free(new_text);
}

void on_equals_clicked(GtkButton *button, gpointer user_data) {
  (void)button;
  CalcApp *app = (CalcApp *)user_data;

  if (app->eval_running) {
    return; // Don't start new evaluation if one is running
  }

  app->eval_running = TRUE;
  app->eval_thread = g_thread_new("evaluator", eval_thread_func, app);
  g_thread_unref(app->eval_thread);
}

void on_clear_clicked(GtkButton *button, gpointer user_data) {
  (void)button;
  CalcApp *app = (CalcApp *)user_data;

  gtk_editable_set_text(GTK_EDITABLE(app->expression_entry), "");
  gtk_label_set_text(GTK_LABEL(app->result_label), "0");
}

void on_backspace_clicked(GtkButton *button, gpointer user_data) {
  (void)button;
  CalcApp *app = (CalcApp *)user_data;

  const char *current =
      gtk_editable_get_text(GTK_EDITABLE(app->expression_entry));
  if (strlen(current) > 0) {
    char *new_text = g_strndup(current, strlen(current) - 1);
    gtk_editable_set_text(GTK_EDITABLE(app->expression_entry), new_text);
    g_free(new_text);
  }
}

// Mode switching
void on_mode_changed(GtkComboBox *combo, gpointer user_data) {
  CalcApp *app = (CalcApp *)user_data;

  gint active = gtk_combo_box_get_active(combo);

  switch (active) {
  case 0:
    app->engine_ctx->mode = MODE_STANDARD;
    break;
  case 1:
    app->engine_ctx->mode = MODE_PROGRAMMER;
    break;
  case 2:
    app->engine_ctx->mode = MODE_STATISTICS;
    break;
  case 3:
    app->engine_ctx->mode = MODE_PROBABILITY;
    break;
  case 4:
    app->engine_ctx->mode = MODE_DISCRETE;
    break;
  case 5:
    app->engine_ctx->mode = MODE_LINEAR_ALGEBRA;
    break;
  default:
    break;
  }

  // Switch visible panel in stack
  gtk_stack_set_visible_child_name(GTK_STACK(app->mode_panel_stack),
                                   gtk_combo_box_get_active_id(combo));
}

void on_base_changed(GtkComboBox *combo, gpointer user_data) {
  CalcApp *app = (CalcApp *)user_data;

  gint active = gtk_combo_box_get_active(combo);

  switch (active) {
  case 0:
    app->engine_ctx->base = 10;
    break;
  case 1:
    app->engine_ctx->base = 16;
    break;
  case 2:
    app->engine_ctx->base = 2;
    break;
  case 3:
    app->engine_ctx->base = 8;
    break;
  default:
    break;
  }
}

// Helper to create a button
static GtkWidget *create_button(const char *label, GCallback callback,
                                gpointer data) {
  GtkWidget *button = gtk_button_new_with_label(label);
  g_signal_connect(button, "clicked", callback, data);
  gtk_widget_set_size_request(button, 60, 40);
  return button;
}

// Create standard mode panel
static GtkWidget *create_standard_panel(CalcApp *app) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  // Number buttons
  const char *numbers[] = {"7", "8", "9", "4", "5", "6", "1",
                           "2", "3", "0", ".", "(", ")"};
  int num_idx = 0;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      if (num_idx >= 13)
        break;
      GtkWidget *btn =
          create_button(numbers[num_idx++], G_CALLBACK(on_number_clicked), app);
      gtk_grid_attach(GTK_GRID(grid), btn, col, row, 1, 1);
    }
  }

  // Operator buttons
  const char *ops[] = {"+", "-", "*", "/", "%"};
  for (int i = 0; i < 5; i++) {
    GtkWidget *btn =
        create_button(ops[i], G_CALLBACK(on_operator_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, 3, i, 1, 1);
  }

  // Special buttons
  GtkWidget *clear = create_button("C", G_CALLBACK(on_clear_clicked), app);
  GtkWidget *backspace =
      create_button("←", G_CALLBACK(on_backspace_clicked), app);
  GtkWidget *equals = create_button("=", G_CALLBACK(on_equals_clicked), app);

  gtk_grid_attach(GTK_GRID(grid), clear, 4, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), backspace, 4, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), equals, 4, 2, 1, 2);

  return grid;
}

// Create programmer mode panel
static GtkWidget *create_programmer_panel(CalcApp *app) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  // Hex digits
  const char *hex[] = {"A", "B", "C", "D", "E", "F"};
  for (int i = 0; i < 6; i++) {
    GtkWidget *btn = create_button(hex[i], G_CALLBACK(on_number_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, i % 3, i / 3, 1, 1);
  }

  // Bitwise operators
  const char *bitwise[] = {"&", "|", "^", "~", "<<", ">>"};
  for (int i = 0; i < 6; i++) {
    GtkWidget *btn =
        create_button(bitwise[i], G_CALLBACK(on_operator_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, i % 3, 2 + i / 3, 1, 1);
  }

  return grid;
}

// Create statistics/probability panel
static GtkWidget *create_stats_panel(CalcApp *app) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  const char *funcs[] = {"mean", "median", "mode", "stddev",   "var",
                         "ncr",  "npr",    "fact", "binomial", "geometric"};

  for (int i = 0; i < 10; i++) {
    GtkWidget *btn =
        create_button(funcs[i], G_CALLBACK(on_function_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, i % 3, i / 3, 1, 1);
  }

  return grid;
}

// Create discrete math panel
static GtkWidget *create_discrete_panel(CalcApp *app) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  const char *funcs[] = {"gcd", "lcm", "mod", "modpow", "is_prime"};

  for (int i = 0; i < 5; i++) {
    GtkWidget *btn =
        create_button(funcs[i], G_CALLBACK(on_function_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, i % 3, i / 3, 1, 1);
  }

  return grid;
}

// Create linear algebra panel
static GtkWidget *create_linalg_panel(CalcApp *app) {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  const char *funcs[] = {"vec_add", "vec_sub", "vec_dot", "vec_mag",
                         "vec_scale"};

  for (int i = 0; i < 5; i++) {
    GtkWidget *btn =
        create_button(funcs[i], G_CALLBACK(on_function_clicked), app);
    gtk_grid_attach(GTK_GRID(grid), btn, i % 3, i / 3, 1, 1);
  }

  return grid;
}

// Application initialization
CalcApp *calc_app_create(void) {
  CalcApp *app = g_malloc0(sizeof(CalcApp));
  app->engine_ctx = engine_context_create(MODE_STANDARD);
  app->eval_running = FALSE;

  // Create main window
  app->window = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(app->window), "CALC42");
  gtk_window_set_default_size(GTK_WINDOW(app->window), 500, 600);

  // Main vertical box
  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(vbox, 10);
  gtk_widget_set_margin_end(vbox, 10);
  gtk_widget_set_margin_top(vbox, 10);
  gtk_widget_set_margin_bottom(vbox, 10);
  gtk_window_set_child(GTK_WINDOW(app->window), vbox);

  // Top bar with mode and base selectors
  GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

  // Mode selector
  app->mode_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "standard",
                            "Standard");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "programmer",
                            "Programmer");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "statistics",
                            "Statistics");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "probability",
                            "Probability");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "discrete",
                            "Discrete Math");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app->mode_combo), "linalg",
                            "Linear Algebra");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->mode_combo), 0);
  g_signal_connect(app->mode_combo, "changed", G_CALLBACK(on_mode_changed),
                   app);

  // Base selector
  app->base_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->base_combo), "DEC");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->base_combo), "HEX");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->base_combo), "BIN");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->base_combo), "OCT");
  gtk_combo_box_set_active(GTK_COMBO_BOX(app->base_combo), 0);
  g_signal_connect(app->base_combo, "changed", G_CALLBACK(on_base_changed),
                   app);

  gtk_box_append(GTK_BOX(top_bar), gtk_label_new("Mode:"));
  gtk_box_append(GTK_BOX(top_bar), app->mode_combo);
  gtk_box_append(GTK_BOX(top_bar), gtk_label_new("Base:"));
  gtk_box_append(GTK_BOX(top_bar), app->base_combo);
  gtk_box_append(GTK_BOX(vbox), top_bar);

  // Expression entry
  app->expression_entry = gtk_entry_new();
  gtk_widget_set_size_request(app->expression_entry, -1, 40);
  gtk_box_append(GTK_BOX(vbox), app->expression_entry);

  // Result display
  app->result_label = gtk_label_new("0");
  gtk_widget_set_halign(app->result_label, GTK_ALIGN_END);
  gtk_widget_set_size_request(app->result_label, -1, 40);
  gtk_box_append(GTK_BOX(vbox), app->result_label);

  // Standard calculator buttons (always visible)
  GtkWidget *std_panel = create_standard_panel(app);
  gtk_box_append(GTK_BOX(vbox), std_panel);

  // Mode-specific panels in a stack
  app->mode_panel_stack = gtk_stack_new();
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack), gtk_label_new(""),
                      "standard");
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack),
                      create_programmer_panel(app), "programmer");
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack), create_stats_panel(app),
                      "statistics");
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack), create_stats_panel(app),
                      "probability");
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack),
                      create_discrete_panel(app), "discrete");
  gtk_stack_add_named(GTK_STACK(app->mode_panel_stack),
                      create_linalg_panel(app), "linalg");

  gtk_box_append(GTK_BOX(vbox), app->mode_panel_stack);

  // History view
  GtkWidget *history_label = gtk_label_new("History:");
  gtk_widget_set_halign(history_label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(vbox), history_label);

  app->history_buffer = gtk_text_buffer_new(NULL);
  app->history_view = gtk_text_view_new_with_buffer(app->history_buffer);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(app->history_view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->history_view), GTK_WRAP_WORD);
  gtk_widget_set_size_request(app->history_view, -1, 100);

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->history_view);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
  gtk_box_append(GTK_BOX(vbox), scroll);

  return app;
}

void calc_app_destroy(CalcApp *app) {
  if (app) {
    if (app->engine_ctx) {
      engine_context_free(app->engine_ctx);
    }
    g_free(app);
  }
}
