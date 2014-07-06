#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

int main(int argc, char * argv[]) {

    int port;
    char ip[20];
    int sock;
    struct sockaddr_in addr;
    char buf[1024];
    int n;

    if (argc < 3) {
        printf("Usage: ./client ip port\n");
        return 0;
    } else {
        strcpy(ip, argv[1]);
        sscanf(argv[2], "%d", &port);
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        printf("Error creating socket.\n");
        return 2;
    }

    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    printf("[CLIENT] Trying to send something to server...\n");

    strcpy(buf, "img.jpg");
    buf[strlen(buf)] = 0;

    sendto(
        sock, buf, strlen(buf), 0,
        (struct sockaddr *) &addr, sizeof(addr)
    );

    char filename[1024];
    strcpy(filename, "new_");
    strcat(filename, buf);
    FILE *fp;
    fp = fopen(filename, "wb");
    while((n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL)) > 0) {
        printf("[CLIENT] Received %d bytes from server!", n);
        fflush(stdout);
        fwrite(buf, n, 0, fp);
    }

    printf("[CLIENT] Terminated.\n");
    fflush(stdout);

    close(sock);

    return 0;
}
