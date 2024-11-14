#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <pqxx/pqxx> // Include libpqxx
#include <string>
#include <memory>

namespace alicia {
    class Database {
    public:
        Database(const std::string& conn_info);
        ~Database();

        pqxx::result executeQuery(const std::string& query);
        void executeCommand(const std::string& command);

    private:
        std::unique_ptr<pqxx::connection> conn;
    };
}

#endif // DATABASE_HPP
