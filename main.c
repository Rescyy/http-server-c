#include "src/app.h"

void finalBuild(HttpRespBuilder *builder)
{
    // builder->addHeader(&builder, "Cache-Control", "max-age=60");
}

HttpResp indexH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    builder.setFileContent(&builder, "index.html")
        ->addHeader(&builder, "Content-Type", "text/html");
    finalBuild(&builder);

    return builder.build(&builder);
}

HttpResp stylesheetH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    builder.setFileContent(&builder, "stylesheet.css")
        ->addHeader(&builder, "Content-Type", "text/css");
    finalBuild(&builder);

    return builder.build(&builder);
}

HttpResp scriptH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    builder.setFileContent(&builder, "script.js")
        ->addHeader(&builder, "Content-Type", "text/javascript");
    finalBuild(&builder);

    return builder.build(&builder);
}

HttpResp notFoundH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    builder.setFileContent(&builder, "404.html")
        ->setStatus(&builder, NOT_FOUND)
        ->addHeader(&builder, "Content-Type", "text/html");
    finalBuild(&builder);

    return builder.build(&builder);
}

HttpResp assetH(HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    char assetPath[128];
    sprintf(assetPath, "assets/%s", request.path.elements[1]);

    builder.setFileContent(&builder, assetPath)
        ->addHeader(&builder, "Content-Type", "image/png");
    finalBuild(&builder);

    return builder.build(&builder);
}

int main(int argc, char **argv)
{
    addEndpoint("/", indexH);
    addEndpoint("/stylesheet", stylesheetH);
    addEndpoint("/assets/<str>", assetH);
    setNotFoundCallback(notFoundH);
    if (argc == 2)
    {
        startApp(argv[1]);
    }
    else
    {
        startApp("80");
    }
}
