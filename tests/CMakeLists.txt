# Включаем поддержку тестирования
enable_testing()

# Находим Google Test
find_package(GTest REQUIRED)

# Создаем исполняемый файл с тестами
add_executable(libmath_tests
    test_libmath.cpp
)

# Подключаем заголовочные файлы libmath
target_include_directories(libmath_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/build_linux/_deps/libmath-src/include
)

# Линкуем с Google Test и libmath (только gtest_main, без gmock)
target_link_libraries(libmath_tests
    PRIVATE
        GTest::gtest_main
        math
)

# Автоматически обнаруживаем и регистрируем тесты
include(GoogleTest)
gtest_discover_tests(libmath_tests)
