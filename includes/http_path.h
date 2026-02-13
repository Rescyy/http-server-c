//
// Created by Rescyy on 2/7/2026.
//

#ifndef HTTPSERVERC_HTTP_PATH_H
#define HTTPSERVERC_HTTP_PATH_H

typedef struct HttpPath {
    char *raw;
    char **elements;
    int elCount;
} HttpPath;

int parsePath(HttpPath *path, const char *str, int n);
int pathMatches(HttpPath endpointPath, HttpPath reqPath);

#endif //HTTPSERVERC_HTTP_PATH_H