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

int send_hello(int fd) {
    // need to create a buffer for the header
    char buff[4096] = {0};
    // create the header
    proto_hdr_t *hdr = (proto_hdr_t*)buff;
    hdr->type = MSG_HELLO_REQ;
    hdr->length = 1;
    // set the protocol message to the buffer, after the header
    proto_hello_req *hello = (proto_hello_req*)&hdr[1];
    hello->proto = htons(PROTO_VER);

    // convert to endian
    hdr->type = htons(hdr->type);
    hdr->length = htons(hdr->length);

    write(fd, buff, sizeof(proto_hdr_t) + sizeof(proto_hello_req));

    // receive the response


    // return success    
    return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {
    int c;
    char *portarg = NULL;
    char *hostarg = NULL;
    unsigned short port = 0;
    struct sockaddr_in clientInfo = {0};

    if (argc < 2) {
        printf("Usage: %s <ip and port of the host>", argv[0]);
        return 0;
    }

    while((c = getopt(argc, argv, "p:h:")) != -1) {
        switch(c) {
            case 'h':
                hostarg = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
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

    send_hello(fd);
    return 0;
}