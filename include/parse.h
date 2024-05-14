#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#define MAX_FIELD_LENGTH 256
#define MAX_CONTACTS 100

typedef struct {
    char name[MAX_FIELD_LENGTH];
    char email[MAX_FIELD_LENGTH];
    char phoneNbr[MAX_FIELD_LENGTH];
} contact_t;

// struct contact_t {
//     char name[MAX_FIELD_LENGTH];
//     char email[MAX_FIELD_LENGTH];
//     char phoneNbr[MAX_FIELD_LENGTH]; 
// };

typedef struct __attribute__((packed)) {
    contact_t contacts[MAX_CONTACTS];
    int count;
} contact_list_t;

// int add_contact(struct contact_t **, char *addstring, char *filepath, FILE **fp, int *count);
int add_contact(contact_t **, char *addstring, char *filepath, FILE **fp, int *count);
// int update_contact(struct contact_t **, char *addstring, char *filepath, FILE **fp, int *count);
int update_contact(contact_t **, char *addstring, char *filepath, FILE **fp, int *count);
// int remove_contact(struct contact_t **, char *removeString, char *filepath, FILE **fp, int *count);
int remove_contact(contact_t **, char *removeString, char *filepath, FILE **fp, int *count);
// int list_contacts(struct contact_t **, FILE **fp, int *count);
int list_contacts(contact_t **, FILE **fp, int *count);

#endif