#include <fiberhttp.h>
//
#include <database.h>
#include <fiberio/loop.hpp>
#include <iostream>
#include <string>
#include <uv.h>

int main() {
    fiberhttp::FiberHttpServer server;
    fiberhttp::Database dbGlobal(fiberhttp::Database::DB_MYSQL);
    // std::cout << dbGlobal.open("global.db", "", "", "") << std::endl;
    // std::cout << boost::this_fiber::get_id() << std::endl;
    auto f = boost::fibers::async([db = &dbGlobal]() {
        try {
            db->open("127.0.0.1", 3306, "root", "root", "sultan");
        } catch (std::exception e) {
            std::cout << "ERROR " << e.what() << std::endl;
        }
        if (db->getDbType() == fiberhttp::Database::DB_SQLITE) {
            auto r = db->query("SELECT name FROM sqlite_master WHERE type = 'table'");
            std::cout << "EMPTY " << r.isEmpty() << std::endl;
            if (r.isEmpty()) {
                try {
                    auto r1 = db->query("CREATE TABLE name(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL)");
                    if (r1.success()) {
                        db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA SATU"});
                        db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA DUA"});
                        db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA TIGA"});
                        db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA EMPAT"});
                        db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA LIMA"});
                    }
                } catch (std::runtime_error e) {
                    std::cout << "Error : " << e.what() << std::endl;
                }
            }
        } else if (db->getDbType() == fiberhttp::Database::DB_MYSQL) {
            try {
                db->query("SELECT * FROM name LIMIT 1");
            } catch (std::runtime_error e) {
                std::cout << "Error : " << e.what() << std::endl;
                db->query(
                    "CREATE TABLE IF NOT EXISTS name(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(255) NOT NULL)");
                db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA SATU"});
                db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA DUA"});
                db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA TIGA"});
                db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA EMPAT"});
                auto r = db->query("INSERT INTO name(name) VALUES (?)", std::vector<std::any>{"NAMA LIMA"});
                std::cout << "LAST ID : " << r.lastInsertedId() << std::endl;
            }
        }
        db->releaseFiber();
    });
    f.wait();
    server.get("/", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Response &res) {
        try {
            auto w = db->query("SELECT * FROM name");
            for (int i = 0; i < w.length(); i++) {
                auto row = w.rowAt(i);
                // std::cout << row.column(0).type().name() << std::endl;
                std::cout << std::any_cast<long long>(row.column(0)) << " : "
                          << std::any_cast<std::string>(row.column(1)) << std::endl;
            }
        } catch (const std::exception &e) {
            std::cout << "Error Exception " << e.what() << std::endl;
        }
        res.set_content("{\"fiberhttp\":true}", "application/json");
    });

    server.run("0.0.0.0", 8990);
    std::cout << "EXIT SUCCESSFULLY" << std::endl;
    return 0;
}