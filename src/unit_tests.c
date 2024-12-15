#include "http_req.h"
#include "http_resp.h"
#include "http_router.h"
#include "http.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define EXPECT(x)                              \
    if (!(x))                                  \
    {                                          \
        printf("LINE: %d %s\n", __LINE__, #x); \
        testResult = 0;                        \
    }

int parsePathTest()
{
    int testResult = 1;

    char pathStr[50] = "/path/to/str";
    int pathLen = strlen(pathStr);
    strcat(pathStr, " HTTP/1.1\r\n");
    HttpPath path;

    EXPECT(parsePath(&path, pathStr, pathLen) != -1);
    EXPECT(path.elCount == 3);
    EXPECT(strcmp(path.elements[0], "path") == 0);
    EXPECT(strcmp(path.elements[1], "to") == 0);
    EXPECT(strcmp(path.elements[2], "str") == 0);

    freePath(&path);
    return testResult;
}

int parsePathTest2()
{
    int testResult = 1;

    char pathStr[] = "/";
    HttpPath path;
    EXPECT(parsePath(&path, pathStr, 1) != -1);
    EXPECT(path.elCount == 0);
    freePath(&path);

    return testResult;
}

int pathEqTest()
{
    int testResult = 1;

    char pathStr[50] = "/path/to/str/";
    HttpPath path1, path2;
    char *pathElements[] = {"path", "to", "str"};
    path2.elements = pathElements;
    path2.elCount = 3;
    EXPECT(parsePath(&path1, pathStr, strlen(pathStr)) != -1);
    EXPECT(pathEq(path1, path2) == 1);
    freePath(&path1);

    return testResult;
}

int parseHeadersTest()
{
    int testResult = 1;

    char headerStr[] =
        "Content-Length: 123\r\n"
        "Accept: application/json\r\n"
        "Cock: and-ball\r\n"
        "\r\n"
        "Shouldn't be parsed";
    HttpHeaders headers;
    EXPECT(parseHeaders(&headers, headerStr, strlen(headerStr)) != -1);
    EXPECT(headers.count == 3);
    EXPECT(strcmp(headers.arr[0].key, "Content-Length") == 0);
    EXPECT(strcmp(headers.arr[1].key, "Accept") == 0);
    EXPECT(strcmp(headers.arr[2].key, "Cock") == 0);
    EXPECT(strcmp(headers.arr[0].value, "123") == 0);
    EXPECT(strcmp(headers.arr[1].value, "application/json") == 0);
    EXPECT(strcmp(headers.arr[2].value, "and-ball") == 0);
    freeHeaders(&headers);

    return testResult;
}

int parseHeadersTest2()
{
    int testResult = 1;

    char headerStr[] =
        "Content-Length: 123\r\n"
        "Accept:  application/json\r\n"
        "Cock: and-ball\r\n"
        "\r\n"
        "Shouldn't be parsed";
    HttpHeaders headers;
    EXPECT(parseHeaders(&headers, headerStr, strlen(headerStr)) == -1);

    return testResult;
}

int parseHeadersTest3()
{
    int testResult = 1;

    char headerStr[] =
        "Content-Length: 123\r\n"
        "Accept: application/\njson\r\n"
        "Cock: and-ball\r\n"
        "\r\n"
        "Shouldn't be parsed";
    HttpHeaders headers;
    EXPECT(parseHeaders(&headers, headerStr, strlen(headerStr)) == -1);

    return testResult;
}

int parseHeadersTest4()
{
    int testResult = 1;

    char headerStr[] =
        "Content-Length: 123\r\n"
        "Acce\rpt: application/json\r\n"
        "Cock: and-ball\r\n"
        "\r\n"
        "Shouldn't be parsed";
    HttpHeaders headers;
    EXPECT(parseHeaders(&headers, headerStr, strlen(headerStr)) == -1);

    return testResult;
}

int findHeaderTest()
{
    int testResult = 1;

    HttpHeaders headers = {
        (HttpHeader[]){
            (HttpHeader){"Content-Length", "5"},
            (HttpHeader){"Content-Type", "text/plain"},
        },
        2};
    HttpHeader *header = findHeader(headers, "content-length");
    EXPECT(headerEq(*header, (HttpHeader){"Content-Length", "5"}));

    return testResult;
}

int parseRequestTest()
{
    int testResult = 1;

    char requestStr[] =
        "GET /test HTTP/1.1\r\n"
        "Accept: application/json\r\n"
        "\r\n";
    HttpReq req1, req2;
    EXPECT(parseRequest(&req1, requestStr, strlen(requestStr)) != -1);
    req2.method = GET;
    req2.path = (HttpPath){(char *[]){"test"}, 1};
    req2.version = "HTTP/1.1";
    req2.headers = (HttpHeaders){
        (HttpHeader[]){
            (HttpHeader){"Accept", "application/json"}},
        1};
    req2.content = NULL;
    req2.contentWritten = 0;
    req2.contentLength = 0;
    EXPECT(reqEq(req1, req2) == 1);
    freeReq(&req1);

    return testResult;
}

int parseRequestTest2()
{
    int testResult = 1;

    char requestStr[] =
        "GET /test HTTP/1.1\r\n"
        "Accept: application/json\r\n"
        "Content-Length: 100\r\n"
        "\r\n";
    HttpReq req1, req2;
    EXPECT(parseRequest(&req1, requestStr, strlen(requestStr)) != -1);
    req2.method = GET;
    req2.path = (HttpPath){(char *[]){"test"}, 1};
    req2.version = "HTTP/1.1";
    req2.headers = (HttpHeaders){
        (HttpHeader[]){
            (HttpHeader){"Accept", "application/json"},
            (HttpHeader){"Content-Length", "100"},
        },
        2};
    req2.content = NULL;
    req2.contentWritten = 0;
    req2.contentLength = 100;
    EXPECT(reqIsComplete(req1) == 0);
    EXPECT(reqIsComplete(req2) == 0);
    freeReq(&req1);

    return testResult;
}

int parseRequestTest3()
{
    int testResult = 1;

    char requestStr[] =
        "GET /test HTTP/1.1\r\n"
        "Accept: application/json\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    HttpReq req1, req2;
    EXPECT(parseRequest(&req1, requestStr, strlen(requestStr)) != -1);
    req2.method = GET;
    req2.path = (HttpPath){(char *[]){"test"}, 1};
    req2.version = "HTTP/1.1";
    req2.headers = (HttpHeaders){
        (HttpHeader[]){
            (HttpHeader){"Accept", "application/json"},
            (HttpHeader){"Content-Length", "5"},
        },
        2};
    req2.content = "Hello";
    req2.contentWritten = 5;
    req2.contentLength = 5;
    EXPECT(reqEq(req1, req2) == 1);
    freeReq(&req1);

    return testResult;
}

int parseRequestTest4()
{
    int testResult = 1;

    char requestStr[] =
        "GET /test  HTTP/1.1\r\n"
        "Accept: application/json\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    HttpReq req1, req2;
    EXPECT(parseRequest(&req1, requestStr, strlen(requestStr)) == -1);

    return testResult;
}

int parseRequestTest5()
{
    int testResult = 1;

    char requestStr[] =
        "GET /test HTTP/1.1 \r\n"
        "Accept: application/json\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";
    HttpReq req1, req2;
    EXPECT(parseRequest(&req1, requestStr, strlen(requestStr)) == -1);

    return testResult;
}

int respToStrTest()
{
    int testResult = 1;

    char responseStr[100];
    HttpResp resp = {
        "HTTP/1.1",
        OK,
        (HttpHeaders){
            (HttpHeader[]){
                (HttpHeader){"Content-Length", "5"},
            },
            1},
        "Hello",
        5};
    static const char expectedStr[] = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
    EXPECT(respToStr(resp, responseStr, 100) == strlen(expectedStr));
    EXPECT(memcmp(responseStr, expectedStr, strlen(expectedStr)) == 0);

    return testResult;
}

int respEqTest()
{
    int testResult = 1;

    HttpResp resp1 = {
        "HTTP/1.1",
        OK,
        (HttpHeaders){
            (HttpHeader[]){
                (HttpHeader){"Content-Type", "text/plain"},
                (HttpHeader){"Content-Length", "5"},
            },
            2},
        "Hello",
        5};
    HttpResp resp2 = {
        "HTTP/1.1",
        OK,
        (HttpHeaders){
            (HttpHeader[]){
                (HttpHeader){"content-length", "5"},
                (HttpHeader){"CONTENT-TYPE", "text/plain"},
            },
            2},
        "Hello",
        5};
    EXPECT(respEq(resp1, resp2) == 1);

    return testResult;
}

int respBuilderTest()
{
    int testResult = 1;

    HttpResp resp = {
        "HTTP/1.1",
        OK,
        (HttpHeaders){
            (HttpHeader[]){
                (HttpHeader){"Content-Length", "5"},
                (HttpHeader){"Content-Type", "text/plain"},
                (HttpHeader){"Server", "http-server-c"},
            },
            3},
        "Hello",
        5};
    HttpRespBuilder builder = newRespBuilder();
    builder.setVersion(&builder, "HTTP/1.1")
        ->setStatus(&builder, OK)
        ->setContent(&builder, "Hello", 5);
    HttpResp builtResp = builder.build(&builder);
    EXPECT(respEq(resp, builtResp) == 1);
    freeResp(&builtResp);

    return testResult;
}

int routerNotFoundRespTest()
{
    int testResult = 1;

    HttpRouter router = emptyRouter();
    HttpReq req;
    HttpResp resp = routeReq(router, req);
    HttpResp notFoundResp = {
        "HTTP/1.1",
        NOT_FOUND,
        (HttpHeaders){(HttpHeader[]){
                          (HttpHeader){"Server", "http-server-c"}},
                      1},
        NULL,
        0};
    printf("%d\n", resp.headers.count);
    EXPECT(respEq(resp, notFoundResp));
    freeResp(&resp);

    return testResult;
}

int newEndpointTest()
{
    int testResult = 1;

    HttpEndpoint endpoint = newEndpoint("/path/to/resource", NULL);
    HttpPath path = {
        .elements = (char *[]){"path", "to", "resource"},
        .elCount = 3};
    EXPECT(pathEq(endpoint.path, path));
    freeEndpoint(&endpoint);

    return testResult;
}

int freeRouterTest()
{
    int testResult = 1;

    HttpEndpoint endpoints[] = {
        newEndpoint("/", NULL),
        newEndpoint("/path", NULL),
    };
    HttpRouter router = newRouter(
        endpoints,
        sizeof(endpoints) / sizeof(endpoints[0]));
    freeRouter(&router);

    return testResult;
}

int setFileContentTest()
{
    int testResult = 1;

    fopen("test.txt", "w");
    fwrite("Hello", 1, 5, fopen("test.txt", "w"));
    fclose(fopen("test.txt", "w"));

    HttpRespBuilder builder = newRespBuilder();
    builder.setFileContent(&builder, "test.txt");
    HttpResp resp = builder.build(&builder);
    HttpResp expectedResp = {
        "HTTP/1.1",
        OK,
        (HttpHeaders){
            (HttpHeader[]){
                (HttpHeader){"Content-Length", "5"},
                (HttpHeader){"Content-Type", "text/plain"},
                (HttpHeader){"Server", "http-server-c"},
            },
            3},
        "Hello",
        5};
    freeResp(&resp);
    remove("test.txt");
    return testResult;
}

int pathMatchesTest()
{
    int testResult = 1;

    char pathStr1[] = "/path/to/resource/1";
    char pathStr2[] = "/path/<str>/resource/<int>";

    HttpPath path1, path2;
    HttpPath expectPath1 = (HttpPath) {
        .elements = (char *[]){"path", "to", "resource", "1"},
        .elCount = 4
    };
    HttpPath expectPath2 = (HttpPath) {
        .elements = (char *[]){"path", "<str>", "resource", "<int>"},
        .elCount = 4
    };
    
    parsePath(&path1, pathStr1, sizeof(pathStr1)-1);
    parsePath(&path2, pathStr2, sizeof(pathStr2)-1);
    EXPECT(pathEq(path1, expectPath1) == 1);
    EXPECT(pathEq(path2, expectPath2) == 1);
    EXPECT(pathMatches(path2, path1) == 1);
    freePath(&path1);
    freePath(&path2);

    return testResult;
}

#define INIT_UNIT_TEST                \
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

#define VALGRIND_DEBUG 1

int main()
{
#if VALGRIND_DEBUG
    INIT_UNIT_TEST;

    UNIT_TEST(parsePathTest);
    UNIT_TEST(parsePathTest2);
    UNIT_TEST(pathEqTest);
    UNIT_TEST(parseHeadersTest);
    UNIT_TEST(parseHeadersTest2);
    UNIT_TEST(parseHeadersTest3);
    UNIT_TEST(parseHeadersTest4);
    UNIT_TEST(findHeaderTest);
    UNIT_TEST(parseRequestTest);
    UNIT_TEST(parseRequestTest2);
    UNIT_TEST(parseRequestTest3);
    UNIT_TEST(parseRequestTest4);
    UNIT_TEST(parseRequestTest5);
    UNIT_TEST(respToStrTest);
    UNIT_TEST(respEqTest);
    UNIT_TEST(respBuilderTest);
    UNIT_TEST(routerNotFoundRespTest);
    UNIT_TEST(newEndpointTest);
    UNIT_TEST(freeRouterTest);
    UNIT_TEST(setFileContentTest);
    UNIT_TEST(pathMatchesTest);

    TEST_RESULTS;
#endif
    return 0;
}