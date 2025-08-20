# Tests CMakeLists

# Small helper function to avoid repetition
function(add_unit_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE untitled_lib)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Example test
add_unit_test(json_test json_test.c)
