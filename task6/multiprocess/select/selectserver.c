#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 3490
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd, max_fd, activity;
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds, activefds;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Chat server started on port %d\n", PORT);

    FD_ZERO(&activefds);
    FD_SET(server_fd, &activefds);
    max_fd = server_fd;

    while (1) {
        readfds = activefds;
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("Select error");
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &readfds)) {
                if (i == server_fd) {
                    // New client connection
                    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen)) < 0) {
                        perror("Accept failed");
                        continue;
                    }

                    printf("New connection, socket fd: %d\n", client_fd);
                    FD_SET(client_fd, &activefds);
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                } else {
                    // Data from a client
                    int bytes_read = read(i, buffer, BUFFER_SIZE);
                    if (bytes_read <= 0) {
                        // Client disconnected
                        printf("Client disconnected, socket fd: %d\n", i);
                        close(i);
                        FD_CLR(i, &activefds);
                    } else {
                        buffer[bytes_read] = '\0';
                        printf("Message from client %d: %s", i, buffer);

                        // Broadcast to all clients
                        for (int j = 0; j <= max_fd; j++) {
                            if (FD_ISSET(j, &activefds) && j != server_fd && j != i) {
                                send(j, buffer, bytes_read, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
