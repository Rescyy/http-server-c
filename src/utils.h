//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <string.h>


typedef struct string {
    char *ptr;
    size_t length;
} string;

char *strnstr(const char *s, const char *find, size_t slen);
int strindex(const char *str, const char *find);
int strnindex(const char *str, int n, const char *find);
int isAlpha(char c);
unsigned int hash(void *data, int len);

#endif
