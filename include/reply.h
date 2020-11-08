#ifndef REPLY_H
#define REPLY_H

#include <map>
#include <nlohmann/json.hpp>
#include <string>

namespace fiberio {
class socket;
}

namespace fiberhttp {
class Reply {
  public:
    void write(fiberio::socket *socket, char *data = nullptr);
    void routeNotFound(fiberio::socket *socket);
    void setContent(const char *value, const std::string &contentType);
    void addHeaders(const std::string &key, const char *value);
    Reply &setStatus(int status);
    void json(const nlohmann::json &json);

  private:
    int mStatus{200};
    size_t mContentLength{};
    std::string mData{};
    std::string mContentType{"text/plain"};
    std::map<std::string, std::string> mHeaders{};
};

} // namespace fiberhttp

#endif