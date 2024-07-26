#include "glm/glm.hpp"
#include <iostream>

void printVec3(const std::string &name, const glm::vec3& vec) {
    std::cout << name << vec[0] << " " << vec[1] << " " << vec[2] << " " << std::endl;
}

void debugUVec4(const glm::uvec4& vec) {
    std::cout << vec[0] << " " << vec[1] << " " << vec[2] << " " <<  vec[3] << " " << std::endl;
}

void debugUVec3(const glm::uvec3& vec) {
    std::cout << vec[0] << " " << vec[1] << " " << vec[2] << " " << std::endl;
}


int main () {
    glm::vec3 v1 = {1.f, 8.f, 6.f};
    glm::vec3 v2 = {4.f, 5.f, 9.f};
    // printVec3("Elem max: ", glm::max(v1, v2));
    // std::cout << "Elem max: " << v1
    glm::uvec4 v3 {4, 5, 6, 7};
    glm::uvec3 v4 {8, 9, 10};
    debugUVec4(glm::vec4(v3) * 0.5f);
    debugUVec3(glm::vec3(v4) * 0.5f);
    return 0;
}
