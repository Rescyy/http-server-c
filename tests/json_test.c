//
// Created by Crucerescu Vladislav on 17.08.2025.
//
#include "../src/json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int __debug__ = 0;
#define DEBUG                        \
    printf("Debug %d\n", __debug__); \
    __debug__++;                     \
    fflush(stdout);

#define EXPECT(x)                              \
    if (!(x))                                  \
    {                                          \
        printf("LINE: %d %s\n", __LINE__, #x); \
        testResult = 0;                        \
    }

#define INIT_UNIT_TESTS               \
    int passed = 0, total = 0;        \
    printf("\nRunning Tests...\n\n"); \
    printf("------------------------------------------------------------------------------\n");
#define TEST_RESULTS printf("\nTests passed %d/%d.\n\n", passed, total);

#define UNIT_TEST(x)                                                                            \
    printf("Running %s...\n\n", #x);                                                            \
    int testResult##x = x();                                                                    \
    passed += testResult##x;                                                                    \
    total++;                                                                                    \
    if (testResult##x)                                                                          \
    {                                                                                           \
        printf("\nPassed\n");                                                                   \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        printf("\nFailed\n");                                                                   \
    }                                                                                           \
    printf("------------------------------------------------------------------------------\n"); \
    fflush(stdout);

#define FREE(x) \
    free(x);    \
    x = NULL;

int test1()
{
    int testResult = 1;

    JToken element = _JNull();
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 4);
    EXPECT(strncmp(buffer, "null", 4) == 0);

    FREE(buffer);
    return testResult;
}

int test2()
{
    int testResult = 1;

    JToken element = _JToken(true);
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 4);
    EXPECT(strncmp(buffer, "true", 4) == 0);

    FREE(buffer);
    return testResult;
}

int test3()
{
    int testResult = 1;

    JToken element = _JToken(false);
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 5);
    printf("buffer: %s\n", buffer);
    EXPECT(strncmp(buffer, "false", 5) == 0);

    FREE(buffer);
    return testResult;
}

int test4()
{
    int testResult = 1;

    int expectedSize = strlen("Hello, World!") + 2;
    JToken element = _JToken("Hello, World!");

    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == expectedSize);
    EXPECT(strncmp(buffer, "\"Hello, World!\"", expectedSize) == 0);
    FREE(buffer);
    return testResult;
}

int test5()
{
    int testResult = 1;

    JToken element = _JToken(3.14);
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 4);
    EXPECT(strncmp(buffer, "3.14", 4) == 0);
    FREE(buffer);
    return testResult;
}

int test6()
{
    int testResult = 1;

    JToken element = _JToken(0.3515);
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 6);
    EXPECT(strncmp(buffer, "0.3515", 6) == 0);
    FREE(buffer);
    return testResult;
}

int test7()
{
    int testResult = 1;

    JToken element = _JToken(0.123456789);
    char *buffer;

    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 8);
    EXPECT(strncmp(buffer, "0.123457", 8) == 0);
    FREE(buffer);
    return testResult;
}

int test8()
{
    int testResult = 1;

    JToken element = _JToken(12);
    char *buffer;
    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 2);
    EXPECT(strncmp(buffer, "12", 2) == 0);
    FREE(buffer);
    return testResult;
}

int test9()
{
    int testResult = 1;

    JToken element = _JToken(9.234e20);
    char *buffer;
    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 9);
    EXPECT(strncmp(buffer, "9.234e+20", 9) == 0);
    FREE(buffer);
    return testResult;
}

int test10()
{
    int testResult = 1;

    JToken element = _JToken(0.0000001);
    char *buffer;
    int size = serializeJson(element, &buffer, 0);
    EXPECT(size == 5);
    EXPECT(strncmp(buffer, "1e-07", 5) == 0);
    FREE(buffer);
    return testResult;
}

int test11()
{
    int testResult = 1;

    JToken elementArray = _JToken(_JList(_JToken(1)));

    char *buffer;
    int size = serializeJson(elementArray, &buffer, 0);
    EXPECT(size == 3);
    EXPECT(strncmp(buffer, "[1]", 3) == 0);
    FREE(buffer);
    return testResult;
}

