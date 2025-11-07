cmake_minimum_required(VERSION 3.28)
project(httpserverc C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -Wall -Wextra -pedantic -g")

# ----------------------------
# Library with all your sources
# ----------------------------
add_library(httpserverc_lib
        src/alloc/alloc.c
        src/connection.c
        src/http/http_req.c
        src/http/http_resp.c
        src/http/http_router.c
        src/http/http.c
        src/utils.c
        src/tcp_stream.c
        src/app.c
        src/app_state.c
        src/logging.c
        src/file_handler.c
        src/json.c
        src/errors.c
        src/alloc/garbage_collector.c
        src/alloc/alloc_entries.c
        src/alloc/arena.c
        src/alloc/destructors.c
        src/helpers/signal_helper.c
        src/iouring/async_producer.c
        src/iouring/async_producer.h
        src/iouring/async_consumer.c
        src/iouring/async_consumer.h
        src/iouring/async_engine.c
        src/iouring/async_engine.h
        src/http/http_parser.c
        src/http/http_parser.h
)

# Include paths for the library
target_include_directories(httpserverc_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/includes  # headers moved here
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ----------------------------
# Main executable
# ----------------------------
add_executable(httpserverc main.c)

target_link_libraries(httpserverc PRIVATE httpserverc_lib)

# Include headers for main target too (CLion parses this)
target_include_directories(httpserverc
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ----------------------------
# Enable testing and add tests
# ----------------------------
enable_testing()
add_subdirectory(tests)
