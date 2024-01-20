#ifndef _GUICONTROLFACTORY_
#define _GUICONTROLFACTORY_

namespace RTE {

	/// A class used to create the different controls based on name.
	class GUIControlFactory {

	public:
		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          CreateControl
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Method used for creating controls
		// Arguments:       Control Type Name.

		static GUIControl* CreateControl(GUIManager* Manager, GUIControlManager* ControlManager, const std::string& ControlName);
	};
}; // namespace RTE
#endif
