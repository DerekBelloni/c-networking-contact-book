#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "parse.h"
#include "common.h"

#define MAX_FIELD_LENGTH 256
#define MAX_LINE_LENGTH 771 

int add_contact(contact_t **contacts, char *addstring, char *filepath, FILE **fp, int *count) {
    char *name, *email, *phoneNbr;
    char *input = strdup(addstring);
    char *token;

    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        name = strdup(token);
    }

    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        email = strdup(token);
    }

    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        phoneNbr = strdup(token);
    }

    (*count)++;

    contact_t *temp = realloc(*contacts, *count * sizeof(contact_t));
    if (temp == NULL) {
        perror("Memory reallocation failed");
        if (*contacts != NULL) {
            free(*contacts);
        }
        free(name);
        free(email);
        free(phoneNbr);
        free(input);
        return -1;
    }

    *contacts = temp;

    strncpy((*contacts)[*count -1].name, name, MAX_FIELD_LENGTH - 1);
    strncpy((*contacts)[*count -1].email, email, MAX_FIELD_LENGTH - 1);
    strncpy((*contacts)[*count -1].phoneNbr, phoneNbr, MAX_FIELD_LENGTH - 1);

    fprintf(*fp, "%s,%s,%s\n", (*contacts)[*count - 1].name, (*contacts)[*count - 1].email, (*contacts)[*count - 1].phoneNbr);
    fflush(*fp);
    free(input);
    return STATUS_SUCCESS;
}

int update_contact(contact_t **contacts, char *updateString, char *filepath, FILE **fp, int *count) {
    char *field, *replacement, *toBeReplaced;
    char *input = strdup(updateString);
    char *token;
    int i;

    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        field = strdup(token);
    }
    
    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        replacement = strdup(token);
    }
    
    token = strsep(&input, ",");
    if (token != NULL && token[0] != '\0') {
        toBeReplaced = strdup(token);
    }
    printf("field: %s\n", field);
    printf("replacement: %s\n", replacement);
    printf("to be replaced: %s\n", toBeReplaced);
    for (i = 0; i < (*count); i++) {
        if (strcmp(field, "name") == 0) {
            if (strcmp((*contacts)[i].name, toBeReplaced) == 0) {
                strncpy((*contacts)[i].name, replacement, MAX_FIELD_LENGTH - 1);
            }       
        } else if (strcmp(field, "email") == 0) {
            if (strcmp((*contacts)[i].email, toBeReplaced) == 0) {
                printf("and here\n");
                strncpy((*contacts)[i].email, replacement, MAX_FIELD_LENGTH - 1);
            }
        } else if (strcmp(field, "phoneNbr") == 0) {
            if (strcmp((*contacts)[i].phoneNbr, toBeReplaced) == 0) {
                strncpy((*contacts)[i].phoneNbr, replacement, MAX_FIELD_LENGTH -1);
            }
        }
    }

    *contacts = realloc(*contacts, (*count) * sizeof(contact_t));
    if (*contacts == NULL) {
        printf("Memory reallocation failed.\n");
        return STATUS_ERROR;
    }

    fclose(*fp);
    *fp = fopen(filepath, "w");
    if (*fp == NULL) {
        printf("Failed to open file for writing.\n");
    }

    for (i = 0; i < (*count); i++) {
        fprintf(*fp, "%s,%s,%s\n", (*contacts)[i].name, (*contacts)[i].email, (*contacts)[i].phoneNbr);
    }

    fflush(*fp);
    fclose(*fp);
    return STATUS_SUCCESS;
}

int remove_contact(contact_t **contacts, char *removeString, char *filepath, FILE **fp, int *count) {
    int i, j, removed;

    for (i = 0; i < (*count); i++) {
        if (strcmp((*contacts)[i].name, removeString) == 0) {
            if ((*count) == 1) {
                (*count) = 0;
                free(*contacts);
                *contacts = NULL;
                removed = 1;
            } else {
                for (j = i; j < (*count) - 1; j++) {
                    (*contacts)[j] = (*contacts)[j + 1];
                }
                (*count)--; 
                removed = 1;
                break;
            }
        }
    }

    if (removed) {
        *contacts = realloc(*contacts, (*count) * sizeof(contact_t));
        if (*contacts == NULL) {
            printf("Memory reallocation failed.\n");
            return STATUS_ERROR;
        }

        fclose(*fp);
        *fp = fopen(filepath, "w");
        if (*fp == NULL) {
            printf("Failed to open file for writing.\n");
            return STATUS_ERROR;
        }

        if ((*count) > 0) {
            for (i = 0; i < (*count); i++) {
                fprintf(*fp, "%s,%s,%s\n", (*contacts)[i].name, (*contacts)[i].email, (*contacts)[i].phoneNbr);
            }
        } 
        fflush(*fp);
        fclose(*fp);
        return STATUS_SUCCESS;
    }
    return STATUS_ERROR;
}

int list_contacts(contact_t **contacts, FILE **fp, int *count)
{
    int i = 0;
    for (i = 0; i < *count; i++) {
        printf("\nContact %d:\n", i + 1);
        printf("Name: %s\n", (*contacts)[i].name);
        printf("Email: %s\n", (*contacts)[i].email);
        printf("Phone Number: %s\n", (*contacts)[i].phoneNbr);
    }

    fclose(*fp);
    return STATUS_SUCCESS;
}