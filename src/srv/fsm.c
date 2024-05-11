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
            proto_file_path *path = (proto_file_path*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req) + sizeof(proto_add_req));

            int count = 0;
            FILE *fp;
            struct contact_t *contacts = NULL;
            char *file_mode = strdup("r+");
            if((char*)path->path) {
                open_contact_file((char*)path->path, &contacts, &fp, &count, file_mode);
                if (fp == NULL) {
                    printf("Unable to open file.\n");
                    fclose(fp);
                    return STATUS_ERROR;
                }
            }

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
        // if (clientHdr->type == MSG_C)
    }
    
    return STATUS_SUCCESS;
}