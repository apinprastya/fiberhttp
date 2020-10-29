#ifndef DRIVER_H
#define DRIVER_H

#include "dbresult.h"
#include <chrono>
#include <memory>
#include <string>

namespace fiberhttp {

class Driver {
  public:
    inline void setup(const std::string &host, const std::string &username, const std::string &password,
                      const std::string &dbName, int port = 0) {
        mHost = host;
        mUsername = username;
        mPassword = password;
        mDbName = dbName;
        mPort = port;
        mLastExec = std::chrono::steady_clock::now();
    }
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual std::shared_ptr<Driver> cloneAndConnect() = 0;
    virtual DbResult query(const std::string &sql, const std::vector<std::any> &params = {}) = 0;
    inline void setLastExec(const std::chrono::steady_clock::time_point &now) { mLastExec = now; };
    inline bool isIdleMax(int second) {
        auto now = std::chrono::steady_clock::now();
        auto diff = int(std::chrono::duration_cast<std::chrono::seconds>(now - mLastExec).count());
        return diff >= second;
    }

  protected:
    std::string mHost{};
    std::string mUsername{};
    std::string mPassword{};
    std::string mDbName{};
    int mPort{};
    std::chrono::steady_clock::time_point mLastExec{};
    bool mIsUsed{};
};
} // namespace fiberhttp

#endif // DRIVER_H