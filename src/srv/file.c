#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parse.h"
#include "common.h"

#define MAX_LINE_LENGTH 771 

void create_contact_file(char *filepath, FILE **fp) {
    *fp = fopen(filepath, "r");

    if (*fp != NULL) {
        printf("A file already exists at the given path.\n");
        fclose(*fp);
        return;
    }

    *fp = fopen(filepath, "w+x");
    if (*fp == NULL) {
        perror("open");
        fclose(*fp);
        return;
    }
}

void open_contact_file(char *filepath, contact_t **contacts, FILE **fp, int *count, char *file_mode) {
    char line[MAX_LINE_LENGTH];
    int i = 0;
    *fp = fopen(filepath, file_mode);
    
    if (*fp == NULL) {
        perror("open");
        fclose(*fp);
    }

    while(fgets(line, sizeof(line), *fp) != NULL) {
        if (strcmp(line, "\n") == 0 || strcmp(line, "\r\n") == 0) {
            continue;
        }
        (*count)++;
    }

    *contacts = malloc((*count) * sizeof(contact_t));
    if (*contacts == NULL) {
        perror("malloc");
        fclose(*fp);
    }

    rewind(*fp);

    while((fgets(line, sizeof(line), *fp)) != NULL) {
        char *name, *email, *phoneNbr = NULL;
        char *token;
        char *line_copy = strdup(line);

        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        token = strsep(&line_copy, ",");
        if (token != NULL && token[0] != '\0') {
            name = strdup(token);
        }
        
        token = strsep(&line_copy, ",");
        if (token != NULL && token[0] != '\0') {
            email = strdup(token);
        }

        token = strsep(&line_copy, ",");
        if (token != NULL && token[0] != '\0') {
            char *newline = strchr(token, '\n');
            if (newline != NULL) {
                *newline = '\0';
            }
            phoneNbr = strdup(token);
        }
        
        if (name != NULL) {
            strncpy((*contacts)[i].name, name, MAX_FIELD_LENGTH);
        }

        if (email != NULL) {
            strncpy((*contacts)[i].email, email, MAX_FIELD_LENGTH);
        }

        if (phoneNbr != NULL) {
            strncpy((*contacts)[i].phoneNbr, phoneNbr, MAX_FIELD_LENGTH);
        }

        i++;
    }
}

