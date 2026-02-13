//
// Created by Rescyy on 2/9/2026.
//

#include <http_version.h>
#include <string.h>

int isVersionValid(const char *str, int n)
{
    if (str == NULL || n != 8 || strncmp(str, "HTTP/", 5) != 0)
    {
        return 0;
    }
#define IS_EQUAL(ver)                  \
if (strncmp(str + 5, ver, 3) == 0) \
return 1;
    IS_EQUAL("0.9");
    IS_EQUAL("1.0");
    IS_EQUAL("1.1");
    IS_EQUAL("2.0");
    IS_EQUAL("3.0");
#undef IS_EQUAL
    return 0;
}

#define DEFAULT_VERSION 11
int getVersionNumber(const char *str, int n)
{
    if (str == NULL)
    {
        return DEFAULT_VERSION;
    }
    if (n != 8 || strncmp(str, "HTTP/", 5) != 0)
    {
        return -1;
    }
#define IS_EQUAL(ver, num)             \
if (strncmp(str + 5, ver, 3) == 0) \
return num;
    IS_EQUAL("0.9", 9);
    IS_EQUAL("1.0", 10);
    IS_EQUAL("1.1", 11);
    IS_EQUAL("2.0", 20);
    IS_EQUAL("3.0", 30);
#undef IS_EQUAL
    return -1;
}