# Включаем поддержку тестирования
enable_testing()

# Находим Google Test
find_package(GTest REQUIRED)

# Создаем исполняемый файл для интеграционных тестов
# Включаем все необходимые исходные файлы приложения
add_executable(integration_tests
    integration_test.cpp
    ${CMAKE_SOURCE_DIR}/src/app.cpp
    ${CMAKE_SOURCE_DIR}/src/dbus_server.cpp
    ${CMAKE_SOURCE_DIR}/src/calculator.cpp
    ${CMAKE_SOURCE_DIR}/src/task.cpp
)

# Подключаем заголовочные файлы
target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/build_linux/_deps/libmath-src/include
)

# Линкуем с необходимыми библиотеками
target_link_libraries(integration_tests
    PRIVATE
        GTest::gtest_main
        math
        SDBusCpp::sdbus-c++
        nlohmann_json::nlohmann_json
        pthread
)

# Автоматически обнаруживаем и регистрируем тесты
include(GoogleTest)
gtest_discover_tests(integration_tests)