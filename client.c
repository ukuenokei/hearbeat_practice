#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PEERIP "127.0.0.1"
#define PORT 8000
#define ECHOSTRING "ImAlive"
#define BUFSIZE 16
#define TIMEOUT 1
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
    char message[BUFSIZE] = ECHOSTRING;
    char buffer[BUFSIZE];
    struct timeval timeout;
    struct nodemap nm;

    // timeout.tv_sec = 0;
    // timeout.tv_usec = 100 * 1000; /*100msec*/
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
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

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
        0) {
        perror("setsockopt(RCVTIMEO) failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (1) {
        if (sendto(sock, message, sizeof(message), 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("sendto() failed");
        }
        /**********************************サーバーからの確認処理***********************************/
        serv_addr_len = sizeof(serv_addr);
        if (recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&serv_addr,
                     &serv_addr_len) < 0) {
            /*recvfromが失敗したとき*/
            if (errno == EWOULDBLOCK) {
                /*タイムアウト時のみ継続*/
                printf("recvfrom() timeout\n");
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
        if (strcmp(buffer, message) != 0) {
            /*期待したメッセージを受信しなかった場合*/
            printf("Message Error : %s\n",buffer);
            break;
        }
        /*期待したメッセージを受信したらここに来る
        recvfrom()> 0 -> strcmp()==0*/
        printf("Peer server is alive\n");
        nm.peer = ALIVE;
        sleep(INTERVAL);
        /*****************************************************************************************/
    }
    return 0;
}