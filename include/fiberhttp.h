#ifndef FIBERHTTP_H
#define FIBERHTTP_H

#include <fiberio/all.hpp>
#include <reply.h>
#include <request.h>

#define MAX_KEEP_ALIVE 5

namespace fiberhttp {

class Router;

class FiberHttpServer {
  public:
    FiberHttpServer();
    ~FiberHttpServer();
    inline void setRouter(Router *router) { mRouter = router; }
    void run(const std::string &address, int port, int backlog = 50);
    void shutdown();

  protected:
    fiberio::server_socket mSocketServer;
    bool mIsRunning{true};
    int mKeepAliveMax = MAX_KEEP_ALIVE;
    Router *mRouter;

    void loop();
};
} // namespace fiberhttp

#endif // FIBERHTTP_H