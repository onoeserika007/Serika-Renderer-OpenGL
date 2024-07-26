#pragma once
#include <iostream>
#include "Base/Globals.h"
#include "Utils/UniversalUtils.h"

class FMaterial;
class Intersectable;

struct Intersection {
    bool bHit = false;
    glm::vec3 impactPoint {};
    glm::vec2 texCoords {};
    glm::vec3 normal {};
    glm::vec3 emission {};
    float traceDistance = 0.f;
    MaterialInfo material{};
};

struct Ray{
    // Destination = origin + t * direction
    glm::vec3 origin;
    glm::vec3 direction, direction_inv{};
    double t;   //transportation time,
    double t_min, t_max;

    Ray(const glm::vec3& ori, const glm::vec3& dir, const double _t = 0.0): origin(ori), direction(dir),t(_t) {
        direction_inv = glm::vec3(1.f / direction.x, 1.f / direction.y, 1.f / direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    glm::vec3 operator() (const float t) const{return origin + direction * t;}

    friend std::ostream &operator<< (std::ostream& os, const Ray& r) {
        os << "[origin:=" << printVec3(r.origin) <<", direction=" << printVec3(r.direction) << ", time=" << r.t<< "]\n";
        return os;
    }
};