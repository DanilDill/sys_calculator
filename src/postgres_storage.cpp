// postgres_storage.cpp
#include "postgres_storage.hpp"
#include <stdexcept>
#include <fmt/format.h>

namespace storage
{

PostgresStorage::PostgresStorage(const std::string& conninfo)
    : m_conn(PQconnectdb(conninfo.c_str()))
{
    if (!m_conn || PQstatus(m_conn.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(fmt::format("postgres connect failed: {}",
            m_conn ? PQerrorMessage(m_conn.get()) : "PQconnectdb returned null"));
    }
}

PGresultPtr PostgresStorage::exec(const char* sql,
                                  const std::vector<std::string>& params)
{
    std::vector<const char*> values;
    values.reserve(params.size());
    for (const auto& p : params) values.push_back(p.c_str());

    PGresultPtr res{PQexecParams(m_conn.get(), sql,
                                 static_cast<int>(values.size()),
                                 nullptr, values.data(),
                                 nullptr, nullptr, 0)};

    const auto st = PQresultStatus(res.get());
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(fmt::format("postgres query failed: {}",
                                 PQerrorMessage(m_conn.get())));
    }
    return res; // ownership перемещается наружу
}

void PostgresStorage::createSchema()
{
    exec(R"sql(
        CREATE TABLE IF NOT EXISTS calc_history(
            value1    INTEGER NOT NULL,
            operation CHAR(1) NOT NULL,
            value2    INTEGER NOT NULL,
            status    INTEGER NOT NULL,
            result    INTEGER NOT NULL,
            PRIMARY KEY (value1, operation, value2)
        ))sql");
}

void PostgresStorage::insert(const calculator::Task& task)
{
    exec(R"sql(
        INSERT INTO calc_history(value1, operation, value2, status, result)
        VALUES ($1, $2, $3, $4, $5)
        ON CONFLICT (value1, operation, value2) DO NOTHING)sql",
        { std::to_string(task.value1),
          std::string(1, task.operation),
          std::to_string(task.value2),
          std::to_string(static_cast<int>(task.status)),
          std::to_string(task.result) });
}

std::vector<calculator::Task> PostgresStorage::loadHistory()
{
    auto res = exec("SELECT value1, operation, value2, status, result FROM calc_history");

    std::vector<calculator::Task> out;
    const int rows = PQntuples(res.get());
    out.reserve(rows);
    for (int i = 0; i < rows; ++i)
    {
        calculator::Task t{};
        t.value1    = std::stoi(PQgetvalue(res.get(), i, 0));
        t.operation = PQgetvalue(res.get(), i, 1)[0];
        t.value2    = std::stoi(PQgetvalue(res.get(), i, 2));
        t.status    = static_cast<calculator::Task::Status>(
                          std::stoi(PQgetvalue(res.get(), i, 3)));
        t.result    = std::stoi(PQgetvalue(res.get(), i, 4));
        out.push_back(t);
    }
    return out;
}

} // namespace storage