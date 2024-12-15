gcc src/unit_tests.c src/http.c src/http_req.c src/http_resp.c src/http_router.c src/utils.c -o unit_tests
./unit_tests
rm unit_tests
