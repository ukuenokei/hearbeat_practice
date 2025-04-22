#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define ECHOSTRING "ImAlive"
#define BUFSIZE 16

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    int client_addr_len;

    char message[BUFSIZE] = ECHOSTRING;
    char buffer[BUFSIZE];

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

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        client_addr_len = sizeof(client_addr);
        if (recvfrom(sock, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client_addr, &client_addr_len) < 0) {
            perror("recvfrom() failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Hndling client : %s\n", inet_ntoa(client_addr.sin_addr));
        }
        if (sendto(sock, message, sizeof(message), 0,
                   (struct sockaddr *)&client_addr, client_addr_len) < 0) {
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}