//
// Created by Rescyy on 2/9/2026.
//

#include <http_query.h>
#include <alloc.h>

#include "logging.h"

int parseQuery(HttpQuery *query, const char *str, size_t len) {

    TRACE("%zu %.*s", len, (int) len, str);

    int ampersands = 0;
    for (size_t i = 0; i < len; i++) {
        ampersands += str[i] == '&';
    }

    query->count = 0;
    query->parameters = gcArenaAllocate(sizeof(HttpQueryParameter) * (ampersands + 1), alignof(HttpQueryParameter));

    size_t prevOffset = 0;
    for (int i = 0; i < ampersands + 1; i++) {

        ssize_t parameterEnd;
        if (i == ampersands) {
            parameterEnd = (ssize_t) (len - prevOffset);
        } else {
            parameterEnd = strnindex(str + prevOffset, (int) (len - prevOffset), "&");
            if (parameterEnd == 0) {
                goto loop_end;
            }
        }

        ssize_t equalOffset = strnindex(str + prevOffset, (int) parameterEnd, "=");
        if (equalOffset == 0) {
            goto loop_end;
        }

        HttpQueryParameter *param = &query->parameters[query->count++];
        string *key = &param->key;
        string *value = &param->value;

        if (equalOffset == -1) {
            *key = copyStringFromSlice(str + prevOffset, parameterEnd);
            *value = copyStringFromSlice(NULL, -1);
        } else {
            *key = copyStringFromSlice(str + prevOffset, equalOffset);
            *value = copyStringFromSlice(str + prevOffset + equalOffset + 1,
                parameterEnd - equalOffset - 1);
        }

        loop_end:
        prevOffset += parameterEnd + 1;
    }

    return 0;
}

HttpQueryParameter *findQueryParameter(HttpQuery *query, const char *key) {
    return (HttpQueryParameter *) findKeyValue((KeyValue *) query->parameters, query->count, key);
}