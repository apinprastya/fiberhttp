#include <fiberhttp.h>
#include <boost/fiber/all.hpp>
#include "httpstream.h"

namespace fibers = boost::fibers;

namespace fiberhttp
{
    FiberHttpServer::FiberHttpServer() {}

    FiberHttpServer::~FiberHttpServer() {}

    void FiberHttpServer::run(const std::string &address, int port, int backlog)
    {
        std::cout << "Server listening at " << address << ":" << port << " with " << backlog << " backlog" << std::endl;
        fiberio::use_on_this_thread();

        mSocketServer.bind(address, port);
        mSocketServer.listen(backlog);
        loop();
    }

    void FiberHttpServer::loop()
    {
        while (mIsRunning)
        {
            fibers::async([this](fiberio::socket client) {
                FiberStream stream(std::move(client));
                bool close = false;
                try
                {
                    mHttpLib.processRequest(stream, close, close, nullptr);
                }
                catch (std::exception e)
                {
                    std::cout << e.what() << std::endl;
                    close = true;
                }
                if (close)
                    client.close();
            },
                          mSocketServer.accept());
        }
    }

    FiberHttpServer &FiberHttpServer::get(const char *pattern, Handler handler)
    {
        mHttpLib.Get(pattern, handler);
        return *this;
    }
    FiberHttpServer &FiberHttpServer::post(const char *pattern, Handler handler)
    {
        mHttpLib.Post(pattern, handler);
        return *this;
    }
    FiberHttpServer &FiberHttpServer::put(const char *pattern, Handler handler)
    {
        mHttpLib.Put(pattern, handler);
        return *this;
    }
    FiberHttpServer &FiberHttpServer::del(const char *pattern, Handler handler)
    {
        mHttpLib.Delete(pattern, handler);
        return *this;
    }
    FiberHttpServer &FiberHttpServer::patch(const char *pattern, Handler handler)
    {
        mHttpLib.Patch(pattern, handler);
        return *this;
    }
    FiberHttpServer &FiberHttpServer::options(const char *pattern, Handler handler)
    {
        mHttpLib.Options(pattern, handler);
        return *this;
    }

} // namespace fiberhttp