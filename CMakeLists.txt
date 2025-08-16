cmake_minimum_required(VERSION 3.30)
project(untitled C)

set(CMAKE_C_STANDARD 23)

# Add compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -g")

add_executable(untitled
        main.c
        src/test.c
        src/alloc.c
        src/connection.c
        src/http_req.c
        src/http_resp.c
        src/http_router.c
        src/http.c
        src/utils.c
        src/tcp_stream.c
        src/app.c
        src/app_state.c
        src/logging.c
        src/test.h
        src/alloc.h
        src/connection.h
        src/http_req.h
        src/http_resp.h
        src/http_router.h
        src/http.h
        src/utils.h
        src/tcp_stream.h
        src/app.h
        src/errors.h
        src/app_state.h
        src/logging.h
        src/file_handler.c
        src/file_handler.h
)
