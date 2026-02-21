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

typedef struct {
    string key;
    string value;
} KeyValue;

char *strnstr(const char *s, const char *find, size_t slen);
int strindex(const char *str, const char *find);
int strnindex(const char *str, int n, const char *find);
int strnindexAny(const char *str, int n, const char *find);
int isAlpha(char c);
unsigned int hash(void *data, int len);
size_t getCurrentFormattedTime(char *buf, size_t size);
string copyString(string str);
string copyStringFromSlice(const char *ptr, ssize_t len);
ssize_t stringCompare(string *str1, string *str2);
ssize_t stringCompareIgnoreCase(string *str1, string *str2);
KeyValue *findKeyValue(KeyValue* keyValues, size_t count, const char *key);

#define DECLARE_CURRENT_TIME(time) char time[128]; getCurrentFormattedTime(time, sizeof(time))
#define EMPTY_STRING (string) {.ptr = NULL, .length = 0}

#endif