int test12()
{
    int testResult = 1;

    JToken elementArray = _JToken(_JListEmpty());

    char *buffer;
    int size = serializeJson(elementArray, &buffer, 0);
    EXPECT(size == 2);
    EXPECT(strncmp(buffer, "[]", 2) == 0);
    FREE(buffer);
    return testResult;
}

int test13()
{
    int testResult = 1;

    JToken elementArray = _JToken(_JList(_JToken(_JListEmpty())));

    char *buffer;
    int size = serializeJson(elementArray, &buffer, 0);
    EXPECT(size == 4);
    EXPECT(strncmp(buffer, "[[]]", 4) == 0);
    FREE(buffer);
    return testResult;
}

int test14()
{
    int testResult = 1;
    JToken elementArray = _JToken(
        _JList(
            _JToken(1.0),
            _JToken("Hello, World!")
        )
    );
    char *buffer;
    int size = serializeJson(elementArray, &buffer, 0);
    EXPECT(size == 19);
    printf("");
    fflush(stdout);
    EXPECT(strncmp(buffer, "[1,\"Hello, World!\"]", 19) == 0);
    FREE(buffer);
    return testResult;
}

int test15()
{
    int testResult = 1;

    JToken elementObject = _JToken(
        _JObject(
            _JProperty("number", 1),
            _JProperty("string", "Hello, World!")
        )
    );

    char *buffer;
    int size = serializeJson(elementObject, &buffer, 0);
    char expectedString[] = "{\"number\":1,\"string\":\"Hello, World!\"}";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);
    return testResult;
}

int test16()
{
    int testResult = 1;

    JToken elementObject = _JToken(
         _JObject(
             _JProperty("number", 1),
             _JProperty("string", "Hello, World!")
         )
    );

    char *buffer;
    int size = serializeJson(elementObject, &buffer, 1);
    char expectedString[] = "{\n \"number\": 1,\n \"string\": \"Hello, World!\"\n}";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);
    return testResult;
}

int test17() {
    int testResult = 1;

    JToken elementObject = _JToken(
        _JObject(
            _JProperty("array0", _JListEmpty()),
            _JProperty("array1", _JList(
                _JToken("Hello")
                )
            )
        )
    );

    char *buffer;
    int size = serializeJson(elementObject, &buffer, 2);
    char expectedString[] = "{\n  \"array0\": [],\n  \"array1\": [\n    \"Hello\"\n  ]\n}";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);

    return testResult;
}

int test18() {
    int testResult = 1;

    JToken token = _JToken(
        _JList(
            _JToken(_JObjectEmpty()),
            _JToken(_JObjectEmpty()),
            _JToken(_JList(
                _JToken(_JObjectEmpty()),
                _JToken(_JObjectEmpty())
            ))
        )
    );

    char *buffer;
    int size = serializeJson(token, &buffer, 1);
    char expectedString[] = "[\n {},\n {},\n [\n  {},\n  {}\n ]\n]";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);

    return testResult;
}

int test19() {
    int testResult = 1;

    JToken token = _JToken(
        _JObject(
            _JProperty("1", _JListEmpty()),
            _JProperty("2", _JListEmpty()),
            _JProperty("3", _JObject(
                _JProperty("1", _JListEmpty()),
                _JProperty("2", _JListEmpty())
                )
            )
        )
    );

    char *buffer;
    int size = serializeJson(token, &buffer, 1);
    char expectedString[] = "{\n \"1\": [],\n \"2\": [],\n \"3\": {\n  \"1\": [],\n  \"2\": []\n }\n}";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);

    return testResult;
}

int test20() {
    int testResult = 1;

    JToken token = _JToken("salut\"/\\\b\f\n\r\tpaca");

    char *buffer;
    int size = serializeJson(token, &buffer, 1);
    char expectedString[] = "\"salut\\\"\\/\\\\\\b\\f\\n\\r\\tpaca\"";
    EXPECT(size == strlen(expectedString));
    EXPECT(strncmp(buffer, expectedString, strlen(expectedString)) == 0);
    FREE(buffer);

    return testResult;
}

