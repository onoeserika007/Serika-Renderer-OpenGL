#include "Utils/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>

std::vector<char> readFile(const std::string& filename)
{
    // ate - at end 文件指针在末尾
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    std::vector<char> buffer;

    try {
        if (!file.is_open())
        {
            //logDebug(filename);
            //std::cout << std::filesystem::current_path() << std::endl;
            throw std::runtime_error("failed to open file! The file directory '" + filename + "' did not exist!\ncurrent dir: " + std::filesystem::current_path().string());
        }

        size_t fileSize = static_cast<size_t>(file.tellg()) + 1; // 要预留一个字节给字符串结尾0
        buffer.resize(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
    }
    catch (std::runtime_error e) {
        logDebug(std::string("RUNTIME_ERROR ") + e.what());
        file.close();
        exit(-1);
    }
#ifdef DEBUG
    std::cout << "File read, size: " << buffer.size() << std::endl;
#endif // DEBUG

    return buffer;
}

void logDebug(const std::string& msg)
{
    std::cout << msg << std::endl;
}

const std::string getFileExtension(const std::string& filename) {
    // 找到最后一个点（.）的位置
    size_t dotIndex = filename.find_last_of('.');

    // 检查是否找到点并且点不在文件名的开头
    if (dotIndex != std::string::npos && dotIndex != 0) {
        // 使用 substr 截取从最后一个点到字符串末尾的部分
        return filename.substr(dotIndex + 1);
    }

    // 如果没有找到点，或者点在文件名的开头，说明没有扩展名
    return {};
}