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
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		ModuleMan() { Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the map of mods which are disabled.
		/// </summary>
		/// <returns>Map of mods which are disabled.</returns>
		std::map<std::string, bool> & GetDisabledModsMap() { return m_DisabledMods; }

		/// <summary>
		/// Gets whether the specified mod is disabled in the settings.
		/// </summary>
		/// <param name="modModule">Mod to check.</param>
		/// <returns>Whether the mod is disabled via settings.</returns>
		bool IsModDisabled(const std::string &modModule) const;
#pragma endregion

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		std::map<std::string, bool> m_DisabledMods; //!< Map of the module names that are disabled.

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