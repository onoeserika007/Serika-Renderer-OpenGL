#include "Utils/UniversalUtils.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <random>

#include "Base/Globals.h"

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
void MathUtils::UniformHemisphereSample(glm::vec3 &outWo, float &outPdf, const glm::vec3 &inWi, const glm::vec3 &inN, int depth, int SobolIndex) {
    // uniform sample on the hemisphere
    // float x_1 = get_random_float(), x_2 = get_random_float();
    float x_1, x_2;
    if (SobolIndex <= -1) {
        x_1 = get_random_float(), x_2 = get_random_float();
    }
    else {
        x_1 = Sobol(depth * 2, SobolIndex);
        x_2 = Sobol(depth * 2 + 1, SobolIndex);
    }

    // float x_1 = MathUtils::SobolGlobalIndex(0), x_2 = MathUtils::SobolGlobalIndex(1);
    float z = std::fabs(1.0f - 2.0f * x_1);
    float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI_DEFINED * x_2;
    glm::vec3 localRay(r*std::cos(phi), r*std::sin(phi), z);
    outWo = toWorld(localRay, inN);

    if (glm::dot(outWo, inN) > 0.f) outPdf = 0.5f / M_PI_DEFINED;
    else outPdf = 0.f; // what if hit back
}

void MathUtils::UniformHemisphereSampleByVolume(glm::vec3 &outWo, float &outPdf, const glm::vec3 &inWi,
                                                const glm::vec3 &inN, bool bNearOrigin, int depth, int SobolIndex) {
    // float x_1 = get_random_float(), x_2 = get_random_float();
    float x_1, x_2, x_3;
    if (SobolIndex <= -1) {
        x_1 = get_random_float(), x_2 = get_random_float(), x_3 = get_random_float();
    }
    else {
        x_1 = Sobol(depth * 3, SobolIndex);
        x_2 = Sobol(depth * 3 + 1, SobolIndex);
        x_3 = Sobol(depth * 3 + 2, SobolIndex);
    }

    // float x_1 = MathUtils::SobolGlobalIndex(0), x_2 = MathUtils::SobolGlobalIndex(1);
    float z = std::fabs(1.0f - 2.0f * x_1);
    float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI_DEFINED * x_2;
    glm::vec3 localRay(r*std::cos(phi), r*std::sin(phi), z);
    outWo = toWorld(localRay, inN);

    if (bNearOrigin) {
        float scale = glm::mix(0.1f, 1.f, x_3 * x_3);
        outWo *= scale;
    }

    if (glm::dot(outWo, inN) > 0.f) outPdf = 0.5f / M_PI_DEFINED;
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
    denom = M_PI_DEFINED * denom * denom;
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

float MathUtils::IntegerRadicalInverse(int Base, int i) {
    int numPoints = 1; // 代表了数位的空间大小，用于转化到小数部分
    int inverse;
    // 此循环将i在"Base"进制下的数字左右Flip
    for(inverse = 0; i > 0; i /= Base) {
        inverse = inverse * Base + (i % Base);
        numPoints = numPoints * Base;
    }

    // 除以Digit将这个数镜像到小数点右边
    return inverse / (float) numPoints;
}

double MathUtils::RadicalInverse(int Base, int i) {
    double Digit, Radical, Inverse;
    Digit = Radical = 1.0 / (double) Base;
    Inverse = 0.0;
    while(i) {
        // i余Base求出i在"Base"进制下的最低位的数
        // 乘以Digit将这个数镜像到小数点右边
        Inverse += Digit * (double) (i % Base);
        Digit *= Radical;

        // i除以Base即可求右一位的数
        i /= Base;
    }
    return Inverse;
}

// 使用埃拉托斯特尼筛法生成质数
std::vector<int> MathUtils::sieve_of_eratosthenes(int limit) {
    std::vector<bool> is_prime(limit + 1, true);
    std::vector<int> primes;
    //  p * p <= limit是因为最大可能质数limit的因数必然小于等于sqrt limit，找因数作为筛选起点只用在这个范围找就行了
    for (int p = 2; p * p <= limit; p++) {
        if (is_prime[p]) {
            // 用i去筛的时候，2~i-1必定都筛过了，现在要筛的k * i中的2~i-1 * i部分至少能被前面的一个索引整除，所以筛掉了
            for (int i = p * p; i <= limit; i += p) {
                is_prime[i] = false;
            }
        }
    }
    for (int p = 2; p <= limit; p++) {
        if (is_prime[p]) {
            primes.push_back(p);
        }
    }
    return primes;
}

std::vector<int> MathUtils::generate_coprimes(int n) {
    static std::vector<int> coprimes;
    if (n > coprimes.size()) {
        // regenerate
        coprimes.clear();
        int estimated_limit = 10 * n; // 估计的质数数量上限
        std::vector<int> primes = sieve_of_eratosthenes(estimated_limit);
        for (int i = 0; i < primes.size() && coprimes.size() < n; i++) {
            coprimes.push_back(primes[i]);
        }
    }

    return coprimes;
}

double MathUtils::Halton(int Dimension, int Index) {
    // 直接用第Dimension个质数作为底数调用RadicalInverse即可
    return RadicalInverse(NthPrimeNumber(Dimension+1), Index);
}

double MathUtils::Hammersley(int Dimension, int Index, int NumSamples) {
    // Hammersley需要事先确定样本的总数
    if (Dimension == 0)
        return Index / (double) NumSamples;
    else
        return RadicalInverse(NthPrimeNumber(Dimension-1+1), Index);
}

glm::uint MathUtils::grayCode(glm::uint i) {
    return i ^ (i>>1);
}

// V[d] 表示第 d 维度的生成矩阵
constexpr glm::uint SobelMatrix[8][32] = {
    2147483648, 1073741824, 536870912, 268435456, 134217728, 67108864, 33554432,
16777216, 8388608, 4194304, 2097152, 1048576, 524288, 262144, 131072, 65536,
32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1,
 2147483648, 3221225472, 2684354560, 4026531840, 2281701376, 3422552064,
2852126720, 4278190080, 2155872256, 3233808384, 2694840320, 4042260480,
2290614272, 3435921408, 2863267840, 4294901760, 2147516416, 3221274624,
2684395520, 4026593280, 2281736192, 3422604288, 2852170240, 4278255360,
2155905152, 3233857728, 2694881440, 4042322160, 2290649224, 3435973836,
2863311530, 4294967295,
 2147483648, 3221225472, 1610612736, 2415919104, 3892314112, 1543503872,
2382364672, 3305111552, 1753219072, 2629828608, 3999268864, 1435500544,
2154299392, 3231449088, 1626210304, 2421489664, 3900735488, 1556135936,
2388680704, 3314585600, 1751705600, 2627492864, 4008611328, 1431684352,
2147543168, 3221249216, 1610649184, 2415969680, 3892340840, 1543543964,
2382425838, 3305133397,
 2147483648, 3221225472, 536870912, 1342177280, 4160749568, 1946157056,
2717908992, 2466250752, 3632267264, 624951296, 1507852288, 3872391168,
2013790208, 3020685312, 2181169152, 3271884800, 546275328, 1363623936,
4226424832, 1977167872, 2693105664, 2437829632, 3689389568, 635137280,
1484783744, 3846176960, 2044723232, 3067084880, 2148008184, 3222012020,
537002146, 1342505107,
 2147483648, 1073741824, 536870912, 2952790016, 4160749568, 3690987520,
2046820352, 2634022912, 1518338048, 801112064, 2707423232, 4038066176,
3666345984, 1875116032, 2170683392, 1085997056, 579305472, 3016343552,
4217741312, 3719483392, 2013407232, 2617981952, 1510979072, 755882752,
2726789248, 4090085440, 3680870432, 1840435376, 2147625208, 1074478300,
537900666, 2953698205,
 2147483648, 1073741824, 1610612736, 805306368, 2818572288, 335544320,
2113929216, 3472883712, 2290089984, 3829399552, 3059744768, 1127219200,
3089629184, 4199809024, 3567124480, 1891565568, 394297344, 3988799488,
920674304, 4193267712, 2950604800, 3977188352, 3250028032, 129093376,
2231568512, 2963678272, 4281226848, 432124720, 803643432, 1633613396,
2672665246, 3170194367,
 2147483648, 3221225472, 2684354560, 3489660928, 1476395008, 2483027968,
1040187392, 3808428032, 3196059648, 599785472, 505413632, 4077912064,
1182269440, 1736704000, 2017853440, 2221342720, 3329785856, 2810494976,
3628507136, 1416089600, 2658719744, 864310272, 3863387648, 3076993792,
553150080, 272922560, 4167467040, 1148698640, 1719673080, 2009075780,
2149644390, 3222291575,
 2147483648, 1073741824, 2684354560, 1342177280, 2281701376, 1946157056,
436207616, 2566914048, 2625634304, 3208642560, 2720006144, 2098200576,
111673344, 2354315264, 3464626176, 4027383808, 2886631424, 3770826752,
1691164672, 3357462528, 1993345024, 3752330240, 873073152, 2870150400,
1700563072, 87021376, 1097028000, 1222351248, 1560027592, 2977959924, 23268898,
437609937
};

double MathUtils::Sobol(unsigned int Dimension, unsigned int i) {
    unsigned r = 0;
    // 将i依次右移，提取2进制里的每一位
    for (glm::uint k = 0; i; i >>= 1, k++)
        if (i & 1) // 若当前位为1，则用异或和矩阵相乘
            r ^= SobelMatrix[Dimension][k];
    return r / (double) (0xFFFFFFFF); // 除以2^M,移到小数点右边
}

double MathUtils::SobolGlobalIndex(unsigned int Dimension) {
    static int index = 0;
    index++;
    return Sobol(Dimension, index);
}

// N start from 1st
int MathUtils::NthPrimeNumber(int n) {
    auto&& NPrimes = generate_coprimes(n);
    assert(n <= NPrimes.size());
    return NPrimes[n-1];
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