// src/simulator/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 9000
#define BUFFER_SIZE 1024

int main(void)
{
	int client_fd;
	struct sockaddr_in server_addr;
    char message[] = "Hello Server";
    //char buffer[BUFFER_SIZE] = {0};

	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0) {
		perror("ERROR socket error");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DEFAULT_PORT);
	if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) {
		perror("ERROR inet_pton");
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("ERROR socket error");
		close(client_fd);
		exit(EXIT_FAILURE);
	}
	
	printf(">");
	scanf("%s", message);
	
	ssize_t send_n = send(client_fd, message, strlen(message), 0);
	if (send_n < 0) {
		perror("ERROR on sending");
		close(client_fd);
		exit(EXIT_FAILURE);
	}

	//ssize_t read_n = read(client_fd, buffer, BUFFER_SIZE - 1);
	
	//if (read_n < 0) {
		//perror("ERROR on reading");
		//close(client_fd);
		//exit(EXIT_FAILURE);
	//}
	//if( read_n == 0 ) {
		//printf("server closed connection\n");
	//}
	//else {
		//buffer[read_n] = '\n';
		//printf("Server reply: %s\n", buffer);
	//}
	//
	close(client_fd);

    return 0;
}
