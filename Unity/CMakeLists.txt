add_library(Unity STATIC
    Unity/src/unity.c
)

target_include_directories(Unity PUBLIC
    Unity/src
)

add_compile_definitions(UNITY_INCLUDE_DOUBLE)
