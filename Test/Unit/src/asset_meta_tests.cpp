#include <gtest/gtest.h>

#include "Asset/AssetMeta.h"

#include <string>

TEST(AssetType, StringRoundTripUsesReadableNames) {
	const Serika::Asset::AssetType types[] = {
	    Serika::Asset::AssetType::Unknown,
	    Serika::Asset::AssetType::Texture,
	    Serika::Asset::AssetType::Mesh,
	    Serika::Asset::AssetType::Shader,
	    Serika::Asset::AssetType::Material,
	    Serika::Asset::AssetType::Scene,
	};

	for (Serika::Asset::AssetType type : types) {
		std::string text = Serika::Asset::assetTypeToString(type);
		Serika::Asset::AssetType parsed = Serika::Asset::AssetType::Unknown;
		EXPECT_TRUE(Serika::Asset::assetTypeFromString(text, parsed));
		EXPECT_EQ(parsed, type);
	}
}

TEST(AssetType, UnknownStringIsRejected) {
	Serika::Asset::AssetType parsed = Serika::Asset::AssetType::Unknown;

	EXPECT_FALSE(Serika::Asset::assetTypeFromString("InvalidAssetType", parsed));
}

TEST(AssetType, AssetTypeFromPathHandlesKnownExtensions) {
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("textures/albedo.PNG"), Serika::Asset::AssetType::Texture);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("models/bunny.obj"), Serika::Asset::AssetType::Mesh);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("models/scene.GLTF"), Serika::Asset::AssetType::Mesh);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("shader/pass.frag"), Serika::Asset::AssetType::Shader);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("materials/rock.mat"), Serika::Asset::AssetType::Material);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("scenes/default.scene"), Serika::Asset::AssetType::Scene);
	EXPECT_EQ(Serika::Asset::assetTypeFromPath("notes/readme.txt"), Serika::Asset::AssetType::Unknown);
}
