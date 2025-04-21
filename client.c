#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PEERIP "127.0.0.1"
#define PORT 8000
#define ECHOSTRING "ImAlive"
#define BUFSIZE 16
#define LOOP_MAX 1000000
#define INTERVAL_SEC 3

int main()
{
    int sock;
    struct sockaddr_in serv_addr;
    int serv_addr_len;
    char message[BUFSIZE] = ECHOSTRING;
    char buffer[BUFSIZE];

    if (&serv_addr == NULL)
    {
        perror("serv_addr is NULL");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(PEERIP);
    serv_addr.sin_port = htons(PORT);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < LOOP_MAX; i++)
    {
        if (sendto(sock, message, sizeof(message), 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }
        serv_addr_len = sizeof(serv_addr);
        if (recvfrom(sock, buffer, BUFSIZE, 0,
                     (struct sockaddr *)&serv_addr, &serv_addr_len) < 0)
        {
            perror("recvfrom() failed");
            exit(EXIT_FAILURE);
        }
        // printf("%s\n", buffer);
        if (strcmp(buffer, message) == 0){
            //printf("Peer server is alive\n");
            // sleep(INTERVAL_SEC);
        }
        else{
            printf("Peer server is down\n");
            break;
        }
    }

    if (close(sock) < 0)
    {
        perror("close() failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}