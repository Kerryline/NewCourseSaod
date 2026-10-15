#pragma once
#include <string>

std::string cp866_to_utf8(const char* data, size_t len);
std::string cp866_display_field(const char* data, size_t len);
std::string utf8_to_cp866(const std::string& text);
