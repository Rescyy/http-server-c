#include "src/app.h"
#include <stdio.h>

// #define TRACK_LEAK
#include <pthread.h>
#include <unistd.h>

#include "src/alloc.h"
#include "logging.h"

HttpResp helloH(HttpReq);
HttpResp indexH(HttpReq);
HttpResp stylesheetH(HttpReq);
HttpResp notFoundH(HttpReq);
HttpResp assetH(HttpReq);
HttpResp jsonFormatterH(HttpReq);
HttpResp crudH(HttpReq);

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
    addEndpoint("/crud", crudH);
    setLogFile("logs.txt");
    setJsonLogFile("logs.json");
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

HttpResp helloH(HttpReq) {
    HttpRespBuilder builder = newRespBuilder();
    return respBuild(&builder);
}

HttpResp indexH(HttpReq)
{
    printf("Index entered\n");
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/index.html");
    respBuilderAddHeader(&builder, "Content-Type", "text/html");

    return respBuild(&builder);
}

HttpResp stylesheetH(HttpReq)
{
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/stylesheet.css");
    respBuilderAddHeader(&builder, "Content-Type", "text/css");

    return respBuild(&builder);
}

HttpResp notFoundH(HttpReq)
{
    HttpRespBuilder builder = newRespBuilder();

    respBuilderSetFileContent(&builder, "resources/404.html");
    respBuilderAddHeader(&builder, "Content-Type", "text/html");

    return respBuild(&builder);
}

HttpResp assetH(const HttpReq request)
{
    HttpRespBuilder builder = newRespBuilder();

    char assetPath[128];
    sprintf(assetPath, "assets/%s", request.path.elements[1]);

    respBuilderSetFileContent(&builder, assetPath);

    return respBuild(&builder);
}

HttpResp jsonFormatterH(const HttpReq request) {
    HttpRespBuilder b = newRespBuilder();

    RESULT_T(JToken) token = deserializeJson(request.content, request.contentLength);

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

static pthread_mutex_t crud_mutex = PTHREAD_MUTEX_INITIALIZER;
HttpResp crudH(HttpReq request) {
    HttpRespBuilder b = newRespBuilder();
    pthread_mutex_lock(&crud_mutex);
    const char *path = "data.json";
    FILE *fp = NULL;
    char *buf = NULL;
    RESULT_T(JToken) token = {.ok = 0};
    RESULT_T(JToken) reqToken = {.ok = 0};

    fp = fopen(path, "a+");
    if (!fp) {
        respBuilderSetStatus(&b, INTERNAL_SERVER_ERROR);
        goto _return;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        respBuilderSetStatus(&b, INTERNAL_SERVER_ERROR);
        goto _return;
    }

    long size = ftell(fp);
    if (size < 0) {
        respBuilderSetStatus(&b, INTERNAL_SERVER_ERROR);
        goto _return;
    }
    rewind(fp);

    buf = allocate(size);
    fread(buf, 1, size, fp);
    token = deserializeJson(buf, size);
    if (!token.ok || token.var.type != JSON_LIST) {
        static const char emptyList[] = "[]";
        token.ok = 1;
        token.var = _JToken(_JListEmpty());
        strcpy(buf, emptyList);
        ftruncate(fileno(fp), 0);
        rewind(fp);
        fwrite(buf, 1, strlen(emptyList), fp);
    }
    deallocate(buf);

    switch (request.method) {
        case GET: {
            size_t length = serializeJson(token.var, &buf, 4);
            respBuilderSetContent(&b, buf, length);
            break;
        }
        case POST: {
            reqToken = deserializeJson(request.content, request.contentLength);
            if (!reqToken.ok) {
                respBuilderSetStatus(&b, BAD_REQUEST);
                goto _return;
            }
            JList *list = &token.var.literal.list;
            token.var.literal.list.tokens = list->tokens = reallocate(list->tokens, (list->count + 1) * sizeof(JToken));
            list->tokens[list->count++] = reqToken.var;
            size = serializeJson(token.var, &buf, 0);
            rewind(fp);
            ftruncate(fileno(fp), 0);
            rewind(fp);
            fwrite(buf, 1, size, fp);
            respBuilderSetStatus(&b, CREATED);
            break;
        }
        default:
            respBuilderSetStatus(&b, METHOD_NOT_ALLOWED);
            break;
    }

_return:
    if (token.ok) freeJson(&token.var);
    if (reqToken.ok) freeJson(&reqToken.var);
    deallocate(buf);
    if (fp) fclose(fp);
    pthread_mutex_unlock(&crud_mutex);
    return respBuild(&b);
}