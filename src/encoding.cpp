#include "encoding.hpp"
#include <iconv.h>
#include <stdexcept>
#include <vector>
#include <cstring>

std::string cp866_to_utf8(const char* data, size_t len) {
    iconv_t cd = iconv_open("UTF-8", "CP866");
    if (cd == (iconv_t)-1)
        throw std::runtime_error("iconv_open failed");

    size_t inleft = len;
    size_t outleft = len * 4 + 1;
    std::vector<char> outbuf(outleft);
    char* inptr = const_cast<char*>(data);
    char* outptr = outbuf.data();

    size_t res = iconv(cd, &inptr, &inleft, &outptr, &outleft);
    iconv_close(cd);

    if (res == (size_t)-1)
        throw std::runtime_error("iconv conversion failed");

    return std::string(outbuf.data(), outbuf.size() - outleft);
}

std::string cp866_display_field(const char* data, size_t len) {
    std::string text = cp866_to_utf8(data, len);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\0'))
        text.pop_back();
    return text;
}

std::string utf8_to_cp866(const std::string& text) {
    iconv_t cd = iconv_open("CP866", "UTF-8");
    if (cd == (iconv_t)-1)
        throw std::runtime_error("iconv_open failed");

    size_t inleft = text.size();
    size_t outleft = text.size() + 1;
    std::vector<char> outbuf(outleft);
    char* inptr = const_cast<char*>(text.data());
    char* outptr = outbuf.data();

    size_t res = iconv(cd, &inptr, &inleft, &outptr, &outleft);
    iconv_close(cd);

    if (res == (size_t)-1)
        return std::string();

    return std::string(outbuf.data(), outbuf.size() - outleft);
}