#include <string>
#include "Material.h"
#include "Texture.h"
//#include "Shader.h"
#include "ShaderGLSL.h"
#include "Renderer.h"

StandardMaterial::StandardMaterial()
{
}

StandardMaterial::StandardMaterial(const std::string& name)
{
}

void StandardMaterial::setupPipeline(Renderer& renderer)
{
	renderer.setupMaterial(*this);
}
