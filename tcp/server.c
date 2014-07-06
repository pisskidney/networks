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

void signal_handler(int sig) {
    if (close (rsock) < 0) {
        perror("\nCould not close socket\n");
        return;
    }
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
    int i;
    int k;
    char buf[1024];
    int nr_ints;
    int nr_floats;
    int ints[1024];
    float floats[1024];
    float sum = 0;

    k = recv(arg->sock, buf, 1024, 0);

    buf[k] = 0;

    //Read ints and floats
    memcpy(&nr_ints, buf, sizeof(int));
    for (i = 0; i < nr_ints; i++) {
        memcpy(&ints[i], buf+sizeof(int)*(i+1), sizeof(int));
    }
    memcpy(&nr_floats, buf+sizeof(int)*(nr_ints+1), sizeof(int));
    for (i = 0; i < nr_floats; i++) {
        memcpy(&floats[i], buf+(nr_ints+2)*sizeof(int)+sizeof(float)*i, sizeof(float));
    }

    //Calculate sum
    for (i = 0; i < nr_ints; i++) {
        sum += ints[i];
    }
    for (i = 0; i < nr_floats; i++) {
        sum += floats[i];
    }

    printf("[SERVER] Sum = %0.2f\n", sum);
    fflush(stdout);

    memcpy(buf, &sum, sizeof(float));
    buf[sizeof(float)+1] = 0;
    send(arg->sock, buf, sizeof(float), 0);

    pthread_mutex_lock(&mtx);
    free(threads[arg->index]);
    threads[arg->index] = NULL;
    close(arg->sock);
    free(arg);
    pthread_mutex_unlock(&mtx);
};

int main(int argc, char * argv[]) {

    int sock;
    unsigned int len;
    int port;
    struct sockaddr_in addr;
    int i;
    int k;

    if (argc < 2) {
        printf("Usage: ./server port\n");
        return 1;
    } else {
        sscanf(argv[1], "%d", &port);
    }

    rsock = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(rsock, 10) < 0) {
        printf("Error listening specified socket.\n");
        return 4;
    }

    for (i = 0; i < 100; i++) {
        threads[i] = NULL;
    }

    signal(SIGINT, signal_handler);

    while (1) {
        printf("[SERVER] Server started.\n");
        sock = accept(rsock, (struct sockaddr *) &addr, &len);

        printf("[SERVER] Connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        k = findFreeThread();
        if(k < 0) {
            close(sock);
            continue;
        }

        struct thrarg* ta = (struct thrarg*)malloc(sizeof(struct thrarg));
        ta->sock = sock;
        ta->index = k;
        threads[k] = (pthread_t*)malloc(sizeof(pthread_t));

        pthread_create(threads[k], NULL, (void *(*)(void*))serve, ta);
    }

    for(i=0; i<100; i++) {
        pthread_t* t = threads[i];
        if(t != NULL) {
            pthread_join(*t, NULL);
        }
    }

    return 0;
}
