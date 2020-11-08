#ifndef ROUTER_H
#define ROUTER_H

#include "reply.h"
#include "request.h"
#include <functional>
#include <regex>
#include <string>
#include <vector>

namespace fiberhttp {

using Handler = std::function<void(const Request &, Reply &)>;

struct HttpHandler {
    HttpHandler(int m, const std::regex &r, const Handler &&h, const std::string &p,
                const std::vector<std::string> &par)
        : method(m), pattern(r), handler(h), path(p), params(par) {}
    int method;
    std::regex pattern;
    Handler handler;
    std::string path;
    std::vector<std::string> params;
};

class Router {
  public:
    Router();
    virtual ~Router() {}
    Router &get(const std::string &pattern, Handler handler);
    Router &post(const std::string &pattern, Handler handler);
    Router &put(const std::string &pattern, Handler handler);
    Router &del(const std::string &pattern, Handler handler);
    Router &patch(const std::string &pattern, Handler handler);
    Router &options(const std::string &pattern, Handler handler);

    HttpHandler *handler(Request &req, Reply &rep);

  private:
    std::vector<HttpHandler> mHandlers;
    Handler mNotFound;

    void insertRouter(int method, const std::string &&pattern, const Handler &&handler);
};
} // namespace fiberhttp

#endif