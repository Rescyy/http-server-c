rm -f appc
gcc main.c src/http.c src/http_req.c src/http_resp.c src/http_router.c src/utils.c src/connection.c src/app.c src/strnstr.c -o appc
exit $?