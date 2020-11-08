#include <fiberio/all.hpp>
#include <iostream>
#include <parser.h>
#include <regex>
#include <router.h>

#define CONTINUE_RESPONSE "HTTP/1.1 100 Continue\r\n\r\n"

namespace fiberhttp {

static bool DEBUG = false;

int on_url(http_parser *p, const char *at, size_t length) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onUrl(std::string{at, length});
};
int on_status(http_parser *p, const char *at, size_t length) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onStatus(std::string{at, length});
};
int on_header_field(http_parser *p, const char *at, size_t length) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onHeaderField(std::string{at, length});
};
int on_header_value(http_parser *p, const char *at, size_t length) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onHeaderValue(std::string{at, length});
};
int on_headers_complete(http_parser *p) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onHeadersComplete();
};
int on_body(http_parser *p, const char *at, size_t length) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onBody(at, length);
};
int on_message_complete(http_parser *p) {
    auto owner = static_cast<Parser *>(p->data);
    return owner->onMessageComplete();
};

Parser::Parser(fiberio::socket *socket) : mSocket(socket) {
    mHttpParser.data = (void *)this;
    http_parser_init(&mHttpParser, HTTP_REQUEST);
    http_parser_settings_init(&mHttpParserSetting);
    mHttpParserSetting.on_url = on_url;
    mHttpParserSetting.on_status = on_status;
    mHttpParserSetting.on_header_field = on_header_field;
    mHttpParserSetting.on_header_value = on_header_value;
    mHttpParserSetting.on_headers_complete = on_headers_complete;
    mHttpParserSetting.on_body = on_body;
    mHttpParserSetting.on_message_complete = on_message_complete;
}

Parser::~Parser() {}

void Parser::process(Router *router) {
    mRouter = router;
    while (!readDone) {
        // TODO: add try catch here
        mReadSize = mSocket->read(mReadBuff, READBUFFSIZE);
        int res = http_parser_execute(&mHttpParser, &mHttpParserSetting, mReadBuff, mReadSize);
        if (res != mReadSize) {
            mSocket->close();
            throw std::runtime_error("Parser error");
        }
    }
    mHandler->handler(mRequest, mReply);
    mReply.write(mSocket);
    mSocket->close();
}

// http_parser callback
int Parser::onUrl(const std::string &val) {
    mRequest.setUrl(val);
    return 0;
}

int Parser::onStatus(const std::string &val) { return 0; }

int Parser::onHeaderField(const std::string &val) {
    mLastField = val;
    return 0;
}
int Parser::onHeaderValue(const std::string &val) {
    mRequest.addHeader(mLastField, val);
    return 0;
}

int Parser::onHeadersComplete() {
    if (DEBUG) {
        std::cout << "HEADER COMPLETE" << std::endl;
        // mRequest.debugHeader();
    }
    mRequest.setMethod(mHttpParser.method);

    mHandler = mRouter->handler(mRequest, mReply);
    if (mHandler == nullptr) {
        if (DEBUG)
            std::cout << "NOT FOUND" << std::endl;
        readDone = true;
        mReply.write(mSocket);
        return -1;
    }
    auto expect = mRequest.header("Expect");
    if (expect.compare("100-continue") == 0) {
        mSocket->write(CONTINUE_RESPONSE, strlen(CONTINUE_RESPONSE));
    }
    return 0;
}

int Parser::onBody(const char *at, size_t length) {
    mRequest.addBufferBody(at, length);
    return 0;
}

int Parser::onMessageComplete() {
    if (DEBUG) {
        std::cout << "MESSAGE COMPLETE " << mRequest.bufferBody().size() << std::endl;
    }
    readDone = true;
    mRequest.parseBody();
    return 0;
}

} // namespace fiberhttp