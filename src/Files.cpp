#include "Files.h"
#include <swiftly-ext/files.h>

std::string Files::Read(std::string path)
{
    path = GeneratePath(path.c_str());
    if (!std::filesystem::exists(path))
        return "";

    auto fp = std::fopen(path.c_str(), "rb");
    std::string s;
    std::fseek(fp, 0u, SEEK_END);
    auto size = std::ftell(fp);
    std::fseek(fp, 0u, SEEK_SET);
    s.resize(size);
    std::fread(&s[0], 1u, size, fp);
    std::fclose(fp);
    return s;
}

bool Files::ExistsPath(std::string path)
{
    return std::filesystem::exists(GeneratePath(path.c_str()));
}