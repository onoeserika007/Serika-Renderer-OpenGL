#include "RenderPassOpenGL.h"
#include "Object.h"
#include "Shader.h"

void RenderPassOpenGL::render(Object& obj)
{
	auto pmaterial = obj.getpMaterial();
	//shaderProgram_ = pmaterial->getShader(shaderPass_);
	//shaderProgram_->use();
}
