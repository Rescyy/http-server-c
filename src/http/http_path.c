//
// Created by Rescyy on 2/7/2026.
//

#include <http_path.h>
#include <alloc.h>
#include <utils.h>

#include "logging.h"

int parsePathTrackQueryParameterStart(HttpPath *path, const char *str, size_t n, ssize_t *queryParameterStart)
{
    int offset, prevOffset = 0;
    path->elCount = 0;
    path->elements = NULL;
    path->raw = gcArenaAllocate(n + 1, alignof(char));
    snprintf(path->raw, n + 1, "%.*s", (int) n, str);
    char *element;
    int slashes = 0;
    if (queryParameterStart != NULL) {
        *queryParameterStart = -1;
    }
    for (size_t i = 0; i < n; i++) {
        if (str[i] == '?') {
            if (queryParameterStart != NULL) {
                *queryParameterStart = (ssize_t) i + 1;
            }
            n = i;
            break;
        }
        slashes += str[i] == '/';
    }
    if (str[prevOffset++] != '/') {
        return -1;
    }
    if (n == 1) {
        return 0;
    }

    path->elements = gcArenaAllocate(sizeof(char *) * slashes, alignof(char *));

    while ((offset = strnindex(str + prevOffset, (int) n - prevOffset, "/")) != -1)
    {
        element = gcArenaAllocate(offset + 1, alignof(char));
        path->elements[path->elCount++] = element;
        snprintf(element, offset + 1, "%.*s", offset, str + prevOffset);

        prevOffset += offset + 1;
    }

    int elCap = n - prevOffset + 1;

    if (elCap > 1)
    {
        element = gcArenaAllocate(elCap, alignof(char));
        path->elements[path->elCount++] = element;
        snprintf(element, elCap, "%.*s", elCap - 1, str + prevOffset);
    }

    return 0;
}

int parsePath(HttpPath *path, const char *str, size_t n) {
    return parsePathTrackQueryParameterStart(path, str, n, NULL);
}

/*
    Matches path to endpoint path of form
    /object/<str>/<int> to match /object/hi/123
*/
int pathMatches(HttpPath *endpointPath, HttpPath *reqPath)
{
    TRACE("%s", "pathMatches begin");
    if (endpointPath->elCount != reqPath->elCount)
    {
        return 0;
    }
    for (int i = 0; i < endpointPath->elCount; i++)
    {
        TRACE("pathMatches i=%d", i);
        if (strcmp(endpointPath->elements[i], "<str>") == 0)
        {
            continue;
        }
        else if (strcmp(endpointPath->elements[i], "<int>") == 0)
        {
            long result = strtol(reqPath->elements[i], NULL, 10);
            if (result == 0 && strcmp(reqPath->elements[i], "0") != 0)
            {
                return 0;
            }
        }
        else if (strcmp(endpointPath->elements[i], reqPath->elements[i]) != 0)
        {
            return 0;
        }
    }
    return 1;
}