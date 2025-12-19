#ifndef SERIKA_RENDERER_INCLUDE_UTILS_UNIVERSALUTILS_H
#define SERIKA_RENDERER_INCLUDE_UTILS_UNIVERSALUTILS_H
#include <vector>
#include <string>
#include <glm/glm.hpp>

std::vector<char> readFile(const std::string& filename);
void logDebug(const std::string& msg);
const std::string getFileExtension(const std::string& filename);

std::string printVec3(const glm::vec3& vec);
void printVec3(const ::std::string &name, const glm::vec3& vec);
void printMat4(const std::string& name, glm::mat4 mat);

namespace Utils {
    void UpdateProgress(float progress);
}

namespace MathUtils {
    float get_random_float(float lowerBound = 0.f, float upperBound = 1.f);
    bool solveQuadratic(const float &a, const float &b, const float &c, float &smallerRoot, float &greaterRoot);
    // I按照入射方向，返回出射方向
    glm::vec3 reflect(const glm::vec3 &I, const glm::vec3 &N);

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    glm::vec3 refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior);

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const glm::vec3 &I, const glm::vec3 &N, const float &ior, float &kr);

    glm::vec3 toWorld(const glm::vec3 &local, const glm::vec3 &N);
    void UniformHemisphereSample(glm::vec3& outWo, float& outPdf, const glm::vec3& inWi, const glm::vec3& inN, int depth, int SobolIndex);
    void UniformHemisphereSampleByVolume(glm::vec3& outWo, float& outPdf, const glm::vec3& inWi, const glm::vec3& inN, bool
                                         bNearOrigin, int depth, int SobolIndex);
    // normal distibution function 法线分布函数
    // a 一般用roughness
    float Trowbridge_Reitz_GGX_D(const glm::vec3& normal, const glm::vec3& halfVector, float a);

    // 自遮挡
    float Schick_GGX(float NdotV,float k);
    float Schick_GGXSmith_G(const glm::vec3& N,const glm::vec3& V,const glm::vec3& L,float k);
    // 菲涅尔项的一种近似
    float Schick_Fresnel_F(float cosTheta,float F0);

    /**
     * Low Descrepancy
     */
    float IntegerRadicalInverse(int Base, int i);
    double RadicalInverse(int Base, int i);
    // Eratosthenes to generate prime
    int NthPrimeNumber(int n);
    std::vector<int> sieve_of_eratosthenes(int limit);
    std::vector<int> generate_coprimes(int n);
    double Halton(int Dimension, int Index);
    double Hammersley(int Dimension, int Index, int NumSamples);

    // sobel
    glm::uint grayCode(glm::uint i);
    // 生成第 dimension 维度的第 i 个 sobol 数
    double Sobol(unsigned int Dimension, unsigned int i);
    double SobolGlobalIndex(unsigned int Dimension);
}

namespace StringUtils {
    bool stringEndsWith(const std::string& str, const std::string& suffix);
    bool stringStartsWith(const std::string &str, const std::string &prefix);
    bool isDirectory(const std::string& str);
    std::string appendToDir(const std::string& dirPath, const std::string& fileName);
}

#define TEST_TIME_COST(stmt, msg) \
    auto start_##msg = std::chrono::steady_clock::now();\
    stmt; \
    auto end_##msg = std::chrono::steady_clock::now();\
    std::cout << #msg << " - Time cost: " << std::chrono::duration_cast<std::chrono::microseconds>(end_##msg - start_##msg).count() << "microseconds." << std::endl;\
    std::cout << std::flush;\

#endif // SERIKA_RENDERER_INCLUDE_UTILS_UNIVERSALUTILS_H
