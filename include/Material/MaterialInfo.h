#pragma once
#include "Base/Globals.h"
#include "Utils/UniversalUtils.h"

struct MaterialInfo {
    EShadingModel shading_model;
    glm::vec3 emission;
    float ior; // 折射率
    glm::vec3 Kd = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 Ks = glm::vec3(0.7f);
    float specularExponent = 32.f;

    bool hasEmission() const { return glm::length(emission) > 0.f; }

    // wi represent ViewDir, incident // wo represents LightDir, goes off the place
    glm::vec3 evalRadiance(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) {
        switch(shading_model){
            case Shading_BaseColor:
            case Shading_Lambertian: {
                // calculate the contribution of diffuse model
                return dot(N, wo) > 0.f? Kd / M_PI : glm::vec3(0.f);
            }
            case Shading_BlinnPhong: {
                // calculate the contribution of diffuse model
                float cosalpha = dot(N, wo) > 0.f;
                if (cosalpha > 0.f) {
                    glm::vec3 f_r {};
                    glm::vec3 V = -wi;
                    glm::vec3 L = wo;
                    glm::vec3 H = glm::normalize(V + L);
                    f_r += Kd / M_PI;
                    f_r += Ks * (specularExponent + 2) / (2.f * M_PI) * pow(dot(H, N), specularExponent);
                    return f_r;
                }
                else {
                    return {};
                }
            }
            case Shading_PBR: {
                float cosalpha = glm::dot(N, wo);
                if (cosalpha > M_EPSILON) {
                    float roughness = 0.02f;
                    glm::vec3 V = -wi;
                    glm::vec3 L = wo;
                    glm::vec3 H = glm::normalize(V + L);
                    float D = MathUtils::Trowbridge_Reitz_GGX_D(N, H, roughness);
                    float G = MathUtils::Schick_GGXSmith_G(N, V, L, roughness);
                    float F = MathUtils::Schick_Fresnel_F(cosalpha, 0.50f);
                    float diffsue = glm::dot(N,V)+0.5f;
                    float divide = 1.0f / ( 4.f * std::max(glm::dot(N, L), M_EPSILON) * std::max(glm::dot(N, V), M_EPSILON));
                    float Specular = D * F * G * divide;
                    //std::cout << D <<"  " << G <<"  "<< F<<"  "<< divide<<"  "<< Specular<<std::endl;
                    return diffsue * Kd / M_PI + Ks * Specular;
                }
                else return {};
            }
            default: {
                return {};
            }
        }
    }
};
