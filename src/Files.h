#ifndef _files_h
#define _files_h

#include "utils.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace Files
{
    std::string Read(std::string path);
    bool ExistsPath(std::string path);
};

#endif