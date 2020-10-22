#ifndef FIBERHTTP_H
#define FIBERHTTP_H

#include <fiberio/all.hpp>
#include <httplib.h>

#define MAX_KEEP_ALIVE 5

namespace fiberhttp {
using Request = httplib::Request;
using Response = httplib::Response;
using Handler = std::function<void(const Request &, Response &)>;

class _HttpLibHelper : public httplib::Server {
  public:
    inline bool processRequest(httplib::Stream &strm, bool close_connection, bool &connection_closed,
                               const std::function<void(httplib::Request &)> &setup_request) {
        return process_request(strm, connection_closed, connection_closed, setup_request);
    }
};

class FiberHttpServer {
  public:
    FiberHttpServer();
    ~FiberHttpServer();
    void run(const std::string &address, int port, int backlog = 50);
    void shutdown();

    FiberHttpServer &get(const char *pattern, Handler handler);
    FiberHttpServer &post(const char *pattern, Handler handler);
    FiberHttpServer &put(const char *pattern, Handler handler);
    FiberHttpServer &del(const char *pattern, Handler handler);
    FiberHttpServer &patch(const char *pattern, Handler handler);
    FiberHttpServer &options(const char *pattern, Handler handler);

  protected:
    fiberio::server_socket mSocketServer;
    _HttpLibHelper mHttpLib;
    bool mIsRunning{true};
    int mKeepAliveMax = MAX_KEEP_ALIVE;
    // uv_signal_t *signal_t;

    void loop();
};
} // namespace fiberhttp

#endif // FIBERHTTP_H