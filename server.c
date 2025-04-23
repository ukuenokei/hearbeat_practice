#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 8000
#define ECHOSTRING "ImAlive"
#define BUFSIZE 16
#define TIMEOUT_SEC 5
#define TIMEOUT_USEC 0
#define INTERVAL 3
#define RETRY_MAX 3

#define ALIVE 0
#define DEAD 1
struct nodemap {
    int self;
    int peer;
};

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    int client_addr_len;
    unsigned int timestamp; /*自分の時刻*/
    unsigned int buffer;    /*相手から受け取った時刻*/
    struct timeval timeout;
    struct nodemap nm;

    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_USEC;
    timestamp = 0;

    if (&serv_addr == NULL) {
        perror("serv_addr is NULL");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
        0) {
        perror("setsockopt(RCVTIMEO) failed");
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (1) {
        /*********************************受信イベント*********************************/
        client_addr_len = sizeof(client_addr);
        if (recvfrom(sock, &buffer, sizeof(buffer), 0,
                (struct sockaddr *)&client_addr, &client_addr_len) < 0) {
            /*recvfromが失敗したとき*/
            if (errno == EWOULDBLOCK) {
                /*タイムアウト時のみ継続*/
                printf("recvfrom() timeout retry %d\n", i);
                i++; /*タイムアウトカウント*/
                if (i == RETRY_MAX) {
                    nm.peer = DEAD;
                }
                if (nm.peer == DEAD) {
                    printf("Peer server is dead\n");
                    break;
                }
                continue;
            } else {
                /*それ以外は終了*/
                perror("recvfrom() failed");
                exit(EXIT_FAILURE);
            }
        }
        // printf("Hndling client : %s\n", inet_ntoa(client_addr.sin_addr));
        timestamp = ((timestamp < buffer) ? buffer : timestamp) + 1;
        printf("[%d]\tServer recv\n", timestamp);
        /*****************************************************************************/
        /*********************************送信イベント*********************************/
        timestamp++; /*送信イベント*/
        if (sendto(sock, &timestamp, sizeof(timestamp), 0,
                (struct sockaddr *)&client_addr, client_addr_len) < 0) {
            perror("sendto() failed");
        }
        /*****************************************************************************/
        printf("[%d]\tServer send\n", timestamp);
    }

    return 0;
}