#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

int send_request(int fd) {
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};
    
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = MSG_HELLO_REQ;
    hdr->length = 1;
    hdr->type = htons(hdr->type);
    hdr->length = htons(hdr->length);
    
    proto_hello_req *hello = (proto_hello_req*)&hdr[1];
    printf("proto ver: %d\n", PROTO_VER);
    hello->proto = htons(PROTO_VER);
    printf("proto ver afte endian: %d\n", hello->proto);

    
    write(fd, writeBuffer, sizeof(proto_hdr_t) + sizeof(proto_hello_req));

    ssize_t bytesRead = read(fd, readBuffer, sizeof(readBuffer));
    if (bytesRead == -1) {
        perror("read");
        exit(1);
    }

    proto_hdr_t *serverHdr = (proto_hdr_t*)readBuffer;

    serverHdr->type = ntohs(serverHdr->type);
    serverHdr->length = ntohs(serverHdr->length);

    if (serverHdr->type == MSG_ERROR) {
        printf("Error type received\n");
        close(fd);
        return STATUS_ERROR;
    }

     
    return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {
    int c;
    char *portarg = NULL;
    char *hostarg = NULL;
    char *addString = NULL;
    unsigned short port = 0;
    struct sockaddr_in clientInfo = {0};

    if (argc < 2) {
        printf("Usage: %s <ip and port of the host>", argv[0]);
        return 0;
    }

    while((c = getopt(argc, argv, "p:h:a:")) != -1) {
        switch(c) {
            case 'h':
                hostarg = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                break;
            case 'a':
                addString = optarg;
                printf("Add string: %s\n", addString);
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    clientInfo.sin_family = AF_INET;
    clientInfo.sin_addr.s_addr = inet_addr(hostarg);
    clientInfo.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&clientInfo, sizeof(clientInfo)) == -1) {
        perror("connect");
        close(fd);
        return -1;
    }

    send_request(fd);
    return 0;
}