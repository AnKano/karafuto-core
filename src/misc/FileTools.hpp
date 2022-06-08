#pragma once

#include <iostream>
#include <fstream>
#include <vector>

namespace KCore {
    static std::vector<uint8_t> fileRead(const std::string &path) {
        std::streampos fileSize;
        std::ifstream file(path, std::ios::binary);

        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        auto result = std::vector<uint8_t>(fileSize);
        file.read((char *) &result[0], fileSize);

        return result;
    }
}