//
// Created by Crucerescu Vladislav on 17.08.2025.
//

#include "json.h"
#include "alloc.h"

#include <limits.h>
#include <string.h>
#include <math.h>

DEFINE_ARRAY_FUNCS(char)

JToken toJToken_JObject(JObject object) {
    return (JToken) {
        .value = (JValue) {.object = object},
        .type = JSON_OBJECT,
    };
}

JToken toJToken_JList(JList array) {
    return (JToken) {
        .value = (JValue) {.list = array},
        .type = JSON_LIST
    };
}

JToken toJToken_JString(JString string) {
    return (JToken) {
        .value = (JValue) {.string = string},
        .type = JSON_STRING,
    };
}

JToken toJToken_JNumber(JNumber number) {
    return (JToken) {
        .value = (JValue) {.number = number},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_JBool(JBool boolean) {
    return (JToken) {
        .value = {.boolean = boolean},
        .type = JSON_BOOLEAN,
    };
}

JToken toJToken_double(double number) {
    return (JToken){
        .value = (JValue) {.number = (JNumber) {.value = number}},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_int(int number) {
    return (JToken) {
        .value = (JValue) {.number = (JNumber) {.value = number}},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_bool(bool boolean) {
    return (JToken) {
        .value = (JValue) {.boolean = (JBool){.value = boolean}},
        .type = JSON_BOOLEAN,
    };
}

JToken toJToken_string(ARRAY_T(char) string) {
    return (JToken) {
        .value = (JValue) {.string = (JString) {
            .value = string.data,
            .size = string.length,
        }},
        .type = JSON_STRING,
    };
}

JToken toJToken_cstring(char *cstring) {
    return (JToken) {
        .value = (JValue) {.string = (JString){
            .value = cstring,
            .size = strlen(cstring),
        }},
        .type = JSON_STRING,
    };
}

JToken toJToken_JToken(JToken token) {
    return token;
}

JToken _JNull() {
    return (JToken) {
        .type = JSON_NULL,
    };
}

JObject toJObject_JProperties(const unsigned int count, ...) {
    va_list args;
    va_start(args, count);
    JObject object = {
        .properties = allocate(sizeof(JProperty) * count),
        .count = count,
    };
    for (unsigned int i = 0; i < count; i++) {
        object.properties[i] = va_arg(args, JProperty);
    }
    va_end(args);
    return object;
}

JList toJList_JTokens(const unsigned int count, ...) {
    va_list args;
    va_start(args, count);
    JList list = {
        .tokens = allocate(sizeof(JToken) * count),
        .count = count,
    };
    for (unsigned int i = 0; i < count; i++) {
        list.tokens[i] = va_arg(args, JToken);
    }
    va_end(args);
    return list;
}


static void serializeElement(JToken element, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeObject(JObject object, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeList(JList array, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeString(JString string_, ARRAY_T(char) *buffer);
static void serializeNumber(JNumber number, ARRAY_T(char) *buffer);
static void serializeBoolean(JBool boolean, ARRAY_T(char) *buffer);

unsigned int serializeJson(JToken element, char **buffer, int indent) {
    ARRAY_T(char) array = ARRAY_WITH_CAPACITY(char, 4);
    serializeElement(element, &array, 0, indent);
    *buffer = array.data;
    return array.length;
}

static void serializeElement(JToken element, ARRAY_T(char) *buffer, int indentDepth, int indent) {
    switch (element.type) {
        case JSON_OBJECT: {
            serializeObject(element.value.object, buffer, indentDepth + 1, indent);
            break;
        }
        case JSON_LIST: {
            serializeList(element.value.list, buffer, indentDepth + 1, indent);
            break;
        }
        case JSON_STRING: {
            serializeString(element.value.string, buffer);
            break;
        }
        case JSON_NUMBER: {
            serializeNumber(element.value.number, buffer);
            break;
        }
        case JSON_BOOLEAN: {
            serializeBoolean(element.value.boolean, buffer);
            break;
        }
        case JSON_NULL: {
            static char nullString[] = "null";
            ARRAY_PUSH_RANGE(char, buffer, nullString, strlen(nullString));
            break;
        }
    }
}

static void serializeObject(JObject object, ARRAY_T(char) *buffer, int indentDepth, int indent) {
    if (object.count == 0) {
        static char *emptyObject = "{}";
        ARRAY_PUSH_RANGE(char, buffer, emptyObject, strlen(emptyObject));
        return;
    }
    ARRAY_PUSH(char, buffer, '{');
    if (indent) {
        ARRAY_PUSH(char, buffer, '\n');
    }
    for (unsigned int i = 0; i < object.count; i++) {
        ARRAY_PUSH_MULTIPLE(char, buffer, ' ', indentDepth * indent);
        serializeString(object.properties[i].key, buffer);
        ARRAY_PUSH(char, buffer, ':');
        if (indent) {
            ARRAY_PUSH(char, buffer, ' ');
        }
        serializeElement(object.properties[i].token, buffer, indentDepth, indent);
        if (i != object.count - 1) {
            ARRAY_PUSH(char, buffer, ',');
        }
        if (indent) {
            ARRAY_PUSH(char, buffer, '\n');
        }
    }
    ARRAY_PUSH_MULTIPLE(char, buffer, ' ', (indentDepth - 1) * indent);
    ARRAY_PUSH(char, buffer, '}');
}

static void serializeList(JList array, ARRAY_T(char) *buffer, int indentDepth, int indent) {
    if (array.count == 0) {
        static char *emptyList = "[]";
        ARRAY_PUSH_RANGE(char, buffer, emptyList, strlen(emptyList));
        return;
    }
    ARRAY_PUSH(char, buffer, '[');
    if (indent) {
        ARRAY_PUSH(char, buffer, '\n');
    }
    for (unsigned int i = 0; i < array.count; i++) {
        ARRAY_PUSH_MULTIPLE(char, buffer, ' ', indentDepth * indent);
        serializeElement(array.tokens[i], buffer, indentDepth, indent);
        if (i != array.count - 1) {
            ARRAY_PUSH(char, buffer, ',');
        }
        if (indent) {
            ARRAY_PUSH(char, buffer, '\n');
        }
    }
    ARRAY_PUSH_MULTIPLE(char, buffer, ' ', (indentDepth - 1) * indent);
    ARRAY_PUSH(char, buffer, ']');
}

static void pushSpecialCharacter(ARRAY_T(char) *buffer, int *specialCharacters, unsigned int newCap, char c) {
    ARRAY_ENSURE_CAPACITY(char, buffer, newCap + (++*specialCharacters));
    buffer->data[buffer->length++] = '\\';
    buffer->data[buffer->length++] = c;
}

static void serializeString(JString string_, ARRAY_T(char) *buffer) {
    ARRAY_PUSH(char, buffer, '"');
    unsigned int newCap = buffer->capacity + string_.size;
    ARRAY_ENSURE_CAPACITY(char, buffer, newCap);
    int specialCharacters = 0;
    for (unsigned int i = 0; i < string_.size; i++) {
        char c = string_.value[i];
        switch (c) {
            case '"':
            case '/':
            case '\\':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, c);
                break;
            case '\b':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, 'b');
                break;
            case '\f':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, 'f');
                break;
            case '\n':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, 'n');
                break;
            case '\r':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, 'r');
                break;
            case '\t':
                pushSpecialCharacter(buffer, &specialCharacters, newCap, 't');
                break;
            default:
                buffer->data[buffer->length++] = c;
                break;
        }
    }
    ARRAY_PUSH(char, buffer, '"');
}

static void serializeNumber(JNumber number, ARRAY_T(char) *buffer) {
    double value = number.value;
    double a;
    ARRAY_ENSURE_CAPACITY(char, buffer, buffer->capacity + 20);
    if (isfinite(value) && modf(value, &a) == 0.0 &&
        value >= (double) LLONG_MIN && value <= (double) LLONG_MAX) {
        buffer->length += snprintf(buffer->data + buffer->length, buffer->capacity - buffer->length, "%lld", (long long) value);
    } else {
        buffer->length += snprintf(buffer->data + buffer->length, buffer->capacity - buffer->length, "%g", value);
    }
}

static void serializeBoolean(JBool boolean, ARRAY_T(char) *buffer) {
    static char trueStr[] = "true";
    static char falseStr[] = "false";
    if (boolean.value) {
        ARRAY_PUSH_RANGE(char, buffer, trueStr, strlen(trueStr));
    } else {
        ARRAY_PUSH_RANGE(char, buffer, falseStr, strlen(falseStr));
    }
}
