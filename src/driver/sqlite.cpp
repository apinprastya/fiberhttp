#include "sqlite.h"
#include <dbresult.h>
#include <fiberio/loop.hpp>
#include <thread>
//
#include <boost/fiber/all.hpp>

namespace fiber = boost::fibers;

namespace fiberhttp {

static bool DEBUG = false;

struct Helper {
    DbResult result;
    fiber::promise<int> promise;
    std::string sql;
    std::vector<std::any> params;
    SqliteDriver *driver;
    Helper(const std::string &&sql, const std::vector<std::any> &&params, SqliteDriver *driver)
        : sql(sql), params(params), driver(driver) {}
};

bool SqliteDriver::open() {
    if (DEBUG)
        std::cout << "OPENNING" << std::endl;
    int ret = sqlite3_open(mHost.c_str(), &mDatabase);
    if (SQLITE_OK == ret && mDatabase != nullptr) {
        sqlite3_config(SQLITE_CONFIG_SERIALIZED);
        return true;
    }
    return false;
}

bool SqliteDriver::close() {
    if (DEBUG)
        std::cout << "CLOSING" << std::endl;
    int ret = sqlite3_close(mDatabase);
    return SQLITE_OK == ret;
}

std::shared_ptr<Driver> SqliteDriver::cloneAndConnect() {
    std::shared_ptr<SqliteDriver> ret = std::make_shared<SqliteDriver>();
    ret->setup(this->mHost, this->mUsername, this->mPassword, this->mDbName);
    ret->open();
    return ret;
}

DbResult SqliteDriver::query(const std::string &sql, const std::vector<std::any> &params) {
    uv_work_t w;
    Helper h(std::move(sql), std::move(params), this);
    fiber::future<int> future(h.promise.get_future());
    w.data = (void *)&h;
    uv_queue_work(
        fiberio::get_uv_loop(), &w,
        [](uv_work_t *req) {
            // using namespace std::chrono_literals;
            // std::this_thread::sleep_for(2s);
            auto helper = static_cast<Helper *>(req->data);
            auto db = helper->driver->mDatabase;
            if (db == nullptr)
                helper->driver->open();
            char *err;
            const char *sql = helper->sql.c_str();
            sqlite3_stmt *stmt;
            int retPrepare = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, nullptr);
            if (retPrepare != SQLITE_OK) {
                helper->result.setError(sqlite3_errmsg(db));
                return;
            } else {
                if (!helper->params.empty()) {
                    int index = 1;
                    for (auto &param : helper->params) {
                        // std::cout << param.type().name() << std::endl;
                        int retBind = SQLITE_ERROR;
                        if (param.type() == typeid(int)) {
                            retBind = sqlite3_bind_int(stmt, index++, std::any_cast<int>(param));
                        } else if (param.type() == typeid(std::string)) {
                            auto str = std::any_cast<std::string>(param);
                            retBind = sqlite3_bind_text(stmt, index++, str.c_str(), str.length(), nullptr);
                        } else if (param.type() == typeid(char *) || param.type() == typeid(const char *)) {
                            const char *str = std::any_cast<const char *>(param);
                            retBind = sqlite3_bind_text(stmt, index++, str, strlen(str), nullptr);
                        }
                        if (retBind != SQLITE_OK) {
                            helper->result.setError(sqlite3_errmsg(db));
                            sqlite3_finalize(stmt);
                            return;
                        }
                    }
                }

                int step = sqlite3_step(stmt);
                int col = 0;
                if (step == SQLITE_ROW) {
                    col = sqlite3_column_count(stmt);
                    while (step == SQLITE_ROW) {
                        DbRow row;
                        for (int i = 0; i < col; i++) {
                            int col = sqlite3_column_type(stmt, i);
                            switch (col) {
                            case SQLITE_INTEGER: {
                                long long val = sqlite3_column_int(stmt, i);
                                row.addColumn(val);
                                break;
                            }
                            case SQLITE_FLOAT: {
                                double val = sqlite3_column_double(stmt, i);
                                row.addColumn(val);
                                break;
                            }
                            case SQLITE_TEXT: {
                                auto s = sqlite3_column_text(stmt, i);
                                row.addColumn(std::string{reinterpret_cast<const char *>(s)});
                                break;
                            }
                            case SQLITE_NULL: {
                                row.addColumn(nullptr);
                                break;
                            }
                            default:
                                std::cout << "NO TYPE PREFFERED " << std::endl;
                                break;
                            }
                        }
                        helper->result.addRow(std::move(row));
                        step = sqlite3_step(stmt);
                    }
                } else if (step == SQLITE_DONE) {
                    helper->result.setLastInsertedId(sqlite3_last_insert_rowid(db));
                    helper->result.setAffectedRows(sqlite3_changes(db));
                } else {
                    helper->result.setError(sqlite3_errmsg(db));
                    sqlite3_finalize(stmt);
                    return;
                }
            }
            sqlite3_finalize(stmt);
        },
        [](uv_work_t *req, int status) {
            auto helper = static_cast<Helper *>(req->data);
            helper->driver->setLastExec(std::chrono::steady_clock::now());
            helper->promise.set_value(status);
        });
    future.get();
    if (!h.result.success())
        throw std::runtime_error(h.result.lastError().c_str());
    return h.result;
}
} // namespace fiberhttp