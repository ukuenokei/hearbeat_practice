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
#define MONITOR_MAX 1000000 /*HBをやり取りする最大値*/
#define TIMEOUT 1
#define INTERVAL 3
#define RETRY_MAX 5

#define ALIVE 0
#define DEAD 1

struct nodemap {
  /*Alive:0, Down 1*/
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

  for (int i = 0; i < MONITOR_MAX; i++) {
    if (nm.peer == DEAD) break;
    if (sendto(sock, message, sizeof(message), 0, (struct sockaddr *)&serv_addr,
               sizeof(serv_addr)) < 0) {
      perror("sendto() failed");
    }

    serv_addr_len = sizeof(serv_addr);

    /**********************************サーバーからの確認処理***********************************/
    if (recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&serv_addr,
                 &serv_addr_len) < 0) {
      /*タイムアウト時のみ継続、そうでないなら落とす*/
      if (errno == EWOULDBLOCK) {
        printf("recvfrom() timeout retry\n");
        nm.peer = DEAD;
        break;
      } else {
        perror("recvfrom() failed");
        exit(EXIT_FAILURE);
      }
    }
    printf("Peer server is alive\n");
    sleep(INTERVAL);
  }
  /*****************************************************************************************/
  if (close(sock) < 0) {
    perror("close() failed");
    exit(EXIT_FAILURE);
  }
  return 0;
}