#include <gtest/gtest.h>

#include "Asset/AssetTypes.h"

#include <string>

namespace {

auto withHyphens(const std::string &guidText) -> std::string {
	return guidText.substr(0, 8) + "-"
	    + guidText.substr(8, 4) + "-"
	    + guidText.substr(12, 4) + "-"
	    + guidText.substr(16, 4) + "-"
	    + guidText.substr(20, 12);
}

} // namespace

TEST(AssetGuid, GeneratedGuidRoundTripsThroughCompactString) {
	Serika::Asset::AssetGuid guid = Serika::Asset::AssetGuid::generate();

	ASSERT_TRUE(guid.valid());
	std::string text = guid.toString();
	EXPECT_EQ(text.size(), static_cast<size_t>(32));

	Serika::Asset::AssetGuid parsed;
	EXPECT_TRUE(Serika::Asset::AssetGuid::fromString(text, parsed));
	EXPECT_TRUE(parsed == guid);
}

TEST(AssetGuid, HyphenatedGuidStringIsAccepted) {
	Serika::Asset::AssetGuid guid = Serika::Asset::AssetGuid::generate();
	std::string hyphenated = withHyphens(guid.toString());

	Serika::Asset::AssetGuid parsed;
	EXPECT_TRUE(Serika::Asset::AssetGuid::fromString(hyphenated, parsed));
	EXPECT_TRUE(parsed == guid);
}

TEST(AssetGuid, InvalidGuidStringIsRejected) {
	Serika::Asset::AssetGuid parsed;

	EXPECT_FALSE(Serika::Asset::AssetGuid::fromString("not-a-guid", parsed));
	EXPECT_FALSE(parsed.valid());
}

TEST(AssetObjectId, ValidRequiresGuidAndLocalId) {
	Serika::Asset::AssetObjectId empty;
	EXPECT_FALSE(empty.valid());

	Serika::Asset::AssetObjectId objectId;
	objectId.guid = Serika::Asset::AssetGuid::generate();
	objectId.localId = Serika::Asset::MAIN_ASSET_LOCAL_ID;
	EXPECT_TRUE(objectId.valid());

	objectId.localId = Serika::Asset::INVALID_ASSET_LOCAL_ID;
	EXPECT_FALSE(objectId.valid());
}
