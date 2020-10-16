#include <iostream>
#include <fiberhttp.h>

int main()
{
    fiberhttp::FiberHttpServer server;
    server.get("/", [](const fiberhttp::Request &req, fiberhttp::Response &res) {
        res.set_content("{\"fiberhttp\":true}", "application/json");
    });
    server.run("0.0.0.0", 8990);
    return 0;
}