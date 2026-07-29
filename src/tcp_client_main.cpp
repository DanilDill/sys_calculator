#include "logger.hpp"
#include "tcp_client.hpp"

#include <getopt.h>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
void print_usage(const char* program)
{
    std::cout << "Usage: " << program
              << " --host HOST --port PORT [--test] [--debug]\n"
              << "Options:\n"
              << "  --host    Server hostname or IP\n"
              << "  --port    Server port\n"
              << "  --test    Run automated test (10 requests)\n"
              << "  --debug   Enable debug logging\n";
}

int run_test_mode(calculator::TCPClient& client)
{
    const std::vector<std::string> test_requests = {
        R"({"firstValue": 5, "operation": "+", "secondValue": 3})",
        R"({"firstValue": 10, "operation": "-", "secondValue": 4})",
        R"({"firstValue": 6, "operation": "*", "secondValue": 7})",
        R"({"firstValue": 20, "operation": "/", "secondValue": 4})",
        R"({"firstValue": 2, "operation": "^", "secondValue": 10})",
        R"({"firstValue": 5, "operation": "!"})",
        R"({"firstValue": 10, "operation": "/", "secondValue": 0})",
        R"({"firstValue": 5, "operation": "%", "secondValue": 3})",
        R"({"firstValue": 100, "operation": "!", "secondValue": 0})",
        R"({"firstValue": -5, "operation": "!", "secondValue": 0})"};

    for (std::size_t i = 0; i < test_requests.size(); ++i)
    {
        std::cout << "\n=== Test " << (i + 1) << " ===\n";
        std::cout << "Request: " << test_requests[i] << "\n";
        try
        {
            std::string response = client.send_request(test_requests[i]);
            std::cout << "Response: " << response << "\n";
            auto json_resp = nlohmann::json::parse(response);
            if (json_resp.contains("status"))
            {
                std::cout << "Status: " << json_resp["status"] << "\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}

int run_interactive_mode(calculator::TCPClient& client)
{
    std::cout << "Interactive mode. Enter JSON requests (Ctrl+D to exit):\n";
    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty())
        {
            continue;
        }
        try
        {
            std::string response = client.send_request(line);
            std::cout << "Response: " << response << "\n";
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
} // namespace

int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"host", required_argument, nullptr, 'H'},
        {"port", required_argument, nullptr, 'p'},
        {"test", no_argument, nullptr, 't'},
        {"debug", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    std::string host;
    int port = 0;
    bool test_mode = false;
    bool debug = false;

    int opt;
    while ((opt = getopt_long(argc, argv, "", long_options, nullptr)) != -1)
    {
        switch (opt)
        {
            case 'H':
                host = optarg;
                break;
            case 'p':
                port = std::atoi(optarg);
                break;
            case 't':
                test_mode = true;
                break;
            case 'd':
                debug = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (host.empty() || port == 0)
    {
        print_usage(argv[0]);
        return 1;
    }

    Logger::init(debug);

    try
    {
        calculator::TCPClient client(host, port);
        client.connect();

        int rc =
            test_mode ? run_test_mode(client) : run_interactive_mode(client);

        client.disconnect();
        return rc;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
