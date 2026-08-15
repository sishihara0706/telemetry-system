// src/simulator/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 9000

int main(void)
{
	int client_fd;
	struct sockaddr_in server_addr;
    const char message[] = "Hello Server";

	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DEFAULT_PORT);
	if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) {
		perror("inet_pton");
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("connect");
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	size_t message_len = strlen(message);
	
	ssize_t send_n = send(client_fd, message, strlen(message), 0);

	// send()自体が失敗したか、システムコールが失敗したかを確かめている
	if (send_n < 0) {
		perror("send");
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	// 要求した全バイトを遅れたかを確かめている
	// ここに引っかかるということは未送信のバイトがある
	if ((size_t)send_n != message_len) {
		fprintf(stderr, "partial send\n");
		// partail send = 部分的な送信
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	close(client_fd);

    return 0;
}
