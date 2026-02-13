//
// Created by Rescyy on 2/7/2026.
//

#include <http_path.h>
#include <alloc.h>
#include <utils.h>

int parsePath(HttpPath *path, const char *str, int n)
{
    int offset, prevOffset = 0;
    path->elCount = 0;
    path->elements = NULL;
    path->raw = gcArenaAllocate(n + 1, alignof(char));
    snprintf(path->raw, n + 1, "%.*s", n, str);
    path->raw[n] = '\0';
    char *element;
    int slashes = 0;
    for (int i = 0; i < n; i++) {
        slashes += str[i] == '/';
    }
    if (str[prevOffset++] != '/') {
        return -1;
    }
    if (n == 1) {
        path->elements = NULL;
        return 0;
    }

    path->elements = gcArenaAllocate(sizeof(char *) * slashes, alignof(char *));

    while ((offset = strnindex(str + prevOffset, n - prevOffset, "/")) != -1)
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


/*
    Matches path to endpoint path of form
    /object/<str>/<int> to match /object/hi/123
*/
int pathMatches(HttpPath endpointPath, HttpPath reqPath)
{
    if (endpointPath.elCount != reqPath.elCount)
    {
        return 0;
    }
    for (int i = 0; i < endpointPath.elCount; i++)
    {
        if (strcmp(endpointPath.elements[i], "<str>") == 0)
        {
            continue;
        }
        else if (strcmp(endpointPath.elements[i], "<int>") == 0)
        {
            long result = strtol(reqPath.elements[i], NULL, 10);
            if (result == 0 && strcmp(reqPath.elements[i], "0") != 0)
            {
                return 0;
            }
        }
        else if (strcmp(endpointPath.elements[i], reqPath.elements[i]) != 0)
        {
            return 0;
        }
    }
    return 1;
}