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

void Utils::UpdateProgress(float progress) {
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
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

// direction of Wi is coming in
void MathUtils::uniformHemisphereSample(glm::vec3 &outWo, float &outPdf, const glm::vec3 &inWi, const glm::vec3 &inN) {
    // uniform sample on the hemisphere
    float x_1 = get_random_float(), x_2 = get_random_float();
    float z = std::fabs(1.0f - 2.0f * x_1);
    float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
    glm::vec3 localRay(r*std::cos(phi), r*std::sin(phi), z);
    outWo = toWorld(localRay, inN);

    if (glm::dot(outWo, inN) > 0.f) outPdf = 0.5f / M_PI;
    else outPdf = 0.f; // what if hit back
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

glm::vec3 MathUtils::reflect(const glm::vec3 &I, const glm::vec3 &N) {
    return I - 2 * dot(I, N) * N;
}

glm::vec3 MathUtils::refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior) {
    float cosi = glm::clamp(dot(I, N), -1.f, 1.f);
    float etai = 1, etat = ior;
    glm::vec3 n = N;
    if (cosi < 0) { cosi = -cosi; } // incident case
    else { std::swap(etai, etat); n= -N; } // emergent case
    float eta = etai / etat; // ior frac{inci}{emerg}
    float k = 1 - eta * eta * (1 - cosi * cosi);
    // when k < 0, reflection should happend instead of refraction
    return k < 0 ? glm::vec3(0.f) : eta * I + (eta * cosi - sqrtf(k)) * n;
}

void MathUtils::fresnel(const glm::vec3 &I, const glm::vec3 &N, const float &ior, float &kr) {
    float cosi = glm::clamp(dot(I, N), -1.f, 1.f);
    float etai = 1, etat = ior;
    if (cosi > 0) {  std::swap(etai, etat); }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}

float MathUtils::Trowbridge_Reitz_GGX_D(const glm::vec3 &normal, const glm::vec3 &halfVector, float a) {
    float a2 = a * a;
    float NdotH =std::max(dot(normal,halfVector),0.f);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = M_PI * denom * denom;
    return nom / std::max(denom, 0.00001f);
}

float MathUtils::Schick_GGX(float NdotV, float k) {
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / std::max(denom,0.00001f);
}

float MathUtils::Schick_GGXSmith_G(const glm::vec3 &N, const glm::vec3 &V, const glm::vec3 &L, float k) {
    k = std::pow(k+1.0f,2.0f) / 8.0f;
    float NdotV = std::max(dot(N,V),0.0f);
    float NdotL = std::max(dot(N,L),0.0f);
    float ggx1 = Schick_GGX(NdotV,k);
    float ggx2 = Schick_GGX(NdotL,k);
    return ggx1 * ggx2;
}

float MathUtils::Schick_Fresnel_F(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * std::pow(1.0 - cosTheta,5.0f);
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