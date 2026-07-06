#pragma once
#include <libpq-fe.h>
#include <memory>
#include <string>
#include <vector>
#include "task.hpp"

namespace storage
{

struct PGconnDeleter  { void operator()(PGconn* c)   const noexcept { PQfinish(c); } };
struct PGresultDeleter{ void operator()(PGresult* r) const noexcept { PQclear(r);  } };

using PGconnPtr   = std::unique_ptr<PGconn,   PGconnDeleter>;
using PGresultPtr = std::unique_ptr<PGresult, PGresultDeleter>;

class PostgresStorage
{
public:
    explicit PostgresStorage(const std::string& conninfo);

    void createSchema();
    void insert(const calculator::Task& task);
    std::vector<calculator::Task> loadHistory();

private:
    PGresultPtr exec(const char* sql,
                     const std::vector<std::string>& params = {});

    PGconnPtr m_conn;
};

}