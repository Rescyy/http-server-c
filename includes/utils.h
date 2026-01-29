//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <sys/types.h>

typedef struct string {
    char *ptr;
    ssize_t length;
} string;

char *strnstr(const char *s, const char *find, size_t slen);
int strindex(const char *str, const char *find);
int strnindex(const char *str, int n, const char *find);
int isAlpha(char c);
unsigned int hash(void *data, int len);
size_t getCurrentFormattedTime(char *buf, size_t size);
string copyString(string str);

#define DECLARE_CURRENT_TIME(time) char time[128]; getCurrentFormattedTime(time, sizeof(time))
#define EMPTY_STRING (string) {.ptr = NULL, .length = 0}

#endif
