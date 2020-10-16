#include <httplib.h>

#define BUFFSIZE 4096

namespace fiberhttp
{
    class FiberStream : public httplib::Stream
    {
    public:
        FiberStream(fiberio::socket &&client) : mSocket(client)
        {
        }
        ~FiberStream() override {}

        bool is_readable() const override
        {
            return true; //mSocket.is_open();
        }
        bool is_writable() const override
        {
            return true; //mSocket.is_open();
        }
        ssize_t read(char *ptr, size_t size) override
        {
            size_t readed = 0;
            size_t left = size;
            while (true)
            {
                if (mLastIndex == 0)
                    mSize = mSocket.read(mBuffer, BUFFSIZE);
                size_t shouldRead = std::min(left, mSize - mLastIndex);
                memcpy(ptr + readed, mBuffer + mLastIndex, shouldRead);
                mLastIndex += shouldRead;
                if (mSize - mLastIndex == 0)
                    mLastIndex = 0;
                readed += shouldRead;
                left -= shouldRead;
                if (readed >= size)
                    return size;
            }
            return size;
        }
        ssize_t write(const char *ptr, size_t size) override
        {
            mSocket.write(ptr, size);
            return size;
        }
        void get_remote_ip_and_port(std::string &ip, int &port) const override
        {
            ip = mIp;
            port = mPort;
        }

    private:
        fiberio::socket mSocket;
        size_t mLastIndex{};
        char mBuffer[BUFFSIZE];
        size_t mSize;
        std::string mIp;
        int mPort{};
    };
} // namespace fiberhttp