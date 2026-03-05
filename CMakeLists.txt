cmake_minimum_required(VERSION 3.16)
project(SystemMonitor VERSION 1.0.0 LANGUAGES CXX)

enable_testing()

# Стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Пути для include
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Установка путей для выходных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Проверка ОС
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(NOT LINUX)
    message(FATAL_ERROR "This application is designed for Linux systems only")
endif()

# Добавляем исходники
add_executable(system_monitor
    src/main.cpp
    src/core/system_monitor.cpp
    src/utils/utils.cpp
)

# Подключаем заголовочные файлы
target_include_directories(system_monitor PRIVATE
    src/core
    src/utils
)

# Компиляция с предупреждениями
if(MSVC)
    target_compile_options(system_monitor PRIVATE /W4)
else()
    target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Сборка тестов
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()