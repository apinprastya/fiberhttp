#include "request.h"
#include <iostream>
#include <regex>

namespace fiberhttp {

void Request::setUrl(const std::string &url) {
    mUrl = url;
    auto qMark = mUrl.find("?");
    if (qMark != std::string::npos) {
        mPath = mUrl.substr(0, qMark);
        while (qMark != std::string::npos) {
            qMark++;
            auto second = mUrl.find("&", qMark);
            auto eq = mUrl.find("=", qMark);
            auto k = mUrl.substr(qMark, eq - qMark);
            if (eq != std::string::npos) {
                if (second != std::string::npos) {
                    auto val = mUrl.substr(eq + 1, second - eq - 1);
                    mQueries.insert(std::pair<std::string, std::string>{k, val});
                } else {
                    auto val = mUrl.substr(eq + 1, std::string::npos);
                    mQueries.insert(std::pair<std::string, std::string>{k, val});
                }
            } else {
                mQueries.insert(std::pair<std::string, std::string>{k, ""});
            }
            qMark = mUrl.find("&", qMark);
        }
    } else {
        mPath = mUrl;
    }
}

std::string Request::header(const std::string &key) {
    auto it = mHeaders.find(key);
    if (it != mHeaders.end()) {
        return it->second;
    }
    return std::string{};
}

void Request::addBufferBody(const char *data, size_t length) { mBufferBody.append(data, length); }

void Request::parseBody() {
    const auto &contentType = header("Content-Type");
    if (contentType.find("application/json") != std::string::npos) {
        mBodyType = BodyType::BODY_JSON;
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        auto s = contentType.find("ary=");
        auto boundary = contentType.substr(s + 4);
        // std::cout << boundary << std::endl;
        size_t start = 0;
        auto pattern = "--" + boundary;
        while (start < mBufferBody.length()) {
            start = mBufferBody.find(pattern, start);
            if (start != std::string::npos) {
                start += pattern.length() + 2;
                auto es = mBufferBody.find(pattern, start);
                parseBodyField(start, es - start - 2);
                start = es;
            } else {
                break;
            }
        }
        // std::cout << "BODY ############ \n" << mBufferBody << std::endl;
    }
}

void Request::debugHeader() {
    for (auto it = mHeaders.begin(); it != mHeaders.end(); it++) {
        std::cout << it->first << " : " << it->second << std::endl;
    }
}

nlohmann::json Request::json() const { return nlohmann::json::parse(mBufferBody.data()); }

void Request::parseBodyField(size_t start, size_t length) {
    // std::cout << start << " : " << length << " : " << std::string{mBufferBody.substr(start, length)} << std::endl;
    static const std::regex re_content_disposition(
        "^Content-Disposition:\\s*form-data;\\s*name=\"(.*?)\"(?:;\\s*filename="
        "\"(.*?)\")?\\s*$",
        std::regex_constants::icase);
    static const std::regex re_content_type("^Content-Type:\\s*(.*?)?\\s*$", std::regex_constants::icase);
    static const std::string clrf{"\r\n"};
    auto view = std::string_view(mBufferBody);
    auto headerEnd = view.find(clrf, start);
    size_t left = length;
    FormField ff;
    if (headerEnd != std::string::npos) {
        auto header = view.substr(start, headerEnd - start);
        std::match_results<std::string_view::const_iterator> m;
        if (std::regex_match(header.cbegin(), header.cend(), m, re_content_disposition)) {
            ff.name = m[1];
            ff.fileName = m[2];
        } else {
            return;
        }
        left -= (header.length() + 2);
        start = headerEnd + 2;
    }
    auto endClrf = view.find(clrf, start);
    if (endClrf != std::string::npos) {
        if (endClrf != start) {
            auto cType = view.substr(start, endClrf - start);
            start = endClrf + 2;
            std::match_results<std::string_view::const_iterator> m2;
            if (std::regex_match(cType.cbegin(), cType.cend(), m2, re_content_type)) {
                ff.contentType = m2[1];
                ff.isFile = true;
            }
            left -= (cType.length() + 2);
            endClrf = view.find(clrf, start);
            if (endClrf == start) {
                start = endClrf + 2;
                left -= 2;
                endClrf = view.find(clrf, start);
                ff.data = mBufferBody.data() + start;
                ff.length = left;
            }
        } else {
            left -= 2;
            endClrf = view.find(clrf, start + 2);
            ff.data = mBufferBody.data() + start + 2;
            ff.length = left;
        }
    }
    formFields.insert({ff.name, ff});
}

} // namespace fiberhttp