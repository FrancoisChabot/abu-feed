add_executable(abu_feed_tests
    test_range_adaptor.cpp
    test_stream_feed.cpp
 )

target_include_directories(abu_feed_tests PRIVATE .)
target_link_libraries(abu_feed_tests PRIVATE gtest_main abu::feed)


if (MSVC)
    target_compile_options(abu_feed_tests PRIVATE /W4 /WX)
else()
    target_compile_options(abu_feed_tests PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_test(abu_feed_tests abu_feed_tests)