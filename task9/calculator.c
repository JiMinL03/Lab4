#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef struct {
    GtkEntry *entry;
} Widgets;

// 간단한 계산기 구현 함수 (단순한 덧셈, 뺄셈, 곱셈, 나눗셈만 처리)
double evaluate_expression(const gchar *expr) {
    double result = 0.0;
    double current_value = 0.0;
    char operator = '+';
    const gchar *ptr = expr;

    while (*ptr != '\0') {
        if (isdigit(*ptr) || (*ptr == '.' && isdigit(*(ptr + 1)))) {
            double num = strtod(ptr, (char **)&ptr);
            if (operator == '+') {
                current_value += num;
            } else if (operator == '-') {
                current_value -= num;
            } else if (operator == '*') {
                current_value *= num;
            } else if (operator == '/') {
                if (num != 0) {
                    current_value /= num;
                } else {
                    return NAN;  // 나누기 0 오류 처리
                }
            }
        } else if (*ptr == '+' || *ptr == '-' || *ptr == '*' || *ptr == '/') {
            operator = *ptr;
            ptr++;
        } else {
            ptr++;
        }
    }

    result = current_value;
    return result;
}

// 버튼 클릭 시 처리 함수
void on_button_clicked(GtkWidget *widget, gpointer user_data) {
    Widgets *widgets = (Widgets *)user_data;
    const gchar *text = gtk_button_get_label(GTK_BUTTON(widget));
    const gchar *current_text = gtk_entry_get_text(widgets->entry);

    // 'C' 버튼 클릭 시 입력 필드 초기화
    if (g_strcmp0(text, "C") == 0) {
        gtk_entry_set_text(widgets->entry, "");
    }
    // '=' 버튼 클릭 시 계산
    else if (g_strcmp0(text, "=") == 0) {
        const gchar *expr = gtk_entry_get_text(widgets->entry);  // 변경: const gchar*
        double result = evaluate_expression(expr);

        if (isnan(result)) {
            gtk_entry_set_text(widgets->entry, "Error");
        } else {
            gchar result_str[64];
            g_snprintf(result_str, sizeof(result_str), "%f", result);
            gtk_entry_set_text(widgets->entry, result_str);
        }
    } else {
        // 숫자 및 연산자 버튼 클릭 시 입력 필드에 추가
        gchar *new_text = g_strconcat(current_text, text, NULL);
        gtk_entry_set_text(widgets->entry, new_text);
        g_free(new_text);
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *entry;
    GtkWidget *buttons[4][4];
    const gchar *button_labels[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"C", "0", "=", "+"}
    };
    Widgets widgets;

    gtk_init(&argc, &argv);

    // 창 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 그리드 레이아웃 생성
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // 입력 필드 생성
    entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 4, 1);

    // 입력 필드를 활성화 가능하도록 설정 (편집 가능 상태로)
    gtk_widget_set_sensitive(entry, TRUE);
    widgets.entry = GTK_ENTRY(entry);

    // 버튼 생성 및 그리드에 배치
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            buttons[row][col] = gtk_button_new_with_label(button_labels[row][col]);
            gtk_grid_attach(GTK_GRID(grid), buttons[row][col], col, row + 1, 1, 1);
            g_signal_connect(buttons[row][col], "clicked", G_CALLBACK(on_button_clicked), &widgets);
        }
    }

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
