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

int process_command() {

   return 0; 
}

int send_update_req(char *updateString, char *filepath, int fd) {
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};

    //Header
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = htons(MSG_CONTACT_UPDATE_REQ);
    hdr->length = htons(sizeof(proto_req) + sizeof(proto_update_req) + sizeof(proto_file_path));

    // Request
    proto_req *req = (proto_req*)(hdr + 1);
    req->proto = htons(PROTO_VER);

    // Update contact data
    proto_update_req *update = (proto_update_req*)(req + 1);
    snprintf((char*)update->data, sizeof(update->data), "%s", updateString);

    // File path
    proto_file_path *path = (proto_file_path*)(update + 1);
    snprintf((char*)path->path, sizeof(path->path), "%s", filepath);

    ssize_t bytesWritten = write(fd, writeBuffer, ntohs(hdr->length) +sizeof(proto_hdr_t));
    if (bytesWritten < 0) {
        perror("write failed");
        return STATUS_ERROR;
    }

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

    if (serverHdr->type == MSG_CONTACT_UPDATE_RESP) {
        printf("Server update contact response received\n");
        return STATUS_SUCCESS;
    }

    return 0;
}

int send_add_req(char *addString, char *filepath, int fd) {
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};

    // Header
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = htons(MSG_CONTACT_ADD_REQ);
    hdr->length = htons(sizeof(proto_req) + sizeof(proto_add_req) + sizeof(proto_file_path));

    // Request
    proto_req *req = (proto_req*)(hdr + 1);
    req->proto = htons(PROTO_VER);

    // Add contact data
    proto_add_req *contact = (proto_add_req*)(req + 1);
    snprintf((char*)contact->data, sizeof(contact->data), "%s", addString);

    // File path
    proto_file_path *path = (proto_file_path*)(contact + 1);
    snprintf((char*)path->path, sizeof(path->path), "%s", filepath);

    // Debug: Print calculated addresses and their expected sizes
    printf("Header starts at: %p, size: %zu\n", (void*)hdr, sizeof(proto_hdr_t));
    printf("Request starts at: %p, size: %zu\n", (void*)req, sizeof(proto_req));
    printf("Contact data starts at: %p, size: %zu\n", (void*)contact, sizeof(proto_add_req));
    printf("File path starts at: %p, size: %zu\n", (void*)path, sizeof(proto_file_path));

    // Send the buffer
    ssize_t bytes_written = write(fd, writeBuffer, hdr->length + sizeof(proto_hdr_t));
    if (bytes_written < 0) {
        perror("write failed");
        return STATUS_ERROR;
    }

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
    
    if (serverHdr->type == MSG_CONTACT_ADD_RESP) {
        printf("Server add contact response received\n");
        return STATUS_SUCCESS;
    }

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
        printf("Server hello response received\n");
        return STATUS_SUCCESS;
    }
     
    return STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {
    int c;
    char *portarg = NULL;
    char *hostarg = NULL;
    char *filepath = NULL;
    unsigned short port = 0;
    struct sockaddr_in clientInfo = {0};
    int initialRequest = 0;  // Flag to determine if initial request should be sent

    if (argc < 2) {
        printf("Usage: %s -h <host> -p <port>\n", argv[0]);
        return 0;
    }

    while ((c = getopt(argc, argv, "h:p:f:")) != -1) {
        switch (c) {
            case 'h':
                hostarg = optarg;
                break;
            case 'p':
                portarg = optarg;
                port = atoi(portarg);
                initialRequest = 1;  // Set flag if port is specified
                break;
            case 'f':
                filepath = optarg;
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

    if (initialRequest) {
        if (send_request(fd) != STATUS_SUCCESS) {
            printf("Error establishing a hello request\n");
            close(fd);
            return -1;
        }
    }

    printf("Enter commands (e.g., '-a name,email,phone'):\n");
    printf("Enter commands (e.g., '-u field to replace,new value,old value'):\n");

    char input[1024];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0; 

        if (strncmp(input, "-a ", 3) == 0) {
            char *addString = input + 3; 
            printf("add string: %s\n", addString);
            if (send_add_req(addString, filepath, fd) != STATUS_SUCCESS) {
                printf("Error adding new contact\n");
                break;
            }
        } else if (strncmp(input, "-u ", 3) == 0) {
            char *updateString = input + 3;
            printf("update string: %s\n", updateString);
            if (send_update_req(updateString, filepath, fd) != STATUS_SUCCESS) {
                printf("Error updating contacts\n");
                break;
            }
        } else {
            printf("Unknown command or format error. Available commands are '-a [contact info]'.\n");
        }
    }

    close(fd);
    return 0;
}