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
#include "parse.h"

int send_list_req(char *filepath, int fd) {
    printf("file path in in list req, client: %s\n", filepath);
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[LIST_BUFFER_SIZE] = {0};

    // Header
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = htons(MSG_CONTACT_LIST_REQ);
    hdr->length = htons(sizeof(proto_req) + sizeof(proto_file_path));

    // Protocol Version
    proto_req *req = (proto_req*)(hdr + 1);
    req->proto = htons(PROTO_VER);

    // File path
    proto_file_path *path = (proto_file_path*)(req + 1);
    snprintf((char*)path->path, sizeof(path->path), "%s", filepath);

    ssize_t bytesWritten = write(fd, writeBuffer, ntohs(hdr->length) + sizeof(proto_hdr_t));
    if (bytesWritten < 0) {
        perror("write failed");
        return STATUS_ERROR;
    }

    ssize_t bytesRead = read(fd, readBuffer, sizeof(readBuffer));
    if (bytesRead == -1) {
        perror("read failed");
        return STATUS_ERROR;
    }

    proto_hdr_t *serverHdr = (proto_hdr_t*)readBuffer;

    printf("server hdr type: %d\n", ntohs(serverHdr->type));

    contact_list_t *contact_list = malloc(sizeof(contact_list_t));
    memcpy(contact_list, serverHdr + 1, sizeof(contact_list_t));

    printf("number of contacts: %d\n", contact_list->count);
    for (int i = 0; i < contact_list->count; i++) {
        printf("Contact %d:\n", i + 1);
        printf("  Name: %s\n", contact_list->contacts[i].name);
        printf("  Email: %s\n", contact_list->contacts[i].email);
        printf("  Phone Number: %s\n", contact_list->contacts[i].phoneNbr);
    }

    free(contact_list);
    return STATUS_SUCCESS;
}

int send_remove_req(char *removeString, char *filepath, int fd) {
    char writeBuffer[BUFFER_SIZE] = {0};
    char readBuffer[BUFFER_SIZE] = {0};

    // Header
    proto_hdr_t *hdr = (proto_hdr_t*)writeBuffer;
    hdr->type = htons(MSG_CONTACT_DEL_REQ);
    hdr->length = htons(sizeof(proto_req) + sizeof(proto_remove_req) + sizeof(proto_file_path));

    // Request
    proto_req *req = (proto_req*)(hdr + 1);
    req->proto = htons(PROTO_VER);

    // Remove contact data
    proto_remove_req *remove = (proto_remove_req*)(req + 1);
    snprintf((char*)remove->data, sizeof(remove->data), "%s", removeString);

    proto_file_path *path = (proto_file_path*)(remove + 1);
    snprintf((char*)path->path, sizeof(path->path), "%s", filepath);

    ssize_t bytesWritten = write(fd, writeBuffer, ntohs(hdr->length) + sizeof(proto_hdr_t));
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
    serverHdr->length = htons(serverHdr->length);

    if (serverHdr->type == MSG_ERROR) {
        printf("Error type received\n");
        close(fd);
        return STATUS_ERROR;
    }

    if (serverHdr->type == MSG_CONTACT_DEL_RESP) {
        printf("Server remove contact response received\n");
        return STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
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

    // Send the buffer
    ssize_t bytes_written = write(fd, writeBuffer, ntohs(hdr->length) + sizeof(proto_hdr_t));
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

int process_command(char *input, char *filepath, int fd) {
    if (strncmp(input, "-a ", 3) == 0) {
        char *addString = input + 3; 
        printf("add string: %s\n", addString);
        if (send_add_req(addString, filepath, fd) != STATUS_SUCCESS) {
            printf("Error adding new contact\n");
            return STATUS_ERROR;
        }
    } else if (strncmp(input, "-u ", 3) == 0) {
        char *updateString = input + 3;
        printf("update string: %s\n", updateString);
        if (send_update_req(updateString, filepath, fd) != STATUS_SUCCESS) {
            printf("Error updating contact.\n");
            return STATUS_ERROR;
        }
    } else if (strncmp(input, "-r ", 3) == 0) {
        char *removeString = input + 3;
        printf("remove string: %s\n", removeString);
        if (send_remove_req(removeString, filepath, fd) != STATUS_SUCCESS) {
            printf("Error removing contact.\n");
            return STATUS_ERROR;
        }
    } else if (strncmp(input, "-l", 2) == 0) {
        printf("list contacts request received in the client\n");
        if (send_list_req(filepath, fd) != STATUS_SUCCESS) {
            printf("Error listing contacts.\n");
            return STATUS_ERROR;
        }
    } else {
        printf("Unknown command or format error. Available commands are '-a [contact info]'.\n");
        return STATUS_ERROR;
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

    if (hostarg == NULL || portarg == NULL || filepath == NULL) {
        printf("Host, port and file path must be specified.\n");
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
        process_command(input, filepath, fd);
    }

    close(fd);
    return 0;
}