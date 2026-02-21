//
// Created by Rescyy on 2/9/2026.
//

#include <http_query.h>
#include <alloc.h>

int parseQuery(HttpQuery *query, const char *str, size_t len) {

    int ampersands = 0;
    for (size_t i = 0; i < len; i++) {
        ampersands += str[i] == '&';
    }

    query->count = ampersands + 1;
    query->parameters = gcArenaAllocate(sizeof(HttpQueryParameter) * query->count, alignof(HttpQueryParameter));

    size_t prevOffset = 0;
    for (int i = 0; i < ampersands + 1; i++) {

        ssize_t ampersandOffset;
        if (i == ampersands) {
            ampersandOffset = (ssize_t) len;
        } else {
            ampersandOffset = strnindex(str + prevOffset, (int) (len - prevOffset), "&");
            if (ampersandOffset == 0) {
                return -1;
            }
        }

        ssize_t equalOffset = strnindex(str + prevOffset, (int) ampersandOffset, "=");
        if (equalOffset == 0) {
            return -1;
        }

        HttpQueryParameter *param = &query->parameters[i];
        string *key = &param->key;
        string *value = &param->value;

        if (equalOffset == -1) {
            *key = copyStringFromSlice(str + prevOffset, ampersandOffset);
            *value = copyStringFromSlice(NULL, -1);
        } else {
            *key = copyStringFromSlice(str + prevOffset, equalOffset);
            *value = copyStringFromSlice(str + prevOffset + equalOffset + 1,
                ampersandOffset - equalOffset - 1);
        }

        prevOffset += ampersandOffset + 1;
    }

    return 0;
}

HttpQueryParameter *findQueryParameter(HttpQuery *query, const char *key) {
    return (HttpQueryParameter *) findKeyValue((KeyValue *) query->parameters, query->count, key);
}