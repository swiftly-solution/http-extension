#include "utils.h"
#include <random>
#include <climits>

std::vector<std::string> explode(std::string s, std::string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

std::string implode(std::vector<std::string> elements, std::string delimiter)
{
    std::string s;
    for (std::vector<std::string>::iterator ii = elements.begin(); ii != elements.end(); ++ii)
    {
        s += (*ii);
        if (ii + 1 != elements.end())
            s += delimiter;
    }
    return s;
}

uint64_t GetTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int32_t genrand()
{
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    return std::uniform_int_distribution<int>(0, INT_MAX)(rng);
}

std::string get_uuid()
{
    return string_format(
        "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
        (genrand() & 0xFFFF), (genrand() & 0xFFFF),
        (genrand() & 0xFFFF),
        ((genrand() & 0x0fff) | 0x4000),
        (genrand() % 0x3fff + 0x8000),
        (genrand() & 0xFFFF), (genrand() & 0xFFFF), (genrand() & 0xFFFF));
}