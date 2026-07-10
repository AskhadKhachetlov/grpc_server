set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

add_library(project_configuration INTERFACE)

target_compile_options(project_configuration
    INTERFACE
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4;/WX;/wd4702>"
        "$<$<NOT:$<COMPILE_LANG_AND_ID:CXX,MSVC>>:-Wall;-Wextra;-Wpedantic;-Werror>"
)

target_compile_features(project_configuration
        INTERFACE
            cxx_std_20
)