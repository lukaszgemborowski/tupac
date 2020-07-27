add_executable(
    tests
    main.cxx)

target_link_libraries(
    tests
    PRIVATE
    tupac
)

add_test(
    NAME unittests
    COMMAND tests)
