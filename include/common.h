#ifndef COMMON_H
#define COMMON_H

#define STATUS_ERROR -1
#define STATUS_SUCCESS 0
#define MAX_CONTACTS 100

#define PROTO_VER 100

typedef enum {
    MSG_HELLO_REQ,
    MSG_HELLO_RESP,
    MSG_CONTACT_LIST_REQ,
    MSG_CONTACT_LIST_RESP,
    MSG_CONTACT_ADD_REQ,
    MSG_CONTACT_ADD_RESP,
    MSG_CONTACT_DEL_REQ,
    MSG_CONTACT_DEL_RESP,
    MSG_CONTACT_UPDATE_REQ,
    MSG_CONTACT_UPDATE_RESP
} msg_type_e;

typedef struct {
    uint16_t type;
    uint16_t length;
} proto_hdr_t;

typedef struct {
    uint16_t proto;
} proto_hello_req;

#endif