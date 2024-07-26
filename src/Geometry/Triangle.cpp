#include "Geometry/Triangle.h"

Triangle::Triangle() = default;

Triangle::~Triangle() = default;

Triangle::Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const MaterialInfo &m): v0_(v0), v1_(v1), v2_(v2), material_info_(m) {
    e1_ = v1_ - v0_;
    e2_ = v2_ - v0_;
    normal_ = glm::normalize(glm::cross(e1_, e2_));
    area_ = glm::length(glm::cross(e1_, e2_)) * 0.5f;
}

bool Triangle::intersect(const Ray &ray) { return true; }

Intersection Triangle::getIntersection(const Ray &ray) {
    Intersection itsc;

    if (glm::dot(ray.direction, normal_) > 0) // semukeru
        return itsc;

    // see moller Trumbore Algorithm
    // O + tD = (1 - u - v) P0 + u P1 + v P2;
    float u, v, trace_t = 0;
    glm::vec3 s1Vec = glm::cross(ray.direction, e2_);
    float det = glm::dot(e1_, s1Vec);

    if (fabs(det) < M_EPSILON) return itsc; // early return

    glm::vec3 sVec = ray.origin - v0_;
    glm::vec3 s2Vec = glm::cross(sVec, e1_);
    float det_inv = 1.f / det;


    u = glm::dot(sVec, s1Vec) * det_inv;
    if (u < 0 || u > 1) return itsc; // early return
    v = glm::dot(ray.direction, s2Vec) * det_inv;
    if (v < 0 || u + v > 1) return itsc; // early return
    trace_t = glm::dot(e2_, s2Vec) * det_inv;

    if(trace_t < 0) return itsc; // early return

    //
    // Triangle hit, begin to sample from material, and update material info
    //
    glm::vec2 texCoord = (1 - u - v) * t0_ + u * t1_ + v * t2_;
    if (auto&& mat = material_.lock()) {
        material_info_.emission = mat->getEmission();
        material_info_.shading_model = mat->shadingMode();
        material_info_.Kd = glm::vec3(mat->sample2D<RGBA>(texCoord.x, texCoord.y, TextureType::TEXTURE_TYPE_DIFFUSE, FilterMode::Filter_NEAREST));
        material_info_.Ks = glm::vec3(mat->sample2D<RGBA>(texCoord.x, texCoord.y, TextureType::TEXTURE_TYPE_SPECULAR, FilterMode::Filter_NEAREST));
        material_info_.specularExponent = 32.f;
    }

    itsc.bHit = true;
    itsc.traceDistance = trace_t;
    itsc.normal= normal_;
    itsc.impactPoint = ray.origin + ray.direction * trace_t;
    itsc.material = material_info_;

    return itsc;
}

BoundingBox Triangle::getBounds() const { return BoundingBox(v0_, v1_).merge(v2_); }

void Triangle::Sample(Intersection &pos, float &pdf) {
    float x = std::sqrt(MathUtils::get_random_float()); // \sqrt(r1)
    float y = MathUtils::get_random_float();    // r2
    pos.impactPoint = v0_ * (1.0f - x) + v1_ * (x * (1.0f - y)) + v2_ * (x * y);
    pos.texCoords = t0_ * (1.f - x) + t1_ * (x * (1.f - y)) + t2_ * (x * y);
    pos.normal = this->normal_;
    pdf = 1.0f / area_;

    // update material_info_ here
    if (auto&& mat = material_.lock()) {
        material_info_.emission = mat->getEmission();
        material_info_.shading_model = mat->shadingMode();
        material_info_.Kd = glm::vec3(mat->sample2D<RGBA>(pos.texCoords.x, pos.texCoords.y, TextureType::TEXTURE_TYPE_DIFFUSE, FilterMode::Filter_NEAREST));
        material_info_.Ks = glm::vec3(mat->sample2D<RGBA>(pos.texCoords.x, pos.texCoords.y, TextureType::TEXTURE_TYPE_SPECULAR, FilterMode::Filter_NEAREST));
        material_info_.specularExponent = 32.f;
    }
}

float Triangle::getArea() const {
    return area_;
}

bool Triangle::hasEmit() const {
    return material_info_.hasEmission();
}