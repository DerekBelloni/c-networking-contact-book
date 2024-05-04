#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "common.h"
#include "file.h"
#include "parse.h"
#include "fsm.h"

int print_usage(char *argv[]) {
    printf("Usage: %s -p <filepath> -f <filename>\n", argv[0]);
    printf("To add a contact use -a <name,email,phoneNbr>\n");
    printf("To remove a contact use -r <name>\n");
    printf("To update a contact use -u <field_to_update,replacement_data,data_to_replace>\n");
    return 1;
}

int handle_protocol_mismatch(int clientFd, char *writeBuffer) {
    // create an error hdr from the writeBuffer
    proto_hdr_t *errorHdr = (proto_hdr_t*)writeBuffer;
    // use MSG_ERROR as the hdr type
    errorHdr->type = htons(MSG_ERROR);
    errorHdr->length = htons(1);
    // use the error_type_e as the specific error type, place in the buffer after the header
    error_resp *errorResp = (error_resp*)&errorHdr[1];
    errorResp->errorType = htons(PROTOCOL_ERROR);

    write(clientFd, writeBuffer, sizeof(proto_hdr_t) + sizeof(error_resp));

    return STATUS_SUCCESS;
}

int handle_connection(unsigned short port) {
    struct sockaddr_in serverInfo = {0};
    struct sockaddr_in clientInfo = {0};
    socklen_t clientSize = sizeof(clientInfo);

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = 0;
    serverInfo.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    if (bind(fd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 0) == -1) {
        perror("bind");
        close(fd);
        return -1;
    } 

    printf("Server listening on port: %d\n", port);

    while (1) {
        int clientFd = accept(fd, (struct sockaddr*)&clientInfo, &clientSize);
       
        if (clientFd == -1) {
            perror("accept");
            close(fd);
            return -1;
        }
     
        // create a buffer for the client header and data
        char readBuffer[BUFFER_SIZE];
        char writeBuffer[BUFFER_SIZE];
        ssize_t bytesRead = read(clientFd, readBuffer, sizeof(readBuffer));
        if (bytesRead == -1) {
            perror("read");
            close(clientFd);
            continue;
        }

        handle_client_fsm(&clientFd, writeBuffer, readBuffer);

        // // set a clientHdr and cast it to the buffer
        // proto_hdr_t *clientHdr = (proto_hdr_t *)readBuffer;

        // // convert to host endian
        // clientHdr->type = ntohs(clientHdr->type);
        // clientHdr->length = ntohs(clientHdr->length);
  
        // proto_hello_req *clientHello = (proto_hello_req*)((char *)clientHdr + sizeof(proto_hdr_t));
        // clientHello->proto = ntohs(clientHello->proto);
        
        // if (clientHdr->type == MSG_HELLO_REQ) {
        //     printf("Hello message received!!!\n");
            
        //     if (clientHello->proto != PROTO_VER) {
        //         handle_protocol_mismatch(clientFd, writeBuffer);
        //     }

        //     // after receiving the hello request, 

        //     continue;
        // }

    }
    return 0;
}

int main(int argc, char *argv[]) {
    int c;
    int count = 0;
    FILE *fp;
    char *filepath = NULL;
    char *filename = NULL;
    char *portarg = NULL;
    char *updateString = NULL;
    char *addString = NULL;
    char *removeString = NULL;
    char *file_mode = NULL;
    bool listContacts = false;
    bool newFile = false;
    unsigned short port = 0;

    struct contact_t *contacts = NULL;

    while ((c = getopt(argc, argv, "nf:p:")) != -1) {
        switch (c) {
            case 'n':
                newFile = true;
                break;
            case 'f':
                filepath = optarg;
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
 
    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return 0;
    }

    if (port == 0) {
        printf("Port number is required. Use -p <port>\n");
        print_usage(argv);
        return 1;
    }

    if (newFile) {
        create_contact_file(filepath, &fp);
        if (fp == NULL) {
            printf("Unable to create new file.\n");
            fclose(fp);
            return STATUS_ERROR;
        }
    } else {
        file_mode = strdup("r+");
        open_contact_file(filepath, &contacts, &fp, &count, file_mode);
        if (fp == NULL) {
            printf("Unable to open file.\n");
            fclose(fp);
            return STATUS_ERROR;
        }
    }

    if (addString) {
       if (add_contact(&contacts, addString, filepath, &fp, &count) != STATUS_SUCCESS) {
            printf("Error adding new contact.\n");
            fclose(fp);
            return STATUS_ERROR;
       }
    }

    if (updateString) {
        if (update_contact(&contacts, updateString, filepath, &fp, &count) != STATUS_SUCCESS) {
            printf("Error updating contact.\n");
            fclose(fp);
            return STATUS_ERROR;
        }
    }

    if (removeString) {
        if (remove_contact(&contacts, removeString, filepath, &fp, &count) != STATUS_SUCCESS) {
            printf("Error removing contact.\n");
            fclose(fp);
            return STATUS_ERROR;
        }
    } 

    if (listContacts) {
        if (list_contacts(&contacts, &fp, &count) != STATUS_SUCCESS) {
            printf("Error listing contacts.\n");
            fclose(fp);
            return STATUS_ERROR;
        }
    }
 
    handle_connection(port);

    return 0;
}