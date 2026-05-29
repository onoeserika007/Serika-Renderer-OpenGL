#ifndef SERIKA_RENDERER_INCLUDE_ASSET_ASSET_TYPES_H
#define SERIKA_RENDERER_INCLUDE_ASSET_ASSET_TYPES_H

#include <array>
#include <cstdint>
#include <string>

namespace Serika::Asset {

class AssetGuid {
public:
	AssetGuid() = default;

	static auto generate() -> AssetGuid;
	static auto fromString(const std::string &text, AssetGuid &outGuid) -> bool;

	auto valid() const -> bool;
	auto toString() const -> std::string;

	auto operator==(const AssetGuid &other) const -> bool;
	auto operator!=(const AssetGuid &other) const -> bool;
	auto operator<(const AssetGuid &other) const -> bool;

private:
	std::array<uint8_t, 16> bytes_ {};
};

using AssetLocalId = uint64_t;

constexpr AssetLocalId INVALID_ASSET_LOCAL_ID = 0;
constexpr AssetLocalId MAIN_ASSET_LOCAL_ID = 1;

struct AssetObjectId {
	AssetGuid guid;
	AssetLocalId localId = INVALID_ASSET_LOCAL_ID;

	auto valid() const -> bool;
	auto operator==(const AssetObjectId &other) const -> bool;
	auto operator!=(const AssetObjectId &other) const -> bool;
	auto operator<(const AssetObjectId &other) const -> bool;
};

} // namespace Serika::Asset

#endif // SERIKA_RENDERER_INCLUDE_ASSET_ASSET_TYPES_H
