#include <fiberhttp.h>
#include <fiberio/loop.hpp>
//
#include <boost/fiber/all.hpp>
//
#include <database.h>
//
#include <parser.h>
#include <uv.h>

namespace fibers = boost::fibers;

#define RESPONSE                                                                                                       \
    "HTTP/1.1 200 OK\r\n"                                                                                              \
    "Content-Type: text/plain\r\n"                                                                                     \
    "Content-Length: 14\r\n"                                                                                           \
    "\r\n"                                                                                                             \
    "Hello, World!\n"

namespace fiberhttp {

FiberHttpServer::FiberHttpServer() { fiberio::use_on_this_thread(); }

FiberHttpServer::~FiberHttpServer() {}

void FiberHttpServer::run(const std::string &address, int port, int backlog) {
    std::cout << "Server listening at " << address << ":" << port << " with " << backlog << " backlog" << std::endl;

    mSocketServer.bind(address, port);
    mSocketServer.listen(backlog);
    auto signal_t = new uv_signal_t();
    signal_t->data = (void *)this;
    uv_signal_init(fiberio::get_uv_loop(), signal_t);
    uv_signal_start(
        signal_t,
        [](uv_signal_t *handle, int signum) {
            auto d = static_cast<FiberHttpServer *>(handle->data);
            d->shutdown();
        },
        SIGINT);

    fibers::fiber(std::bind(&FiberHttpServer::loop, this)).join();
    std::cout << "Server exited" << std::endl;
}

void FiberHttpServer::shutdown() {
    std::cout << "Shuting down server http" << std::endl;
    Database::shutdown();
    mIsRunning = false;
    mSocketServer.close();
}

void FiberHttpServer::loop() {
    fibers::future<void> fi(fibers::async([this]() {
        while (mIsRunning) {
            try {
                fibers::async(
                    [this](fiberio::socket client) {
                        try {
                            Parser parser(&client);
                            parser.process(mRouter);
                        } catch (std::exception e) {
                        }
                        Database::releaseFiber();
                    },
                    mSocketServer.accept());
            } catch (std::runtime_error e) {
                std::cout << e.what() << std::endl;
                mIsRunning = false;
            }
        }
    }));
    fi.wait();
}

} // namespace fiberhttp