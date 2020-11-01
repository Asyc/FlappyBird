#ifndef FLAPPYBIRD_FILE_HPP
#define FLAPPYBIRD_FILE_HPP

#include <string>
#include <vector>

class File {
public:
    File(std::string path);

    std::vector<char> readFileContent();
    std::string readFileToString();
private:
    std::string m_Path;
};


#endif //FLAPPYBIRD_FILE_HPP
