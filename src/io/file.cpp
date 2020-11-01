#include "file.hpp"

#include <fstream>
#include <streambuf>

File::File(std::string path) : m_Path(std::move(path)) {}

std::vector<char> File::readFileContent() {
    std::ifstream input(m_Path, std::ios_base::binary | std::ios_base::ate);
    auto position = input.tellg();
    input.seekg(0);

    std::vector<char> buffer(position);
    input.read(buffer.data(), buffer.size());

    return std::move(buffer);
}

std::string File::readFileToString() {
    std::ifstream input(m_Path, std::ios_base::binary | std::ios_base::ate);
    auto position = input.tellg();
    input.seekg(0);

    std::string buffer;
    buffer.reserve(position);

    buffer.assign((std::istreambuf_iterator<char>(input)),
                  std::istreambuf_iterator<char>());

    return std::move(buffer);
}
