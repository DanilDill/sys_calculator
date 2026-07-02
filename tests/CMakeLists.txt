# Включаем поддержку тестирования
enable_testing()

# Находим Google Test
find_package(GTest REQUIRED)

add_executable(integration_tests
    integration_test.cpp
)

# Подключаем заголовочные файлы
target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Линкуем с необходимыми библиотеками
target_link_libraries(integration_tests
    PRIVATE
        GTest::gtest_main
        pthread
        calculator_logic
)

# Автоматически обнаруживаем и регистрируем тесты
include(GoogleTest)
gtest_discover_tests(integration_tests)