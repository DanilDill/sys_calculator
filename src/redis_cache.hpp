#pragma once
#include <sw/redis++/redis++.h>
#include <optional>
#include <fmt/format.h>
#include "task.hpp"

namespace storage
{

class RedisCache
{
public:
    explicit RedisCache(const std::string& uri) : m_redis(uri) {}

    static std::string key(const calculator::Task& t)
    {
        return fmt::format("calc:{}:{}:{}", t.value1, t.operation, t.value2);
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

    std::optional<calculator::Task> get(calculator::Task t)
    {
        auto val = m_redis.get(key(t));
        if (!val) return std::nullopt;
        // формат значения: "result:status"
        const auto pos = val->find(':');
        t.result = std::stoi(val->substr(0, pos));
        t.status = static_cast<calculator::Task::Status>(
                       std::stoi(val->substr(pos + 1)));
        return t;
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