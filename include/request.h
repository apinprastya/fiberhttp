#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace fiberhttp {

enum class BodyType { BODY_FORM, BODY_JSON };

struct FormField {
    std::string name{};
    std::string fileName{};
    std::string contentType{};
    char *data{};
    size_t length{};
    bool isFile{};

    void debugString() { std::cout << name << " : " << std::string{data, length} << std::endl; }
    std::string toString() { return std::string{data, length}; }
};

class Request {
  public:
    void setUrl(const std::string &url);
    inline void addHeader(const std::string &key, const std::string &value) {
        mHeaders.insert(std::pair<std::string, std::string>{key, value});
    }
    inline std::string url() const { return mUrl; }
    inline std::string path() const { return mPath; }
    inline std::map<std::string, std::string> headers() const { return mHeaders; }
    std::string header(const std::string &key);
    void addBufferBody(const char *data, size_t length);
    inline std::string bufferBody() const { return mBufferBody; }
    void parseBody();
    void debugHeader();
    nlohmann::json json() const;
    inline bool isJson() const { return mBodyType == BodyType::BODY_JSON; }
    inline bool isForm() const { return mBodyType == BodyType::BODY_FORM; }
    inline FormField formField(const std::string &key) const { return formFields.at(key); }
    inline std::map<std::string_view, std::string_view> query() const { return mQueries; }
    inline void setMethod(int method) { mMethod = method; }
    inline int method() const { return mMethod; }
    inline void addParam(const std::string &key, const std::string &value) { mParams.insert({key, value}); }
    inline std::string param(const std::string &key) const { return mParams.at(key); }

  private:
    int mMethod{};
    std::string mUrl{};
    std::string mPath{};
    std::map<std::string_view, std::string_view> mQueries{};
    std::map<std::string, std::string> mHeaders{};
    std::string mBufferBody{};
    BodyType mBodyType;
    std::map<std::string, FormField> formFields{};
    std::map<std::string, std::string> mParams{};

    void parseBodyField(size_t from, size_t length);
};

} // namespace fiberhttp

#endif