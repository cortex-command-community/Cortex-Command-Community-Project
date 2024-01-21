#pragma once

/// Header file for the ActivityMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "GameActivity.h"
#include "GlobalScript.h"
#include "Box.h"

#include "LuabindObjectWrapper.h"

namespace RTE {

	class ACraft;

	/// Scripted activity
	class GAScripted : public GameActivity {

		friend class LuaMan;
		friend class ActivityMan;

		/// Public member variable, method and friend function declarations
	public:
		ScriptFunctionNames("StartActivity", "UpdateActivity", "PauseActivity", "EndActivity", "OnSave", "CraftEnteredOrbit", "OnMessage", "OnGlobalMessage");

		// Concrete allocation and cloning definitions
		EntityAllocation(GAScripted);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a GAScripted object in system
		/// memory. Create() should be called before using the object.
		GAScripted();

		/// Destructor method used to clean up a GAScripted object before deletion
		/// from system memory.
		~GAScripted() override;

		/// Makes the GAScripted object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Makes the GAScripted object ready for use.
		/// @param scriptPath The filepath to the script that defines this' Lua-defined derivation
		/// of this class.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(std::string scriptPath, std::string scriptClassName) {
			m_ScriptPath = scriptPath;
			m_LuaClassName = scriptClassName;
			return Create();
		};

		/// Creates a GAScripted to be identical to another, by deep copy.
		/// @param reference A reference to the GAScripted to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const GAScripted& reference);

		/// Resets the entire GAScripted, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the GAScripted object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Reloads the preset scripts of this object, from the same script file
		/// path as was originally defined. This will also update the original
		/// preset in the PresetMan with the updated scripts so future objects
		/// spawned will use the new scripts.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int ReloadScripts() override;

		/// Refreshes our activity functions to find any changes from script.
		void RefreshActivityFunctions();

		/// Gets the class name of the Lua-derived class defined in this' script.
		/// @return A string with the friendly-formatted Lua type name of this object.
		const std::string& GetLuaClassName() const { return m_LuaClassName; }

		/// Tells if a particular Scene supports this specific Activity on it.
		/// Usually that means certain Area:s need to be defined in the Scene.
		/// @param pScene The Scene to check if it supports this Activiy. Ownership IS NOT TRANSFERRED!
		/// @param teams How many teams we're checking for. Some scenes may support and activity (default: -1)
		/// but only for a limited number of teams. If -1, not applicable.
		/// @return Whether the Scene has the right stuff.
		bool SceneIsCompatible(Scene* pScene, int teams = -1) override;

		/// Handles when an ACraft has left the game scene and entered orbit, though does not delete it. Ownership is NOT transferred, as the ACraft's inventory is just 'unloaded'.
		/// @param orbitedCraft The ACraft instance that entered orbit. Ownership is NOT transferred!
		void HandleCraftEnteringOrbit(ACraft* orbitedCraft) override;

		/// Officially starts the game accroding to parameters previously set.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Start() override;

		/// Pauses and unpauses the game.
		/// @param pause Whether to pause the game or not. (default: true)
		void SetPaused(bool pause = true) override;

		/// Forces the current game's end.
		void End() override;

		/// Updates the state of this ActivityMan. Supposed to be done every frame
		/// before drawing.
		void Update() override;

		/// Updates globals scripts loaded with this activity.
		/// @param lateUpdate Whether it's an early update, during Activity update, or late update, after MovableMan
		void UpdateGlobalScripts(bool lateUpdate);

		/// Draws the currently active GUI of a screen to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a screen-sized BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		/// @param which Which screen's GUI to draw onto the bitmap. (default: 0)
		void DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int which = 0) override;

		/// Draws this ActivityMan's current graphical representation to a
		/// BITMAP of choice. This includes all game-related graphics.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) override;

		int RunLuaFunction(const std::string& functionName, const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>(), const std::vector<LuabindObjectWrapper*>& functionObjectArguments = std::vector<LuabindObjectWrapper*>());

		/// Protected member variable and method declarations
	protected:
		/// Goes through the script file and checks for any mentions and uses of
		/// Area:s that are required for this Activity to run in a Scene.
		void CollectRequiredAreas();

		/// Does nothing - we do this in script! Just overrides the base behaviour.
		void InitAIs() override{};

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The path to the lua script file that defines this' behaviors with overrides of its virtual functions
		std::string m_ScriptPath;
		// The name of the class (table) defining the logic of this in Lua, as specified in the script file
		std::string m_LuaClassName;
		// The list of Area:s required in a Scene to play this Activity on it
		std::set<std::string> m_RequiredAreas;
		std::vector<std::unique_ptr<PieSlice>> m_PieSlicesToAdd; //!< A vector of PieSlices that should be added to any PieMenus opened while this GAScripted is running.
		// The list of global scripts allowed to run during this activity
		std::vector<GlobalScript*> m_GlobalScriptsList;

		std::unordered_map<std::string, std::unique_ptr<LuabindObjectWrapper>> m_ScriptFunctions; //!< A map of LuabindObjectWrappers that hold Lua functions. Used to maintain script execution order and avoid extraneous Lua calls.

		/// Private member variable and method declarations
	private:
		/// Returns whether this GAScripted has an OnSave function, to act as a default for whether saving is allowed or not.
		/// @return Whether this GAScripted has an OnSave function
		bool HasSaveFunction() const;

		/// Adds this GAScripted's PieSlices, and any active GlobalScripts' PieSlices, to any active PieMenus.
		void AddPieSlicesToActiveActorPieMenus();

		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
