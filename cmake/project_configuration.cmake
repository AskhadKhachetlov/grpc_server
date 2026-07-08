set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

add_library(ProjectConfiguration INTERFACE)

target_compile_options(ProjectConfiguration
    INTERFACE
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4;/WX;/wd4702>"
        "$<$<NOT:$<COMPILE_LANG_AND_ID:CXX,MSVC>>:-Wall;-Wextra;-Wpedantic;-Werror>"
)

target_compile_features(ProjectConfiguration
        INTERFACE
            cxx_std_20
)