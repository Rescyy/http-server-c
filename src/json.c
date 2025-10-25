//
// Created by Crucerescu Vladislav on 17.08.2025.
//

#include "../includes/json.h"
#include "../includes/alloc.h"

#include <limits.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <errno.h>

#include "../includes/logging.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

DEFINE_ARRAY_FUNCS(char, gcAllocate, gcReallocate)

static void serializeElement(JToken element, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeObject(JObject object, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeList(JList array, ARRAY_T(char) *buffer, int indentDepth, int indent);
static void serializeString(JString string_, ARRAY_T(char) *buffer);
static void serializeNumber(JNumber number, ARRAY_T(char) *buffer);
static void serializeBoolean(JBool boolean, ARRAY_T(char) *buffer);

size_t serializeJson(JToken element, char **buffer, int indent) {
    ARRAY_T(char) array = ARRAY_WITH_CAPACITY(char, 4);
    serializeElement(element, &array, 0, indent);
    *buffer = array.data;
    return array.length;
}

static void serializeElement(JToken element, ARRAY_T(char) *buffer, int indentDepth, int indent) {
    switch (element.type) {
        case JSON_OBJECT: {
            serializeObject(element.literal.object, buffer, indentDepth + 1, indent);
            break;
        }
        case JSON_LIST: {
            serializeList(element.literal.list, buffer, indentDepth + 1, indent);
            break;
        }
        case JSON_STRING: {
            serializeString(element.literal.string, buffer);
            break;
        }
        case JSON_NUMBER: {
            serializeNumber(element.literal.number, buffer);
            break;
        }
        case JSON_BOOLEAN: {
            serializeBoolean(element.literal.boolean, buffer);
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
        serializeElement(object.properties[i].value, buffer, indentDepth, indent);
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

static void pushSpecialCharacter(ARRAY_T(char) *buffer, char c) {
    ARRAY_PUSH(char, buffer, '\\');
    ARRAY_PUSH(char, buffer, c);
}

static void serializeString(JString string_, ARRAY_T(char) *buffer) {

    ARRAY_ENSURE_CAPACITY(char, buffer, buffer->length + 2 * string_.size);
    ARRAY_PUSH(char, buffer, '"');
    for (unsigned int i = 0; i < string_.size; i++) {
        char c = string_.value[i];
        switch (c) {
            case '"':
            case '/':
            case '\\':
                pushSpecialCharacter(buffer, c);
                break;
            case '\b':
                pushSpecialCharacter(buffer, 'b');
                break;
            case '\f':
                pushSpecialCharacter(buffer, 'f');
                break;
            case '\n':
                pushSpecialCharacter(buffer, 'n');
                break;
            case '\r':
                pushSpecialCharacter(buffer, 'r');
                break;
            case '\t':
                pushSpecialCharacter(buffer, 't');
                break;
            default:
                ARRAY_PUSH(char, buffer, c);
                break;
        }
    }
    ARRAY_PUSH(char, buffer, '"');

}

static void serializeNumber(JNumber number, ARRAY_T(char) *buffer) {

    double value = number.value;
    double a;
    ARRAY_ENSURE_CAPACITY(char, buffer, buffer->length + 20);
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

TYPEDEF_RESULT(JObject);
TYPEDEF_RESULT(JList);
TYPEDEF_RESULT(JNumber);
TYPEDEF_RESULT(JBool);
TYPEDEF_RESULT(JString);

typedef struct {
    const char *ptr;
    size_t len;
} buffer_t;

#define LEN ((buffer)->len)
#define CHAR_AT(i) ((LEN > (i)) ? (buffer)->ptr[i] : 0)
#define CHAR CHAR_AT(0)
#define PTR ((buffer)->ptr)
#define CMP(str) (strncmp(PTR, str, MIN(strlen(str), LEN)) == 0)
#define ADD(num) (buffer)->ptr += (num), (buffer)->len -= (num)
#define SKIP_WHITESPACE(type, action) if (!skipWhitespace(buffer)) {action;}
#define ASSERT_CHAR(type, c, action) if (CHAR != (c)) {action;} ADD(1)
#define IF_ERROR(type, result, action) if (!result.ok) {action;}

static RESULT_T(JToken) deserializeToken(buffer_t *buffer);
static RESULT_T(JString) deserializeString(buffer_t *buffer);
static RESULT_T(JObject) deserializeObject(buffer_t *buffer);
static RESULT_T(JList) deserializeList(buffer_t *buffer);
static RESULT_T(JNumber) deserializeNumber(buffer_t *buffer);
static RESULT_T(JBool) deserializeBoolean(buffer_t *buffer);
static int skipWhitespace(buffer_t *buffer);

RESULT_T(JToken) deserializeJson(const char *buffer, const size_t len) {
    if (buffer == NULL || len == 0) return RESULT_ERROR(JToken);
    buffer_t buffer_ = { .ptr = buffer, .len = len };
    RESULT_T(JToken) token = deserializeToken(&buffer_);
    if (!skipWhitespace(&buffer_)) return RESULT_ERROR(JToken);
    if (buffer_.len != 0) return RESULT_ERROR(JToken);
    return token;
}

static RESULT_T(JToken) deserializeToken(buffer_t *buffer) {

    JToken token;
    SKIP_WHITESPACE(JToken, return RESULT_ERROR(JToken));
    switch (CHAR) {
        case '{': {
            RESULT_T(JObject) object = deserializeObject(buffer);
            IF_ERROR_RETURN(JToken, object);
            token = toJToken_JObject(object.var);
            break;
        }
        case '[': {
            RESULT_T(JList) list = deserializeList(buffer);
            IF_ERROR_RETURN(JToken, list);
            token = toJToken_JList(list.var);
            break;
        }
        case '"': {
            RESULT_T(JString) string = deserializeString(buffer);
            IF_ERROR_RETURN(JToken, string);
            token = toJToken_JString(string.var);
            break;
        }
        case 't':
        case 'f': {
            RESULT_T(JBool) boolean = deserializeBoolean(buffer);
            IF_ERROR_RETURN(JToken, boolean);
            token = toJToken_JBool(boolean.var);
            break;
        }
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            RESULT_T(JNumber) number = deserializeNumber(buffer);
            IF_ERROR_RETURN(JToken, number);
            token = toJToken_JNumber(number.var);
            break;
        }
        case 'n': {
            static const char nullString[] = "null";
            if (CMP(nullString)) {
                token = _JNull();
                ADD(strlen(nullString));
            } else {

                return RESULT_ERROR(JToken);
            }
            break;
        }
        default:

            return RESULT_ERROR(JToken);
    }

    return RESULT_FROM_VAR(JToken, token);
}

static RESULT_T(JString) deserializeString(buffer_t *buffer) {

    ASSERT_CHAR(JString, '"', goto _returnError);
    if (CHAR == '"') {
        JString string = _JStringEmpty();
        ADD(1);
        return RESULT_FROM_VAR(JString, string);
    }
    size_t stringLen = 0;
    int backslash = 0;
    char *string = gcArenaAllocate(LEN, alignof(char));
    for (size_t i = 0; i < LEN; i++) {
        const char c = CHAR_AT(i);
        if (backslash) {
            backslash = 0;
            switch (c) {
                case '\\':
                case '"':
                case '/':
                    string[stringLen++] = c;
                    break;
                case 'n':
                    string[stringLen++] = '\n';
                    break;
                case 'b':
                    string[stringLen++] = '\b';
                    break;
                case 't':
                    string[stringLen++] = '\t';
                    break;
                case 'r':
                    string[stringLen++] = '\r';
                    break;
                case 'f':
                    string[stringLen++] = '\f';
                    break;
                default:
                    goto _returnError;
            }
        } else switch (c) {
            case '\\':
                backslash = 1;
                break;
            case '"': {
                gcArenaGiveBack(LEN - stringLen);
                JString jstring = {.value = string, .size = stringLen};
                ADD(i + 1);
                return RESULT_FROM_VAR(JString, jstring);
            }
            default:
                if (c > 0 && c < ' ') {
                    goto _returnError;
                }
                string[stringLen++] = c;
                break;
        }
    }

_returnError:
    return RESULT_ERROR(JString);
}

TYPEDEF_ARRAY(JProperty);
DEFINE_ARRAY_C(JProperty)

static RESULT_T(JObject) deserializeObject(buffer_t *buffer) {
    ASSERT_CHAR(JObject, '{', goto _returnError);
    SKIP_WHITESPACE(JObject, goto _returnError);
    if (CHAR == '}') {
        JObject object = _JObjectEmpty();
        ADD(1);
        return RESULT_FROM_VAR(JObject, object);
    }
    ARRAY_T(JProperty) properties = ARRAY_NEW(JProperty);
    for (;;) {
        RESULT_T(JString) key = deserializeString(buffer);
        IF_ERROR(JObject, key, break);
        SKIP_WHITESPACE(JObject, break);
        ASSERT_CHAR(JObject, ':', break);
        SKIP_WHITESPACE(JObject, break);
        RESULT_T(JToken) value = deserializeToken(buffer);
        IF_ERROR(JObject, value, break);
        SKIP_WHITESPACE(JObject, break);
        JProperty property = {
            .key = key.var,
            .value = value.var,
        };
        ARRAY_PUSH(JProperty, &properties, property);
        if (CHAR == '}') {
            ARRAY_SHRINK_TO_FIT(JProperty, &properties);
            JObject object = {
                .properties = properties.data,
                .count = properties.length,
            };
            ADD(1);
            return RESULT_FROM_VAR(JObject, object);
        }
        ASSERT_CHAR(JObject, ',', goto _returnError);
        SKIP_WHITESPACE(JObject, goto _returnError);
    }

_returnError:
    return RESULT_ERROR(JObject);
}

TYPEDEF_ARRAY(JToken);
DEFINE_ARRAY_C(JToken)

static RESULT_T(JList) deserializeList(buffer_t *buffer) {
    ASSERT_CHAR(JList, '[', goto _returnError);
    SKIP_WHITESPACE(JList, goto _returnError);
    if (CHAR == ']') {
        JList list = _JListEmpty();
        ADD(1);
        return RESULT_FROM_VAR(JList, list);
    }
    ARRAY_T(JToken) tokens = ARRAY_NEW(JToken);
    for (;;) {
        RESULT_T(JToken) token = deserializeToken(buffer);
        IF_ERROR(JList, token, goto _returnError);
        SKIP_WHITESPACE(JList, goto _returnError);
        ARRAY_PUSH(JToken, &tokens, token.var);
        if (CHAR == ']') {
            ARRAY_SHRINK_TO_FIT(JToken, &tokens);
            JList list = {
                .tokens = tokens.data,
                .count = tokens.length,
            };
            ADD(1);
            return RESULT_FROM_VAR(JList, list);
        }
        ASSERT_CHAR(JList, ',', goto _returnError);
        SKIP_WHITESPACE(JList, goto _returnError);
    }

_returnError:
    return RESULT_ERROR(JList);
}

static RESULT_T(JNumber) deserializeNumber(buffer_t *buffer) {
    char parseable[128] = {0};
    memcpy(parseable, PTR, MIN(128, LEN));

    errno = 0;
    char *numEnd;
    double value = strtod(parseable, &numEnd);
    size_t numLen = numEnd - parseable;

    if (numLen == 0 || LEN < numLen || errno == ERANGE) {
        return RESULT_ERROR(JNumber);
    }

    JNumber number = {.value = value};
    ADD(numLen);
    return RESULT_FROM_VAR(JNumber, number);
}

static RESULT_T(JBool) deserializeBoolean(buffer_t *buffer) {

    static const char trueString[] = "true";
    static const char falseString[] = "false";
    JBool boolean;
    if (CMP(trueString)) {
        ADD(strlen(trueString));
        boolean.value = 1;
    } else if (CMP(falseString)) {
        ADD(strlen(falseString));
        boolean.value = 0;
    } else {
        return RESULT_ERROR(JBool);
    }
    return RESULT_FROM_VAR(JBool, boolean);
}

static int skipWhitespace(buffer_t *buffer) {
    if (LEN == 0) return 1;
    for (size_t i = 0; i < LEN; i++) {
        char c = CHAR_AT(i);
        switch (c) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            default:
                if (c < ' ') {
                    return 0;
                }
                ADD(i);
                return 1;
        }
    }
    ADD(LEN);
    return 1;
}

#undef LEN
#undef CHAR_AT
#undef CHAR
#undef CMP
#undef ADD
#undef SKIP_WHITESPACE
#undef ASSERT_CHAR

static int equalsToken(JToken *token1, JToken *token2);
static int equalsString(JString *string1, JString *string2);
static int equalsObject(JObject *object1, JObject *object2);
static int equalsList(JList *list1, JList *list2);
static int equalsNumber(JNumber *number1, JNumber *number);
static int equalsBoolean(JBool *boolean1, JBool *boolean2);

int equalsJson(JToken *token1, JToken *token2) {
    return equalsToken(token1, token2);
}

static int equalsToken(JToken *token1, JToken *token2) {
    if (token1 == NULL || token2 == NULL) return 0;
    if (token1 == token2) return 1;
    if (token1->type != token2->type) return 0;
    JValue *value1 = &token1->literal, *value2 = &token2->literal;
    switch (token1->type) {
        case JSON_OBJECT:
            return equalsObject((JObject*) value1, (JObject*) value2);
        case JSON_LIST:
            return equalsList((JList*) value1, (JList*) value2);
        case JSON_NUMBER:
            return equalsNumber((JNumber*) value1, (JNumber*) value2);
        case JSON_STRING:
            return equalsString((JString*) value1, (JString*) value2);
        case JSON_BOOLEAN:
            return equalsBoolean((JBool*) value1, (JBool*) value2);
        case JSON_NULL:
            return 1;
        default:
            return 0;
    }
}

static int equalsString(JString *string1, JString *string2) {
    if (string1->size != string2->size) return 0;
    return strncmp(string1->value, string2->value, string1->size) == 0;
}

static int equalsObject(JObject *object1, JObject *object2) {
    if (object1->count != object2->count) return 0;
    for (size_t i = 0; i < object1->count; i++) {
        JToken *token = getValueJObject(object2, &object1->properties[i].key);
        if (!equalsToken(token, &object1->properties[i].value)) return 0;
    }
    return 1;
}

static int equalsList(JList *list1, JList *list2) {
    if (list1->count != list2->count) return 0;
    for (size_t i = 0; i < list1->count; i++) {
        if (!equalsToken(&list1->tokens[i], &list2->tokens[i])) return 0;
    }
    return 1;
}

static int equalsNumber(JNumber *number1, JNumber *number2) {
    return number1->value == number2->value;
}

static int equalsBoolean(JBool *boolean1, JBool *boolean2) {
    return boolean1->value == boolean2->value;
}

JToken *getValueJObject(JObject *object, JString *key) {
    for (size_t i = 0; i < object->count; i++) {
        JString *key2 = &object->properties[i].key;
        if (equalsString(key, key2)) {
            return &object->properties[i].value;
        }
    }
    return NULL;
}

JToken toJToken_JObject(JObject object) {
    return (JToken) {
        .literal = (JValue) {.object = object},
        .type = JSON_OBJECT,
    };
}

JToken toJToken_JList(JList array) {
    return (JToken) {
        .literal = (JValue) {.list = array},
        .type = JSON_LIST
    };
}

JToken toJToken_JString(JString string) {
    return (JToken) {
        .literal = (JValue) {.string = string},
        .type = JSON_STRING,
    };
}

JToken toJToken_JNumber(JNumber number) {
    return (JToken) {
        .literal = (JValue) {.number = number},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_JBool(JBool boolean) {
    return (JToken) {
        .literal = {.boolean = boolean},
        .type = JSON_BOOLEAN,
    };
}

JToken toJToken_double(double number) {
    return (JToken){
        .literal = (JValue) {.number = (JNumber) {.value = number}},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_int(int number) {
    return (JToken) {
        .literal = (JValue) {.number = (JNumber) {.value = number}},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_long(long number) {
    return (JToken) {
        .literal = (JValue) {.number = (JNumber) {.value = number}},
        .type = JSON_NUMBER,
    };
}

JToken toJToken_bool(bool boolean) {
    return (JToken) {
        .literal = (JValue) {.boolean = (JBool){.value = boolean}},
        .type = JSON_BOOLEAN,
    };
}

JToken toJToken_string(ARRAY_T(char) string) {
    return (JToken) {
        .literal = (JValue) {.string = (JString) {
            .value = string.data,
            .size = string.length,
        }},
        .type = JSON_STRING,
    };
}

JToken toJToken_cstring(const char *cstring) {
    return (JToken) {
        .literal = (JValue) {.string = (JString){
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

JList _JListEmpty() {
    return (JList) {
        .tokens = NULL,
        .count = 0,
    };
}
JObject _JObjectEmpty() {
    return (JObject) {
        .properties = NULL,
        .count = 0,
    };
}

JString _JStringEmpty() {
    return (JString) {
        .value = NULL,
        .size = 0,
    };
}

JObject toJObject_JProperties(const unsigned int count, ...) {
    va_list args;
    va_start(args, count);
    JObject object = {
        .properties = gcArenaAllocate(sizeof(JProperty) * count, alignof(JProperty)),
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
        .tokens = gcArenaAllocate(sizeof(JToken) * count, alignof(JToken)),
        .count = count,
    };
    for (unsigned int i = 0; i < count; i++) {
        list.tokens[i] = va_arg(args, JToken);
    }
    va_end(args);
    return list;
}
