#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "ULight.h"
#include "Base/GLMInc.h"
#include "Utils/utils.h"

 PhongMaterial::PhongMaterial() : ambient_(1.0f), diffuse_(1.0f), specular_(1.0f), shininess_(1.0f) {}

 PhongMaterial::PhongMaterial(std::string name, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
	: ambient_(ambient), diffuse_(diffuse), specular_(specular), shininess_(shininess), Material(name, nullptr) {
	init();
}

 void PhongMaterial::init() {
}

 void PhongMaterial::print()
 {
	 auto pshader = getpshader();
	 std::cout << "Material name: " << getName() << std::endl;
	 std::cout << "Shader ID: " << pshader->getId() << std::endl;
	 std::cout << "Ambient: ";
	 printVec3(ambient_);
	 std::cout << "Diffuse: ";
	 printVec3(diffuse_);
	 std::cout << "Specular: ";
	 printVec3(specular_);
	 std::cout << "Shininess: " << shininess_ << std::endl;
 }



 void PhongMaterial::setName(const std::string name) {
	Material::setName(name);
	init();
}
