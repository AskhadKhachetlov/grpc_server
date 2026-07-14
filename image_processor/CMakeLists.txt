add_subdirectory(src)

option(IMAGE_PROCESSOR_BUILD_TESTS "Build image_processor tests" ON)

if(IMAGE_PROCESSOR_BUILD_TESTS)
    #enable_testing()
    add_subdirectory(tests)
endif()