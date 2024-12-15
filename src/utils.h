#ifndef UTILS_H
#define UTILS_H

int strindex(const char *str, const char *find);
int strnindex(const char *str, int n, const char *find);
int isAlpha(char c);
unsigned int hash(void *data, int len);

#endif