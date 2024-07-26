#include "Geometry/Primitives.h"
#include <vector>
#include "../../include/Geometry/BufferAttribute.h"
#include "Geometry/Geometry.h"
#include "Geometry/UMesh.h"

namespace Cube {
    std::shared_ptr<UMesh> Cube::loadCubeMesh(bool bReverseFace) {
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
}
