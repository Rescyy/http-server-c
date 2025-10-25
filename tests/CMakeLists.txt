# Small helper function to avoid repetition
function(add_unit_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE httpserverc_lib)

    # Add include directories for tests
    target_include_directories(${name}
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/../includes
    )

    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Example test
add_unit_test(json_test json_test.c)
