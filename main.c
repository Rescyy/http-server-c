#include "src/app.h"

HttpResp indexH(HttpReq);
HttpResp stylesheetH(HttpReq);
HttpResp notFoundH(HttpReq);
HttpResp assetH(HttpReq);

int main(int argc, char **argv)
{
    addEndpoint("/", indexH);
    addEndpoint("/stylesheet", stylesheetH);
    addEndpoint("/assets/<str>", assetH);
    setNotFoundCallback(notFoundH);
    setLogFile("log.txt");

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