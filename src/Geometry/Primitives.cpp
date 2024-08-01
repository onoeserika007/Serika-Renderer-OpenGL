#include "Geometry/Primitives.h"
#include <vector>
#include "../../include/Geometry/BufferAttribute.h"
#include "Geometry/Geometry.h"
#include "Geometry/UMesh.h"

namespace MeshMakers {
    std::shared_ptr<UMesh> loadCubeMesh(bool bReverseFace) {
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

    std::shared_ptr<UMesh> loadSphereMesh(float radius, int widthSegments, int heightSegments, float phiStart, float phiLength, float thetaStart, float
                                          thetaLength) {
        widthSegments = std::max(3, widthSegments);
        heightSegments = std::max(2, heightSegments);
        float thetaEnd = std::min(thetaStart + thetaLength, glm::pi<float>());

        unsigned int index = 0;
        std::vector<float> vertices, normals, uvs;
        std::vector<unsigned int> indices;

        std::vector<std::vector<unsigned int>> grid;

        for (int iy = 0; iy <= heightSegments; ++iy) {
            std::vector<unsigned int> verticesRow;
            float v = static_cast<float>(iy) / heightSegments;

            float uOffset = 0;
            if (iy == 0 && thetaStart == 0) {
                uOffset = 0.5f / widthSegments;
            } else if (iy == heightSegments && thetaEnd == glm::pi<float>()) {
                uOffset = -0.5f / widthSegments;
            }

            for (int ix = 0; ix <= widthSegments; ++ix) {
                float u = static_cast<float>(ix) / widthSegments;

                // Vertex
                glm::vec3 vertex;
                vertex.x = -radius * std::cos(phiStart + u * phiLength) * std::sin(thetaStart + v * thetaLength);
                vertex.y = radius * std::cos(thetaStart + v * thetaLength);
                vertex.z = radius * std::sin(phiStart + u * phiLength) * std::sin(thetaStart + v * thetaLength);
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);

                // Normal
                glm::vec3 normal = glm::normalize(vertex);
                normals.push_back(normal.x);
                normals.push_back(normal.y);
                normals.push_back(normal.z);

                // UV
                uvs.push_back(u + uOffset);
                uvs.push_back(1.0f - v);

                verticesRow.push_back(index++);
            }
            grid.push_back(verticesRow);
        }

        // Indices
        for (int iy = 0; iy < heightSegments; ++iy) {
            for (int ix = 0; ix < widthSegments; ++ix) {
                unsigned int a = grid[iy][ix + 1];
                unsigned int b = grid[iy][ix];
                unsigned int c = grid[iy + 1][ix];
                unsigned int d = grid[iy + 1][ix + 1];

                if (iy != 0 || thetaStart > 0) indices.push_back(a), indices.push_back(b), indices.push_back(d);
                if (iy != heightSegments - 1 || thetaEnd < glm::pi<float>()) indices.push_back(b), indices.push_back(c), indices.push_back(d);
            }
        }

        BufferAttribute PosAttr(vertices, 3);
        BufferAttribute TexAttr(uvs, 2);
        BufferAttribute NormAttr(normals, 3);

        auto sphereGeom = std::make_shared<FGeometry>();
        sphereGeom->setAttribute(EBA_Position, PosAttr, true);
        sphereGeom->setAttribute(EBA_TexCoord, TexAttr);
        sphereGeom->setAttribute(EBA_Normal, NormAttr);
        sphereGeom->setIndex(indices);

        return UMesh::makeMesh(sphereGeom, std::make_shared<FMaterial>());
    }
}

