add_executable(image_processor_test image_processor_test.cc)

target_link_libraries(image_processor_test
    PRIVATE
        image_processor
        GTest::gtest_main
        project_configuration
)

gtest_discover_tests(image_processor_test)