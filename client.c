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
#define TIMEOUT_SEC 5
#define TIMEOUT_USEC 0
#define INTERVAL 3
#define RETRY_MAX 3
#define MAX_NODENUM 2

#define ALIVE 0
#define DEAD 1

#define RETURN_SUCCESS 0
#define RETURN_FAILURE 1
#define RETURN_TIMEOUT 2
struct nodemap {
    int self;
    int peer;
};
struct nodemap nm;

// int send_timestamp(unsigned int timestamp, int sock, struct sockaddr_in peer_addr) {
//     if (sendto(sock, &timestamp, sizeof(timestamp), 0,
//                (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
//         perror("sendto() failed");
//         return RETURN_FAILURE;
//     }
//     return EXIT_SUCCESS;
// }

int recv_timestamp(unsigned int *buffer, int sock, struct sockaddr_in peer_addr, int to_count) {
    int ret_val = EXIT_SUCCESS;
    int i = 0;
    int serv_addr_len = sizeof(peer_addr);
    if (recvfrom(sock, buffer, sizeof(*buffer), 0,
                 (struct sockaddr *)&peer_addr, &serv_addr_len) < 0) {
        if (errno == EWOULDBLOCK) {
            i++; /*タイムアウトカウント増加*/
            printf("recvfrom() timeout retry %d\n", i);
        } else {
            perror("recvfrom() failed");
            return RETURN_FAILURE;
        }
        if (nm.peer == DEAD) {
            printf("Peer server is dead\n");
            return RETURN_FAILURE;
        }
    }
}

void print_event(char *str, unsigned int *vc, int node_num) {
    int i;
    for (i = 0; i < node_num; i++) {
        printf("[%d]\t", vc[i]);
    }
    printf("%d\t %d\t", nm.self, nm.peer);
    printf("%s\n", str);
}

int main(int argc, char **argv) {
    int sock;
    struct sockaddr_in serv_addr;
    int serv_addr_len;
    struct timeval timeout;
    int node_num = MAX_NODENUM; /*FIXME:今は仮で2ノードのみ取り扱う*/
    int self_nodenum;
    unsigned int vc[MAX_NODENUM];
    unsigned int buffer[MAX_NODENUM]; /*送受信用バッファ*/
    int retry_count = 0;
    if (argc != 2) {
        printf("Usage: %s node number\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    self_nodenum = atoi(argv[1]);

    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = TIMEOUT_USEC;
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
    memset(&vc, 0, sizeof(vc));

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
        0) {
        perror("setsockopt(RCVTIMEO) failed");
        exit(EXIT_FAILURE);
    }
    print_event("Program start", vc, node_num);

    while (1) {
        /*********************************送信イベント*********************************/
        serv_addr_len = sizeof(serv_addr);
        vc[self_nodenum]++; /*送信を一つのイベントとする*/
        memcpy(buffer, vc, sizeof(vc));
        if (sendto(sock, &buffer, sizeof(buffer), 0,
                   (struct sockaddr *)&serv_addr, serv_addr_len) < 0) {
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }
        print_event("Client send", vc, node_num);
        /*****************************************************************************/
        sleep(INTERVAL);
        /*********************************受信イベント*********************************/
        if (recvfrom(sock, &buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&serv_addr, &serv_addr_len) < 0) {
            if (errno == EWOULDBLOCK) {
                /*タイムアウト時のみ継続*/
                printf("recvfrom() timeout retry %d\n", retry_count);
                retry_count++; /*タイムアウトカウント*/
                if (retry_count == RETRY_MAX) {
                    nm.peer = DEAD;
                }
                if (nm.peer == DEAD) {
                    printf("Peer server is dead\n");
                    // break;
                }
                // continue;
            } else {
                /*それ以外は終了*/
                perror("recvfrom() failed");
                exit(EXIT_FAILURE);
            }
        } else {
            retry_count = 0;
            nm.peer = ALIVE;
            vc[self_nodenum]++; /*メッセージ受信イベントで自身のクロックを増加*/
            /*自身のベクトルクロックの値と受信したペアのベクトルの値（各要素について）の
            最大値をとってベクトル内の各要素を更新*/
            for (int j = 0; j < node_num; j++) {
                vc[j] = (vc[j] < buffer[j]) ? buffer[j] : vc[j];
            }
            print_event("Client recv", vc, node_num);
        }
        /*****************************************************************************/
        // nm.peer = ALIVE;
        sleep(INTERVAL);
    }
    return 0;
}