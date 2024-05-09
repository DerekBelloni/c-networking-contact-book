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

int send_add_req(char *addString, int fd) {
    printf("Add string in send_add_req: %s\n", addString);
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};
    
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = MSG_CONTACT_ADD_REQ;
    hdr->length = sizeof(proto_req) + sizeof(proto_add_req);
    hdr->type = htons(hdr->type);
    hdr->length = htons(hdr->length);

    proto_req *add = (proto_req*)&hdr[1];
    add->proto = htons(PROTO_VER);

    proto_add_req *contact = malloc(sizeof(proto_add_req));
    if (contact == NULL) {
        perror("malloc");
        STATUS_ERROR;
    }

    memset(contact, 0, sizeof(proto_add_req));
    snprintf((char *)contact->data, sizeof(contact->data), "%s", addString);
    memcpy(&add[1], contact, sizeof(proto_add_req));
    write(fd, writeBuffer, sizeof(proto_hdr_t) + sizeof(proto_req) + sizeof(proto_add_req));
    free(contact);

    return 0;
}

int send_request(int fd) {
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};
    
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = MSG_HELLO_REQ;
    hdr->length = sizeof(proto_req);
    hdr->type = htons(hdr->type);
    hdr->length = htons(hdr->length);
    
    proto_req *hello = (proto_req*)&hdr[1];
    hello->proto = htons(PROTO_VER);

    write(fd, writeBuffer, sizeof(proto_hdr_t) + sizeof(proto_req));

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

    if (serverHdr->type == MSG_HELLO_RESP) {
        printf("Relax, breath, its ok\n");
        return STATUS_SUCCESS;
    }
     
    return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {
    int c;
    char *portarg = NULL;
    char *hostarg = NULL;
    unsigned short port = 0;
    struct sockaddr_in clientInfo = {0};
    int initialRequest = 0;  // Flag to determine if initial request should be sent

    if (argc < 2) {
        printf("Usage: %s -h <host> -p <port>\n", argv[0]);
        return 0;
    }

    while ((c = getopt(argc, argv, "h:p:")) != -1) {
        switch (c) {
            case 'h':
                hostarg = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                initialRequest = 1;  // Set flag if port is specified
                break;
            case '?':
                printf("Unknown option -%c\n", optopt);
                return -1;
        }
    }

    if (hostarg == NULL || portarg == NULL) {
        printf("Host and port must be specified.\n");
        return -1;
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

    printf("Connected to %s:%d\n", hostarg, port);

    // Send initial request if appropriate
    if (initialRequest) {
        if (send_request(fd) != STATUS_SUCCESS) {
            printf("Error establishing a hello request\n");
            close(fd);
            return -1;
        }
    }

    printf("Enter commands (e.g., '-a name,email,phone'):\n");

    char input[1024];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0; // Strip newline character

        if (strncmp(input, "-a ", 3) == 0) {
            char *addString = input + 3;  // Point to the contact info part
            printf("add string: %s\n", addString);
            if (send_add_req(addString, fd) != STATUS_SUCCESS) {
                printf("Error adding new contact\n");
                break;
            }
        } else {
            printf("Unknown command or format error. Available commands are '-a [contact info]'.\n");
        }
    }

    close(fd);
    return 0;
}