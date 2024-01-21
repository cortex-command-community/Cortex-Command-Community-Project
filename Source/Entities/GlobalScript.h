#pragma once

#include "PieSlice.h"

namespace RTE {

	class ACraft;

	/// The supporting object for a lua script that can be turned on and off by the player, and run independent of Activities.
	class GlobalScript : public Entity {
		friend struct EntityLuaBindings;

	public:
		EntityAllocation(GlobalScript);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a GlobalScript object in system memory. Create() should be called before using the object.
		GlobalScript();

		/// Makes the GlobalScript object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override { return 0; }

		/// Creates an GlobalScript to be identical to another, by deep copy.
		/// @param reference A reference to the GlobalScript to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const GlobalScript& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a GlobalScript object before deletion from system memory.
		~GlobalScript() override;

		/// Destroys and resets (through Clear()) the GlobalScript object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire GlobalScript, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether or not this GlobalScript is active. Active scripts can be deactivated automatically if it fails to execute it's Update function without errors to avoid filling the console with error messages.
		/// @return Whether or not this GlobalScript is active.
		bool IsActive() const { return m_IsActive; }

		/// Sets whether or not this GlobalScript should be active.
		/// @param active Whether or not this GlobalScript should be active.
		void SetActive(bool active) { m_IsActive = active; }

		/// Gets whether or not this GlobalScript should be updated late, i.e. after the standard MovableMan update.
		/// @return Whether or not this GlobalScript should be updated late.
		bool ShouldLateUpdate() const { return m_LateUpdate; }

		/// Gets the list of PieSlices this GlobalScript adds to any active Actor PieMenus.
		/// @return The list of PieSilces this GlobalScript adds to any active Actor PieMenus
		const std::vector<std::unique_ptr<PieSlice>>& GetPieSlicesToAdd() const;
#pragma endregion

#pragma region Concrete Methods
		/// Reloads the Lua script specified by this GlobalScript. This will also update the original preset in the PresetMan with the update scripts so future objects spawned will use the new scripts.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int ReloadScripts() override;

		/// Starts this GlobalScript by running the appropriate Lua function.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Start();

		/// Pauses and unpauses this GlobalScript by running the appropriate Lua function.
		/// @param pause Whether or not this GlobalScript should be paused.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Pause(bool pause = true) const;

		/// Ends this GlobalScript by running the appropriate Lua function
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int End() const;

		/// Handles when an ACraft has left the game scene and entered orbit by running the appropriate Lua function. Ownership is NOT transferred!
		/// @param orbitedCraft The ACraft instance that entered orbit. Ownership is NOT transferred!
		void HandleCraftEnteringOrbit(const ACraft* orbitedCraft);

		/// Updates the state of this GlobalScript every frame.
		void Update();
#pragma endregion

	private:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::string m_ScriptPath; //!< The path to the Lua script file that defines this' behaviors in update.
		std::string m_LuaClassName; //!< The name of the class (table) defining the logic of this in Lua, as specified in the script file.
		bool m_IsActive; //!< Whether this GlobalScript is currently allowed to run.
		bool m_HasStarted; //!< Whether this script has already been started.
		bool m_LateUpdate; //!< Whether or not this GlobalScript should be updated late, i.e. after the standard MovableMan update.

		std::vector<std::unique_ptr<PieSlice>> m_PieSlicesToAdd; //!< A vector of PieSlices that should be added to any PieMenus opened while this GlobalScript is active.

		/// Clears all the member variables of this GlobalScript, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		GlobalScript(const GlobalScript& reference) = delete;
		GlobalScript& operator=(const GlobalScript& rhs) = delete;
	};
} // namespace RTE
