# Включаем поддержку тестирования
enable_testing()

# Находим Google Test
find_package(GTest REQUIRED)
find_program(VALGRIND_EXECUTABLE valgrind)

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

if(VALGRIND_EXECUTABLE)
    add_test(NAME integration_tests_memcheck
        COMMAND ${VALGRIND_EXECUTABLE}
            --leak-check=full
            --show-leak-kinds=definite,indirect
            --track-origins=yes
            --error-exitcode=1
            $<TARGET_FILE:integration_tests>
    )
    set_tests_properties(integration_tests_memcheck PROPERTIES
        TIMEOUT 300          # valgrind медленнее в 10-30 раз
        LABELS "memcheck"
    )
endif()