cmake_minimum_required(VERSION 3.12)
project(my_executable)

# Установка версии C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Настройка выходного пути для исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../tests/test_c/)
set(EXECUTABLE_NAME RUN_TESTS)

# Поиск Python
find_package(Python3 REQUIRED COMPONENTS Development)

# Поиск исходных файлов
file(GLOB SOURCE_FILES
    "pysatl_tsp/c/*.c"
    "tests/test_c/*.c"
)

file(GLOB HEADER_FILES
    "pysatl_tsp/c/*.h"
)

# Добавление библиотеки для тестирования
add_subdirectory(Unity)

# Создание исполняемого файла вместо библиотеки
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Включение директорий с заголовками
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${Python3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/pysatl_tsp/c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_c
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    UNITY_INCLUDE_DOUBLE
    UNITY_INCLUDE_FLOAT
)
# Связывание с библиотеками
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${Python3_LIBRARIES} Unity)

# Компиляционные флаги
target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer)

target_link_options(${EXECUTABLE_NAME} PRIVATE
    -fsanitize=address
)
