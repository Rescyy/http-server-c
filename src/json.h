//
// Created by Crucerescu Vladislav on 17.08.2025.
//

#ifndef JSON_H
#define JSON_H

#include "array.h"
#include "result.h"

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
    size_t count;
} JObject;

typedef struct {
    JToken *tokens;
    size_t count;
} JList;

typedef struct {
    char *value;
    size_t size;
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

DEFINE_ARRAY_H(char);

struct JToken
{
    JValue literal;
    JType type;
};

struct JProperty
{
    JString key;
    JToken value;
};

TYPEDEF_RESULT(JToken);

/*
Takes in a JsonElement struct, a pointer to a pointer of a buffer.
The function will automatically allocate enough bytes in the buffer.
The *buffer needs to be freed after use.
Returns the amount of bytes written in the buffer.
Note that the amount allocated will not always equal size.
If you need to save space, reallocate the buffer.
*/
size_t serializeJson(JToken element, char **buffer, int indent);
RESULT_T(JToken) deserializeJson(const char *buffer, size_t len);
void freeJson(JToken *token);
int equalsJson(JToken *a, JToken *b);
JToken *getValueJObject(JObject *object, JString *key);
void addProperty(JObject *object, const char *key, JToken *token);

JToken toJToken_JObject(JObject object);
JToken toJToken_JList(JList array);
JToken toJToken_JString(JString string);
JToken toJToken_JNumber(JNumber number);
JToken toJToken_JBool(JBool boolean);
JToken toJToken_double(double number);
JToken toJToken_int(int number);
JToken toJToken_long(long number);
JToken toJToken_bool(bool boolean);
JToken toJToken_string(ARRAY_T(char) string);
JToken toJToken_cstring(char *cstring);
JToken toJToken_JToken(JToken token);
JToken _JNull();
JList _JListEmpty();
JObject _JObjectEmpty();
JString _JStringEmpty();
#define _JToken(value) ((JToken) _Generic((value),\
    JObject: toJToken_JObject,\
    JList: toJToken_JList,\
    JString: toJToken_JString,\
    JNumber: toJToken_JNumber,\
    JBool: toJToken_JBool,\
    double: toJToken_double,\
    int: toJToken_int,\
    long: toJToken_long,\
    bool: toJToken_bool,\
    char *: toJToken_cstring,\
    const char*: toJToken_cstring,\
    JToken: toJToken_JToken\
)(value))

JObject toJObject_JProperties(const unsigned int count, ...);
JList toJList_JTokens(const unsigned int count, ...);

#define _JString(cstring) ((JString) {.value = cstring, .size = strlen(cstring)})
#define _JProperty(cstring, token) ((JProperty) {.key = _JString(cstring), .value = _JToken(token)})
#define _JList(...) toJList_JTokens((sizeof((JToken[]){__VA_ARGS__})/sizeof(JToken)), __VA_ARGS__)
#define _JObject(...) toJObject_JProperties((sizeof((JProperty[]){__VA_ARGS__})/sizeof(JProperty)), __VA_ARGS__)

#endif //JSON_H
