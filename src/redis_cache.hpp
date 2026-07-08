#pragma once
#include <sw/redis++/redis++.h>
#include <optional>
#include <fmt/format.h>
#include "task.hpp"
#include <string_view>
#include <charconv>
#include "logger.hpp"
namespace storage
{

class RedisCache
{
public:
    explicit RedisCache(const std::string& uri) : m_redis(uri) {}

    static std::string key(const calculator::Task& t)
    {
        int a = t.value1;
        int b = t.value2;
        if ((t.operation == '+' || t.operation == '*') && a > b)
        {
            std::swap(a, b);
        }
        return fmt::format("calc:{}:{}:{}", a, t.operation, b);
    }

    void clear()
    {
        // чистим только свои ключи, а не FLUSHDB — Redis может быть общим
        std::vector<std::string> keys;
        auto cursor = 0LL;
        do {
            cursor = m_redis.scan(cursor, "calc:*", 100,
                                  std::back_inserter(keys));
        } while (cursor != 0);
        if (!keys.empty())
            m_redis.del(keys.begin(), keys.end());
    }

    static std::optional<std::pair<int, calculator::Task::Status>> parseValue(
        const std::string& val) noexcept
    { 
        const auto pos = val.find(':');
        if (pos == std::string::npos)
            return std::nullopt;
        int result = 0;
        int status = 0;
        auto parseInt = [](std::string_view sv, int& out)
        {
            const auto [ptr, ec] =
                std::from_chars(sv.data(), sv.data() + sv.size(), out);
            return ec == std::errc{} && ptr == sv.data() + sv.size();
        };
        const std::string_view sv{val};
        if (!parseInt(sv.substr(0, pos), result) ||
            !parseInt(sv.substr(pos + 1), status))
            return std::nullopt;

        return std::make_pair(result,
                              static_cast<calculator::Task::Status>(status));
    }

    std::optional<calculator::Task> get(calculator::Task t)
    {
        auto val = m_redis.get(key(t));
        if (!val) return std::nullopt;

        if (auto parsed = parseValue(*val))
        {
            t.result = parsed->first;
            t.status = parsed->second;
            return t;
        }
        Logger::instance().error(fmt::format(
        "corrupt cache value for {}: '{}', evicting", key(t), *val));
        m_redis.del(key(t));
        return std::nullopt;
    }

    void put(const calculator::Task& t)
    {
        m_redis.set(key(t),
                    fmt::format("{}:{}", t.result, static_cast<int>(t.status)));
    }

    void warmup(const std::vector<calculator::Task>& history)
    {
        auto pipe = m_redis.pipeline(false);
        for (const auto& t : history)
            pipe.set(key(t), fmt::format("{}:{}", t.result,
                                         static_cast<int>(t.status)));
        pipe.exec();
    }

private:
    sw::redis::Redis m_redis;
};

} 