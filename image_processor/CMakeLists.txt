add_library(image_processor src/image_processor.cc)

target_include_directories(image_processor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(image_processor
    PUBLIC
        project_configuration
        ${OpenCV_LIBS}
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()