#ifndef SERIKA_RENDERER_INCLUDE_ORBITCONTROLLER_H
#define SERIKA_RENDERER_INCLUDE_ORBITCONTROLLER_H

#include <glm/glm.hpp>

class FCamera;

constexpr float MIN_ORBIT_ARM_LENGTH = 1.f;

class OrbitController {
public:
    OrbitController(FCamera& camera);

    void update();
    void recalculateCenter();

    void panByPixels(double dx, double dy);
    void rotateByPixels(double dx, double dy);
    void zoomByPixels(double dx, double dy);

    void reset();

public:
    double zoomX = 0;
    double zoomY = 0;
    double rotateX = 0;
    double rotateY = 0;
    double panX = 0;
    double panY = 0;

private:
    FCamera &camera_;

    glm::vec3 INIT_EYE;
    glm::vec3 INIT_FOCUS;
    glm::vec3 INIT_UP;

    glm::vec3 eye_{};
    glm::vec3 center_{};
    glm::vec3 up_{};

    float armLength_ = 0.f;
    glm::vec3 armDir_{};

    const double motionEps = 0.001f;
    const double motionSensitivity = 1.2f;

    float panSensitivity_ = 0.1f;
    float zoomSensitivity_ = 0.2f;
    float rotateSensitivity_ = 0.2f;
};

#endif // SERIKA_RENDERER_INCLUDE_ORBITCONTROLLER_H
