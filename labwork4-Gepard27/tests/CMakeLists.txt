enable_testing()

add_executable(
    argparser_tests
    argparser_test.cpp
)

target_link_libraries(
    argparser_tests
    PRIVATE
    argparser
)

target_include_directories(argparser_tests PUBLIC ${PROJECT_SOURCE_DIR})

add_test(NAME ArgParserTests COMMAND argparser_tests)