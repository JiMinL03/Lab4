#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8081
#define BUFFER_SIZE 4096

void handle_client(int client_sock);
void handle_get(int client_sock, char *path);
void handle_post(int client_sock, char *path, char *body);
void execute_cgi(int client_sock, char *path, char *body);

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Web server started on port %d\n", PORT);

    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) == -1) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected\n");

        handle_client(client_sock);
        close(client_sock);
    }

    close(server_sock);
    return 0;
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    char method[16], path[256], protocol[16];
    char *body = NULL;

    int received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (received < 0) {
        perror("Receive failed");
        return;
    }
    buffer[received] = '\0';

    printf("Request:\n%s\n", buffer);

    sscanf(buffer, "%s %s %s", method, path, protocol);

    // Parse POST body
    body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4; // Skip the "\r\n\r\n"
    }

    if (strcmp(method, "GET") == 0) {
        handle_get(client_sock, path);
    } else if (strcmp(method, "POST") == 0) {
        handle_post(client_sock, path, body);
    } else {
        char *response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
    }
}

void handle_get(int client_sock, char *path) {
    char filepath[256];
    FILE *file;
    char response[BUFFER_SIZE];
    size_t bytes_read;

    if (strcmp(path, "/") == 0) {
        // 기본 경로 요청 시 hello.cgi 실행
        execute_cgi(client_sock, "./cgi-bin/hello.cgi", NULL);
        return;
    }

    snprintf(filepath, sizeof(filepath), ".%s", path);

    file = fopen(filepath, "r");
    if (!file) {
        char *not_found = "HTTP/1.1 404 Not Found\r\n\r\nFile not found.";
        send(client_sock, not_found, strlen(not_found), 0);
        return;
    }

    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n");
    send(client_sock, response, strlen(response), 0);

    while ((bytes_read = fread(response, 1, BUFFER_SIZE, file)) > 0) {
        send(client_sock, response, bytes_read, 0);
    }

    fclose(file);
}

void handle_post(int client_sock, char *path, char *body) {
    if (strstr(path, "/cgi-bin") == path) {
        execute_cgi(client_sock, path + 9, body); // Skip "/cgi-bin"
    } else {
        char *response = "HTTP/1.1 404 Not Found\r\n\r\nCGI not found.";
        send(client_sock, response, strlen(response), 0);
    }
}

void execute_cgi(int client_sock, char *path, char *body) {
    int pipe_fd[2];
    pid_t pid;

    if (pipe(pipe_fd) < 0) {
        perror("Pipe failed");
        return;
    }

    if ((pid = fork()) < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        // 자식 프로세스
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);  // CGI 프로그램의 출력이 클라이언트로 전달되도록 설정
        close(pipe_fd[1]);

        setenv("CONTENT_LENGTH", body ? strlen(body) : "0", 1);  // POST 데이터 길이 설정
        setenv("REQUEST_METHOD", "POST", 1);  // 요청 메서드 설정
        setenv("QUERY_STRING", body ? body : "", 1);  // POST body 데이터를 쿼리 문자열로 설정
        
        // CGI 프로그램 실행
        execl(path, path, NULL);
        perror("Exec failed");  // exec 실패시 출력
        exit(EXIT_FAILURE);
    } else {
        // 부모 프로세스
        close(pipe_fd[1]);

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n");
        send(client_sock, response, strlen(response), 0);

        int bytes_read;
        while ((bytes_read = read(pipe_fd[0], response, BUFFER_SIZE)) > 0) {
            send(client_sock, response, bytes_read, 0);
        }

        close(pipe_fd[0]);
        waitpid(pid, NULL, 0);
    }
}