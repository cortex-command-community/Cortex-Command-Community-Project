#pragma once

#include "System/Semver200/semver200.h"

namespace RTE {

#pragma region Game Version
	static constexpr const char* c_VersionString = "7.0.0";
	static const version::Semver200_version c_GameVersion = version::Semver200_version(c_VersionString);
#pragma endregion

#pragma region Save Format Version
	/// The format of saved games. Increment this whenever a change means saved games made before it can't be read anymore.
	static constexpr int c_SaveFormatVersion = 1;
#pragma endregion
} // namespace RTE
