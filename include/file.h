#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "parse.h"

void create_contact_file(char *filepath, FILE **fp);
void open_contact_file(char *filepath, contact_t **, FILE **fp, int *count, char *file_mode);

#endif