#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024



char webpage[] = "HTTP/1.1 200 OK\r\n"
								 "Server:Linux Web Server"
								 "Content-Type: text/html; charset=UTF-8\r\n\r\n"
								 "<!DOCTYPE html>\r\n"
								 "<html><head><title> My Web Page </title>\r\n"
								 "<style>body {background-color: #FFFF00 }</style></head>\r\n"
								 "<body><center><h1>Hello world!!</h1><br>\r\n"
								 "<img src=\"game.jpg\"></center></body></html>\r\n";


void send_response_header(int client_sock) {
    char header[BUFFER_SIZE];
    strcpy(header, "HTTP/1.1 200 OK\n");
    strcat(header, "Content-Type: text/html\n\n");
    send(client_sock, header, strlen(header), 0);
}

void handle_get_request(int client_sock) {
    char html_response[] = "HTTP/1.1 200 OK\r\n"
								 "Server:Linux Web Server"
								 "Content-Type: text/html; charset=UTF-8\r\n\r\n"
								 "<!DOCTYPE html>\r\n"
								 "<html><head><title> My Web Page </title>\r\n"
								 "<style>body {background-color: #FFFF00 }</style></head>\r\n"
								 "<body><center><h1>Hello world!!</h1><br>\r\n"
								 "<img src=\"game.jpg\"></center></body></html>\r\n";

    send_response_header(client_sock);
    send(client_sock, html_response, strlen(html_response), 0);
}

void handle_post_request(int client_sock, char* data) {
    char response[BUFFER_SIZE];
    sprintf(response, "<html><body><h1>POST Data: %s</h1></body></html>", data);
    send_response_header(client_sock);
    send(client_sock, response, strlen(response), 0);
}

void handle_client_request(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    recv(client_sock, buffer, sizeof(buffer), 0);

    if (strncmp(buffer, "GET", 3) == 0) {
        handle_get_request(client_sock);
    } else if (strncmp(buffer, "POST", 4) == 0) {
        char* content_ptr = strstr(buffer, "\r\n\r\n");
        if (content_ptr != NULL) {
            char* data = content_ptr + 4;
            handle_post_request(client_sock, data);
        }
    }
    close(client_sock);
}

char* readImageFile(const char* filename, size_t* file_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char* content = (char*)malloc(*file_size);
    if (!content) {
        fclose(file);
        fprintf(stderr, "Failed to allocate memory for file content.\n");
        return NULL;
    }

    if (fread(content, 1, *file_size, file) != *file_size) {
        fclose(file);
        free(content);
        fprintf(stderr, "Failed to read file: %s\n", filename);
        return NULL;
    }

    fclose(file);
    return content;
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

		if(argc!=2)
		{
			printf("Usage :%s <port>\n",argv[0]);
			exit(1);
		}
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server started. Listening on port %s...\n",argv[1] );

	while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("Failed to accept client connection");
            continue;
        }

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0) {
            perror("Failed to receive client request");
            close(client_sock);
            continue;
        }

        char* request_line = strtok(buffer, "\r\n");
        char* method = strtok(request_line, " ");
        char* path = strtok(NULL, " ");

        if (strcmp(path, "/") == 0) {

            send(client_sock, webpage, strlen(webpage), 0);
        } else if (strcmp(path, "/game.jpg") == 0) {

            size_t image_size;
            char* image_content = readImageFile("game.jpg", &image_size);
            if (!image_content) {
                const char* response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                send(client_sock, response, strlen(response), 0);
            } else {

                const char* response_template =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: Linux Web Server\r\n"
                    "Content-Type: image/jpeg\r\n"
                    "Content-Length: %zu\r\n\r\n";
                int response_size = snprintf(NULL, 0, response_template, image_size);
                char* http_response = (char*)malloc(response_size + image_size);
                if (!http_response) {
                    fprintf(stderr, "Failed to allocate memory for HTTP response.\n");
                    free(image_content);
                    close(client_sock);
                    continue;
                }

                sprintf(http_response, response_template, image_size);
                memcpy(http_response + response_size, image_content, image_size);


                send(client_sock, http_response, response_size + image_size, 0);

                free(http_response);
                free(image_content);
            }
        } else {

            const char* response = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_sock, response, strlen(response), 0);
        }

        close(client_sock);
    }
    close(server_sock);
    return 0;
}

