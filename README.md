Simple HTTP Server library written in C

How to use?
```
#include "src/app.h"
#include <stdio.h>

HttpResp indexH(HttpReq request) {
    HttpRespBuilder builder = newRespBuilder();

    char msg[100];
    int size = snprintf(msg, 100, "Hello World! %s", methodToStr(request.method));

    respBuilderSetContent(&builder, msg, size);
    respBuilderAddHeader(&builder, "Content-Type", "text/html");
    respBuilderSetStatus(&builder, 200);

    return respBuild(&builder);
}

int main() {
    addEndpoint("/", indexH);

    startApp("8080");
}
```