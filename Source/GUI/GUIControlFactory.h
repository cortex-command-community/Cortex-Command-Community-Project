#pragma once

namespace RTE {

	/// A class used to create the different controls based on name.
	class GUIControlFactory {

	public:
		/// Method used for creating controls
		/// @param Manager Control Type Name.
		static GUIControl* CreateControl(GUIManager* Manager, GUIControlManager* ControlManager, const std::string& ControlName);
	};
} // namespace RTE
