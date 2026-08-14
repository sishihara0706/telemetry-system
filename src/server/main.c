// src/server/main.c

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 9000
#define BACKLOG 8
#define BUFFER_SIZE 1024

static volatile sig_atomic_t g_stop_requested = 0;

static void handle_signal(int signo)
{
    (void)signo;
    g_stop_requested = 1;
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    /*
     * TODO 1:
     * Ctrl+C(SIGINT) を受け取ったときに
     * handle_signal() が呼ばれるようにする。
     *
     * signal() または sigaction() を調べてみる。
     */
	signal(SIGINT, handle_signal);

    /*
     * TODO 2:
     * TCP/IPv4 socket を作成する。
     *
     * socket(
     *     address family,
     *     socket type,
     *     protocol
     * )
     *
     * 失敗時の戻り値にも注意。
     */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

    /*
     * TODO 3:
     * SO_REUSEADDR を設定する。
     *
     * setsockopt() を使用する。
     */

    memset(&server_addr, 0, sizeof(server_addr));

    /*
     * TODO 4:
     * server_addr を設定する。
     *
     * family:
     *   IPv4
     *
     * address:
     *   0.0.0.0
     *
     * port:
     *   DEFAULT_PORT
     *
     * htons() / htonl() が何をしているかも確認する。
     */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(DEFAULT_PORT);
	

    /*
     * TODO 5:
     * bind() する。
     *
     * server_fd と server_addr を関連付ける。
     */
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("ERROR on binding");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

		
    /*
     * TODO 6:
     * listen() する。
     *
     * BACKLOG を使用する。
     */
	listen(server_fd, BACKLOG);

    printf("Telemetry Server listening on port %d\n", DEFAULT_PORT);

    /*
     * 今回は複数クライアント同時処理はしない。
     *
     * まずは1クライアントをacceptして、
     * そのクライアントからrecvする。
     */
    while (!g_stop_requested) {

        printf("Waiting for client...\n");

        /*
         * TODO 7:
         * accept() でクライアント接続を受け付ける。
         *
         * client_fd
         * client_addr
         * client_addr_len
         *
         * を使用する。
         *
         * Ctrl+Cによってaccept()が失敗する場合、
         * errno == EINTR についても調べる。
         */
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if(client_fd < 0) {
			perror("ERROR on accepting");
			close(server_fd);
			exit(EXIT_FAILURE);
		}

        printf("Client connected\n");

        while (!g_stop_requested) {

            /*
             * TODO 8:
             * recv() でデータを受信する。
             *
             * 戻り値:
             *
             * > 0 : 受信したバイト数
             * = 0 : クライアント切断
             * < 0 : エラー
             */
			
            ssize_t received = 0;

            /*
             * TODO:
             * received = recv(...);
             */
			received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (received > 0) {

                /*
                 * TODO 9:
                 * 受信した内容をstdoutへ表示する。
                 *
                 * 注意:
                 * recv() が受信するデータは
                 * C文字列とは限らない。
                 *
                 * received バイトだけ扱う方法を考える。
                 */
				printf("Received item: %s\n", buffer);

            } else if (received == 0) {

                printf("Client disconnected\n");
                break;

            } else {

                if (errno == EINTR && g_stop_requested) {
                    break;
                }

                perror("recv");
                break;
            }
        }

        /*
         * TODO 10:
         * client_fd をcloseする。
         */
		close(client_fd);
        client_fd = -1;

        /*
         * Issue #2では、
         * ここで終了してもいいし、
         * 次のclientをacceptしてもよい。
         *
         * ただし後続Issueの
         * "multiple sequential clients"
         * を先取りしすぎないよう注意。
         */

        break;
    }

    /*
     * TODO 11:
     * server_fd をcloseする。
     */
	close(server_fd);
    printf("Telemetry Server stopped\n");

    return EXIT_SUCCESS;
}
