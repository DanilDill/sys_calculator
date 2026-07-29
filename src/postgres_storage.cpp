// postgres_storage.cpp
#include "postgres_storage.hpp"

#include "logger.hpp"

#include <fmt/format.h>

#include <stdexcept>
namespace storage
{
namespace
{
constexpr const char* kCreateSchemaSql = R"sql(
        CREATE TABLE IF NOT EXISTS calc_history(
            value1    INTEGER NOT NULL,
            operation CHAR(1) NOT NULL,
            value2    INTEGER NOT NULL,
            status    INTEGER NOT NULL,
            result    INTEGER NOT NULL,
            PRIMARY KEY (value1, operation, value2)
        ))sql";
}

PostgresStorage::PostgresStorage(const std::string& conninfo) :
    m_conn(PQconnectdb(conninfo.c_str()))
{
    if (!m_conn || PQstatus(m_conn.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(
            fmt::format("postgres connect failed: {}",
                        m_conn ? PQerrorMessage(m_conn.get())
                               : "PQconnectdb returned null"));
    }
}

PGresultPtr PostgresStorage::try_exec(const char* sql,
                                      const std::vector<const char*>& values)
{
    return PGresultPtr{PQexecParams(m_conn.get(), sql,
                                    static_cast<int>(values.size()), nullptr,
                                    values.data(), nullptr, nullptr, 0)};
}

void PostgresStorage::ensureConnection()
{
    if (PQstatus(m_conn.get()) == CONNECTION_OK)
        return;

    Logger::instance().info("postgres connection lost, resetting");
    PQreset(m_conn.get()); // переподключение с исходными параметрами

    if (PQstatus(m_conn.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(fmt::format("postgres reconnect failed: {}",
                                             PQerrorMessage(m_conn.get())));
    }
    // схему восстанавливаем напрямую через try_exec: вызов exec() отсюда
    // зациклил бы ensureConnection при повторном обрыве соединения,
    // а ошибка DDL не должна ронять исходный запрос — таблица обычно уже есть
    auto res = try_exec(kCreateSchemaSql, {});
    if (PQresultStatus(res.get()) != PGRES_COMMAND_OK)
    {
        Logger::instance().warning(
            fmt::format("schema restore after reconnect failed: {}",
                        PQerrorMessage(m_conn.get())));
    }
    Logger::instance().info("postgres connection restored");
}
PGresultPtr PostgresStorage::exec(const char* sql,
                                  const std::vector<std::string>& params)
{
    std::vector<const char*> values;
    values.reserve(params.size());
    for (const auto& p : params)
        values.push_back(p.c_str());

    ensureConnection();
    auto res = try_exec(sql, values);

    auto ok = [](PGresult* r) {
        const auto st = PQresultStatus(r);
        return st == PGRES_COMMAND_OK || st == PGRES_TUPLES_OK;
    };

    if (!ok(res.get()) && PQstatus(m_conn.get()) != CONNECTION_OK)
    {
        Logger::instance().info(
            "query failed due to lost connection, retrying");
        ensureConnection();
        res = try_exec(sql, values);
    }

    if (!ok(res.get()))
    {
        throw std::runtime_error(fmt::format("postgres query failed: {}",
                                             PQerrorMessage(m_conn.get())));
    }
    return res;
}

void PostgresStorage::createSchema()
{
    exec(kCreateSchemaSql);
}

void PostgresStorage::insert(const calculator::Task& task)
{
    exec(R"sql(
        INSERT INTO calc_history(value1, operation, value2, status, result)
        VALUES ($1, $2, $3, $4, $5)
        ON CONFLICT (value1, operation, value2) DO NOTHING)sql",
         {std::to_string(task.value1), std::string(1, task.operation),
          std::to_string(task.value2),
          std::to_string(static_cast<int>(task.status)),
          std::to_string(task.result)});
}

std::vector<calculator::Task> PostgresStorage::loadHistory()
{
    auto res = exec(
        "SELECT value1, operation, value2, status, result FROM calc_history");

    std::vector<calculator::Task> out;
    const int rows = PQntuples(res.get());
    out.reserve(rows);
    for (int i = 0; i < rows; ++i)
    {
        calculator::Task t{};
        t.value1 = std::stoi(PQgetvalue(res.get(), i, 0));
        t.operation = PQgetvalue(res.get(), i, 1)[0];
        t.value2 = std::stoi(PQgetvalue(res.get(), i, 2));
        t.status = static_cast<calculator::Task::Status>(
            std::stoi(PQgetvalue(res.get(), i, 3)));
        t.result = std::stoi(PQgetvalue(res.get(), i, 4));
        out.push_back(t);
    }
    return out;
}

} // namespace storage