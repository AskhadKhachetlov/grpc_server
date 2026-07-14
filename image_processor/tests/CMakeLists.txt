include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(image_processor_test image_processor_test.cc)

target_link_libraries(image_processor_test
    PRIVATE
        image_processor
        GTest::gtest_main
)

#include(GoogleTest)
#gtest_discover_tests(image_processor_test DISCOVERY_MODE PRE_TEST)
#add_test(NAME image_processor_test COMMAND image_processor_test)