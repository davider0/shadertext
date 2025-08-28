// shadertext_gtk.c — Port GTK + GtkSourceView del editor tipo "kilo"
// Requisitos: GTK+3 y GtkSourceView 3.x
// Atajos: Ctrl+S (guardar), Ctrl+Q (salir), Ctrl+F (buscar)
// Compilación: ver Makefile adjunto

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <stdio.h>

#define APP_ID "es.davidescribano.shadertext"

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkSourceView *view;
    GtkSourceBuffer *buffer;
    GtkWidget *statusbar;
    guint status_ctx;
    gchar *filename; // ruta del archivo abierto
    GtkWidget *search_revealer;
    GtkWidget *search_entry;
    GtkSourceSearchContext *search_ctx;
    GtkSourceSearchSettings *search_settings;
} App;

static void update_status(App *a) {
    GtkTextIter iter;
    GtkTextMark *insert_mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(a->buffer));
    gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(a->buffer), &iter, insert_mark);
    int line = gtk_text_iter_get_line(&iter) + 1;
    int col  = gtk_text_iter_get_line_offset(&iter) + 1;
    gboolean modified = gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(a->buffer));

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(a->buffer), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(a->buffer), &end);
    int chars = gtk_text_iter_get_chars_in_line(&start); // no fiable para todo el doc
    // Para contar líneas, pedimos el índice de la última línea
    int nlines = gtk_text_buffer_get_line_count(GTK_TEXT_BUFFER(a->buffer));

    gchar *msg = g_strdup_printf("%s — %d líneas — %s — Ln %d, Col %d",
                                 a->filename ? a->filename : "(sin nombre)",
                                 nlines,
                                 modified ? "modificado" : "guardado",
                                 line, col);
    gtk_statusbar_pop(GTK_STATUSBAR(a->statusbar), a->status_ctx);
    gtk_statusbar_push(GTK_STATUSBAR(a->statusbar), a->status_ctx, msg);
    g_free(msg);
}

static gboolean on_key_release(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    App *a = user_data;
    update_status(a);
    return FALSE;
}

static void on_mark_set(GtkTextBuffer *buffer, const GtkTextIter *location, GtkTextMark *mark, gpointer user_data) {
    App *a = user_data;
    update_status(a);
}

static void on_modified_changed(GtkTextBuffer *buffer, GParamSpec *pspec, gpointer user_data) {
    App *a = user_data;
    update_status(a);
}

static void action_quit(GSimpleAction *action, GVariant *param, gpointer user_data) {
    App *a = user_data;
    gboolean modified = gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(a->buffer));
    if (modified) {
        GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(a->window), GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
                                                "Hay cambios sin guardar. ¿Salir?");
        gtk_dialog_add_buttons(GTK_DIALOG(dlg), "Cancelar", GTK_RESPONSE_CANCEL, "Salir", GTK_RESPONSE_ACCEPT, NULL);
        gint resp = gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        if (resp != GTK_RESPONSE_ACCEPT) return;
    }
    g_application_quit(G_APPLICATION(a->app));
}

static gboolean save_to_path(App *a, const char *path) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(a->buffer), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(a->buffer), &end);
    gchar *text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(a->buffer), &start, &end, FALSE);

    GError *err = NULL;
    gboolean ok = g_file_set_contents(path, text, -1, &err);
    if (!ok) {
        GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(a->window), GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                                "No se pudo guardar: %s", err->message);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_clear_error(&err);
    } else {
        gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(a->buffer), FALSE);
        if (a->filename) g_free(a->filename);
        a->filename = g_strdup(path);
    }

    g_free(text);
    update_status(a);
    return ok;
}

static void do_save_as(App *a) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Guardar como", GTK_WINDOW(a->window),
                                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                                 "Cancelar", GTK_RESPONSE_CANCEL,
                                                 "Guardar", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), TRUE);
    if (a->filename) gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dlg), a->filename);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        save_to_path(a, path);
        g_free(path);
    }
    gtk_widget_destroy(dlg);
}

static void action_save(GSimpleAction *action, GVariant *param, gpointer user_data) {
    App *a = user_data;
    if (a->filename) {
        save_to_path(a, a->filename);
    } else {
        do_save_as(a);
    }
}

// --- BÚSQUEDA -------------------------------------------------------------
static void do_search_forward(App *a) {
    const gchar *q = gtk_entry_get_text(GTK_ENTRY(a->search_entry));
    if (!q || !*q) return;

    gtksource_search_settings_set_search_text(a->search_settings, q);

    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(a->buffer), &start, &end);
    if (!gtk_text_iter_is_end(&end)) start = end; else gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(a->buffer), &start);

    GtkTextIter match_start, match_end;
    gboolean found = gtksource_search_context_forward2(a->search_ctx, &start, &match_start, &match_end, NULL);
    if (found) {
        gtk_text_buffer_select_range(GTK_TEXT_BUFFER(a->buffer), &match_start, &match_end);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(a->view), &match_start, 0.2, TRUE, 0.0, 0.5);
    }
}

static void on_search_changed(GtkEditable *editable, gpointer user_data) {
    App *a = user_data;
    const gchar *q = gtk_entry_get_text(GTK_ENTRY(a->search_entry));
    gboolean has = q && *q;
    gtksource_search_settings_set_search_text(a->search_settings, has ? q : "");
}

static void action_find(GSimpleAction *action, GVariant *param, gpointer user_data) {
    App *a = user_data;
    gtk_revealer_set_reveal_child(GTK_REVEALER(a->search_revealer), TRUE);
    gtk_widget_grab_focus(a->search_entry);
}

