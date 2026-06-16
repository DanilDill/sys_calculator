#include <gtest/gtest.h>
#include <sdbus-c++/sdbus-c++.h>
#include <thread>
#include <chrono>
#include <memory>

#include "../src/app.hpp"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 1;
        char* argv[] = {const_cast<char*>("calculator_test"), nullptr};
        test_app = std::make_unique<app>(argc, argv);
        
        server_thread = std::thread([this]() {
            test_app->run();
        });
        
        // Даем время серверу запуститься
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    void TearDown() override {
        if (test_app) {
            test_app->stop();
        }
        
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    std::unique_ptr<app> test_app;
    std::thread server_thread;
};

TEST_F(IntegrationTest, AdditionOperation) {
    try {
        // Создаем клиентское соединение с session bus
        auto connection = sdbus::createSystemBusConnection();
        
        // Создаем прокси для вызова метода
        auto proxy = sdbus::createProxy(*connection, "com.example.CalculatorService", "/com/example/CalculatorObject");
        
        // Подготавливаем JSON запрос
        std::string json_request = R"({"firstValue": 5, "operation": "+", "secondValue": 3})";
        
        // Вызываем метод Calculate и получаем результат
        std::string result_json;
        proxy->callMethod("Calculate")
              .onInterface("com.example.CalculatorInterface")
              .withArguments(json_request)
              .storeResultsTo(result_json);
        
        // Проверяем, что получили непустой результат
        EXPECT_FALSE(result_json.empty());
        
        // Парсим результат и проверяем конкретные значения
        auto result = nlohmann::json::parse(result_json);
        EXPECT_EQ(result["status"], "success");
        EXPECT_EQ(result["result"], 8);
        
    } catch (const sdbus::Error& e) {
        FAIL() << "D-Bus error: " << e.getName() << " - " << e.getMessage();
    } catch (const nlohmann::json::exception& e) {
        FAIL() << "JSON parse error: " << e.what();
    }
}