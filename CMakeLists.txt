cmake_minimum_required(VERSION 3.28)
project(httpserverc C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -g")

# ----------------------------
# Library with all your sources
# ----------------------------
add_library(httpserverc_lib
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
        src/file_handler.c
        src/json.c
        src/result.h
        src/errors.c
)

target_include_directories(httpserverc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ----------------------------
# Main executable
# ----------------------------
add_executable(httpserverc main.c)
target_link_libraries(httpserverc PRIVATE httpserverc_lib)

# ----------------------------
# Enable testing and add tests
# ----------------------------
#enable_testing()
#add_subdirectory(tests)
