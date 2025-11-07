Simple HTTP Server library written in C

```
git pull https://github.com/Rescyy/http-server-c.git
cd http-server-c
```

Example main.c
```
#include "includes/app.h" // the main include used for this project
#include <stdio.h>

HttpResp indexH(HttpReq request) { // HttpReq struct with all the information of the request
    HttpRespBuilder builder = newRespBuilder(); // builder pattern for responses

    const int msgSize = 100;
    char *msg = gcArenaAllocate(msgSize, alignof(char)); // garbage collector for arena that cleans up after the response is sent
    int size = snprintf(msg, msgSize, "Hello World! %s", methodToStr(request.method));

    respBuilderSetContent(&builder, msg, size, 0); // setting content, 0 means no copy
    respBuilderAddHeader(&builder, "Content-Type", "text/plain"); // setting headers

    return respBuild(&builder); // building the response struct, after which it's handled, built and sent as a stream of bytes
}

int main() {
	initApp(); // always initialize app
    addEndpoint("/", indexH); // adding endpoint of type function pointer of HttpResp endpointH(HttpReq)
    startApp("8080"); // start application listening on port 8080
    // the program will not go further
}
```

Build:
`docker builder -t httpserverc .`

Run: 
`docker run -it -p 8080:8080 httpserverc`

Output
```[DEBUG|2025-11-07 09:54:20.425|MAIN] Initialising Session State Factory
[DEBUG|2025-11-07 09:54:20.425|MAIN] Initialising Garbage Collector
[INFO|2025-11-07 09:54:20.425|MAIN] Adding Endpoint /
[INFO|2025-11-07 09:54:20.425|MAIN] Listening to port 8080
[INFO|2025-11-07 09:54:37.331|MAIN] Connection accepted from client 172.17.0.1 with index 2
[DEBUG|2025-11-07 09:54:37.332|1.1] Parsing Method
[DEBUG|2025-11-07 09:54:37.332|1.1] poll({.fd = 4, .events = POLLIN}, 1, 60000) returned 1 and set .revents to 0001
[DEBUG|2025-11-07 09:54:37.332|1.1] recv(4, 0x51900000ff80, 1024, 0) returned 727
[DEBUG|2025-11-07 09:54:37.332|1.1] Parsing Path
[DEBUG|2025-11-07 09:54:37.332|1.1] Parsing Version
[DEBUG|2025-11-07 09:54:37.332|1.1] Parsing Headers
[DEBUG|2025-11-07 09:54:37.333|1.1] Parsing Content
[DEBUG|2025-11-07 09:54:37.333|1.1] Connection keep alive
[DEBUG|2025-11-07 09:54:37.333|1.1] Routing request
Request 1.1 Thread 140291253204672 172.17.0.1       GET     /                                                  | 200 OK
[DEBUG|2025-11-07 09:54:37.333|1.1] poll({.fd = 4, .events = POLLOUT}, 1, 10000) returned 1 and set .revents to 0004
[DEBUG|2025-11-07 09:54:37.333|1.1] send(4, 0x519000010480, 88, 0) return 88
[DEBUG|2025-11-07 09:54:37.333|1.1] poll({.fd = 4, .events = POLLOUT}, 1, 10000) returned 1 and set .revents to 0004
[DEBUG|2025-11-07 09:54:37.333|1.1] send(4, 0x5210000017b2, 16, 0) return 16
[DEBUG|2025-11-07 09:54:37.333|1.1] Cleaning up allocations
[DEBUG|2025-11-07 09:54:37.333|1.1] Cleaning up Entries 6 to deallocate 3
[DEBUG|2025-11-07 09:54:37.333|1.1] Cleaning up Arena 1 Chunks
[DEBUG|2025-11-07 09:54:37.333|1.2] Parsing Method
[DEBUG|2025-11-07 09:55:37.390|1.2] poll({.fd = 4, .events = POLLIN}, 1, 60000) returned 0 and set .revents to 0000
[INFO|2025-11-07 09:55:37.390|1.2] Timeout Waiting For Client. Closing Connection.
[INFO|2025-11-07 09:55:37.394|MAIN] Closing connection 1 with 172.17.0.1
[DEBUG|2025-11-07 09:55:37.394|MAIN] Invoking destructors 3
[DEBUG|2025-11-07 09:55:37.394|MAIN] Freeing tcp stream
[DEBUG|2025-11-07 09:55:37.394|MAIN] Cleaning up Arena 1 Chunks
[DEBUG|2025-11-07 09:55:37.394|MAIN] Cleaning up Entries 0 to deallocate 0
```