# Находим или скачиваем Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Подключаем заголовочные файлы основного проекта
include_directories(
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/utils
)

# Создаем тестовый исполняемый файл
add_executable(unit_tests
    test_system_monitor.cpp
    test_utils.cpp
    test_main.cpp
    ${CMAKE_SOURCE_DIR}/src/core/system_monitor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/utils.cpp
)

# Линкуем с Google Test
target_link_libraries(unit_tests
    GTest::gtest_main
    pthread
)

# Включаем предупреждения
target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic)

# Регистрируем тесты в CTest
include(GoogleTest)
gtest_discover_tests(unit_tests)