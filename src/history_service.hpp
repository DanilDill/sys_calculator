#pragma once

#include "postgres_storage.hpp"
#include "redis_cache.hpp"
#include "calculator.hpp"
#include "logger.hpp"
class HistoryService
{
public:
    HistoryService(std::unique_ptr<storage::PostgresStorage> db,
                   std::unique_ptr<storage::RedisCache> cache);
    void process(calculator::Task& task);
private:
    void insert_cache(const calculator::Task& task);
    void insert_db(const calculator::Task& task);
    std::unique_ptr<storage::PostgresStorage> m_db;
    std::unique_ptr<storage::RedisCache> m_cache;
};