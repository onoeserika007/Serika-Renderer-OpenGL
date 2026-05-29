#include <gtest/gtest.h>

#include "Asset/AssetManager.h"
#include "Geometry/UMesh.h"

#include <string>

namespace {

auto meshDescForTest(const char *name) -> MeshDesc {
	MeshDesc desc;
	desc.path = std::string("unit-test/") + name + ".obj";
	return desc;
}

auto textureDescForTest() -> Texture {
	TextureInfo textureInfo;
	textureInfo.width = 64;
	textureInfo.height = 32;
	textureInfo.type = TEXTURE_TYPE_BASE_COLOR;
	textureInfo.target = TextureTarget_TEXTURE_2D;
	textureInfo.format = TextureFormat_RGBA8;
	textureInfo.usage = TextureUsage_Sampler;
	textureInfo.border = 0;
	textureInfo.useMipmaps = true;
	textureInfo.multiSample = false;

	SamplerInfo samplerInfo;
	samplerInfo.filterMin = Filter_LINEAR_MIPMAP_LINEAR;
	samplerInfo.filterMag = Filter_LINEAR;
	samplerInfo.wrapS = Wrap_REPEAT;
	samplerInfo.wrapT = Wrap_REPEAT;
	samplerInfo.wrapR = Wrap_CLAMP_TO_EDGE;
	samplerInfo.borderColor = Border_BLACK;

	TextureData textureData;
	textureData.path = "unit-test/albedo.png";
	textureData.loadedTextureType = TEXTURE_TYPE_BASE_COLOR;

	return Texture(textureInfo, samplerInfo, textureData);
}

auto textureDescForTest(const TextureInfo &textureInfo, const SamplerInfo &samplerInfo, const TextureData &textureData) -> Texture {
	return Texture(textureInfo, samplerInfo, textureData);
}

} // namespace

TEST(AssetManagerHandle, DefaultHandleIsInvalid) {
	AssetHandle<UMesh> handle;

	EXPECT_FALSE(handle.valid());
	EXPECT_FALSE(static_cast<bool>(handle));
	EXPECT_EQ(handle.resolve(), nullptr);
	EXPECT_EQ(handle.load(), nullptr);
}

TEST(AssetManagerHandle, InvalidReleaseKeepsHandleInvalid) {
	AssetHandle<UMesh> handle;

	handle.release();

	EXPECT_FALSE(handle.valid());
	EXPECT_FALSE(static_cast<bool>(handle));
}

TEST(AssetManagerHandle, AcquireCreatesUnloadedHandleWithoutChangingLoadedStats) {
	AssetManager &assetManager = AssetManager::getInstance();
	AssetStats before = assetManager.stats();
	MeshDesc desc = meshDescForTest("acquire-unloaded");

	AssetHandle<UMesh> handle = assetManager.acquire<UMesh>(desc);

	EXPECT_TRUE(handle.valid());
	EXPECT_EQ(handle.resolve(), nullptr);

	AssetStats afterAcquire = assetManager.stats();
	EXPECT_EQ(afterAcquire.meshCount, before.meshCount);
	EXPECT_EQ(afterAcquire.textureCount, before.textureCount);
	EXPECT_EQ(afterAcquire.shaderCount, before.shaderCount);

	handle.release();
	EXPECT_FALSE(handle.valid());
}

TEST(AssetManagerHandle, AcquireSameDescriptorReusesHandleSlot) {
	AssetManager &assetManager = AssetManager::getInstance();
	MeshDesc desc = meshDescForTest("same-descriptor");

	AssetHandle<UMesh> first = assetManager.acquire<UMesh>(desc);
	AssetHandle<UMesh> second = assetManager.acquire<UMesh>(desc);

	EXPECT_TRUE(first.valid());
	EXPECT_TRUE(second.valid());
	EXPECT_TRUE(first == second);
	EXPECT_EQ(first.resolve(), nullptr);
	EXPECT_EQ(second.resolve(), nullptr);

	first.release();
	second.release();
}

TEST(AssetManagerHandle, DifferentDescriptorsUseDifferentHandleSlots) {
	AssetManager &assetManager = AssetManager::getInstance();

	AssetHandle<UMesh> first = assetManager.acquire<UMesh>(meshDescForTest("different-a"));
	AssetHandle<UMesh> second = assetManager.acquire<UMesh>(meshDescForTest("different-b"));

	EXPECT_TRUE(first.valid());
	EXPECT_TRUE(second.valid());
	EXPECT_TRUE(first != second);

	first.release();
	second.release();
}

TEST(AssetDescriptors, MeshDescEqualityUsesPath) {
	MeshDesc base = meshDescForTest("mesh-desc");
	MeshDesc same = base;
	MeshDesc different = meshDescForTest("mesh-desc-other");

	EXPECT_TRUE(base == same);
	EXPECT_FALSE(base == different);
}

TEST(AssetDescriptors, ShaderDescEqualityIncludesGeometryPathAndDefines) {
	ShaderDesc base;
	base.vertexPath = "shader/base.vert";
	base.fragmentPath = "shader/base.frag";
	base.geometryPath = "shader/base.geom";
	base.defines = {"USE_SHADOWS"};

	ShaderDesc same = base;
	EXPECT_TRUE(base == same);

	ShaderDesc differentGeometry = base;
	differentGeometry.geometryPath = "";
	EXPECT_FALSE(base == differentGeometry);

	ShaderDesc differentDefines = base;
	differentDefines.defines.push_back("USE_IBL");
	EXPECT_FALSE(base == differentDefines);
}

TEST(AssetDescriptors, TextureEqualityIncludesDataInfoAndSamplerFields) {
	Texture base = textureDescForTest();
	Texture same = textureDescForTest();

	EXPECT_TRUE(base == same);

	TextureInfo baseInfo = base.getTextureInfo();
	SamplerInfo baseSampler = base.getSamplerInfo();
	TextureData baseData = base.getTextureData();

	Texture differentPath = textureDescForTest();
	TextureData differentPathData = differentPath.getTextureData();
	differentPathData.path = "unit-test/normal.png";
	differentPath.loadTextureData(differentPathData);
	EXPECT_FALSE(base == differentPath);

	TextureInfo differentFormatInfo = baseInfo;
	differentFormatInfo.format = TextureFormat_RGB8;
	Texture differentFormat = textureDescForTest(differentFormatInfo, baseSampler, baseData);
	EXPECT_FALSE(base == differentFormat);

	SamplerInfo differentSamplerInfo = baseSampler;
	differentSamplerInfo.wrapS = Wrap_CLAMP_TO_EDGE;
	Texture differentSampler = textureDescForTest(baseInfo, differentSamplerInfo, baseData);
	EXPECT_FALSE(base == differentSampler);
}

TEST(Texture, LoadTextureDataAcceptsEmptyDataWithoutChangingSize) {
	Texture texture = textureDescForTest();
	TextureData emptyData;
	emptyData.path = "unit-test/missing.png";

	texture.loadTextureData(emptyData);

	EXPECT_EQ(texture.getTextureData().path, emptyData.path);
	EXPECT_EQ(texture.width(), 64);
	EXPECT_EQ(texture.height(), 32);
}
