#ifndef _utils_h
#define _utils_h

#include <chrono>
#include <string>
#include <vector>

std::string implode(std::vector<std::string> elements, std::string delimiter);
std::vector<std::string> explode(std::string str, std::string delimiter);
uint64_t GetTime();
std::string get_uuid();

template <typename... Args>
std::string string_format(const std::string &format, Args... args)
{
    int size_s = snprintf(nullptr, 0, format.c_str(), args...) + 1;
    if (size_s <= 0)
        return "";

    size_t size = static_cast<size_t>(size_s);
    char* buf = new char[size];
    snprintf(buf, size, format.c_str(), args...);
    std::string out = std::string(buf, buf + size - 1);
    delete buf;
    return out;
}

#endif