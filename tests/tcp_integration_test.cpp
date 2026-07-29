#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include <nlohmann/json.hpp>

#include "calculator.hpp"
#include "task.hpp"
#include "tcp_client.hpp"
#include "tcp_server.hpp"

// Тесты используют Calculator::execute напрямую (как integration_test.cpp),
// чтобы не зависеть от живых PostgreSQL/Redis.
class TCPIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_handler = [](const std::string& input) -> std::string
        {
            try
            {
                auto task =
                    nlohmann::json::parse(input).get<calculator::Task>();
                calculator::Calculator::execute(task);
                nlohmann::json responce = task;
                return responce.dump();
            }
            catch (const nlohmann::json::exception& e)
            {
                nlohmann::json err;
                err["status"] = "error";
                err["message"] = e.what();
                return err.dump();
            }
        };
    }

    void start_server(int port)
    {
        m_server = std::make_unique<calculator::TCPServer>("127.0.0.1", port,
                                                           m_handler);
        m_server->async_run();

        // Busy-wait: ждём готовности сервера принимать соединения
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start <
               std::chrono::seconds(3))
        {
            try
            {
                calculator::TCPClient probe("127.0.0.1", port);
                probe.connect();
                probe.disconnect();
                return;
            }
            catch (...)
            {
                std::this_thread::yield();
            }
        }
        FAIL() << "Server failed to start within timeout";
    }

    void TearDown() override
    {
        if (m_server)
        {
            m_server->stop();
        }
    }

    std::function<std::string(const std::string&)> m_handler;
    std::unique_ptr<calculator::TCPServer> m_server;
};

TEST_F(TCPIntegrationTest, BasicAddition)
{
    start_server(12345);
    calculator::TCPClient client("127.0.0.1", 12345);
    client.connect();

    std::string request =
        R"({"firstValue": 5, "operation": "+", "secondValue": 3})";
    auto json_resp = nlohmann::json::parse(client.send_request(request));

    EXPECT_EQ(json_resp["result"], 8);
    EXPECT_EQ(json_resp["status"], "success");
    client.disconnect();
}

TEST_F(TCPIntegrationTest, DivisionByZero)
{
    start_server(12346);
    calculator::TCPClient client("127.0.0.1", 12346);
    client.connect();

    std::string request =
        R"({"firstValue": 10, "operation": "/", "secondValue": 0})";
    auto json_resp = nlohmann::json::parse(client.send_request(request));

    EXPECT_EQ(json_resp["status"], "Error! Division by zero!");
    EXPECT_FALSE(json_resp.contains("result"));
    client.disconnect();
}

TEST_F(TCPIntegrationTest, UnknownOperation)
{
    start_server(12347);
    calculator::TCPClient client("127.0.0.1", 12347);
    client.connect();

    // '%' не поддерживается -> Calculator ставит INCORRECT_ARGUMENTS (-3)
    std::string request =
        R"({"firstValue": 5, "operation": "%", "secondValue": 3})";
    auto json_resp = nlohmann::json::parse(client.send_request(request));

    EXPECT_EQ(json_resp["status"], "Error! Incorrect arguments!");
    client.disconnect();
}

TEST_F(TCPIntegrationTest, InvalidJSON)
{
    start_server(12348);
    calculator::TCPClient client("127.0.0.1", 12348);
    client.connect();

    std::string request = "{invalid json}";
    auto json_resp = nlohmann::json::parse(client.send_request(request));

    EXPECT_EQ(json_resp["status"], "error");
    EXPECT_TRUE(json_resp.contains("message"));
    client.disconnect();
}

TEST_F(TCPIntegrationTest, MultipleRequestsInSession)
{
    start_server(12351);
    calculator::TCPClient client("127.0.0.1", 12351);
    client.connect();

    for (int i = 0; i < 5; ++i)
    {
        nlohmann::json req = {{"firstValue", i},
                              {"operation", "+"},
                              {"secondValue", i + 1}};
        auto json_resp = nlohmann::json::parse(client.send_request(req.dump()));
        EXPECT_EQ(json_resp["result"], 2 * i + 1);
        EXPECT_EQ(json_resp["status"], "success");
    }
    client.disconnect();
}

TEST_F(TCPIntegrationTest, MultipleClientsSequential)
{
    start_server(12352);

    for (int c = 0; c < 3; ++c)
    {
        calculator::TCPClient client("127.0.0.1", 12352);
        client.connect();
        std::string request =
            R"({"firstValue": 6, "operation": "*", "secondValue": 7})";
        auto json_resp = nlohmann::json::parse(client.send_request(request));
        EXPECT_EQ(json_resp["result"], 42);
        client.disconnect();
    }
}
