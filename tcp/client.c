#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {

    char ip[20];
    int port;
    struct sockaddr_in addr;
    int sock;
    char buf[1024];
    int i;
    int k;
    
    if (argc < 3) {
        printf("Usage: ./client ip port\n");
        return 0;
    } else {
        strcpy(ip, argv[1]);
        sscanf(argv[2], "%d", &port);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error creating socket.\n");
        return 2;
    }

    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        printf("Could not connect to host.\n");
        return 3;
    }

    printf("[CLIENT] Connected to host.\n");

    //Construct request
    //3 integers, 2 floats (2 decimals)
    int ints[3] = {23, 29, 31};
    float floats[2] = {13.37, 800.85};

    int nr_ints = sizeof(ints)/sizeof(ints[0]);
    int nr_floats = sizeof(floats)/sizeof(floats[0]);
    memcpy(buf, &nr_ints, sizeof(int));
    for (i = 0; i < nr_ints; i++) {
        memcpy(buf+(i+1)*sizeof(int), &ints[i], sizeof(int));
    }
    memcpy(buf+(nr_ints+1)*sizeof(int), &nr_floats, sizeof(int));
    for (i = 0; i < nr_floats; i++) {
        memcpy(buf+(nr_ints+2)*sizeof(int)+sizeof(float)*i, &floats[i], sizeof(float));
    }

    send(sock, buf, sizeof(int)*(nr_ints+2)+sizeof(float)*nr_floats, 0);
    
    while(1) {
        k = recv(sock, buf, 1024, 0);
        if(k <= 0) {
            break;
        }

        float sum = 0;
        memcpy(&sum, buf, sizeof(float));
        printf("[CLIENT] Received sum from server! sum = %0.2f.\n", sum);
    }

    if (close(sock) < 0) {
        printf("Could not close socket\n");
        return 5;
    }

    return 0;
}
