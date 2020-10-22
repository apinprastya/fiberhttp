#include <fiberhttp.h>
//
#include <database.h>
#include <iostream>
#include <string>

int main() {
    fiberhttp::FiberHttpServer server;
    fiberhttp::Database dbGlobal(fiberhttp::Database::DB_SQLITE);
    std::cout << dbGlobal.open("global.db", "", "", "") << std::endl;
    std::cout << boost::this_fiber::get_id() << std::endl;
    /*auto f = boost::fibers::async([db = &dbGlobal]() {
        auto r = db->query("SELECT name FROM sqlite_master WHERE type = 'table'");
        std::cout << "LENGTH " << r.length() << std::endl;
    });
    f.wait();*/
    server.get("/", [db = &dbGlobal](const fiberhttp::Request &req, fiberhttp::Response &res) {
        try {
            // auto w = db->query("INSERT INTO NAME(ID, NAME) VALUES (?, ?)", std::vector<std::any>{5, "NAMA LIMA"});
            // auto w = db->query("UPDATE NAME SET name = ? WHERE id = ?", std::vector<std::any>{"MAGDALENA", 5});
            // std::cout << "LAST ID " << w.affectedRows() << std::endl;
            auto w = db->query("SELECT 25633");
            for (int i = 0; i < w.length(); i++) {
                auto row = w.rowAt(i);
                std::cout << std::any_cast<int>(row.column(0)) << std::endl;
                /*for (int j = 0; j < 2; j++) {
                    auto aa = row.column(j);
                    if (j == 0)
                        std::cout << std::any_cast<int>(aa) << std::endl;
                    else
                        std::cout << std::any_cast<std::string>(aa) << std::endl;
                    std::cout << (typeid(std::string) == aa.type()) << " : " << (typeid(int) == aa.type()) << std::endl;
                }*/
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