#include "driver/sqlite.h"
#include <database.h>

namespace fiberhttp {

static bool DEBUG = false;

std::vector<Database *> sDatabases;

class dummy_lock {
  public:
    void lock() {}
    void unlock() {}
};

Database::Database(DbType dbType, int maxConnection, int maxIdleConnection, int idleConnectionTimeout)
    : mDbType(dbType), mMaxConnection(maxConnection), mMaxIdleConnection(maxIdleConnection),
      mIdleConnectionTimeout(idleConnectionTimeout) {
    mDriver.reserve(maxConnection);
    sDatabases.push_back(this);
}

Database::~Database() {}

void Database::releaseFiber() {
    for (auto it : sDatabases) {
        it->release();
    }
}

void Database::shutdown() {
    for (auto it : sDatabases) {
        it->shutdownExec();
    }
}

bool Database::open(const std::string &host, const std::string &username, const std::string &password,
                    const std::string &dbName) {
    if (mOpenned)
        return true;
    mOpenned = true;
    mIsRunning = true;
    std::shared_ptr<Driver> driver;
    if (mDbType == DB_SQLITE) {
        driver = std::make_shared<SqliteDriver>();
    } else if (mDbType == DB_MYSQL) {
    } else if (mDbType == DB_POSTGRESQL) {
    }
    driver->setup(host, username, password, dbName);
    if (driver->open()) {
        mFreeDriver.push_back(driver);
        mDriver.push_back(driver);
        checkIdleConnection();
        return true;
    }
    return false;
}

DbResult Database::query(const std::string &sql, const std::vector<std::any> params) {
    auto id = boost::this_fiber::get_id();
    auto it = mUsedDriver.find(id);
    if (it != mUsedDriver.end()) {
        if (DEBUG)
            std::cout << "USED FOUND : " << id << std::endl;
        return execQuery(it->second, std::move(sql), std::move(params));
    }
    if (!mFreeDriver.empty()) {
        if (DEBUG)
            std::cout << "NOT EMPTY" << std::endl;
        auto driver = freeDriver();
        return execQuery(driver, std::move(sql), std::move(params));
    }
    if (mDriver.size() < mMaxConnection) {
        if (DEBUG)
            std::cout << "CREATE NEW CONNECTION" << std::endl;
        auto driver = mDriver.front()->cloneAndConnect();
        mDriver.push_back(driver);
        mUsedDriver.insert(std::pair<boost::fibers::fiber::id, std::shared_ptr<Driver>>{id, driver});
        return execQuery(driver, std::move(sql), std::move(params));
    }
    dummy_lock dummy;
    while (mFreeDriver.empty()) {
        mCond.wait(dummy);
    }
    if (!mIsRunning)
        throw std::runtime_error("Database already exited!");
    auto driver = freeDriver();
    return execQuery(driver, std::move(sql), std::move(params));
}

std::shared_ptr<Driver> Database::freeDriver() {
    auto id = boost::this_fiber::get_id();
    auto driver = mFreeDriver.front();
    mFreeDriver.pop_front();
    mUsedDriver.insert(std::pair<boost::fibers::fiber::id, std::shared_ptr<Driver>>{id, driver});
    return driver;
}

DbResult Database::execQuery(std::shared_ptr<Driver> driver, const std::string &sql,
                             const std::vector<std::any> params) {
    return driver->query(std::move(sql), std::move(params));
}

void Database::release() {
    auto id = boost::this_fiber::get_id();
    auto it = mUsedDriver.find(id);
    if (it != mUsedDriver.end()) {
        if (DEBUG)
            std::cout << "fiber released" << std::endl;
        mFreeDriver.push_back(it->second);
        mUsedDriver.erase(it);
        mCond.notify_all();
    }
}

void Database::shutdownExec() {
    if (DEBUG)
        std::cout << "DATABASE EXIT" << std::endl;
    mIsRunning = false;
    mCond.notify_all();
}

void Database::checkIdleConnection() {
    using namespace std::chrono_literals;
    boost::fibers::async([this]() {
        dummy_lock dummy;
        if (DEBUG)
            std::cout << "checking idle connection started" << std::endl;
        while (mIsRunning) {
            if (mDriver.size() <= mMaxIdleConnection) {
                boost::this_fiber::sleep_for(500ms);
                continue;
            }
            if (mIsRunning) {
                if (DEBUG)
                    std::cout << "checking idle connection" << std::endl;

                std::vector<std::shared_ptr<Driver>> toRemove{};
                for (auto it = mFreeDriver.begin(); it != mFreeDriver.end(); it++) {
                    if ((*it)->isIdleMax(mIdleConnectionTimeout)) {
                        toRemove.push_back(*it);
                    }
                }
                for (auto it = toRemove.begin(); it != toRemove.end(); it++) {
                    auto freeItem = std::find(mFreeDriver.begin(), mFreeDriver.end(), *it);
                    if (freeItem != mFreeDriver.end()) {
                        mFreeDriver.erase(freeItem);
                    }
                    auto item = std::find(mDriver.begin(), mDriver.end(), *it);
                    if (item != mDriver.end()) {
                        mDriver.erase(item);
                    }
                    if (mDriver.size() <= mMaxIdleConnection) {
                        break;
                    }
                }
                boost::this_fiber::sleep_for(500ms);
            }
        }
    });
}

} // namespace fiberhttp