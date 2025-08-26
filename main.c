#include "src/app.h"
#include <stdio.h>

// #define TRACK_LEAK
#include "src/alloc.h"
#include "logging.h"

HttpResp indexH(HttpReq);
HttpResp stylesheetH(HttpReq);
HttpResp notFoundH(HttpReq);
HttpResp assetH(HttpReq);

int main(int argc, char **argv)
{
#ifdef TRACK_LEAK
    printf("Tracking leaks\n");
    init_alloc();
#endif

    addEndpoint("/", indexH);
    addEndpoint("/stylesheet", stylesheetH);
    addEndpoint("/assets/<str>", assetH);
    setLogFile("hello.txt");
    setLogFlags(JSON_LOG);
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