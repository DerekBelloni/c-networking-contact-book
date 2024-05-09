#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"
#include "fsm.h"


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
        clientHdr->type = htons(MSG_HELLO_RESP);
        clientHdr->length = htons(0);

        write(client->fd, client->buffer, sizeof(proto_hdr_t));
    }
    
    client->state = STATE_MSG;
    
    if (clientHdr->type == MSG_CONTACT_ADD_REQ && client->state == STATE_MSG) {
        printf("add contact request received!\n");

         if (clientHello->proto != PROTO_VER) {
            handle_protocol_mismatch(client);
        }

        proto_add_req *addString = (proto_add_req*)((char*)client->buffer + sizeof(proto_hdr_t) + sizeof(proto_req));
        printf("Add string from client: %s\n", addString->data);

    }
    return STATUS_SUCCESS;
}