#include "mysql_driver.h"
#include <boost/fiber/all.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace fiberhttp {

static bool DEBUG = false;

using namespace std::chrono_literals;

void bindParams(std::string &sql, const std::vector<std::any> &params) {
    for (auto param : params) {
        auto f = sql.find('?');
        if (f != std::string::npos) {
            if (param.type() == typeid(int)) {
                sql.replace(f, 1, std::to_string(std::any_cast<int>(param)));
            } else if (param.type() == typeid(std::string)) {
                auto str = std::any_cast<std::string>(param);
                std::ostringstream s;
                s << "\'" << str << "\'";
                sql.replace(f, 1, s.str());
            } else if (param.type() == typeid(char *) || param.type() == typeid(const char *)) {
                const char *str = std::any_cast<const char *>(param);
                std::ostringstream s;
                s << "\'" << str << "\'";
                sql.replace(f, 1, s.str());
            }
        }
    }
}

bool MySQLDriver::open() {
    if (DEBUG)
        std::cout << "OPENNING MYSQL" << std::endl;
    mDatabase = mysql_init(nullptr);
    if (mDatabase == nullptr)
        throw std::runtime_error("can not init mysql connection");
    if (DEBUG)
        std::cout << "CONNECTING TO SERVER";
    net_async_status status = mysql_real_connect_nonblocking(mDatabase, mHost.c_str(), mUsername.c_str(),
                                                             mPassword.c_str(), mDbName.c_str(), mPort, nullptr, 0);
    if (DEBUG)
        std::cout << status << std::endl;
    while (status == NET_ASYNC_NOT_READY) {
        boost::this_fiber::sleep_for(1ms);
        status = mysql_real_connect_nonblocking(mDatabase, mHost.c_str(), mUsername.c_str(), mPassword.c_str(),
                                                mDbName.c_str(), mPort, nullptr, 0);
    }
    if (status == NET_ASYNC_ERROR) {
        std::cout << mysql_error(mDatabase) << std::endl;
        throw std::runtime_error(mysql_error(mDatabase));
    }
    return true;
};

bool MySQLDriver::close() {
    mysql_close(mDatabase);
    if (DEBUG)
        std::cout << "CLOSING MYSQL CONNECTION" << std::endl;
    return true;
}

std::shared_ptr<Driver> MySQLDriver::cloneAndConnect() {
    std::shared_ptr<MySQLDriver> ret = std::make_shared<MySQLDriver>();
    ret->setup(this->mHost, this->mUsername, this->mPassword, this->mDbName, this->mPort);
    ret->open();
    return ret;
}

DbResult MySQLDriver::query(const std::string &sql, const std::vector<std::any> &params) {
    if (DEBUG)
        std::cout << "QUERY MYSQL" << std::endl;
    DbResult result;
    std::string base = std::move(sql);
    bindParams(base, std::move(params));
    MYSQL_RES *res;
    MYSQL_ROW row;
    net_async_status status = mysql_real_query_nonblocking(mDatabase, base.c_str(), base.length());
    while (status == NET_ASYNC_NOT_READY) {
        boost::this_fiber::sleep_for(1ms);
        status = mysql_real_query_nonblocking(mDatabase, base.c_str(), base.length());
    }
    if (status == NET_ASYNC_ERROR)
        throw std::runtime_error(mysql_error(mDatabase));

    status = mysql_store_result_nonblocking(mDatabase, &res);
    while (status == NET_ASYNC_NOT_READY) {
        boost::this_fiber::sleep_for(1ms);
        status = mysql_store_result_nonblocking(mDatabase, &res);
    }
    if (status == NET_ASYNC_ERROR) {
        mysql_free_result(res);
        throw std::runtime_error(mysql_error(mDatabase));
    }
    if (res != nullptr) {
        int numField = mysql_num_fields(res);
        auto fieldNames = mysql_fetch_fields(res);
        std::vector<enum_field_types> fieldTypes{};
        if (DEBUG)
            std::cout << "NUM FIELD : " << numField << std::endl;
        for (int i = 0; i < numField; i++) {
            result.pushColumnName(std::string{fieldNames[i].name});
            fieldTypes.push_back(fieldNames[i].type);
            if (DEBUG)
                std::cout << fieldNames[i].name << std::endl;
        }

        while (true) {
            boost::this_fiber::sleep_for(1ms);
            status = mysql_fetch_row_nonblocking(res, &row);
            if (status == NET_ASYNC_COMPLETE) {
                if (row == nullptr)
                    break;
                DbRow dbRow;
                for (int i = 0; i < numField; i++) {
                    if (row[i] == nullptr) {
                        dbRow.addColumn(nullptr);
                    } else {
                        switch (fieldTypes.at(i)) {
                        case MYSQL_TYPE_TINY:
                        case MYSQL_TYPE_SHORT:
                        case MYSQL_TYPE_LONG:
                        case MYSQL_TYPE_INT24: {
                            long long val = strtol(row[i], nullptr, 0);
                            dbRow.addColumn(val);
                            break;
                        }
                        case MYSQL_TYPE_LONGLONG: {
                            long long val = strtoll(row[i], nullptr, 0);
                            dbRow.addColumn(val);
                            break;
                        }
                        case MYSQL_TYPE_FLOAT: {
                            float val = strtof(row[i], nullptr);
                            dbRow.addColumn(val);
                            break;
                        }
                        case MYSQL_TYPE_DOUBLE:
                        case MYSQL_TYPE_NEWDECIMAL:
                        case MYSQL_TYPE_DECIMAL: {
                            double val = strtod(row[i], nullptr);
                            dbRow.addColumn(val);
                            break;
                        }
                        case MYSQL_TYPE_VAR_STRING:
                        case MYSQL_TYPE_STRING:
                        case MYSQL_TYPE_VARCHAR: {
                            dbRow.addColumn(std::string(row[i]));
                            break;
                        }
                        default:
                            dbRow.addColumn(std::string(row[i]));
                        }
                    }
                }
                result.addRow(std::move(dbRow));
            } else if (status == NET_ASYNC_ERROR) {
                mysql_free_result(res);
                throw std::runtime_error(mysql_error(mDatabase));
            }
        }
        mysql_free_result(res);
    } else {
        result.setAffectedRows(size_t(mysql_affected_rows(mDatabase)));
        result.setLastInsertedId(size_t(mysql_insert_id(mDatabase)));
    }

    return result;
}

} // namespace fiberhttp