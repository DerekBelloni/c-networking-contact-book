#ifndef FSM_H
#define FSM_H

#include "common.h"

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_HELLO,
    STATE_MSG,
    STATE_GOODBYE
} state_e;

typedef struct {
    int fd;
    state_e state;
    char buffer[BUFFER_SIZE];
} clientstate_t;

int handle_client_fsm(clientstate_t *client);
int handle_protocol_mismatch(clientstate_t *client);
proto_file_path* initialize_contact_file(char *buffer, size_t req_type_offset, contact_t **contacts, FILE **fp, int *count);

#endif