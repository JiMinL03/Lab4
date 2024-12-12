#include <gtk/gtk.h>

// 버튼 클릭 시 호출되는 콜백 함수
void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Hello, World!\n");
}

int main(int argc, char *argv[]) {
    GtkWidget *window;  // 메인 윈도우
    GtkWidget *button;  // 버튼

    // GTK 초기화
    gtk_init(&argc, &argv);

    // 윈도우 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    // 닫기 버튼 클릭 시 프로그램 종료 설정
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 버튼 생성
    button = gtk_button_new_with_label("Click Me!");

    // 버튼 클릭 시 호출될 콜백 함수 연결
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);

    // 버튼을 윈도우에 추가
    gtk_container_add(GTK_CONTAINER(window), button);

    // 모든 위젯 표시
    gtk_widget_show_all(window);

    // GTK 메인 루프 실행
    gtk_main();

    return 0;
}
