#pragma once

#include "System/Semver200/semver200.h"

namespace RTE {

#pragma region Game Version
	static constexpr const char* c_VersionString = "6.2.2";
	static const version::Semver200_version c_GameVersion = version::Semver200_version(c_VersionString);
#pragma endregion
} // namespace RTE
