//
// Created by Rescyy on 2/7/2026.
//

#ifndef HTTPSERVERC_HTTP_PATH_H
#define HTTPSERVERC_HTTP_PATH_H

#include <sys/types.h>

typedef struct HttpPath {
    char *raw;
    char **elements;
    int elCount;
} HttpPath;

int parsePathTrackQueryParameterStart(HttpPath *path, const char *str, size_t n, ssize_t *queryParameterStart);
int parsePath(HttpPath *path, const char *str, size_t n);
int pathMatches(HttpPath endpointPath, HttpPath reqPath);

#endif //HTTPSERVERC_HTTP_PATH_H