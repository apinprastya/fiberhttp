#ifndef DATABASE_H
#define DATABASE_H

#include "dbresult.h"
#include "driver.h"
#include <boost/fiber/all.hpp>
#include <map>
#include <string>
#include <vector>

namespace fiberhttp {
class Database {
  public:
    enum DbType {
        DB_SQLITE,
        DB_MYSQL,
        DB_POSTGRESQL,
    };

    Database(DbType dbType, int maxConnection = 100, int maxIdleConnection = 10, int idleConnectionTimeout = 30);
    ~Database();
    static void releaseFiber();
    static void shutdown();
    bool open(const std::string &host, const std::string &username, const std::string &password,
              const std::string &dbName);
    DbResult query(const std::string &sql, const std::vector<std::any> params = {});

  private:
    DbType mDbType;
    int mMaxConnection;
    int mMaxIdleConnection;
    int mIdleConnectionTimeout;
    std::vector<std::shared_ptr<Driver>> mDriver{};
    std::deque<std::shared_ptr<Driver>> mFreeDriver{};
    // saved used driver by fiber id
    std::map<boost::fibers::fiber::id, std::shared_ptr<Driver>> mUsedDriver{};
    boost::fibers::condition_variable_any mCond;
    bool mOpenned{};
    bool mIsRunning{};

    std::shared_ptr<Driver> freeDriver();
    DbResult execQuery(std::shared_ptr<Driver> driver, const std::string &sql, const std::vector<std::any> params);
    void checkIdleConnection();
    void release();
    void shutdownExec();
};
} // namespace fiberhttp

#endif // DATABASE_H