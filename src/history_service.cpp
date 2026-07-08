#include "history_service.hpp"

HistoryService::HistoryService(std::unique_ptr<storage::PostgresStorage> db,
                               std::unique_ptr<storage::RedisCache> cache) :
    m_db(std::move(db)), m_cache(std::move(cache))
{
    try
    {
        m_db->createSchema();
        m_cache->clear();
        m_cache->warmup(m_db->loadHistory());
        Logger::instance().info("cache warmed up");
    }
    catch (const std::exception& e)
    {
        Logger::instance().error(fmt::format(
            "cache warmup failed, running with cold cache: {}", e.what()));
    }
}

void HistoryService::insert_cache(const calculator::Task& task)
{
    try
    {
        m_cache->put(task);
    }
    catch (const std::exception& e)
    {
        Logger::instance().error(fmt::format("Redis failure: {}", e.what()));
    }
}
void HistoryService::insert_db(const calculator::Task& task)
{
    try
    {
        m_db->insert(task);
    }
    catch (const std::exception& e)
    {
        Logger::instance().error(
            fmt::format("PostgresSql failure: {}", e.what()));
    }
}
void HistoryService::process(calculator::Task& task)
{
    try
    {
        if (auto cached = m_cache->get(task))
        {
            task.result = cached->result;
            task.status = cached->status;
            Logger::instance().debug("cache hit: " +
                                     storage::RedisCache::key(task));
            return;
        }

        Logger::instance().debug("cache miss: " +
                                 storage::RedisCache::key(task));
    }
    catch (const std::exception& e)
    {
        Logger::instance().error(fmt::format("cache lookup failed, "
                                             "falling back to compute: {}",
                                             e.what()));
    }
    calculator::Calculator::execute(task);
    insert_db(task);
    insert_cache(task);
}
