#ifndef PARSER_H
#define PARSER_H

#include <functional>
#include <http_parser.h>
#include <map>
#include <regex>
#include <reply.h>
#include <request.h>
#include <string>

#define READBUFFSIZE 4096

#define HTTP_DELETE 0
#define HTTP_GET 1
#define HTTP_HEAD 2
#define HTTP_POST 3
#define HTTP_PUT 4
#define HTTP_OPTION 6
#define HTTP_PATCH 28

namespace fiberio {
class socket;
} // namespace fiberio

namespace fiberhttp {

class Router;
struct HttpHandler;

using Handler = std::function<void(const Request &, Reply &)>;

class Parser {
  public:
    Parser(fiberio::socket *socket);
    ~Parser();
    void process(Router *router);
    // http_parser callback
    int onUrl(const std::string &val);
    int onStatus(const std::string &val);
    int onHeaderField(const std::string &val);
    int onHeaderValue(const std::string &val);
    int onHeadersComplete();
    int onBody(const char *at, size_t length);
    int onMessageComplete();

  private:
    fiberio::socket *mSocket;
    char mReadBuff[READBUFFSIZE];
    size_t mReadSize{};
    http_parser mHttpParser{};
    http_parser_settings mHttpParserSetting{};
    bool mReadStarted{};
    std::string mLastField;
    bool readDone{};

    Request mRequest;
    Reply mReply;
    Router *mRouter;
    HttpHandler *mHandler;
};
} // namespace fiberhttp

#endif