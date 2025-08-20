//
// Created by Crucerescu Vladislav on 17.08.2025.
//

#ifndef JSON_H
#define JSON_H

#include "array.h"
#include <stdbool.h>

typedef enum JType
{
    JSON_OBJECT,
    JSON_LIST,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL,
} JType;

typedef struct JToken JToken;
typedef struct JProperty JProperty;

typedef struct {
    JProperty *properties;
    unsigned int count;
} JObject;

typedef struct {
    JToken *tokens;
    unsigned int count;
} JList;

typedef struct {
    char *value;
    unsigned int size;
} JString;

typedef struct
{
    double value;
} JNumber;

typedef struct
{
    int value;
} JBool;

typedef union
{
    JObject object;
    JList list;
    JString string;
    JNumber number;
    JBool boolean;
} JValue;

DEFINE_ARRAY_H(char)
typedef ARRAY_T(char) string;

struct JToken
{
    JValue value;
    JType type;
};

struct JProperty
{
    JString key;
    JToken token;
};

/*
Takes in a JsonElement struct, a pointer to a pointer of a buffer.
The function will automatically allocate enough bytes in the buffer.
The *buffer needs to be freed after use.
Returns the amount of bytes written in the buffer.
Note that the amount allocated will not always equal size.
If you need to save space, reallocate the buffer.
*/
unsigned int serializeJson(JToken element, char **buffer, int indent);
JToken deserializeJson(char *buffer);
JToken toJToken_JObject(JObject object);
JToken toJToken_JList(JList array);
JToken toJToken_JString(JString string);
JToken toJToken_JNumber(JNumber number);
JToken toJToken_JBool(JBool boolean);
JToken toJToken_double(double number);
JToken toJToken_int(int number);
JToken toJToken_bool(bool boolean);
JToken toJToken_string(string string);
JToken toJToken_cstring(char *cstring);
JToken toJToken_JToken(JToken token);
JToken _JNull();
#define _JToken(value) (JToken) _Generic((value),\
    JObject: toJToken_JObject,\
    JList: toJToken_JList,\
    JString: toJToken_JString,\
    JNumber: toJToken_JNumber,\
    JBool: toJToken_JBool,\
    double: toJToken_double,\
    int: toJToken_int,\
    bool: toJToken_bool,\
    string: toJToken_string,\
    char *: toJToken_cstring,\
    JToken: toJToken_JToken\
)(value)

#define _JString(cstring) ((JString) {.value = cstring, .size = strlen(cstring)})
#define _JProperty(cstring, value) ((JProperty) {.key = _JString(cstring), .token = _JToken(value)})
#define _JList(...) ((JList) {.tokens = ((JToken[]) {__VA_ARGS__}), .count = (sizeof((JToken[]){__VA_ARGS__}) / sizeof(JToken))})
#define _JObject(...) ((JObject) {.properties = ((JProperty[]) {__VA_ARGS__}), .count = (sizeof((JProperty[]){__VA_ARGS__}) / sizeof(JProperty))})

#endif //JSON_H
