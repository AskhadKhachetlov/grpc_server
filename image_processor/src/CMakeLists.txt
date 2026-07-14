add_library(image_processor image_processor.cc)

target_include_directories(image_processor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(image_processor
    PUBLIC
        project_configuration
        ${OpenCV_LIBS}
)