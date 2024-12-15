gcc unit_tests.c http_resp.c http.c http_req.c utils.c -o unit_tests -g
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./unit_tests
rm unit_tests