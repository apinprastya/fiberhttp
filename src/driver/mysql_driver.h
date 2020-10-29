#ifndef MYSQL_H
#define MYSQL_H

#include <driver.h>
#include <memory>
#include <mysql/mysql.h>

namespace fiberhttp {
class MySQLDriver : public Driver, public std::enable_shared_from_this<MySQLDriver> {
  public:
    bool open() override;
    bool close() override;
    std::shared_ptr<Driver> cloneAndConnect();
    DbResult query(const std::string &sql, const std::vector<std::any> &params = {}) override;

  private:
    MYSQL *mDatabase{};
};
} // namespace fiberhttp

#endif