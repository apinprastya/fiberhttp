# FiberHTTP

C++ asynchronous http server with fiber. This library built using :
- boost::fiber
- libuv
- fiberio [https://github.com/hampus/fiberio](https://github.com/hampus/fiberio)
- nlohmann-json [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
- fmt [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)

This library test compiled running on Linux and Windows.
Library is still in early phase of development, big changes may happen anytimes.

# Requirement
- git, cmake, C++ compiler (GCC, MSVC, Clang)
- vcpkg and install package boost::fibers libuv libmysql sqlite3 fmt nlohmann-json

## Additional step
- copy others/vcpkg/libmysql to [vcpkg_installed_folder]/installed/[triplet]/share/

# Compile
```bash
$ git clone --recursive https://github.com/apinprastya/fiberhttp
$ cd fiberhttp
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=[your_vcpkg_folder]/scripts/buildsystems/vcpkg.cmake ..
$ make
```

# Example

```cpp
#include <fiberhttp.h>
#include <uv.h>
//
#include <database.h>
#include <fiberio/loop.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <router.h>
#include <string>

int main() {
    fiberhttp::FiberHttpServer server;
    // fiberhttp::Database dbGlobal(fiberhttp::Database::DB_MYSQL);
    fiberhttp::Database dbGlobal(fiberhttp::Database::DB_SQLITE);
    auto f = boost::fibers::async([db = &dbGlobal]() {
        try {
            // db->open("127.0.0.1", 3306, "apin", "apin", "apin");
            db->open("my.db", 0, "", "", "");
        } catch (std::exception e) {
            std::cout << "ERROR " << e.what() << std::endl;
            exit(-1);
        }
        if (db->getDbType() == fiberhttp::Database::DB_SQLITE) {
            auto r = db->query("SELECT name FROM sqlite_master WHERE type = 'table'");
            std::cout << "EMPTY " << r.isEmpty() << std::endl;
            if (r.isEmpty()) {
                try {
                    auto r1 = db->query("CREATE TABLE user(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL)");
                    if (r1.success()) {
                        db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA SATU"});
                        db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA DUA"});
                        db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA TIGA"});
                        db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA EMPAT"});
                        db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA LIMA"});
                    }
                } catch (std::runtime_error e) {
                    std::cout << "Error : " << e.what() << std::endl;
                }
            }
        } else if (db->getDbType() == fiberhttp::Database::DB_MYSQL) {
            try {
                db->query("SELECT * FROM user LIMIT 1");
            } catch (std::runtime_error e) {
                std::cout << "Error : " << e.what() << std::endl;
                db->query(
                    "CREATE TABLE IF NOT EXISTS user(id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(255) NOT NULL)");
                db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA SATU"});
                db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA DUA"});
                db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA TIGA"});
                db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA EMPAT"});
                auto r = db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{"NAMA LIMA"});
                std::cout << "LAST ID : " << r.lastInsertedId() << std::endl;
            }
        }
        db->releaseFiber();
    });
    f.wait();
    fiberhttp::Router router;
    router.get("/", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        res.setContent("{\"fiberhttp\": \"root\"}", "application/json");
    });
    router.get("/user", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        try {
            nlohmann::json jRet;
            auto r = db->query("SELECT * FROM user");
            std::vector<nlohmann::json> datas;
            for (int i = 0; i < r.length(); i++) {
                auto row = r.rowAt(i);
                nlohmann::json rr;
                rr["id"] = std::any_cast<long long>(row.column(0));
                rr["name"] = std::any_cast<std::string>(row.column(1));
                datas.push_back(rr);
            }
            jRet["data"] = datas;
            jRet["total"] = r.length();
            res.json(jRet);
        } catch (std::exception e) {
            std::cout << "ERROR " << e.what() << std::endl;
        }
    });
    router.get("/user/:id", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        nlohmann::json jRet;
        auto r = db->query("SELECT * FROM user WHERE id = ?", std::vector<std::any>{std::stoi(req.param(":id"))});
        if (!r.isEmpty()) {
            auto row = r.rowAt(0);
            jRet["id"] = std::any_cast<long long>(row.column(0));
            jRet["name"] = std::any_cast<std::string>(row.column(1));
        }
        res.json(jRet);
    });
    router.post("/user", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        try {
            auto json = req.json();
            auto name = json["name"].get<std::string>();
            auto r = db->query("INSERT INTO user(name) VALUES (?)", std::vector<std::any>{std::move(name)});
            nlohmann::json jRet;
            jRet["id"] = r.lastInsertedId();
            res.json(jRet);
        } catch (std::exception e) {
            std::cout << "ERROR " << e.what() << std::endl;
        }
    });
    router.put("/user/:id", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        try {
            auto json = req.json();
            auto name = json["name"].get<std::string>();
            auto r = db->query("UPDATE user SET name = ? WHERE id = ?",
                               std::vector<std::any>{std::move(name), std::stoi(req.param(":id"))});
            nlohmann::json jRet;
            jRet["affected"] = r.affectedRows();
            res.json(jRet);
        } catch (std::exception e) {
            std::cout << "ERROR " << e.what() << std::endl;
        }
    });
    router.del("/user/:id", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Reply &res) {
        nlohmann::json jRet;
        auto r = db->query("DELETE FROM user WHERE id = ?", std::vector<std::any>{std::stoi(req.param(":id"))});
        jRet["affected"] = r.affectedRows();
        res.json(jRet);
    });

    server.setRouter(&router);
    server.run("0.0.0.0", 8990, 100);
    std::cout << "EXIT SUCCESSFULLY" << std::endl;
    return 0;
}
```

Full example can be found at example folder.

# Todo
- [ ] Keep alive
- [x] Http parser using nodejs http-parse
- [ ] GZip compression
- [ ] Middleware
- [ ] Error handling
- [x] Adding database access
- [x] SQLite driver
- [x] MySQL driver
- [ ] Postgres driver
- [ ] SQL builder and/or ORM
- [x] Gracefull exit
- [ ] SSL?
- [ ] HTML template?

# License
MIT

# Contribute
Any contributions are welcome
