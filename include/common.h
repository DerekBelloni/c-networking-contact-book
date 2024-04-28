#ifndef COMMON_H
#define COMMON_H

#define STATUS_ERROR -1
#define STATUS_SUCCESS 0
#define MAX_CONTACTS 100

#define PROTO_VER 100

typedef enum {
    MSG_HELLO_REQ,
    MSG_HELLO_RESP
} msg_type_e;

typedef struct {
    msg_type_e type;
    uint16_t length;
} proto_hdr_t;

typedef struct {
    uint16_t proto;
} proto_hello_req;

#endif