int test21() {
    int testResult = 1;

    char nullString[] = "null";
    RESULT_T(JToken) token = deserializeJson(nullString, strlen(nullString));

    EXPECT(token.ok);
    EXPECT(token.var.type == JSON_NULL);

    freeJson(&token.var);

    return testResult;
}

int test22() {
    int testResult = 1;

    char trueString[] = "true";
    RESULT_T(JToken) token = deserializeJson(trueString, strlen(trueString));
    EXPECT(token.ok);
    EXPECT(token.var.type == JSON_BOOLEAN);
    EXPECT(token.var.literal.boolean.value);
    JToken expectedToken = _JToken(true);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test23() {
    int testResult = 1;

    char falseString[] = "false";
    RESULT_T(JToken) token = deserializeJson(falseString, strlen(falseString));
    EXPECT(token.ok);
    EXPECT(token.var.type == JSON_BOOLEAN);
    EXPECT(!token.var.literal.boolean.value);
    JToken expectedToken = _JToken(false);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test24() {
    int testResult = 1;

    char numString[] = "123";
    RESULT_T(JToken) token = deserializeJson(numString, strlen(numString));
    JToken expectedToken = _JToken(123);
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test25() {
    int testResult = 1;

    char numString[] = "1.001";
    RESULT_T(JToken) token = deserializeJson(numString, strlen(numString));
    JToken expectedToken = _JToken(1.001);
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test26() {
    int testResult = 1;

    char numString[] = "-12345.6789";
    RESULT_T(JToken) token = deserializeJson(numString, strlen(numString));
    JToken expectedToken = _JToken(-12345.6789);
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test27() {
    int testResult = 1;

    char numString[] = "4.68e20";
    RESULT_T(JToken) token = deserializeJson(numString, strlen(numString));
    JToken expectedToken = _JToken(4.68e20);
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test28() {
    int testResult = 1;

    char numString[] = "-0.023456789e-7";
    RESULT_T(JToken) token = deserializeJson(numString, strlen(numString));
    JToken expectedToken = _JToken(-0.023456789e-7);
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test29() {
    int testResult = 1;

    char cstring[] = "\"Hello world!\"";
    RESULT_T(JToken) token = deserializeJson(cstring, strlen(cstring));
    JToken expectedToken = _JToken("Hello world!");
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test30() {
    int testResult = 1;

    char cstring[] = "\"\\\\\\\"\\/\\n\\b\\t\\r\\f\"";
    RESULT_T(JToken) token = deserializeJson(cstring, strlen(cstring));
    JToken expectedToken = _JToken("\\\"/\n\b\t\r\f");
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test31() {
    int testResult = 1;

    char objectString[] = "{\n}";
    RESULT_T(JToken) token = deserializeJson(objectString, strlen(objectString));
    JToken expectedToken = _JToken(_JObjectEmpty());
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test32() {
    int testResult = 1;

    char cstring[] = "\"\"";
    RESULT_T(JToken) token = deserializeJson(cstring, strlen(cstring));
    JToken expectedToken = _JToken(_JStringEmpty());
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test33() {
    int testResult = 1;

    char objectString[] = "{\n\"salut\": \n\n\"paca\"   \n}";
    RESULT_T(JToken) token = deserializeJson(objectString, strlen(objectString));
    JToken expectedToken = _JToken(
        _JObject(
            _JProperty("salut", "paca")
        )
    );
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test34() {
    int testResult = 1;

    char objectString[] = "{\"null\": null, \"1\": 1, \"1.2\": 1.2, \"1.2e3\": 1.2e3, \"true\": true, \"false\": false, \"-0.1e23\": -0.1e23}";
    RESULT_T(JToken) token = deserializeJson(objectString, strlen(objectString));
    JToken expectedToken = _JToken(
        _JObject(
            _JProperty("null", _JNull()),
            _JProperty("1.2e3", 1.2e3),
            _JProperty("1.2", 1.2),
            _JProperty("false", false),
            _JProperty("1", 1),
            _JProperty("true", true),
            _JProperty("-0.1e23", -0.1e23)
        )
    );
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test35() {
    int testResult = 1;

    char objectString[] = "\n \n\r\t\r [\"null\"\n, null\n, \"1\"  , {} \n\r\t , 1, \"1.2\" , 1.2, \"1.2e3\", 1.2e3, \"true\", true, \"false\"  \n , false, \"-0.1e23\", \n -0.1e23\n\n]\n  \n ";
    RESULT_T(JToken) token = deserializeJson(objectString, strlen(objectString));
    JToken expectedToken = _JToken(
        _JList(
            _JToken("null"),
            _JNull(),
            _JToken("1"),
            _JToken(_JObjectEmpty()),
            _JToken(1),
            _JToken("1.2"),
            _JToken(1.2),
            _JToken("1.2e3"),
            _JToken(1.2e3),
            _JToken("true"),
            _JToken(true),
            _JToken("false"),
            _JToken(false),
            _JToken("-0.1e23"),
            _JToken(-0.1e23)
        )
    );
    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test36() {
    int testResult = 1;

    char objectString[] = "{\n \"list\":[1, \"salut\", {\"paca\": false}\n ]\n }";
    RESULT_T(JToken) token = deserializeJson(objectString, strlen(objectString));
    JToken expectedToken = _JToken(
        _JObject(
            _JProperty("list",
                _JList(
                    _JToken(1),
                    _JToken("salut"),
                    _JToken(_JObject(
                        _JProperty("paca", false)
                    ))
                )
            )
        )
    );

    EXPECT(token.ok);
    EXPECT(equalsJson(&token.var, &expectedToken));

    freeJson(&token.var);

    return testResult;
}

int test37() {
    int testResult = 1;

    char errorString[] = "string";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test38() {
    int testResult = 1;

    char errorString[] = "{ hello: paca }";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test39() {
    int testResult = 1;

    char errorString[] = "\"salut";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test40() {
    int testResult = 1;

    char errorString[] = "1.2.3";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test41() {
    int testResult = 1;

    char errorString[] = "{ \"not full\": 0";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test42() {
    int testResult = 1;

    char errorString[] = "[ { ] }";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test43() {
    int testResult = 1;

    char errorString[] = "\"salut}";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int test44() {
    int testResult = 1;

    char errorString[] = "1.5 - 4.5";

    RESULT_T(JToken) token = deserializeJson(errorString, strlen(errorString));
    EXPECT(!token.ok);

    return testResult;
}

int main()
{
    INIT_UNIT_TESTS
    UNIT_TEST(test1)
    UNIT_TEST(test2)
    UNIT_TEST(test3)
    UNIT_TEST(test4)
    UNIT_TEST(test5)
    UNIT_TEST(test6)
    UNIT_TEST(test7)
    UNIT_TEST(test8)
    UNIT_TEST(test9)
    UNIT_TEST(test10)
    UNIT_TEST(test11)
    UNIT_TEST(test12)
    UNIT_TEST(test13)
    UNIT_TEST(test14)
    UNIT_TEST(test15)
    UNIT_TEST(test16)
    UNIT_TEST(test17)
    UNIT_TEST(test18)
    UNIT_TEST(test19)
    UNIT_TEST(test20)
    UNIT_TEST(test21)
    UNIT_TEST(test22)
    UNIT_TEST(test23)
    UNIT_TEST(test24)
    UNIT_TEST(test25)
    UNIT_TEST(test26)
    UNIT_TEST(test27)
    UNIT_TEST(test28)
    UNIT_TEST(test29)
    UNIT_TEST(test30)
    UNIT_TEST(test31)
    UNIT_TEST(test32)
    UNIT_TEST(test33)
    UNIT_TEST(test34)
    UNIT_TEST(test35)
    UNIT_TEST(test36)
    UNIT_TEST(test37)
    UNIT_TEST(test38)
    UNIT_TEST(test39)
    UNIT_TEST(test40)
    UNIT_TEST(test41)
    UNIT_TEST(test42)
    UNIT_TEST(test43)
    UNIT_TEST(test44)
    TEST_RESULTS
    return total - passed;
}