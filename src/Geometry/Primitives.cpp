#include "Geometry/Primitives.h"
#include <vector>
#include "../../include/Geometry/BufferAttribute.h"
#include "Geometry/Geometry.h"
#include "Geometry/UMesh.h"

namespace MeshMakers {
    std::shared_ptr<UMesh> MeshMakers::loadCubeMesh(bool bReverseFace) {
        std::vector<float> cubePosArray(CubeVertices, CubeVertices + sizeof(CubeVertices) / sizeof(float));
        std::vector<float> cubeUvArray(CubeUVs, CubeUVs + sizeof(CubeUVs) / sizeof(float));
        std::vector<float> cubeNormalArray(CubeNormals, CubeNormals + sizeof(CubeNormals) / sizeof(float));

        BufferAttribute cubePosAttribute(cubePosArray, 3), cubeUvAttribute(cubeUvArray, 2), cubeNormalAttribute(cubeNormalArray, 3);

        auto cubeGeometry = std::make_shared<FGeometry>();
        cubeGeometry->setAttribute(EBA_Position, cubePosAttribute, true);
        cubeGeometry->setAttribute(EBA_TexCoord, cubeUvAttribute);
        cubeGeometry->setAttribute(EBA_Normal, cubeNormalAttribute);
        if (!bReverseFace) {
            cubeGeometry->setIndex({CubeIndices, CubeIndices + sizeof(CubeIndices) / sizeof(unsigned)});
        }
        else {
            cubeGeometry->setIndex({CubeReverseFaceIndices, CubeReverseFaceIndices + sizeof(CubeReverseFaceIndices) / sizeof(unsigned)});
        }
        return UMesh::makeMesh(cubeGeometry, std::make_shared<FMaterial>());
    }

    std::shared_ptr<UMesh> loadTriangleMesh(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) {

        auto e1 = v1 - v0;
        auto e2 = v2 - v0;
        auto normal = glm::normalize(glm::cross(e2, e1));
        std::vector<glm::vec3> vertices;
        vertices.emplace_back(v0);
        vertices.emplace_back(v1);
        vertices.emplace_back(v2);
        std::vector<glm::vec3> normals;
        normals.emplace_back(normal);
        normals.emplace_back(normal);
        normals.emplace_back(normal);

        std::vector<float> vertexArray;
        for (auto && v: vertices) {
            vertexArray.push_back(v.x);
            vertexArray.push_back(v.y);
            vertexArray.push_back(v.z);
        }

        std::vector<float> normalArray;
        for (auto&& n: normals) {
            normalArray.emplace_back(n.x);
            normalArray.emplace_back(n.y);
            normalArray.emplace_back(n.z);
        }

        BufferAttribute PosAttr(vertexArray, 3);
        BufferAttribute NormAttr(normalArray, 3);
        std::vector<unsigned> indices = {0, 2, 1};

        auto triangleGeom = std::make_shared<FGeometry>();
        triangleGeom->setAttribute(EBA_Position, PosAttr, true);
        triangleGeom->setAttribute(EBA_Normal, NormAttr);
        triangleGeom->setIndex(indices);

        return UMesh::makeMesh(triangleGeom, std::make_shared<FMaterial>());
    }
}

