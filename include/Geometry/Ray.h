#pragma once
#include <iostream>
#include "Base/Globals.h"
#include "Material/FMaterial.h"
#include "Utils/UniversalUtils.h"

class FMaterial;
class Intersectable;

struct Intersection {
    bool bHit = false;
    glm::vec3 impactPoint {};
    glm::vec2 texCoords {};
    glm::vec3 normal {};
    float traceDistance = FLOAT_MAX;
    std::weak_ptr<FMaterial> material{};
    std::weak_ptr<Intersectable> primitive{};

    NO_DISCARD bool hasEmission() const {
        if (auto&& mat = material.lock()) {
            return mat->hasEmission();
        }
        return false;
    }

    NO_DISCARD glm::vec3 getEmission() const {
        if (auto&& mat = material.lock()) {
            return mat->getEmission();
        }
        return {};
    }

    glm::vec3 evalRadiance(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) {
        if (auto&& mat = material.lock()) {
            return mat->evalRadiance(wi, wo, N, texCoords.x, texCoords.y);
        }
        return {};
    }

};

struct Ray{
    // Destination = origin + t * direction
    glm::vec3 origin;
    glm::vec3 direction, direction_inv{};
    float t;   //transportation time,
    float t_min, t_max;

    Ray(const glm::vec3& ori, const glm::vec3& dir, const float _t = 0.0): origin(ori), direction(dir),t(_t) {
        direction_inv = glm::vec3(1.f / direction.x, 1.f / direction.y, 1.f / direction.z);
        t_min = 0.0;
        t_max = FLOAT_MAX;
    }

    void setDirection(const glm::vec3& dir) {
        direction = dir;
        direction_inv = 1.f / dir;
    }

    glm::vec3 operator() (const float t) const{return origin + direction * t;}

    friend std::ostream &operator<< (std::ostream& os, const Ray& r) {
        os << "[origin:=" << printVec3(r.origin) <<", direction=" << printVec3(r.direction) << ", time=" << r.t<< "]\n";
        return os;
    }
};