#pragma once
#include <iostream>
#include "Base/Globals.h"
#include "Utils/UniversalUtils.h"
#include "Material/MaterialInfo.h"

class Intersectable;

struct Intersection {
    bool bHit = false;
    glm::vec3 impactPoint {};
    glm::vec2 texCoords {};
    glm::vec3 normal {};
    float traceDistance = FLOAT_MAX;
    MaterialInfo material{};
    std::weak_ptr<Intersectable> primitive;
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