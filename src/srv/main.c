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
            close(clientFd);
            return -1;
        }
        printf("Client connected from %s\n", inet_ntoa(clientInfo.sin_addr));

        while(1) {
            clientstate_t *client = malloc(sizeof(clientstate_t));
            client->state = STATE_NEW;
            client->fd = clientFd;
            memset(client->buffer, 0, BUFFER_SIZE);

            ssize_t bytesRead = read(client->fd, client->buffer, BUFFER_SIZE);
            if (bytesRead == -1) {
                perror("read");
                close(clientFd);
                free(client);
                continue;
            }

            if (handle_client_fsm(client) != STATUS_SUCCESS) {
                    printf("Error handling client fsm\n");
                    continue;
            }
        }
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

    // struct contact_t *contacts = NULL;
    contact_t *contacts = NULL;

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
 
    handle_connection(port);

    return 0;
}