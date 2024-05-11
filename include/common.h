#ifndef COMMON_H
#define COMMON_H

#define STATUS_ERROR -1
#define STATUS_SUCCESS 0
#define MAX_CONTACTS 100
#define BUFFER_SIZE (sizeof(proto_hdr_t) + sizeof(proto_req) + sizeof(proto_add_req) + sizeof(proto_file_path))

#define FILE_PATH_MAX 1024
#define FILE_DATA_MAX 1024

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
    MSG_CONTACT_UPDATE_RESP,
    MSG_ERROR
} msg_type_e;

typedef enum {
    PROTOCOL_ERROR,
    TYPE_ERROR
} error_type_e;

typedef struct {
    uint16_t errorType;
} error_resp;

typedef struct __attribute__((packed)) {
    uint16_t type;
    uint16_t length;
} proto_hdr_t;

typedef struct __attribute__((packed)) {
    uint16_t proto;
} proto_req;

typedef struct __attribute((packed)) {
    uint8_t data[FILE_DATA_MAX];
} proto_add_req;

typedef struct __attribute((packed)) {
    uint8_t path[FILE_PATH_MAX];
} proto_file_path;



#endif