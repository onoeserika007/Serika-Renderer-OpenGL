#include "glm/glm.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Utils/ImageUtils.h"
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

void ImageReadAndWriteTest() {
    std::string path = "assets/models/cyborg/cyborg_diffuse.png";
    auto&& imageRead = ImageUtils::readImageRGBA(path, false);
    ImageUtils::writeImage("./readImg.png", imageRead, false);

    auto&& copiedOne = Buffer<glm::vec4>::makeBuffer();
    copiedOne->copyFrom(*imageRead);

    auto&& writeImg = Buffer<RGBA>::makeBuffer();
    writeImg->copyFrom(*copiedOne);
    ImageUtils::writeImage("./writeImg.png", writeImg, false);
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

void reApplyModelMatrixTest() {
    // 注意：glm的默认构造是列主序
    auto model = glm::mat4(1.0f);
    // Apply random transformations (scale, rotate, translate)
    // 看源码可以发现，原矩阵是放在左边的，
    model = glm::scale(model, glm::vec3(1.5f, 2.0f, 0.5f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(3.12f, -1.56f, 4.89f));
    // reApplyModelMatrix(model);
    printMat4("Origin: ", model);

    glm::vec3 scale;
    glm::vec3 translation;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(model, scale, orientation, translation, skew, perspective);

    glm::mat4 newModel{1.f};
    newModel = glm::scale(newModel, scale);
    newModel = newModel * glm::mat4_cast(orientation);
    newModel = glm::translate(newModel, translation);
    printMat4("After: ", model);

}

int main () {
    glm::vec3 v1 = {1.f, 8.f, 6.f};
    glm::vec3 v2 = {4.f, 5.f, 9.f};
    // printVec3("Elem max: ", glm::max(v1, v2));
    // std::cout << "Elem max: " << v1
    glm::uvec4 v3 {4, 5, 6, 0};
    glm::uvec3 v4 {8, 9, 10};
    debugUVec4(glm::vec4(v3) * 0.5f);
    debugUVec3(glm::vec3(v4) * 0.5f);

    reApplyModelMatrixTest();

    ImageReadAndWriteTest();
    return 0;
}
