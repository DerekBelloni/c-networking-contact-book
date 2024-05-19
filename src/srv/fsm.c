#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"
#include "fsm.h"
#include "file.h"

proto_file_path* initialize_contact_file(char *buffer, size_t req_type_offset, contact_t **contacts, FILE **fp, int *count) {
    printf("here\n");
    proto_file_path *path = (proto_file_path*)((char*)buffer + sizeof(proto_hdr_t) + sizeof(proto_req) + req_type_offset);

    char *file_mode = strdup("r+");
    if((char*)path->path) {
        printf("in if\n");
        open_contact_file((char*)path->path, contacts, fp, count, file_mode);
        free(file_mode);
        if (*fp == NULL) {
            printf("Unable to open file.\n");
            fclose(*fp);
            return NULL;
        }
    }
    printf("mango\n");
    return path;
}


int handle_protocol_mismatch(clientstate_t *client) {
    proto_hdr_t *errorHdr = (proto_hdr_t*)client->buffer;
    errorHdr->type = htons(MSG_ERROR);
    errorHdr->length = htons(1);
    
    error_resp *errorResp = (error_resp*)&errorHdr[1];
    errorResp->errorType = htons(PROTOCOL_ERROR);

    write(client->fd, client->buffer, sizeof(proto_hdr_t) + sizeof(error_resp));

    return STATUS_SUCCESS;
}

