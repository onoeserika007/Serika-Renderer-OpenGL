#ifndef UTILS_H
#define UTILS_H

#include <vector>;
#include <string>
#include <iostream>
#include <glad/glad.h>
#include "Base/GLMInc.h"
#include "Utils/Logger.h"


std::vector<char> readFile(const std::string& filename);
void logDebug(const std::string& msg);
const std::string getFileExtension(const std::string& filename);
GLenum glCheckError_(const char* file, int line);
void printVec3(const glm::vec3& vec);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 


#endif // !UTILS_H
