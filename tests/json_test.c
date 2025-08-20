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

    JToken elementArray = _JToken(_JList());

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

    JToken elementArray = _JToken(_JList(_JToken(_JList())));

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
            _JProperty("array0", _JList()),
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
            _JToken(_JObject()),
            _JToken(_JObject()),
            _JToken(_JList(
                _JToken(_JObject()),
                _JToken(_JObject())
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
            _JProperty("1", _JList()),
            _JProperty("2", _JList()),
            _JProperty("3", _JObject(
                _JProperty("1", _JList()),
                _JProperty("2", _JList())
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
    TEST_RESULTS
    return total - passed;
}