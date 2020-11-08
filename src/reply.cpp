#include "reply.h"
#include <cstring>
#include <fiberio/socket.hpp>
#include <fmt/core.h>

#define RESPONSE                                                                                                       \
    "HTTP/1.1 200 OK\r\n"                                                                                              \
    "Content-Type: text/plain\r\n"                                                                                     \
    "Content-Length: 14\r\n"                                                                                           \
    "\r\n"                                                                                                             \
    "Hello, World!\n"

#define NOTFOUND_RESPONSE                                                                                              \
    "HTTP/1.1 404 OK\r\n"                                                                                              \
    "Content-Type: text/plain\r\n"                                                                                     \
    "Content-Length: 15\r\n"                                                                                           \
    "\r\n"                                                                                                             \
    "route not found"

namespace fiberhttp {

inline const char *statusMessage(int status) {
    switch (status) {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocol";
    case 102:
        return "Processing";
    case 103:
        return "Early Hints";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";
    case 208:
        return "Already Reported";
    case 226:
        return "IM Used";
    case 300:
        return "Multiple Choice";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 306:
        return "unused";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 418:
        return "I'm a teapot";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Entity";
    case 423:
        return "Locked";
    case 424:
        return "Failed Dependency";
    case 425:
        return "Too Early";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage";
    case 508:
        return "Loop Detected";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";

    default:
    case 500:
        return "Internal Server Error";
    }
}

void Reply::addHeaders(const std::string &key, const char *value) { mHeaders.insert({key, std::string{value}}); }

void Reply::write(fiberio::socket *socket, char *data) {
    if (data == nullptr) {
        if (mContentLength == 0 && mData.length() > 0)
            mContentLength = mData.length();
        auto header = fmt::format("HTTP/1.1 {} {}\r\n", mStatus, statusMessage(mStatus));
        header.append(fmt::format("Content-Type: {}\r\n", mContentType));
        header.append(fmt::format("Content-Length: {}\r\n", mContentLength));
        for (auto h : mHeaders) {
            header.append(fmt::format("{}: {}", h.first, h.second));
        }
        header.append("\r\n");
        socket->write(header.c_str(), header.length());
        socket->write(mData.c_str(), mData.length());
    }
}

void Reply::routeNotFound(fiberio::socket *socket) { socket->write(NOTFOUND_RESPONSE, strlen(NOTFOUND_RESPONSE)); }

void Reply::setContent(const char *value, const std::string &contentType) {
    mData = std::string(value);
    mContentType = contentType;
}

Reply &Reply::setStatus(int status) {
    mStatus = status;
    return *this;
}

void Reply::json(const nlohmann::json &json) {
    mData = json.dump();
    mContentType = "application/json";
}

} // namespace fiberhttp