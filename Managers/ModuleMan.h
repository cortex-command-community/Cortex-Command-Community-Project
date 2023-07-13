#ifndef _RTEMODULEMAN_
#define _RTEMODULEMAN_

#include "Singleton.h"
#include "DataModule.h"

#define g_ModuleMan ModuleMan::Instance()

namespace RTE {

	/// <summary>
	/// Manager responsible for loading and unloading of DataModules.
	/// </summary>
	class ModuleMan : public Singleton<ModuleMan> {

	public:

		/// <summary>
		/// Constructor method used to instantiate a SettingsMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		ModuleMan() { Clear(); }

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// <summary>
		/// Clears all the member variables of this ModuleMan.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ModuleMan(const ModuleMan &reference) = delete;
		ModuleMan & operator=(const ModuleMan &rhs) = delete;
	};
}
#endif