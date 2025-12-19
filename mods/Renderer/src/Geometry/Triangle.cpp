#include "Geometry/Triangle.h"

Triangle::Triangle() = default;

Triangle::~Triangle() = default;

Triangle::Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<FMaterial> &m)
    : v0_(v0), v1_(v1), v2_(v2), material_(m)
{
    e1_ = v1_ - v0_;
    e2_ = v2_ - v0_;
    normal_ = glm::normalize(glm::cross(e1_, e2_));
    area_ = glm::length(glm::cross(e1_, e2_)) * 0.5f;
}

Triangle_Encoded Triangle::encode() const {
    Triangle_Encoded triangle_encoded{};
    if (auto&& mat = material_.lock()) {
        // geomerty
        triangle_encoded.v0_ = v0_;
        triangle_encoded.v1_ = v1_;
        triangle_encoded.v2_ = v2_;
        triangle_encoded.n0_ = n0_;
        triangle_encoded.n1_ = n1_;
        triangle_encoded.n2_ = n2_;
        // matrial
        triangle_encoded.emissive_ = mat->getEmission();
        triangle_encoded.baseColor_ = mat->getDiffuse();

    }
    else {
        LOGE("Triangle has no material when encoded!");
    }
    return {};;
}

Triangle & Triangle::deconde(const Triangle_Encoded &triangle_encoded) {
    return *this;
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

    itsc.bHit = true;
    itsc.traceDistance = trace_t;
    itsc.normal= normal_;
    itsc.impactPoint = ray.origin + ray.direction * trace_t;
    itsc.material = material_;
    itsc.primitive = shared_from_this();

    return itsc;
}

BoundingBox Triangle::getBounds() const { return BoundingBox(v0_, v1_).merge(v2_); }

void Triangle::Sample(Intersection &pos, float &pdf) {
    float x = std::sqrt(MathUtils::SobolGlobalIndex(0)); // \sqrt(r1)
    float y = MathUtils::SobolGlobalIndex(0);    // r2
    pos.impactPoint = v0_ * (1.0f - x) + v1_ * (x * (1.0f - y)) + v2_ * (x * y);
    pos.texCoords = t0_ * (1.f - x) + t1_ * (x * (1.f - y)) + t2_ * (x * y);
    pos.normal = this->normal_;
    pdf = 1.0f / area_;

    // update material_info_ here
    pos.material = material_;
}

float Triangle::getArea() const {
    return area_;
}

bool Triangle::hasEmit() const {
    if (auto&& mat = material_.lock()) {
        return mat->hasEmission();
    }
    return {};
}

void Triangle::transform(const glm::mat4 &modelMatrix) {
    glm::vec4 tmp = modelMatrix * glm::vec4(v0_, 1.f);
    v0_ = tmp;
    v1_ = modelMatrix * glm::vec4(v1_, 1.f);
    v2_ = modelMatrix * glm::vec4(v2_, 1.f);

    e1_ = v1_ - v0_;
    e2_ = v2_ - v0_;
    normal_ = glm::normalize(glm::cross(e1_, e2_));
    area_ = glm::length(glm::cross(e1_, e2_)) * 0.5f;
}
