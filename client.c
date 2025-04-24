#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PEERIP "127.0.0.1"
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
    int serv_addr_len;
    unsigned int timestamp; /*自分の時刻*/
    unsigned int buffer;    /*相手から受け取った時刻*/

    struct nodemap nm;

    timestamp = 0;
    nm.self = ALIVE;
    nm.peer = ALIVE;

    if (&serv_addr == NULL) {
        perror("serv_addr is NULL");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(PEERIP);
    serv_addr.sin_port = htons(PORT);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (1) {
        struct timeval timeout;
        fd_set sock_set;
        FD_ZERO(&sock_set);
        FD_SET(sock, &sock_set);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_USEC;
        /*********************************送信イベント*********************************/
        timestamp++; /*送信イベント*/
        int ret = select(sock + 1, &sock_set, NULL, NULL, &timeout);
        printf("[%d]\tClient send\n", timestamp);
        if (sendto(sock, &timestamp, sizeof(timestamp), 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("sendto() failed");
        }
        /*****************************************************************************/
        /*********************************受信イベント*********************************/
        if (ret == 0) {
            /*タイムアウト*/
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
        } else if (ret < 0) {
            /*タイムアウト以外の異常は終了*/
            perror("recvfrom() failed");
            exit(EXIT_FAILURE);
        } else {
            if (FD_ISSET(sock, &sock_set)) {
                serv_addr_len = sizeof(serv_addr);
                if (recvfrom(sock, &buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&serv_addr, &serv_addr_len) < 0) {
                    perror("recvfrom() failed");
                }
                timestamp = ((timestamp < buffer) ? buffer : timestamp) +
                            1; /*受信イベント*/
                printf("[%d]\tClient recv\n", timestamp);
            } else {
                perror("FD_ISSET() failed");
            }
        }

        /*****************************************************************************/
        sleep(INTERVAL);
    }
    return 0;
}