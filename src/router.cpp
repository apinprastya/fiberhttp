#include "router.h"
#include "parser.h"
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>

namespace fiberhttp {
Router::Router() {
    mNotFound = [](const Request &req, Reply &rep) {
        rep.setStatus(404).setContent("{\"error\": \"No route found\"}", "application/json");
    };
}

Router &Router::get(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_GET, std::move(pattern), std::move(handler));
    return *this;
}

Router &Router::post(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_POST, std::move(pattern), std::move(handler));
    return *this;
}

Router &Router::put(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_PUT, std::move(pattern), std::move(handler));
    return *this;
}

Router &Router::del(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_DELETE, std::move(pattern), std::move(handler));
    return *this;
}

Router &Router::patch(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_PATCH, std::move(pattern), std::move(handler));
    return *this;
}

Router &Router::options(const std::string &pattern, Handler handler) {
    insertRouter(HTTP_OPTION, std::move(pattern), std::move(handler));
    return *this;
}

HttpHandler *Router::handler(Request &req, Reply &rep) {
    std::smatch match;
    auto path = req.path();
    for (auto it = mHandlers.begin(); it != mHandlers.end(); it++) {
        if (it->method == req.method() && std::regex_match(path, match, it->pattern)) {
            if (!it->params.empty()) {
                for (int i = 0; i < it->params.size(); i++) {
                    req.addParam(it->params.at(i), match[i + 1]);
                }
            }
            return &(*it);
        }
    }
    mNotFound(req, rep);
    return nullptr;
}

void Router::insertRouter(int method, const std::string &&pattern, const Handler &&handler) {
    std::regex r1("(:\\w*)", std::regex_constants::icase);
    std::smatch m;
    auto str = pattern;
    if (str.length() > 1 && boost::algorithm::ends_with(str, "/"))
        str = str.substr(0, str.length() - 1);
    std::vector<std::string> params;
    while (std::regex_search(str, m, r1)) {
        params.push_back(m.str(1));
        str = m.suffix().str();
    }
    std::regex reg{pattern};
    if (!params.empty()) {
        auto str = pattern;
        for (int i = 0; i < params.size(); i++) {
            auto pos = str.find(params.at(i));
            str.replace(pos, params.at(i).length(), "(\\w*?)");
        }
        str += "$";
        reg = std::regex{str};
    }
    mHandlers.emplace_back(method, reg, std::move(handler), pattern, params);
}

} // namespace fiberhttp