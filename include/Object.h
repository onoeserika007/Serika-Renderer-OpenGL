#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>

class Object {
	std::shared_ptr<Geometry> pgeometry_;
	std::shared_ptr<Shader> pshader_;
	std::unordered_map<std::string, unsigned> buffer_map_;
	unsigned VAO;
	void BindObjectBuffer() {
		if (!VAO) {
			glGenVertexArrays(1, &VAO);
		}
	}
public:
	Object(std::shared_ptr<Geometry> pgeometry): pgeometry_(pgeometry), VAO(0) {}
};