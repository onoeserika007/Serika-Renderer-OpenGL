#include "stb/stb_image.h"
#include "Utils/ImageUtils.h"
#include "Material/Texture.h"
#include "Base/Globals.h"
#include "Base/ResourceLoader.h"
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
	"SHADOW_MAP",
	"SHADOW_MAP_CUBE",
	"CUBE_MAP",
	"EQUIRECTANGULAR_MAP"
};

#define CASE_ENUM_STR(type) case type: return #type

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
		case TEXTURE_TYPE_NONE:					return "uNoneMap";
		case TEXTURE_TYPE_DIFFUSE:				return "uDiffuseMap";
		case TEXTURE_TYPE_SPECULAR:				return "uSpecularMap";
		case TEXTURE_TYPE_AMBIENT:				return "uAmbientMap";
		case TEXTURE_TYPE_EMISSIVE:				return "uEmissiveMap";
		case TEXTURE_TYPE_HEIGHT:				return "uHeightMap";
		case TEXTURE_TYPE_NORMALS:				return "uNormalsMap";
		case TEXTURE_TYPE_SHININESS:			return "uShininessMap";
		case TEXTURE_TYPE_OPACITY:				return "uOpacityMap";
		case TEXTURE_TYPE_DISPLACEMENT:			return "uDisplacementMap";
		case TEXTURE_TYPE_LIGHTMAP:				return "uLightmapMap";
		case TEXTURE_TYPE_REFLECTION:			return "uReflectionMap";
		case TEXTURE_TYPE_BASE_COLOR:			return "uBaseColorMap";
		case TEXTURE_TYPE_NORMAL_CAMERA:		return "uNormalCameraMap";
		case TEXTURE_TYPE_EMISSION_COLOR:		return "uEmissionColorMap";
		case TEXTURE_TYPE_METALNESS:			return "uMetalnessMap";
		case TEXTURE_TYPE_DIFFUSE_ROUGHNESS:	return "uDiffuseRoughnessMap";
		case TEXTURE_TYPE_AMBIENT_OCCLUSION:	return "uAmbientOcclusionMap";
		case TEXTURE_TYPE_UNKNOWN:				return "uUnknownMap";
		case TEXTURE_TYPE_SHEEN:				return "uSheenMap";
		case TEXTURE_TYPE_CLEARCOAT:			return "uClearcoatMap";
		case TEXTURE_TYPE_TRANSMISSION:			return "uTransmissionMap";
		case TEXTURE_TYPE_SHADOWMAP:			return "uShadowMap";
		case TEXTURE_TYPE_SHADOWMAP_CUBE:		return "uShadowMapCube";
		case TEXTURE_TYPE_CUBE:					return "uCubeMap";
		case TEXTURE_TYPE_EQUIRECTANGULAR:		return "uEquiRectangular";
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
	auto bufferData = ResourceLoader::getInstance().loadTexture(picture);
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

const TextureInfo& Texture::getTextureInfo() const {
	// TODO: 在此处插入 return 语句
	return textureInfo_;
}

void Texture::setSamplerInfo(const SamplerInfo& info)
{
	samplerInfo_ = info;
}

const SamplerInfo& Texture::getSamplerInfo() const {
	// TODO: 在此处插入 return 语句
	return samplerInfo_;
}

TextureType Texture::getType() const {
	return static_cast<TextureType>(textureInfo_.type);
}

const TextureData& Texture::getTextureData() const
{
	return textureData_;
}

int Texture::width() const {
	return textureInfo_.width;
}

int Texture::height() const {
	return textureInfo_.height;
}

bool Texture::useMipmaps() const {
	return textureInfo_.useMipmaps;
}

bool Texture::multiSample() const {
	return textureInfo_.multiSample;
}

bool Texture::ready() const {
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
	// 宽高可以不一样，只是会发生裁剪
	// if (!(width() == other.width() && height() == other.height())) {
	// 	throw std::exception("Src and dist size not compatible, texture copy failed.");
	// }
	assert(textureInfo_.target == otherTexInfo.target && textureInfo_.format == otherTexInfo.format, "Src and dist format not compatible, texture copy failed.");

	TextureData tmp;
	tmp.path = textureData_.path;
	for(const auto& elem: textureData_.dataArray) {
		tmp.dataArray.emplace_back(Buffer<RGBA>::makeBuffer(*elem));
	}
	other.textureData_ = std::move(tmp);
}
