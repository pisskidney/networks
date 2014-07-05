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

int rsock;
pthread_t* threads[100];
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

struct thrarg {
    int sock;
    int index;
};

int main(int argc, char * argv[]) {

    int i;
    int k;
    int port;
    char ip[20];
    struct sockaddr_in addr;
    unsigned int len;
    char buf[1024];

    if (argc < 3) {
        printf("Usage: ./server ip port\n");
        return 1;
    } else {
        strcpy(ip, argv[1]);
        sscanf(argv[2], "%d", &port);
    }

    rsock = socket(AF_INET, SOCK_DGRAM, 0);

    if (rsock < 0) {
        printf("Error creating socket.\n");
        return 2;
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (bind(rsock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        printf("Error binding socket.\n");
        return 3;
    }

    for (i = 0; i < 100; i++) {
        threads[i] = NULL;
    }

    while (1) {
        printf("[SERVER] Server started.\n");
        k = recvfrom(rsock, buf, 1024, 0, (struct sockaddr *)&addr, &len);
        printf("[SERVER] Received %s from client!", buf);




    }



return 0;
}
