#ifndef SQLITE_H
#define SQLITE_H

#include <driver.h>
#include <sqlite3.h>

namespace fiberhttp {
class SqliteDriver : public Driver, public std::enable_shared_from_this<SqliteDriver> {
  public:
    bool open() override;
    bool close() override;
    std::shared_ptr<Driver> cloneAndConnect();
    DbResult query(const std::string &sql, const std::vector<std::any> params = {}) override;

  private:
    sqlite3 *mDatabase{};
};
} // namespace fiberhttp

#endif // SQLITE_H