int handle_client_fsm(clientstate_t *client) {
    proto_hdr_t *clientHdr = (proto_hdr_t *)client->buffer;

    clientHdr->type = ntohs(clientHdr->type);
    clientHdr->length = ntohs(clientHdr->length);
    
    proto_req *clientHello = (proto_req*)((char *)clientHdr + sizeof(proto_hdr_t));
    clientHello->proto = ntohs(clientHello->proto);

    if (clientHdr->type == MSG_HELLO_REQ) {
        printf("Hello message received\n");
            
        if (clientHello->proto != PROTO_VER) {
            handle_protocol_mismatch(client);
        }

        client->state = STATE_HELLO;

        proto_hdr_t *responseHdr = (proto_hdr_t*)client->buffer;
        responseHdr->type = htons(MSG_HELLO_RESP);
        responseHdr->length = htons(0);

        write(client->fd, responseHdr, sizeof(proto_hdr_t));
        memset(responseHdr, 0, sizeof(proto_hdr_t));
    }
    
    client->state = STATE_MSG;

    if (client->state == STATE_MSG) {
        // Adding a contract
        if (clientHdr->type == MSG_CONTACT_ADD_REQ) {
            printf("add contact request received!\n");

            if (clientHello->proto != PROTO_VER) {
                handle_protocol_mismatch(client);
            }

            proto_add_req *addString = (proto_add_req*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req));
            
            int count = 0;
            FILE *fp;
            contact_t *contacts = NULL;
            proto_file_path *path = NULL;

            path = initialize_contact_file(client->buffer, sizeof(proto_add_req), &contacts, &fp, &count);
            if (path == NULL) {
                printf("Error initializing contact file\n");
                return STATUS_ERROR;
            }

            printf("after initialize: %s", (char*)path->path);

            if (add_contact(&contacts, (char*)addString, (char*)path->path, &fp, &count) != STATUS_SUCCESS) {
                printf("Error adding new contact.\n");
                fclose(fp);
                return STATUS_ERROR;
            }

            proto_hdr_t *responseHdr = (proto_hdr_t*)client->buffer;
            responseHdr->type = htons(MSG_CONTACT_ADD_RESP);
            responseHdr->length = htons(0);

            write(client->fd, responseHdr, sizeof(proto_hdr_t));
            memset(responseHdr, 0, sizeof(proto_hdr_t));
        }

        // Updating a contact
        if (clientHdr->type == MSG_CONTACT_UPDATE_REQ) {
            printf("update contact request received!\n");

            if (clientHello->proto != PROTO_VER) {
                handle_protocol_mismatch(client);
            }
            
            proto_update_req *updateString = (proto_update_req*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req));;
            int count = 0;
            FILE *fp;
            contact_t *contacts = NULL;
            proto_file_path *path = NULL;

            path = initialize_contact_file(client->buffer, sizeof(proto_update_req), &contacts, &fp, &count);
            if (path == NULL) {
                printf("Error initializing contact file\n");
                return STATUS_ERROR;
            }

            if (update_contact(&contacts, (char*)updateString, (char*)path->path, &fp, &count) != STATUS_SUCCESS) {
                printf("Error updating contact\n");
                fclose(fp);
                return STATUS_ERROR;
            }

            
            proto_hdr_t *responseHdr = (proto_hdr_t*)client->buffer;
            responseHdr->type = htons(MSG_CONTACT_UPDATE_RESP);
            responseHdr->length = htons(0);

            write(client->fd, responseHdr, sizeof(proto_hdr_t));
            memset(responseHdr, 0, sizeof(proto_hdr_t));
        }

        if (clientHdr->type == MSG_CONTACT_DEL_REQ) {
            printf("delete contact request received!\n");

            if (clientHello->proto != PROTO_VER) {
                handle_protocol_mismatch(client);
            }

            proto_remove_req *removeString = (proto_remove_req*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req));

            int count = 0;
            FILE *fp;
            contact_t *contacts = NULL;
            proto_file_path *path = NULL;

            path = initialize_contact_file(client->buffer, sizeof(proto_update_req), &contacts, &fp, &count);
            printf("path: %s", path->path);
            if (path == NULL) {
                printf("Error initializing contact file\n");
                return STATUS_ERROR;
            }

            if (remove_contact(&contacts, (char*)removeString, (char*)path->path, &fp, &count) != STATUS_SUCCESS) {
                printf("Error removing contact\n");
                fclose(fp);
                return STATUS_ERROR;
            }

            proto_hdr_t *responseHdr = (proto_hdr_t*)client->buffer;
            responseHdr->type = htons(MSG_CONTACT_DEL_RESP);
            responseHdr->length = htons(0);

            write(client->fd, responseHdr, sizeof(proto_hdr_t));
            memset(responseHdr, 0, sizeof(proto_hdr_t));
        }

        if (clientHdr->type == MSG_CONTACT_LIST_REQ) {
            printf("list contacts request received!\n");

            if (clientHello->proto != PROTO_VER) {
                handle_protocol_mismatch(client);
            }

            contact_list_t *contact_list = malloc(sizeof(contact_list_t));
            if (contact_list == NULL) {
                perror("malloc failed");
                return STATUS_ERROR;
            }

            proto_file_path *path = (proto_file_path*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req));
            int count = 0;
            FILE *fp;
            contact_t *contacts = NULL;
            char *file_mode = strdup("r+");
            if((char*)path->path) {
                open_contact_file((char*)path->path, &contacts, &fp, &count, file_mode);
                if (fp == NULL) {
                    printf("Unable to open file.\n");
                    fclose(fp);
                    return STATUS_ERROR;
                }
            }

            contact_list->count = count;

            for (int i = 0; i < count && i < MAX_CONTACTS; i++) {
                strncpy(contact_list->contacts[i].name, contacts[i].name, MAX_FIELD_LENGTH - 1);
                contact_list->contacts[i].name[MAX_FIELD_LENGTH - 1] = '\0';
               
                strncpy(contact_list->contacts[i].email, contacts[i].email, MAX_FIELD_LENGTH - 1);
                contact_list->contacts[i].email[MAX_FIELD_LENGTH - 1] = '\0';
               
                strncpy(contact_list->contacts[i].phoneNbr, contacts[i].phoneNbr, MAX_FIELD_LENGTH - 1);
                contact_list->contacts[i].phoneNbr[MAX_FIELD_LENGTH - 1] = '\0';
            }

            printf("Server side contacts:\n");
            for (int i = 0; i < contact_list->count; i++) {
                printf("Contact %d:\n", i + 1);
                printf("  Name: %s\n", contact_list->contacts[i].name);
                printf("  Email: %s\n", contact_list->contacts[i].email);
                printf("  Phone Number: %s\n", contact_list->contacts[i].phoneNbr);
            }

            fclose(fp);

            proto_hdr_t *responseHdr = (proto_hdr_t*)client->buffer;
            responseHdr->type = htons(MSG_CONTACT_LIST_RESP);
            responseHdr->length = htons(sizeof(contact_list_t));

            memcpy(responseHdr + 1, contact_list, sizeof(contact_list_t));

            ssize_t bytesWritten = write(client->fd, client->buffer, sizeof(proto_hdr_t) + sizeof(contact_list_t));
            if (bytesWritten < 0) {
                perror("write failed");
                free(contact_list);
                return STATUS_ERROR;
            }

            free(contact_list); 
        }
    }
    
    return STATUS_SUCCESS;
}