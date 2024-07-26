#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>

#include "Base/Globals.h"

std::vector<char> readFile(const std::string& filename);
void logDebug(const std::string& msg);
const std::string getFileExtension(const std::string& filename);
GLenum glCheckError_(const char* file, int line);

std::string printVec3(const glm::vec3& vec);
void printVec3(const ::std::string &name, const glm::vec3& vec);
void printMat4(const std::string& name, glm::mat4 mat);


namespace MathUtils {
    float get_random_float(float lowerBound = 0.f, float upperBound = 1.f);
    bool solveQuadratic(const float &a, const float &b, const float &c, float &smallerRoot, float &greaterRoot);
    glm::vec3 toWorld(const glm::vec3 &local, const glm::vec3 &N);
}

namespace StringUtils {
    bool stringEndsWith(const std::string& str, const std::string& suffix);
    bool stringStartsWith(const std::string &str, const std::string &prefix);
    bool isDirectory(const std::string& str);
    std::string appendToDir(const std::string& dirPath, const std::string& fileName);
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

#define TEST_TIME_COST(stmt, msg) \
    auto start_##msg = std::chrono::steady_clock::now();\
    stmt; \
    auto end_##msg = std::chrono::steady_clock::now();\
    std::cout << #msg << " - Time cost: " << std::chrono::duration_cast<std::chrono::microseconds>(end_##msg - start_##msg).count() << "microseconds." << std::endl;\