static void on_search_activate(GtkEntry *entry, gpointer user_data) {
    App *a = user_data;
    do_search_forward(a);
}

static void on_search_escape(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    if (keyval == GDK_KEY_Escape) {
        App *a = user_data;
        gtk_revealer_set_reveal_child(GTK_REVEALER(a->search_revealer), FALSE);
        gtk_widget_grab_focus(GTK_WIDGET(a->view));
    }
}

// --- CARGA DE ARCHIVO -----------------------------------------------------
static void load_file(App *a, const char *path) {
    GError *err = NULL;
    gchar *contents = NULL;
    gsize len = 0;
    if (!g_file_get_contents(path, &contents, &len, &err)) {
        GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(a->window), GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                                "No se pudo abrir %s: %s", path, err->message);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_clear_error(&err);
        return;
    }
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(a->buffer), contents, (gint)len);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(a->buffer), FALSE);
    if (a->filename) g_free(a->filename);
    a->filename = g_strdup(path);
    g_free(contents);

    // Lenguaje C para resaltado
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();
    GtkSourceLanguage *lang = gtk_source_language_manager_get_language(lm, "c");
    if (lang) gtk_source_buffer_set_language(a->buffer, lang);

    update_status(a);
}

// --- CONSTRUCCIÓN DE UI ---------------------------------------------------
static void build_ui(App *a) {
    a->window = gtk_application_window_new(a->app);
    gtk_window_set_title(GTK_WINDOW(a->window), "ShaderText (GTK)");
    gtk_window_set_default_size(GTK_WINDOW(a->window), 900, 700);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(a->window), vbox);

    // Barra de búsqueda (oculta)
    a->search_revealer = gtk_revealer_new();
    gtk_box_pack_start(GTK_BOX(vbox), a->search_revealer, FALSE, FALSE, 0);
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(a->search_revealer), search_box);
    a->search_entry = gtk_search_entry_new();
    gtk_box_pack_start(GTK_BOX(search_box), a->search_entry, TRUE, TRUE, 6);
    GtkWidget *find_next_btn = gtk_button_new_with_label("Buscar siguiente");
    gtk_box_pack_start(GTK_BOX(search_box), find_next_btn, FALSE, FALSE, 6);

    // Vista de código
    a->buffer = GTK_SOURCE_BUFFER(gtk_source_buffer_new(NULL));
    a->view = GTK_SOURCE_VIEW(gtk_source_view_new_with_buffer(a->buffer));
    gtk_source_view_set_show_line_numbers(a->view, TRUE);
    gtk_source_view_set_tab_width(a->view, 4);
    gtk_source_view_set_insert_spaces_instead_of_tabs(a->view, TRUE);
    gtk_source_view_set_highlight_current_line(a->view, TRUE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(a->view));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // Barra de estado
    a->statusbar = gtk_statusbar_new();
    a->status_ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(a->statusbar), "main");
    gtk_box_pack_end(GTK_BOX(vbox), a->statusbar, FALSE, FALSE, 0);

    // Controladores/Señales
    g_signal_connect(a->buffer, "notify::modified", G_CALLBACK(on_modified_changed), a);
    g_signal_connect(a->buffer, "mark-set", G_CALLBACK(on_mark_set), a);

// Búsqueda
g_signal_connect(a->search_entry, "changed", G_CALLBACK(on_search_changed), a);
g_signal_connect(a->search_entry, "activate", G_CALLBACK(on_search_activate), a);
GtkEventController *esc = gtk_event_controller_key_new();
g_signal_connect(esc, "key-pressed", G_CALLBACK(on_search_escape), a);
gtk_widget_add_controller(a->search_entry, esc);
g_signal_connect(find_next_btn, "clicked", G_CALLBACK(on_search_activate), a);

gtk_widget_show_all(a->window);
}

// --- ACCIONES / ATAJOS ----------------------------------------------------
static void add_actions(App *a) {
    const GActionEntry entries[] = {
        {"quit", action_quit, NULL, NULL, NULL},
        {"save", action_save, NULL, NULL, NULL},
        {"find", action_find, NULL, NULL, NULL},
    };
    g_action_map_add_action_entries(G_ACTION_MAP(a->app), entries, G_N_ELEMENTS(entries), a);

    // Aceleradores
    const char *accels_quit[] = { "<Primary>q", NULL };
    const char *accels_save[] = { "<Primary>s", NULL };
    const char *accels_find[] = { "<Primary>f", NULL };
    gtk_application_set_accels_for_action(a->app, "app.quit", accels_quit);
    gtk_application_set_accels_for_action(a->app, "app.save", accels_save);
    gtk_application_set_accels_for_action(a->app, "app.find", accels_find);
}

// --- APP LIFECYCLE --------------------------------------------------------
static void on_activate(GtkApplication *app, gpointer user_data) {
    App *a = user_data;
    a->app = app;
    build_ui(a);
    add_actions(a);
    update_status(a);
}

static void on_open(GtkApplication *app, GFile **files, gint n_files, const gchar *hint, gpointer user_data) {
    App *a = user_data;
    if (!a->window) on_activate(app, user_data);
    if (n_files > 0) {
        char *path = g_file_get_path(files[0]);
        if (path) {
            load_file(a, path);
            g_free(path);
        }
    }
}

int main(int argc, char **argv) {
    App app = {0};
    GtkApplication *gtk_app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);

    g_signal_connect(gtk_app, "activate", G_CALLBACK(on_activate), &app);
    g_signal_connect(gtk_app, "open", G_CALLBACK(on_open), &app);

    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    if (app.filename) g_free(app.filename);
    g_object_unref(gtk_app);
    return status;
}
