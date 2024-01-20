#pragma once

/// Contains hacks and workarounds for missing or wrong allegro functionality.
/// Note: Prefer fixing in allegro itself over adding hacks here.
namespace RTE {
#pragma region True Alpha Blending

	/// Workaround for allegro's missing true alpha blender, use instead of set_alpha_blender when alpha values are desired or necessary after draw:
	/// ``` set_blender_mode_ex(_blender_black, _blender_black, _blender_black, TrueAlphaBlender, _blender_black, _blender_black, _blender_black, 0, 0, 0, 0);```
	unsigned long TrueAlphaBlender(unsigned long x, unsigned long y, unsigned long n);
	/// Sets the 32bit allegro blender mode to TrueAlphaBlender
	void SetTrueAlphaBlender();
#pragma endregion
} // namespace RTE
