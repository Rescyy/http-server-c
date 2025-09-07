#include "src/app.h"
#include <stdio.h>

// #define TRACK_LEAK
#include "src/alloc.h"
#include "logging.h"

HttpResp helloH(HttpReq);
HttpResp indexH(HttpReq);
HttpResp stylesheetH(HttpReq);
HttpResp notFoundH(HttpReq);
HttpResp assetH(HttpReq);
HttpResp jsonFormatterH(HttpReq);

int main(int argc, char **argv)
{
#ifdef TRACK_LEAK
    printf("Tracking leaks\n");
    init_alloc();
#endif
    respBuilderSetDefaultFlags(0);
    addEndpoint("/hello", helloH);
    addEndpoint("/", indexH);
    addEndpoint("/stylesheet", stylesheetH);
    addEndpoint("/assets/<str>", assetH);
    addEndpoint("/jsonFormatter", jsonFormatterH);
    setLogFile("hello.txt");
    setNotFoundCallback(notFoundH);

    if (argc == 2)
    {
        startApp(argv[1]);
    }
    else
    {
        startApp("8080");
    }
}

HttpResp helloH(HttpReq request) {
    HttpRespBuilder builder = newRespBuilder();
    return respBuild(&builder);
}

HttpResp indexH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/index.html");
    respBuilderAddHeader(&builder, "Content-Type", "text/html");

    return respBuild(&builder);
}

HttpResp stylesheetH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/stylesheet.css");
    respBuilderAddHeader(&builder, "Content-Type", "text/css");

    return respBuild(&builder);
}

HttpResp notFoundH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/404.html");
    respBuilderAddHeader(&builder, "Content-Type", "text/html");

    return respBuild(&builder);
}

HttpResp assetH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    char assetPath[128];
    sprintf(assetPath, "assets/%s", request.path.elements[1]);

    respBuilderSetFileContent(&builder, assetPath);

    return respBuild(&builder);
}

HttpResp jsonFormatterH(HttpReq request) {
    HttpRespBuilder b = newRespBuilder();

    RESULT_T(JToken) token = deserializeJson(request.content, request.contentLength);

    printf("Content: (%d) %.*s\n", request.contentLength, request.contentLength, (char*) request.content);

    if (!token.ok) {
        respBuilderSetStatus(&b, BAD_REQUEST);
    } else {
        char *formatted;
        const size_t formattedSize = serializeJson(token.var, &formatted, 4);
        respBuilderSetContent(&b, formatted, formattedSize);
        freeJson(&token.var);
        deallocate(formatted);
    }

    return respBuild(&b);
}