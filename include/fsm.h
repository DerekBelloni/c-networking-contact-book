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
    int clientFd;
    state_e state;
    char buffer[BUFFER_SIZE];
} clientstate_t;

int handle_client_fsm(int *clientFd, char *writeBuffer, char *readBuffer);
// int handle_protocol_mismatch();

#endif