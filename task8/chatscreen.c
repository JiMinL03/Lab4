#include <gtk/gtk.h>

// Structure to hold widgets
typedef struct {
    GtkTextView *text_view;
    GtkEntry *entry;
    const gchar *username;  // 현재 사용자의 이름을 저장
    GtkTextTag *right_align_tag;  // 오른쪽 정렬 태그
    GtkTextTag *left_align_tag;   // 왼쪽 정렬 태그
} Widgets;

// Callback function to handle send button click
gboolean on_send_button_clicked(GtkWidget *widget, gpointer user_data) {
    Widgets *widgets = (Widgets *)user_data;

    const gchar *message = gtk_entry_get_text(widgets->entry);

    if (message && *message) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(widgets->text_view);
        GtkTextIter end_iter;

        gtk_text_buffer_get_end_iter(buffer, &end_iter);

        // 본인 메시지는 오른쪽 정렬로 출력
        if (g_strcmp0(widgets->username, "my_username") == 0) {
            gtk_text_buffer_insert_with_tags(buffer, &end_iter, message, -1, widgets->right_align_tag, NULL);
        } else {
            // 다른 클라이언트의 메시지는 왼쪽 정렬로 출력
            gtk_text_buffer_insert_with_tags(buffer, &end_iter, message, -1, widgets->left_align_tag, NULL);
        }

        gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

        gtk_entry_set_text(widgets->entry, ""); // Clear the input field
    }

    return FALSE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *text_view;
    GtkWidget *entry;
    GtkWidget *send_button;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Chat App");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create vertical box layout
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create text view for chat messages
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_window), text_view);
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);

    // Create horizontal layout for entry and button
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(hbox), send_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Create structure to hold widgets
    Widgets widgets = {GTK_TEXT_VIEW(text_view), GTK_ENTRY(entry), "my_username", NULL, NULL};  // 현재 사용자 이름 설정

    // Get text buffer and tag table
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(widgets.text_view);
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);

    // Check if tags already exist, otherwise create and add them
    widgets.right_align_tag = gtk_text_tag_new("right-align");
    g_object_set(widgets.right_align_tag, "justification", GTK_JUSTIFY_RIGHT, NULL);
    gtk_text_tag_table_add(tag_table, widgets.right_align_tag);

    widgets.left_align_tag = gtk_text_tag_new("left-align");
    g_object_set(widgets.left_align_tag, "justification", GTK_JUSTIFY_LEFT, NULL);
    gtk_text_tag_table_add(tag_table, widgets.left_align_tag);

    // Connect send button to the handler
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), &widgets);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
