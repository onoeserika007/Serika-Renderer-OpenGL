#include "Asset/AssetTypes.h"

#include <iomanip>
#include <random>
#include <sstream>

namespace Serika::Asset {

namespace {

auto fromHex(char value, uint8_t &outValue) -> bool {
	if (value >= '0' && value <= '9') {
		outValue = static_cast<uint8_t>(value - '0');
		return true;
	}
	if (value >= 'a' && value <= 'f') {
		outValue = static_cast<uint8_t>(value - 'a' + 10);
		return true;
	}
	if (value >= 'A' && value <= 'F') {
		outValue = static_cast<uint8_t>(value - 'A' + 10);
		return true;
	}
	return false;
}

auto normalizeGuidString(const std::string &text, std::string &outText) -> bool {
	outText.clear();
	outText.reserve(32);
	for (char value : text) {
		if (value == '-') continue;
		outText.push_back(value);
	}
	return outText.size() == static_cast<size_t>(32);
}

} // namespace

auto AssetGuid::generate() -> AssetGuid {
	std::random_device randomDevice;
	AssetGuid guid;
	for (uint8_t &byte : guid.bytes_) {
		byte = static_cast<uint8_t>(randomDevice());
	}

	guid.bytes_[6] = static_cast<uint8_t>((guid.bytes_[6] & 0x0f) | 0x40);
	guid.bytes_[8] = static_cast<uint8_t>((guid.bytes_[8] & 0x3f) | 0x80);
	return guid;
}

auto AssetGuid::fromString(const std::string &text, AssetGuid &outGuid) -> bool {
	std::string normalized;
	if (!normalizeGuidString(text, normalized)) return false;

	AssetGuid parsed;
	for (size_t index = 0; index < parsed.bytes_.size(); ++index) {
		uint8_t high = 0;
		uint8_t low = 0;
		if (!fromHex(normalized[index * 2], high)) return false;
		if (!fromHex(normalized[index * 2 + 1], low)) return false;
		parsed.bytes_[index] = static_cast<uint8_t>((high << 4) | low);
	}

	outGuid = parsed;
	return true;
}

auto AssetGuid::valid() const -> bool {
	const std::array<uint8_t, 16> empty {};
	return bytes_ != empty;
}

auto AssetGuid::toString() const -> std::string {
	std::ostringstream stream;
	stream << std::hex << std::setfill('0');
	for (size_t index = 0; index < bytes_.size(); ++index) {
		stream << std::setw(2) << static_cast<int>(bytes_[index]);
	}
	return stream.str();
}

auto AssetGuid::operator==(const AssetGuid &other) const -> bool {
	return bytes_ == other.bytes_;
}

auto AssetGuid::operator!=(const AssetGuid &other) const -> bool {
	return !(*this == other);
}

auto AssetGuid::operator<(const AssetGuid &other) const -> bool {
	return bytes_ < other.bytes_;
}

auto AssetObjectId::valid() const -> bool {
	return guid.valid() && localId != INVALID_ASSET_LOCAL_ID;
}

auto AssetObjectId::operator==(const AssetObjectId &other) const -> bool {
	return guid == other.guid && localId == other.localId;
}

auto AssetObjectId::operator!=(const AssetObjectId &other) const -> bool {
	return !(*this == other);
}

auto AssetObjectId::operator<(const AssetObjectId &other) const -> bool {
	if (guid != other.guid) return guid < other.guid;
	return localId < other.localId;
}

} // namespace Serika::Asset
