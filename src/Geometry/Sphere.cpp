#include "Geometry/Sphere.h"

Sphere::Sphere(const glm::vec3 &center, float radius, const MaterialInfo &mt): center_(center), radius(radius), radius2(radius * radius), material_info_(mt), area(4 * M_PI * radius * radius) {}

Sphere::~Sphere() = default;

bool Sphere::intersect(const Ray &ray) {
    // analytic solution
    glm::vec3 L = ray.origin - center_;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2 * glm::dot(ray.direction, L);
    float c = glm::dot(L, L) - radius2;
    float t0, t1;
    float area = 4. * M_PI * radius2;
    if (!MathUtils::solveQuadratic(a, b, c, t0, t1)) return false;
    if (t0 < 0) t0 = t1;
    if (t0 < 0) return false;
    return true;
}

Intersection Sphere::getIntersection(const Ray &ray) {
    Intersection result;
    result.bHit = false;
    glm::vec3 L = ray.origin - center_;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2 * glm::dot(ray.direction, L);
    float c = glm::dot(L, L) - radius2;
    float t0, t1;
    if (!MathUtils::solveQuadratic(a, b, c, t0, t1)) return result;
    if (t0 < 0) t0 = t1;
    if (t0 < 0.5f) return result;
    result.bHit=true;

    result.impactPoint = ray.origin + ray.direction * t0;
    result.normal = glm::normalize(result.impactPoint - center_);
    result.material = material_info_;
    result.traceDistance = t0;
    return result;

}

BoundingBox Sphere::getBounds() const {
    return {center_ - glm::vec3(radius), center_ + glm::vec3(radius) };
}

void Sphere::Sample(Intersection &intersection, float &pdf) {
    float theta = 2.0 * M_PI * MathUtils::get_random_float();
    float phi = M_PI * MathUtils::get_random_float();
    glm::vec3 dir(std::cos(phi), std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta));
    intersection.impactPoint = center_ + radius * dir;
    intersection.normal = dir;
    intersection.emission = material_info_.hasEmission() ? material_info_.emission : glm::vec3(0.f);
    pdf = 1.0f / area;
}

float Sphere::getArea() const { return area; }

bool Sphere::hasEmit() const { return material_info_.hasEmission(); }
