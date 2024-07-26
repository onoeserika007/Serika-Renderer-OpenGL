#include "Utils/UniversalUtils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

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
    // std::cout << "File read, size: " << buffer.size() << std::endl;
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

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

void printVec3(const std::string &name, const glm::vec3& vec) {
    std::cout << name << vec[0] << " " << vec[1] << " " << vec[2] << " " << std::endl;
}

std::string printVec3(const glm::vec3& vec) {
    std::stringstream ss;
    ss << "[ " << vec[0] << ", " << vec[1] << ", " << vec[2] << " ]";
    return ss.str();
}

void printMat4(const std::string &name, glm::mat4 mat) {
    std::cout << "Matrix manually:" + name + "\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// 这种转换方法在选择正交基时有一个旋转的自由度，故不能将局部坐标的向量正确地转换到world下，但是对于采样来说是无关紧要的。
glm::vec3 MathUtils::toWorld(const glm::vec3 &local, const glm::vec3 &N) {
    glm::vec3 B, C; // c = x axis, B = z axis
    if (std::fabs(N.x) > std::fabs(N.y)){
        float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
        C = glm::vec3(N.z * invLen, 0.0f, -N.x *invLen);
    }
    else {
        float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
        C = glm::vec3(0.0f, N.z * invLen, -N.y *invLen);
    }
    B = glm::cross(C, N); // TBN矩阵
    return local.x * B + local.y * C + local.z * N;
}

float MathUtils::get_random_float(float lowerBound, float upperBound) {
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(lowerBound, upperBound); // distribution in range [1, 6]

    return dist(rng);
}

bool MathUtils::solveQuadratic(const float &a, const float &b, const float &c, float &smallerRoot, float &greaterRoot) {
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) smallerRoot = greaterRoot = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
                  -0.5 * (b + sqrt(discr)) :
                  -0.5 * (b - sqrt(discr));
        smallerRoot = q / a;
        greaterRoot = c / q;
    }
    if (smallerRoot > greaterRoot) std::swap(smallerRoot, greaterRoot);
    return true; // if have solution
}

bool StringUtils::stringEndsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

bool StringUtils::stringStartsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool StringUtils::isDirectory(const std::string &str) {
    return fs::exists(str) && fs::is_directory(str);
}

std::string StringUtils::appendToDir(const std::string& dirPath, const std::string& fileName) {
    fs::path dir(dirPath);
    if (!isDirectory(dirPath)) {
        std::cerr << "Error: " << dirPath << " is not a valid directory." << std::endl;
        return "";
    }
    fs::path filePath = dir / fileName;
    return filePath.string();
}