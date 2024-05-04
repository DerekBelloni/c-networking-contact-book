#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"

int handle_client_fsm(int clientFd, char *writeBuffer, char *readBuffer) {
    // proto_hdr_t *clientHdr = (proto_hdr_t *)readBuffer
    printf("In fsm.c\n");
    proto_hdr_t *clientHdr = (proto_hdr_t *)readBuffer;

        // convert to host endian
    clientHdr->type = ntohs(clientHdr->type);
    clientHdr->length = ntohs(clientHdr->length);
  
    proto_hello_req *clientHello = (proto_hello_req*)((char *)clientHdr + sizeof(proto_hdr_t));
    clientHello->proto = ntohs(clientHello->proto);
        
    if (clientHdr->type == MSG_HELLO_REQ) {
        printf("Hello message received!!!\n");
            
        if (clientHello->proto != PROTO_VER) {
            // handle_protocol_mismatch(clientFd, writeBuffer);
        }

        // after receiving the hello request, 
    }
    return 0;
}