#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>

class Object {
	std::shared_ptr<Geometry> geometry_;
	std::unordered_map<std::string, unsigned> buffer_map_;
	unsigned VAO;
	void BindObjectBuffer() {
		if (!VAO) {
			glGenVertexArrays(1, &VAO);
		}
	}
public:
	Object(std::shared_ptr<Geometry> geometry): geometry_(geometry), VAO(0) {}
};