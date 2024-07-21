#include "stb/stb_image.h"
#include "Utils//utils.h"
#include "Utils/ImageUtils.h"
#include "glad/glad.h"
#include "Texture.h"
#include "Shader.h"
#include <iostream>
#include "Base/GLMInc.h"
#include "../include/Base/ResourceLoader.h"
#include "Renderer.h"

const char* SamplerDefinesToTextureType[] = {
	"NONE_MAP",
	"DIFFUSE_MAP",
	"SPECULAR_MAP",
	"AMBIENT_MAP",
	"EMISSIVE_MAP",
	"HEIGHT_MAP",
	"NORMALS_MAP",
	"SHININESS_MAP",
	"OPACITY_MAP",
	"DISPLACEMENT_MAP",
	"LIGHTMAP_MAP",
	"REFLECTION_MAP",
	"BASE_COLOR_MAP",
	"NORMAL_CAMERA_MAP",
	"EMISSION_COLOR_MAP",
	"METALNESS_MAP",
	"DIFFUSE_ROUGHNESS_MAP",
	"AMBIENT_OCCLUSION_MAP",
	"UNKNOWN_MAP",
	"SHEEN_MAP",
	"CLEARCOAT_MAP",
	"TRANSMISSION_MAP",
	"SHADOW_MAP"
};

#define CASE_ENUM_STR(type) case type: return #type

// std::shared_ptr<Texture> Texture::createTexture2DDefault(int width, int height, TextureFormat format, TextureUsage usage)
// {
// 	TextureInfo info;
// 	info.width = width;
// 	info.height = height;
// 	info.format = format;
// 	info.usage = usage;
// 	info.target = TextureTarget_2D;
// 	return std::make_shared<Texture>(info);
// }

const char* Texture::materialTexTypeStr(TextureType usage)
{
	switch (usage)
	{
#define TEXTURETYPE_ENUM(x) case x: return #x;
#include "Base/TextureTypeEnum.def"
	default:
		break;
	}
	return "";
}

const char* Texture::samplerDefine(TextureType usage)
{
	auto len = sizeof(SamplerDefinesToTextureType) / sizeof(const char*);
	if (usage < len) {
		return SamplerDefinesToTextureType[usage];
	}
	return "";
}

const char* Texture::samplerName(TextureType usage)
{
	switch (usage) {
		case TEXTURE_NONE:				return "uNoneMap";
		case TEXTURE_DIFFUSE:			return "uDiffuseMap";
		case TEXTURE_SPECULAR:			return "uSpecularMap";
		case TEXTURE_AMBIENT:			return "uAmbientMap";
		case TEXTURE_EMISSIVE:			return "uEmissiveMap";
		case TEXTURE_HEIGHT:			return "uHeightMap";
		case TEXTURE_NORMALS:			return "uNormalsMap";
		case TEXTURE_SHININESS:			return "uShininessMap";
		case TEXTURE_OPACITY:			return "uOpacityMap";
		case TEXTURE_DISPLACEMENT:		return "uDisplacementMap";
		case TEXTURE_LIGHTMAP:			return "uLightmapMap";
		case TEXTURE_REFLECTION:		return "uReflectionMap";
		case TEXTURE_BASE_COLOR:		return "uBaseColorMap";
		case TEXTURE_NORMAL_CAMERA:		return "uNormalCameraMap";
		case TEXTURE_EMISSION_COLOR:	return "uEmissionColorMap";
		case TEXTURE_METALNESS:			return "uMetalnessMap";
		case TEXTURE_DIFFUSE_ROUGHNESS: return "uDiffuseRoughnessMap";
		case TEXTURE_AMBIENT_OCCLUSION: return "uAmbientOcclusionMap";
		case TEXTURE_UNKNOWN:			return "uUnknownMap";
		case TEXTURE_SHEEN:				return "uSheenMap";
		case TEXTURE_CLEARCOAT:			return "uClearcoatMap";
		case TEXTURE_TRANSMISSION:		return "uTransmissionMap";
		case TEXTURE_SHADOW:			return "uShadowMap";
		default: return "";
	}

	return nullptr;
}

Texture::~Texture()
{
}

Texture::Texture(TextureType type)
{
	textureInfo_.type = type;
}

Texture::Texture(const TextureInfo& info)
{
	textureInfo_ = info;
}

Texture::Texture(const TextureInfo& texInfo, const SamplerInfo& smInfo)
{
	textureInfo_ = texInfo;
	samplerInfo_ = smInfo;
}

Texture::Texture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData)
{
	textureInfo_ = texInfo;
	samplerInfo_ = smInfo;
	textureData_ = texData;
}

void Texture::loadTextureData(const std::string& picture)
{
	auto bufferData = ResourceLoader::loadTexture(picture);
	TextureData texData;
	texData.dataArray = { bufferData };
	texData.path = picture;
	loadTextureData(texData);
}

void Texture::loadTextureData(TextureData data)
{
	textureData_ = data;
	textureInfo_.width = data.dataArray[0]->width();
	textureInfo_.height = data.dataArray[0]->height();
}

//void Texture::setName(const std::string& name)
//{
//	name_ = name;
//}

void Texture::setTextureInfo(const TextureInfo& info)
{
	textureInfo_ = info;
	pipelineReady_ = false;
}

const TextureInfo& Texture::getTextureInfo()
{
	// TODO: 在此处插入 return 语句
	return textureInfo_;
}

void Texture::setSamplerInfo(const SamplerInfo& info)
{
	samplerInfo_ = info;
}

const SamplerInfo& Texture::getSamplerInfo()
{
	// TODO: 在此处插入 return 语句
	return samplerInfo_;
}

TextureType Texture::getType()
{
	return static_cast<TextureType>(textureInfo_.type);
}

const TextureData& Texture::getTextureData() const
{
	return textureData_;
}

int Texture::width()
{
	return textureInfo_.width;
}

int Texture::height()
{
	return textureInfo_.height;
}

bool Texture::useMipmaps()
{
	return textureInfo_.useMipmaps;
}

bool Texture::multiSample()
{
	return textureInfo_.multiSample;
}

bool Texture::ready()
{
	return pipelineReady_;
}

void Texture::setReady(bool flag)
{
	pipelineReady_ = flag;
}

unsigned Texture::getId() const {
	return textureId_;
}

void Texture::setId(unsigned id)
{
	textureId_ = id;
}

void Texture::setMipmaps(bool flag)
{
	textureInfo_.useMipmaps = flag;
}

void Texture::setMultiSample(bool flag)
{
	textureInfo_.multiSample = flag;
}

void Texture::copyDataTo(Texture &other) {
	const auto& otherTexInfo = other.getTextureInfo();
	if (!(width() == other.width() && height() == other.height())) {
		throw std::exception("Src and dist size not compatible, texture copy failed.");
	}
	assert(textureInfo_.target == otherTexInfo.target && textureInfo_.format == otherTexInfo.format, "Src and dist format not compatible, texture copy failed.");

	TextureData tmp;
	tmp.path = textureData_.path;
	for(const auto& elem: textureData_.dataArray) {
		tmp.dataArray.emplace_back(std::make_shared<Buffer<RGBA>>(*elem));
	}
	other.textureData_ = std::move(tmp);
}
