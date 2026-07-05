#include "postgres_storage.hpp"
#include "redis_cache.hpp"
#include "calculator.hpp"
#include "logger.hpp"
class HistoryService
{
public:
    HistoryService(std::unique_ptr<storage::PostgresStorage> db,
                   std::unique_ptr<storage::RedisCache> cache)
        : m_db(std::move(db)), m_cache(std::move(cache))
    {
        m_db->createSchema();
        m_cache->clear();
        m_cache->warmup(m_db->loadHistory());
    }

    void process(calculator::Task& task)
    {
        if (auto cached = m_cache->get(task))
        {
            task = *cached;
            Logger::instance().debug("cache hit: " + storage::RedisCache::key(task));
            return;
        }
        Logger::instance().debug("cache miss: " + storage::RedisCache::key(task));
        calculator::Calculator::execute(task);

        try
        {
            m_cache->put(task);
            m_db->insert(task);
        }
        catch (const std::exception& e)
        {
            // деградация: расчёт выполнен, недоступность БД не должна ломать ответ
            Logger::instance().error(fmt::format("storage failure: {}", e.what()));
        }
    }

private:
    std::unique_ptr<storage::PostgresStorage> m_db;
    std::unique_ptr<storage::RedisCache> m_cache;
};