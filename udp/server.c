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
    char buf[1024];
    struct sockaddr_in addr;
};

int findFreeThread() {
    int i;

    pthread_mutex_lock(&mtx);
    for(i=0; i<100; i++) {
        if(threads[i] == NULL) {
            pthread_mutex_unlock(&mtx);
            return i;
        }
    }
    pthread_mutex_unlock(&mtx);

    return -1;
};

void serve(struct thrarg* arg) {

    char buf[1024];

    printf("[SERVER] Received %s from client!\n", arg->buf);
    fflush(stdout);

    FILE *fp;
    fp = fopen(arg->buf, "rb");
    
    if (!fp) {
        printf("[SERVER] No such file or file can't be opened!\n");
    } else {
        while (fread(buf, sizeof(buf), 1, fp) == 1) {
            sendto(
                arg->sock, buf, sizeof(buf), 0,
                (struct sockaddr *) &arg->addr, sizeof(arg->addr)
            );
        }
    }

    printf("[SERVER] Worker #%d terminated.\n", arg->index); 

    pthread_mutex_lock(&mtx);
    free(threads[arg->index]);
    threads[arg->index] = NULL;
    free(arg);
    pthread_mutex_unlock(&mtx);
}

int main(int argc, char * argv[]) {

    int i;
    int k;
    int n;
    int port;
    struct sockaddr_in addr;
    unsigned int len;
    char buf[1024];

    if (argc < 2) {
        printf("Usage: ./server port\n");
        return 1;
    } else {
        sscanf(argv[1], "%d", &port);
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

    printf("[SERVER] Server started.\n");

    while (1) {
        n = recvfrom(rsock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len);

        if (n <=0 ) {
            continue;
        }

        k = findFreeThread();
        if(k < 0) {
            printf("[SERVER] All workers are full!\n");
            return 7;
        }

        struct thrarg* ta = (struct thrarg*)malloc(sizeof(struct thrarg));
        ta->sock = rsock;
        ta->index = k;
        ta->addr = addr;
        strcpy(ta->buf, buf);
        threads[k] = (pthread_t*)malloc(sizeof(pthread_t));

        pthread_create(threads[k], NULL, (void *(*)(void*))serve, ta);
        sleep(1);
    }

    for(i=0; i<100; i++) {
        pthread_t* t = threads[i];
        if(t != NULL) {
            pthread_join(*t, NULL);
        }
    }

    close(rsock);

    return 0;
}
