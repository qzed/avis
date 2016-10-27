#pragma once

#include <vector>
#include <fstream>


namespace avis {
namespace utils {

std::vector<char> read_file_to_vector(const std::string& filename) {
    auto file = std::ifstream();
    file.exceptions(std::ios::failbit | std::ios::badbit);
    file.open(filename, std::ios::ate | std::ios::binary);

    auto filesize = static_cast<std::size_t>(file.tellg());
    auto buffer = std::vector<char>(filesize);

    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();

    return buffer;
}

} /* namespace utils */
} /* namespace avis */
