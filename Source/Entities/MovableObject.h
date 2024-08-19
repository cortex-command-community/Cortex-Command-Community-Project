#pragma once

/// Header file for the MovableObject class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files, forward declarations, namespace stuff
#include "SceneObject.h"
#include "Vector.h"
#include "Matrix.h"
#include "Timer.h"
#include "LuabindObjectWrapper.h"
#include "Material.h"
#include "MovableMan.h"

#include <set>

struct BITMAP;

namespace RTE {

	struct HitData;

	class MOSRotating;
	class PieMenu;
	class SLTerrain;
	class LuaStateWrapper;

	/// A movable object with mass.
	class MovableObject : public SceneObject {

		friend class Atom;
		friend struct EntityLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		ScriptFunctionNames("Create", "Destroy", "Update", "ThreadedUpdate", "SyncedUpdate", "OnScriptDisable", "OnScriptEnable", "OnCollideWithTerrain", "OnCollideWithMO", "WhilePieMenuOpen", "OnSave", "OnMessage", "OnGlobalMessage");
		SerializableOverrideMethods;
		ClassInfoGetters;

		enum MOType {
			TypeGeneric = 0,
			TypeActor,
			TypeHeldDevice,
			TypeThrownDevice
		};

		/// Constructor method used to instantiate a MovableObject object in system
		/// memory. Create() should be called before using the object.
		MovableObject();

		/// Destructor method used to clean up a MovableObject object before deletion
		/// from system memory.
		~MovableObject() override;

		/// Makes the MovableObject object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Makes the MovableObject object ready for use.
		/// @param mass A float specifying the object's mass in Kilograms (kg).
		/// @param position A Vector specifying the initial position. (default: Vector(0)
		/// @param 0) A Vector specifying the initial velocity.
		/// @param velocity The rotation angle in r. (default: Vector(0)
		/// @param 0) The angular velocity in r/s.
		/// @param rotAngle The amount of time in ms this MovableObject will exist. 0 means unlim. (default: 0)
		/// @param angleVel Whether or not this MO will collide with other MO's while travelling. (default: 0)
		/// @param lifetime Whether or not this MO be collided with bt other MO's during their travel. (default: 0)
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(float mass, const Vector& position = Vector(0, 0), const Vector& velocity = Vector(0, 0), float rotAngle = 0, float angleVel = 0, unsigned long lifetime = 0, bool hitMOs = true, bool getHitByMOs = false);

		/// Creates a MovableObject to be identical to another, by deep copy.
		/// @param reference A reference to the MovableObject to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const MovableObject& reference);

		/// Resets the entire MovableObject, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			SceneObject::Reset();
		}

		/// Destroys and resets (through Clear()) the MovableObject object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

#pragma region Script Handling
		/// Loads the script at the given script path onto the object, checking for appropriately named functions within it.
		/// If the script contains a Create function and this MO's scripts are running, the Create function will be run immediately.
		/// @param scriptPath The path to the script to load.
		/// @param loadAsEnabledScript Whether or not the script should load as enabled. Defaults to true.
		/// @return 0 on success. -1 if scriptPath is empty. -2 if the script is already loaded. -3 if setup to load the script or modify the global lua state fails. -4 if the script fails to load.
		virtual int LoadScript(const std::string& scriptPath, bool loadAsEnabledScript = true);

		/// Reloads the all of the scripts on this object. This will also reload scripts for the original preset in PresetMan so future objects spawned will use the new scripts.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int ReloadScripts() final;

		/// Gets whether or not the object has a script name, and there were no errors when initializing its Lua scripts. If there were, the object would need to be reloaded.
		/// @return Whether or not the object's scripts have been successfully initialized.
		bool ObjectScriptsInitialized() const { return !m_ScriptObjectName.empty() && m_ScriptObjectName != "ERROR"; }

		/// Checks if this MO has any scripts on it.
		/// @return Whether or not this MO has any scripts on it.
		bool HasAnyScripts() const { return !m_AllLoadedScripts.empty(); }

		/// Checks if the script at the given path is one of the scripts on this MO.
		/// @param scriptPath The path to the script to check.
		/// @return Whether or not the script is on this MO.
		bool HasScript(const std::string& scriptPath) const { return m_AllLoadedScripts.find(scriptPath) != m_AllLoadedScripts.end(); }

		/// Checks if the script at the given path is one of the enabled scripts on this MO.
		/// @param scriptPath The path to the script to check.
		/// @return Whether or not the script is enabled on this MO.
		bool ScriptEnabled(const std::string& scriptPath) const {
			auto scriptPathIterator = m_AllLoadedScripts.find(scriptPath);
			return scriptPathIterator != m_AllLoadedScripts.end() && scriptPathIterator->second == true;
		}

		/// Enables or dsiableds the script at the given path on this MO.
		/// @param scriptPath The path to the script to enable or disable
		/// @param enableScript Whether to enable the script, or disable it.
		/// @return Whether or not the script was successfully eanbled/disabled.
		bool EnableOrDisableScript(const std::string& scriptPath, bool enableScript);

		/// Enables or disables all scripts on this MovableObject.
		/// @param enableScripts Whether to enable (true) or disable (false) all scripts on this MovableObject.
		void EnableOrDisableAllScripts(bool enableScripts);

		/// Runs the given function in all scripts that have it, with the given arguments, with the ability to not run on disabled scripts and to cease running if there's an error.
		/// The first argument to the function will always be 'self'. If either argument list is not empty, its entries will be passed into the Lua function in order, with entity arguments first.
		/// @param functionName The name of the function to run.
		/// @param runOnDisabledScripts Whether to run the function on disabled scripts. Defaults to false.
		/// @param stopOnError Whether to stop if there's an error running any script, or simply print it to the console and continue. Defaults to false.
		/// @param functionEntityArguments Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.
		/// @param functionLiteralArguments Optional vector of strings, that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int RunScriptedFunctionInAppropriateScripts(const std::string& functionName, bool runOnDisabledScripts = false, bool stopOnError = false, const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>(), const std::vector<LuabindObjectWrapper*>& functionObjectArguments = std::vector<LuabindObjectWrapper*>());

		/// Cleans up and destroys the script state of this object, calling the Destroy callback in lua
		virtual void DestroyScriptState();
#pragma endregion

		/// Gets the MO type code of this MO. Either Actor, Item, or Generic.
		/// @return An int describing the MO Type code of this MovableObject.
		int GetMOType() const { return m_MOType; }

		/// Gets the mass value of this MovableObject.
		/// @return A float describing the mass value in Kilograms (kg).
		virtual float GetMass() const { return m_Mass; }

		/// Gets the previous position vector of this MovableObject, prior to this frame.
		/// @return A Vector describing the previous position vector.
		const Vector& GetPrevPos() const { return m_PrevPos; }

		/// Gets the velocity vector of this MovableObject.
		/// @return A Vector describing the current velocity vector.
		const Vector& GetVel() const { return m_Vel; }

		/// Gets the previous velocity vector of this MovableObject, prior to this frame.
		/// @return A Vector describing the previous velocity vector.
		const Vector& GetPrevVel() const { return m_PrevVel; }

		/// Gets the amount of distance this MO has travelled since its creation, in pixels.
		/// @return The amount of distance this MO has travelled, in pixels.
		float GetDistanceTravelled() const { return m_DistanceTravelled; }

		/// Gets the current angular velocity of this MovableObject. Positive is
		/// a counter-clockwise rotation.
		/// @return The angular velocity in radians per second.
		virtual float GetAngularVel() const { return 0.0; }

		/// Gets the largest radius of this in pixels.
		/// @return The radius from its center to the edge of its graphical representation.
		virtual float GetRadius() const { return 1.0f; }

		/// Gets the largest diameter of this in pixels.
		/// @return The largest diameter across its graphical representation.
		virtual float GetDiameter() const { return 2.0F; }

		/// Gets the current scale of this MOSRotating. This is mostly for fun.
		/// @return The normalized scale.
		float GetScale() const { return m_Scale; }

		/// Gets How this is affected by global effects, from +1.0 to -1.0.
		/// Something with a negative value will 'float' upward.
		/// @return The global acceleration scalar.
		float GetGlobalAccScalar() const { return m_GlobalAccScalar; }

		/// Sets How this is affected by global effects, from +1.0 to -1.0.
		/// Something with a negative value will 'float' upward.
		/// @param newValue The global acceleration scalar.
		void SetGlobalAccScalar(float newValue) { m_GlobalAccScalar = newValue; }

		/// How much this is affected by air resistance when traveling over a
		/// second, 0 to 1.0, with 0 as default
		/// @return The air resistance coefficient.
		float GetAirResistance() const { return m_AirResistance; }

		/// Sets how much this is affected by air resistance when traveling over a
		/// second, 0 to 1.0, with 0 as default
		/// @param newValue The air resistance coefficient.
		void SetAirResistance(float newValue) { m_AirResistance = newValue; }

		/// At which threshold of velocity, in m/s, the effect of AirResistance
		/// kicks in.
		/// @return The air threshold speed.
		float GetAirThreshold() const { return m_AirThreshold; }

		/// Sets at which threshold of velocity, in m/s, the effect of AirResistance
		/// kicks in.
		/// @param newValue The air threshold speed.
		void SetAirThreshold(float newValue) { m_AirThreshold = newValue; }

		/// Gets real time age of this MovableObject.
		/// @return A unsigned long describing the current age in ms.
		unsigned long GetAge() const { return m_AgeTimer.GetElapsedSimTimeMS(); }

		/// Gets the amount of time this MovableObject will exist from creation.
		/// @return A unsigned long describing the current lifetime in ms. 0 means unlimited.
		unsigned long GetLifetime() const { return m_Lifetime; }

		/// Gets the MOID of this MovableObject for this frame.
		/// @return An int specifying the MOID that this MovableObject is
		/// assigned for the current frame only.
		MOID GetID() const { return m_MOID; }

		/// Gets the MOID of the MovableObject which is the root MO of this MO for
		/// this frame. If same as what GetID returns, then this is owned by
		/// MovableMan.
		/// @return An int specifying the MOID of the MO that this MovableObject
		/// is owned by for the current frame only.
		MOID GetRootID() const { return m_RootMOID; }

		/// Gets how many total (subsequent) MOID's this MO and all its children
		/// are taking up this frame. ie if this MO has no children, this will
		/// likely be 1. Note this is only valid for this frame!
		/// @return The number of MOID indices this MO and all its children are taking up.
		int GetMOIDFootprint() const { return m_MOIDFootprint; }

		/// Returns whether or not this MovableObject has ever been added to MovableMan. Does not account for removal from MovableMan.
		/// @return Whether or not this MovableObject has ever been added to MovableMan.
		bool HasEverBeenAddedToMovableMan() const { return m_HasEverBeenAddedToMovableMan; }

		/// Gets the sharpness factor of this MO.
		/// @return The sharpness factor of this MO. 1.0 means normal sharpness, no alter-
		/// ation to any of the impulses.
		float GetSharpness() const { return m_Sharpness; }

		/// Placeholder method to allow for ease of use with Attachables. Returns nullptr for classes that aren't derived from Attachable.
		/// @return Nothing.
		virtual MOSRotating* GetParent() { return nullptr; }

		/// Placeholder method to allow for ease of use with Attachables. Returns nullptr for classes that aren't derived from Attachable.
		/// @return Nothing.
		virtual const MOSRotating* GetParent() const { return nullptr; }

		/// Returns a pointer to this MO, this is to enable Attachables to get their root nodes.
		/// @return A pointer to this MovableObject.
		virtual MovableObject* GetRootParent() { return this; }

		/// Returns a pointer to this MO, this is to enable Attachables to get their root nodes.
		/// @return A pointer to this MovableObject.
		virtual const MovableObject* GetRootParent() const { return this; }

		/// Gets the altitide of this' pos (or appropriate low point) over the
		/// terrain, in pixels.
		/// @param max The max altitude you care to check for. 0 Means check the whole scene's height. (default: 0)
		/// @param accuracy The accuracy within which measurement is acceptable. Higher number (default: 0)
		/// here means less calculation.
		/// @return The rough altitude over the terrain, in pixels.
		virtual float GetAltitude(int max = 0, int accuracy = 0);

		/// Gets the absoltue position of the top of this' HUD stack.
		/// @return A Vector with the absolute position of this' HUD stack top point.
		virtual Vector GetAboveHUDPos() const { return m_Pos; }

		/// Shows whether this may have started to intersect the terrain since the
		/// last frame, e g due to flipping.
		/// @return Whether this may have started to intersect the terrain since last frame.
		bool IntersectionWarning() const { return m_CheckTerrIntersection; }

		/// Gets whether this MovableObject is set to collide with other
		/// MovableObject:s during its travel.
		/// @return Whether this hits other MO's during its travel, or not.
		bool HitsMOs() const { return m_HitsMOs; }

		/// Gets whether this MovableObject is set to be able to get hit by other
		/// MovableObject:s during their travel.
		/// @return Whether this can get hit by MO's, or not.
		bool GetsHitByMOs() const { return m_GetsHitByMOs; }

		/// Sets the team of this MovableObject.
		/// @param team The new team to assign.
		void SetTeam(int team) override;

		/// Sets whether this will collide with any other MO of the same team.
		/// @param ignoreTeam Whether this can hit or get hit by other MOs of the same team. (default: true)
		void SetIgnoresTeamHits(bool ignoreTeam = true) { m_IgnoresTeamHits = ignoreTeam; }

		/// Tells whether this will collide with any other MO of the same team.
		/// @return Whether this can hit or get hit by other MOs of the same team.
		bool IgnoresTeamHits() const { return m_IgnoresTeamHits; }

		/// Tells which team this would be ignoring hits with, if we're ignoring
		/// hits at all.
		/// @return Which team this ignores hits with, if any.
		int IgnoresWhichTeam() const { return m_IgnoresTeamHits ? m_Team : Activity::NoTeam; }

		/// Sets whether this will collide with any other MO that uses an
		/// AtomGroup as a physical representation. This also overrides the
		/// IgnoresAGHitsWhenSlowerThan property.
		/// @param ignoreAG Whether this can hit or get hit by other MOs which use AGs. (default: true)
		void SetIgnoresAtomGroupHits(bool ignoreAG = true) {
			m_IgnoresAtomGroupHits = ignoreAG;
			if (ignoreAG)
				m_IgnoresAGHitsWhenSlowerThan = -1;
		}

		/// Tells whether this will collide with any MO that uses an AtomGroup
		/// as physical representation. (as opposed to single-atom ones)
		/// @return Whether this can hit or get hit by other MOs that use AGs.
		bool IgnoresAtomGroupHits() const { return m_IgnoresAtomGroupHits; }

		/// Tells whether this will collide with any Terrain
		/// @return Whether this can hit terrain.
		bool IgnoreTerrain() const { return m_IgnoreTerrain; }

		/// Sets whether this will collide with any Terrain
		/// @param ignores Whether this can hit terrain.
		void SetIgnoreTerrain(bool ignores) { m_IgnoreTerrain = ignores; }

		/// Gets whether this MO ignores collisions with actors.
		/// @return Whether this MO ignores collisions with actors.
		bool GetIgnoresActorHits() const { return m_IgnoresActorHits; }

		/// Sets whether this MO ignores collisions with actors.
		/// @param value Whether this MO will ignore collisions with actors.
		void SetIgnoresActorHits(bool value) { m_IgnoresActorHits = value; }

		/// Gets the main material of this MovableObject.
		/// @return The the material of this MovableObject.
		virtual Material const* GetMaterial() const = 0;

		/// Gets the drawing priority of this MovableObject, if two things were
		/// overlap when copying to the terrain, the higher priority MO would
		/// end up getting drawn.
		/// @return The the priority  of this MovableObject. Higher number, the higher
		/// priority.
		virtual int GetDrawPriority() const = 0;

		/// Gets the screen effect this has loaded, which can be applied to post
		/// rendering. Ownership is NOT transferred!
		/// @return The 32bpp screen effect BITMAP. Ownership is NOT transferred!
		BITMAP* GetScreenEffect() const { return m_pScreenEffect; }

		/// Gets the hash of the path of this object's screen effect file. Used to
		/// transfer glow effects over network. The hash itself is calculated during
		/// load.
		/// @return This effect's unique hash.
		size_t GetScreenEffectHash() const { return m_ScreenEffectHash; }

		/// Sets the mass of this MovableObject.
		/// @param newMass A float specifying the new mass value in Kilograms (kg).
		virtual void SetMass(const float newMass) { m_Mass = newMass; }

		/// Sets the position at the start of the sim update.
		/// @param newPrevPos A Vector specifying the new 'prev' pos.
		void SetPrevPos(const Vector& newPrevPos) { m_PrevPos = newPrevPos; }

		/// Sets the velocity vector of this MovableObject.
		/// @param newVel A Vector specifying the new velocity vector.
		void SetVel(const Vector& newVel) { m_Vel = newVel; }

		/// Sets the current absolute angle of rotation of this MovableObject.
		/// @param newAngle The new absolute angle in radians.
		void SetRotAngle(float newAngle) override {}

		/// Sets the current absolute angle of rotation of this MovableObject's effect.
		/// @param newAngle The new absolute angle in radians.
		void SetEffectRotAngle(float newAngle) { m_EffectRotAngle = newAngle; }

		/// Gets the file path of this MovableObject's current screen effect.
		/// @return A string containing the file path of the screen effect.
		std::string GetScreenEffectPath() const { return m_ScreenEffectFile.GetDataPath(); }

		/// Gets the file path of this MovableObject's current screen effect.
		/// @param pathToFile A string containing the file path of the new screen effect.
		void SetScreenEffectPath(std::string pathToFile) {
			m_ScreenEffectFile.SetDataPath(pathToFile);
			m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();
			m_ScreenEffectHash = m_ScreenEffectFile.GetHash();
		}

		/// Gets the current absolute angle of rotation of this MovableObject's effect.
		/// @return The absolute angle in radians.
		float GetEffectRotAngle() const { return m_EffectRotAngle; }

		/// Gets the starting strength of this MovableObject's effect.
		/// @return The starting strength of the effect, 0-255.
		int GetEffectStartStrength() const { return m_EffectStartStrength; }

		/// Gets the starting strength of this MovableObject's effect as a float.
		/// @return The starting strength of the effect, 0.0-1.0.
		float GetEffectStartStrengthFloat() const { return static_cast<float>(m_EffectStartStrength) / 255.0f; }

		/// Sets the starting strength of this MovableObject's effect.
		/// @param strength The new starting strength of the effect, 0.0-1.0.
		void SetEffectStartStrengthFloat(float strength) { m_EffectStartStrength = std::floor(255.0F * strength); }

		/// Gets the stopping strength of this MovableObject's effect.
		/// @return The stopping strength of the effect, 0-255.
		int GetEffectStopStrength() const { return m_EffectStopStrength; }

		/// Gets the stopping strength of this MovableObject's effect as a float.
		/// @return The stopping strength of the effect, 0.0-1.0.
		float GetEffectStopStrengthFloat() const { return static_cast<float>(m_EffectStopStrength) / 255.0f; }

		/// Sets the stopping strength of this MovableObject's effect.
		/// @param strength The new stopping strength of the effect, 0.0-1.0.
		void SetEffectStopStrengthFloat(float strength) { m_EffectStopStrength = std::floor(255.0F * strength); }

		/// Sets both strengths of this MovableObject's effect.
		/// @param strength The new strengths of the effect, 0.0-1.0.
		void SetEffectStrength(float strength) { m_EffectStartStrength = m_EffectStopStrength = std::floor(255.0F * strength); }

		/// Gets whether or not this MovableObject's effect is drawn every frame.
		/// @return Boolean indicating whether or not the effect is drawn.
		bool GetPostEffectEnabled() const { return m_PostEffectEnabled; }

		/// Sets whether or not to draw this MovableObject's effect every frame.
		/// @param newValue Boolean indicating whether or not to draw the effect.
		void SetPostEffectEnabled(bool newValue) { m_PostEffectEnabled = newValue; }

		/// Gets whether or not this MovableObject's effect can be obscured.
		/// @return Boolean indicating whether or not the effect can be obscured.
		bool GetEffectAlwaysShows() const { return m_EffectAlwaysShows; }

		/// Sets whether or not this MovableObject's effect can be obscured.
		/// @param newValue Boolean indicating whether or not the effect can be obscured.
		void SetEffectAlwaysShows(bool newValue) { m_EffectAlwaysShows = newValue; }

		/// Sets the current angular velocity of this MovableObject. Positive is
		/// a counter clockwise rotation.
		/// @param newRotVel The new angular velocity in radians per second.
		virtual void SetAngularVel(float newRotVel) {}

		/// Sets the current scale of this MOSRotating. This is mostly for fun.
		/// @param newScale The new normalized scale.
		void SetScale(float newScale) { m_Scale = newScale; }

		/// Sets the amount of time this MovableObject will exist.
		/// @param newLifetime A unsigned long specifying amount of time in ms. 0 means unlimited life. (default: 0)
		void SetLifetime(const int newLifetime = 0) { m_Lifetime = newLifetime; }

		/// Sets this' age timer to a specific value, in ms.
		/// @param newAge The new age of this, in MS. (default: 0) { m_AgeTimer.SetElapsedSimTimeMS(newAge)
		void SetAge(double newAge = 0) { m_AgeTimer.SetElapsedSimTimeMS(newAge); }

		/// Sets the MOID of this MovableObject to be g_NoMOID (255) for this frame.
		virtual void SetAsNoID() { m_MOID = g_NoMOID; }

		/// Sets this MovableObject as having been added to MovableMan. Should only really be done in MovableMan::Add/Remove Actor/Item/Particle.
		/// @param addedToMovableMan Whether or not this MovableObject has been added to MovableMan.
		void SetAsAddedToMovableMan(bool addedToMovableMan = true) {
			if (addedToMovableMan) {
				m_HasEverBeenAddedToMovableMan = true;
			}
		}

		/// Sets the sharpness factor of this MO.
		/// @param sharpness The sharpness factor of this MO. 1.0 means normal sharpness, no alter-
		/// ation to any of the impulses.
		void SetSharpness(const float sharpness) { m_Sharpness = sharpness; }

		/// Sets this MovableObject to collide with other MovableObjects during
		/// travel.
		/// @param hitMOs Whether to hit other MO's during travel, or not. (default: true)
		void SetToHitMOs(bool hitMOs = true) { m_HitsMOs = hitMOs; }

		/// Sets this MovableObject to be able to be collided with by other
		/// MovableObjects during their travel.
		/// @param getHitByMOs Whether this should get hit by other MO's during travel, or not. (default: true)
		void SetToGetHitByMOs(bool getHitByMOs = true) { m_GetsHitByMOs = getHitByMOs; }

		/// Gets the MO this MO is set not to hit even when MO hitting is enabled on this MO.
		/// @return The MO this MO is set not to hit.
		const MovableObject* GetWhichMOToNotHit() const { return m_pMOToNotHit; }

		/// Sets this MO to not hit a specific other MO and all its children even when MO hitting is enabled on this MO.
		/// @param moToNotHit A pointer to the MO to not be hitting. Null pointer means don't ignore anyhting. Ownership is NOT transferred!
		/// @param forHowLong How long, in seconds, to ignore the specified MO. A negative number means forever.
		virtual void SetWhichMOToNotHit(MovableObject* moToNotHit = nullptr, float forHowLong = -1) {
			m_pMOToNotHit = moToNotHit;
			m_MOIgnoreTimer.Reset();
			m_MOIgnoreTimer.SetSimTimeLimitS(forHowLong);
		}

		/// Enables or disables double drawing of this across wrapping seams.
		/// @param wrapDraw Wheter to enable or not. (default: true)
		void SetWrapDoubleDrawing(bool wrapDraw = true) { m_WrapDoubleDraw = wrapDraw; }

		/// Marks this MovableObject for settling onto the terrain at the end of
		/// the MovableMan update.
		/// @param toSettle Whether to mark this MO for settling or not. (default: true)
		void SetToSettle(bool toSettle = true) { m_ToSettle = toSettle; }

		/// Marks this MovableObject for deletion at the end of the MovableMan
		/// update.
		/// @param toDelete Whether to mark this MO for deletion or not. (default: true)
		void SetToDelete(bool toDelete = true) { m_ToDelete = toDelete; }

		/// Tells if this MovableObject is marked for deletion at the end of the
		/// update.
		/// @return Whether this is marked for deletion or not.
		bool IsSetToDelete() const { return m_ToDelete; }

		/// Shows whether this is mission critical and should therefore NEVER be
		/// settled or otherwise destroyed during teh course of a mission.
		/// @return Whetehr this should be immune to settling and destruction.
		bool IsMissionCritical() const { return m_MissionCritical; }

		/// Sets whether this is mission critical and should therefore NEVER be
		/// settled or otherwise destroyed during teh course of a mission.
		/// @param missionCritical Whether this should be immune to settling and destruction.
		void SetMissionCritical(bool missionCritical) { m_MissionCritical = missionCritical; }

		/// Shows whether this can be squished by getting pushed into the ground.
		/// @return Whetehr this should be immune to squishing or not.
		bool CanBeSquished() const { return m_CanBeSquished; }

		/// Tells whether this Actor's HUD is drawn or not.
		/// @return Whether this' HUD gets drawn or not.
		void SetHUDVisible(bool visible) { m_HUDVisible = visible; }

		/// Tells whether this Actor's HUD is drawn or not.
		/// @return Whether this' HUD gets drawn or not.
		bool GetHUDVisible() const { return m_HUDVisible; }

		/// Indicates whether this MO is moving or rotating stupidly fast in a way
		/// that will screw up the simulation.
		/// @return Whether this is either moving or rotating too fast.
		virtual bool IsTooFast() const { return m_Vel.MagnitudeIsGreaterThan(500.0F); }

		/// Slows the speed of anyhting that is deemed to be too fast to within
		/// acceptable rates.
		virtual void FixTooFast() {
			if (IsTooFast()) {
				m_Vel.SetMagnitude(450.0F);
			}
		}

		/// Indicates whether this MO is an Generic or not.
		/// @return Whether this MovableObject is of Type Generic or not.
		bool IsGeneric() const { return m_MOType == TypeGeneric; }

		/// Indicates whether this MO is an Actor or not.
		/// @return Whether this MovableObject is of Type Actor or not.
		bool IsActor() const { return m_MOType == TypeActor; }

		/// Indicates whether this MO is a Device or not.
		/// @return Whether this MovableObject is of Type Device (Held or Thrown) or not.
		bool IsDevice() const { return m_MOType == TypeHeldDevice || m_MOType == TypeThrownDevice; }

		/// Indicates whether this MO is a HeldDevice or not.
		/// @return Whether this MovableObject is of Type HeldDevice or not.
		// LEGACY CRAP
		bool IsHeldDevice() const { return m_MOType == TypeHeldDevice || m_MOType == TypeThrownDevice; }

		/// Indicates whether this MO is a ThrownDevice or not.
		/// @return Whether this MovableObject is of Type ThrownDevice or not.
		bool IsThrownDevice() const { return m_MOType == TypeThrownDevice; }

		/// Indicates whether this MO is made of Gold or not.
		/// @return Whether this MovableObject is of Gold or not.
		bool IsGold() const { return m_MOType == TypeGeneric && GetMaterial()->GetIndex() == c_GoldMaterialID; }

		/// Indicates whether this MovableObject is to be drawn after
		/// (in front of) or before (behind) the parent.
		/// @return Whether it's to be drawn after parent or not.
		virtual bool IsDrawnAfterParent() const { return true; }

		/// Whether a set of X, Y coordinates overlap us (in world space).
		/// @param pixelX The given X coordinate, in world space.
		/// @param pixelY The given Y coordinate, in world space.
		/// @return Whether the given coordinate overlap us.
		virtual bool HitTestAtPixel(int pixelX, int pixelY) const { return false; }

		/// Shows whether this is or carries a specifically named object in its
		/// inventory. Also looks through the inventories of potential passengers,
		/// as applicable.
		/// @param objectName The Preset name of the object to look for.
		/// @return Whetehr the object was found carried by this.
		virtual bool HasObject(std::string objectName) const { return m_PresetName == objectName; }

		/// Shows whether this is or carries a specifically grouped object in its
		/// inventory. Also looks through the inventories of potential passengers,
		/// as applicable.
		/// @param const_cast<MovableObject*>(this)->IsInGroup(groupName The name of the group to look for.
		/// @return Whetehr the object in the group was found carried by this.
		virtual bool HasObjectInGroup(std::string groupName) const { return const_cast<MovableObject*>(this)->IsInGroup(groupName); }

		/// Adds force to this MovableObject for the next time Update() is called.
		/// @param force An Vector with the external force vector that will be added to this
		/// @param offset MovableObject and affect its path next Update(). In N or kg * m/s^2. (default: Vector()) { m_Forces.push_back(std::make_pair(force)
		/// @param offset) A Vector with the offset, in METERS, of where the force is being
		/// applied relative to the center of this MovableObject.
		/// @return None.A
		void AddForce(const Vector& force, const Vector& offset = Vector()) { m_Forces.push_back(std::make_pair(force, offset)); }

		/// Adds force to this MovableObject for the next time Update() is called.
		/// @param force An Vector with the external force vector that will be added to this
		/// @param absPos MovableObject and affect its path next Update(). In N or kg * m/s^2.
		/// A Vector with the absolute world coordinates, in PIXELS, of where the
		/// force is being applied to the center of this MovableObject.
		void AddAbsForce(const Vector& force, const Vector& absPos);

		/// Adds impulse force (or instant momentum) to this MovableObject for
		/// the next time Update() is called.
		/// @param impulse An Vector with the impulse force vector that will directly be added
		/// to this MovableObject's momentum  next Update(). In kg * m/s.
		/// @param offset A Vector with the offset, in METERS, of where the impulse is being (default: Vector())
		/// applied relative to the center of this MovableObject.
		void AddImpulseForce(const Vector& impulse, const Vector& offset = Vector()) {

#ifndef RELEASE_BUILD
			RTEAssert(impulse.MagnitudeIsLessThan(500000.0F), "HUEG IMPULSE FORCE");
			RTEAssert(offset.MagnitudeIsLessThan(5000.0F), "HUGE IMPULSE FORCE OFFSET");
#endif

			m_ImpulseForces.push_back({impulse, offset});
		}

		/// Adds impulse force (or instant momentum) to this MovableObject for
		/// the next time Update() is called.
		/// @param impulse An Vector with the impulse force vector that will directly be added
		/// to this MovableObject's momentum  next Update(). In kg * m/s.
		/// @param absPos A Vector with the absolute world coordinates, in PIXELS, of where the
		/// force is being applied to the center of this MovableObject.
		void AddAbsImpulseForce(const Vector& impulse, const Vector& absPos);

		/// Clears out all the forces this MO has accumulated during this frame.
		void ClearForces() { m_Forces.clear(); }

		/// Clears out all the impulses this MO has accumulated during this frame.
		void ClearImpulseForces() { m_ImpulseForces.clear(); }

		/// Gets the impulse force threshold which has to be exceeded to
		/// 'shake loose' this from a 'pinned' state. Pinned MOs don't get moved
		/// by travel algos. If 0, this isn't pinned.
		/// @return The impulse threshold in kg * (m/s). 0 means no pinning
		float GetPinStrength() const { return m_PinStrength; }

		/// Sets a impulse force threshold which has to be exceeded to
		/// 'shake loose' this from a 'pinned' state. Pinned MOs don't get moved
		/// by travel algos. If 0, this isn't pinned.
		/// @param pinStrength The impulse threshold in kg * (m/s). 0 means no pinning
		void SetPinStrength(float pinStrength) { m_PinStrength = pinStrength; }

		/// Resest all the timers used by this. Can be emitters, etc. This is to
		/// prevent backed up emissions to come out all at once while this has been
		/// held dormant in an inventory.
		virtual void ResetAllTimers() {}

		/// Does the calculations necessary to detect whether this MovableObject is at rest or not. IsAtRest() retrieves the answer.
		virtual void RestDetection();

		/// Forces this MovableObject out of resting conditions.
		virtual void NotResting() {
			m_RestTimer.Reset();
			m_ToSettle = false;
			m_VelOscillations = 0;
		}

		/// Indicates whether this MovableObject has been at rest with no movement for longer than its RestThreshold.
		virtual bool IsAtRest();

		/// Indicates wheter this MovableObject has been updated yet during this
		/// frame.
		/// @return Wheter or not the MovableObject has been updated yet during this frame.
		bool IsUpdated() const { return m_IsUpdated; }

		/// Tell this MovableObject that a new frame has started.
		void NewFrame() { m_IsUpdated = false; }

		/// Indicates whether this MO is marked for settling at the end of the
		/// MovableMan update.
		/// @return Whether this MO is marked for settling ontot the terrain or not.
		bool ToSettle() const { return !m_MissionCritical && m_ToSettle; }

		/// Indicates whether this MO is marked for deletion at the end of the
		/// MovableMan update.
		/// @return Whether this MO is marked for deletion or not.
		bool ToDelete() const { return m_ToDelete; }

		/// Indicates whether this MO moved across the scene wrap seam during the
		/// last update.
		/// @return Whether this MO wrapped or not.
		bool DidWrap() { return m_DidWrap; }

		/// Calculates the collision response when another MO's Atom collides with
		/// this MO's physical representation. The effects will be applied
		/// directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This
		/// will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard
		/// any impulses in the Hitdata.
		virtual bool CollideAtPoint(HitData& hitData) = 0;

		/// Defines what should happen when this MovableObject hits another MO.
		/// This is called by the owned Atom/AtomGroup of this MovableObject during
		/// travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Wheter the MovableObject should immediately halt any travel going on
		/// after this hit.
		bool OnMOHit(HitData& hd);

		/// Defines what should happen when this MovableObject hits and then
		/// bounces off of something. This is called by the owned Atom/AtomGroup
		/// of this MovableObject during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Wheter the MovableObject should immediately halt any travel going on
		/// after this bounce.
		virtual bool OnBounce(HitData& hd) = 0;

		/// Defines what should happen when this MovableObject hits and then
		/// sink into something. This is called by the owned Atom/AtomGroup
		/// of this MovableObject during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Wheter the MovableObject should immediately halt any travel going on
		/// after this sinkage.
		virtual bool OnSink(HitData& hd) = 0;

		/// Checks whether any of the Atom:s in this MovableObject are on top of
		/// terrain pixels, and if so, attempt to move this out so none of this'
		/// Atoms are on top of the terrain any more.
		/// @param strongerThan Only consider materials stronger than this in the terrain for (default: g_MaterialAir)
		/// intersections.
		/// @return Whether any intersection was successfully resolved. Will return true
		/// even if there wasn't any intersections to begin with.
		virtual bool MoveOutOfTerrain(unsigned char strongerThan = g_MaterialAir) { return true; }

		/// Rotates a vector offset from this MORotating's position according to
		/// the rotate angle and flipping.
		/// @param offset A const reference the offset Vector to rotate.
		/// @return A new vector that is the result of the rotation.
		virtual Vector RotateOffset(const Vector& offset) const { return offset; }

		/// Gathers and applies the global and accumulated forces. Then it clears
		/// out the force list.Note that this does NOT apply the accumulated
		/// impulses (impulse forces)!
		virtual void ApplyForces();

		/// Gathers and applies the accumulated impulse forces. Then it clears
		/// out the impulse list.Note that this does NOT apply the accumulated
		/// regular forces (non-impulse forces)!
		virtual void ApplyImpulses();

		/// Returns the number of Forces vectors to apply.
		/// @return Number of entries in Forces list.
		int GetForcesCount() { return m_Forces.size(); };

		/// Returns force vector in newtons of the specified Force record.
		/// @param n Force record index to get data from.
		/// @return Force vector in newtons of the specified Force record.
		Vector GetForceVector(size_t n) {
			if (n > 0 && n < m_Forces.size())
				return m_Forces[n].first;
			else
				return Vector(0, 0);
		}

		/// Gets the total sum of all forces applied to this MovableObject in a single Vector.
		/// @return The total sum of all forces applied to this MovableObject.
		virtual Vector GetTotalForce();

		/// Returns offset vector in METERS (not pixels) of the specified Force record.
		/// @param n Force record index to get data from.
		/// @return Offset vector in meters of the specified Force record.
		Vector GetForceOffset(size_t n) {
			if (n > 0 && n < m_Forces.size())
				return m_Forces[n].second;
			else
				return Vector(0, 0);
		}

		/// Sets force vector in newtons of the specified Force record.
		/// @param n Force record index to get data from. New Vector force value in newtons.
		void SetForceVector(size_t n, Vector v) {
			if (n > 0 && n < m_Forces.size())
				m_Forces[n].first = v;
		}

		/// Sets offset vector in METERS (not pixels) of the specified Force record.
		/// @param n Force record index to get data from. New Vector offset value in meters.
		void SetForceOffset(size_t n, Vector v) {
			if (n > 0 && n < m_Forces.size())
				m_Forces[n].second = v;
		}

		/// Gets the pairs of impulse forces and their offsets that have to be applied.
		/// @return A constant reference to the deque of impulses for this MovableObject.
		const std::deque<std::pair<Vector, Vector>>& GetImpulses() { return m_ImpulseForces; }

		/// Returns the number of ImpulseForces vectors to apply.
		/// @return Number of entries in ImpulseForces list.
		int GetImpulsesCount() { return m_ImpulseForces.size(); }

		/// Returns Impulse vector in newtons of the specified Impulse record.
		/// @param n Impulse record index to get data from.
		/// @return Impulse vector in newtons of the specified Impulse record.
		Vector GetImpulseVector(size_t n) {
			if (n > 0 && n < m_ImpulseForces.size())
				return m_ImpulseForces[n].first;
			else
				return Vector(0, 0);
		}

		/// Returns offset vector in METERS (not pixels) of the specified Impulse record.
		/// @param n Impulse record index to get data from.
		/// @return Offset vector in meters of the specified Impulse record.
		Vector GetImpulseOffset(size_t n) {
			if (n > 0 && n < m_ImpulseForces.size())
				return m_ImpulseForces[n].second;
			else
				return Vector(0, 0);
		}

		/// Returns offset vector in METERS (not pixels) of the specified Impulse record.
		/// @param n Impulse record index to get data from.
		/// @return Offset vector in meters of the specified Impulse record.
		void SetImpulseVector(size_t n, Vector v) {
			if (n > 0 && n < m_ImpulseForces.size())
				m_ImpulseForces[n].first = v;
		}

		/// Sets offset vector in METERS (not pixels) of the specified Impulse record.
		/// @param n Impulse record index to get data from. New Vector offset value in meters.
		void SetImpulseOffset(size_t n, Vector v) {
			if (n > 0 && n < m_ImpulseForces.size())
				m_ImpulseForces[n].second = v;
		}

		/// Gets the number of Sim updates that run between each script update for this MovableObject.
		/// @return The number of Sim updates that run between each script update for this MovableObject.
		int GetSimUpdatesBetweenScriptedUpdates() const { return m_SimUpdatesBetweenScriptedUpdates; }

		/// Sets the number of Sim updates that run between each script update for this MovableObject.
		/// @param newSimUpdatesBetweenScriptedUpdates The new number of Sim updates that run between each script update for this MovableObject.
		void SetSimUpdatesBetweenScriptedUpdates(int newSimUpdatesBetweenScriptedUpdates) { m_SimUpdatesBetweenScriptedUpdates = std::max(1, newSimUpdatesBetweenScriptedUpdates); }

		/// Does stuff that needs to be done before Travel(). Always call before
		/// calling Travel.
		virtual void PreTravel();

		/// Travels this MovableObject, using its physical representation.
		virtual void Travel();

		/// Does stuff that needs to be done after Travel(). Always call after
		/// calling Travel.
		virtual void PostTravel();

		/// Update called prior to controller update. Ugly hack. Supposed to be done every frame.
		virtual void PreControllerUpdate(){};

		/// Updates this MovableObject. Supposed to be done every frame. This also
		/// applies and clear the accumulated impulse forces (impulses), and the
		/// transferred forces of MOs attached to this.
		void Update() override;

		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Updates this MovableObject's Lua scripts.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int UpdateScripts();

		/// Gets a const reference to this MOSRotating's map of string values.
		/// @return A const reference to this MOSRotating's map of string values.
		const std::unordered_map<std::string, std::string>& GetStringValueMap() const { return m_StringValueMap; }

		/// Gets a const reference to this MOSRotating's map of number values.
		/// @return A const reference to this MOSRotating's map of number values.
		const std::unordered_map<std::string, double>& GetNumberValueMap() const { return m_NumberValueMap; }

		/// Returns the string value associated with the specified key or "" if it does not exist.
		/// @param key Key to retrieve value.
		/// @return The value associated with the key.
		const std::string& GetStringValue(const std::string& key) const;

		/// Returns an encoded string value associated with the specified key or "" if it does not exist.
		/// @param key Key to retrieve value.
		/// @return The value associated with the key.
		std::string GetEncodedStringValue(const std::string& key) const;

		/// Returns the number value associated with the specified key or 0 if it does not exist.
		/// @param key Key to retrieve value.
		/// @return The value associated with the key.
		double GetNumberValue(const std::string& key) const;

		/// Returns the entity value associated with the specified key or nullptr if it does not exist.
		/// @param key Key to retrieve value.
		/// @return The value associated with the key.
		Entity* GetObjectValue(const std::string& key) const;

		/// Sets the string value associated with the specified key.
		/// @param key Key to retrieve value.
		/// @param value The new value to be associated with the key.
		void SetStringValue(const std::string& key, const std::string& value);

		/// Sets the string value associated with the specified key.
		/// @param key Key to retrieve value.
		/// @param value The new value to be associated with the key.
		void SetEncodedStringValue(const std::string& key, const std::string& value);

		/// Sets the number value associated with the specified key.
		/// @param key Key to retrieve value.
		/// @param value The new value to be associated with the key.
		void SetNumberValue(const std::string& key, double value);

		/// Sets the entity value associated with the specified key.
		/// @param key Key to retrieve value.
		/// @param value The new value to be associated with the key.
		void SetObjectValue(const std::string& key, Entity* value);

		/// Remove the string value associated with the specified key.
		/// @param key The key to remove.
		void RemoveStringValue(const std::string& key);

		/// Remove the number value associated with the specified key.
		/// @param key The key to remove.
		void RemoveNumberValue(const std::string& key);

		/// Remove the entity value associated with the specified key.
		/// @param key The key to remove.
		void RemoveObjectValue(const std::string& key);

		/// Checks whether the string value associated with the specified key exists.
		/// @param key The key to check.
		/// @return Whether or not there is an associated value for this key.
		bool StringValueExists(const std::string& key) const;

		/// Checks whether the number value associated with the specified key exists.
		/// @param key The key to check.
		/// @return Whether or not there is an associated value for this key.
		bool NumberValueExists(const std::string& key) const;

		/// Checks whether the entity value associated with the specified key exists.
		/// @param key The key to check.
		/// @return Whether or not there is an associated value for this key.
		bool ObjectValueExists(const std::string& key) const;

		/// Event listener to be run while this MovableObject's PieMenu is opened.
		/// @param pieMenu The PieMenu this event listener needs to listen to. This will always be this' m_PieMenu and only exists for std::bind.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		virtual int WhilePieMenuOpenListener(const PieMenu* pieMenu);

		/// Updates this' and its its childrens' MOID's and foorprint. Should
		/// be done every frame.
		void UpdateMOID(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true);

		/// Draws this' current graphical HUD overlay representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the draw bitmap's upper left corner in the Scene. (default: Vector())
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements (default: 0)
		/// get drawn etc.
		virtual void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) { return; }

		/// Returns current rest threshold for this MO
		/// @return Rest threshold of this MO
		int GetRestThreshold() const { return m_RestThreshold; }

		/// Sets current rest threshold for this MO
		/// @param newRestThreshold New rest threshold value
		void SetRestThreshold(int newRestThreshold) { m_RestThreshold = newRestThreshold; }

		/// Returns the next unique id for MO's and increments unique ID counter
		/// @return Returns the next unique id.
		static long GetNextUniqueID() { return ++m_UniqueIDCounter; }

		/// Returns this MO's unique persistent ID
		/// @return Returns this MO's unique persistent ID
		long GetUniqueID() const { return m_UniqueID; }

		/// Gets the preset name and unique ID of this MO, often useful for error messages.
		/// @return A string containing the unique ID and preset name of this MO.
		std::string GetPresetNameAndUniqueID() const { return m_PresetName + ", UID: " + std::to_string(m_UniqueID); }

		/// If not zero applyies specified ammount of damage points to actors on
		/// collision even without penetration.
		/// @return Amount of damage to apply.
		float DamageOnCollision() const { return m_DamageOnCollision; }

		/// If not zero applyies specified ammount of damage points to actors on
		/// collision even without penetration.
		/// @param value Amount of damage to apply.
		void SetDamageOnCollision(float value) { m_DamageOnCollision = value; }

		/// If not zero applies specified ammount of damage points to actors on
		/// collision if penetration occured.
		/// @return Amount of damage to apply.
		float DamageOnPenetration() const { return m_DamageOnPenetration; }

		/// If not zero applies specified ammount of damage points to actors on
		/// collision if penetration occured.
		/// @param value Amount of damage to apply.
		void SetDamageOnPenetration(float value) { m_DamageOnPenetration = value; }

		/// Returns damage multiplier transferred to wound inflicted by this object on penetration
		/// @return Damage multiplier to apply to wound.
		float WoundDamageMultiplier() const { return m_WoundDamageMultiplier; }

		/// Sets damage multiplier transferred to wound inflicted by this object on penetration
		/// @param value New damage multiplier to apply to wound.
		void SetWoundDamageMultiplier(float value) { m_WoundDamageMultiplier = value; }

		/// Gets whether or not this MovableObject should apply wound damage when it collides with another MovableObject.
		/// @return Whether or not this MovableObject should apply wound damage when it collides with another MovableObject.
		bool GetApplyWoundDamageOnCollision() const { return m_ApplyWoundDamageOnCollision; }

		/// Sets whether or not this MovableObject should apply wound damage when it collides with another MovableObject.
		/// @param applyWoundDamageOnCollision Whether or not this MovableObject should apply wound damage on collision.
		void SetApplyWoundDamageOnCollision(bool applyWoundDamageOnCollision) { m_ApplyWoundDamageOnCollision = applyWoundDamageOnCollision; }

		/// Gets whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.
		/// @return Whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.
		bool GetApplyWoundBurstDamageOnCollision() const { return m_ApplyWoundBurstDamageOnCollision; }

		/// Sets whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.
		/// @param applyWoundDamageOnCollision Whether or not this MovableObject should apply burst wound damage on collision.
		void SetApplyWoundBurstDamageOnCollision(bool applyWoundBurstDamageOnCollision) { m_ApplyWoundBurstDamageOnCollision = applyWoundBurstDamageOnCollision; }

		/// Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
		/// @param MOIDs Vector to store MOIDs
		virtual void GetMOIDs(std::vector<MOID>& MOIDs) const;

		/// Returns the ID of the MO hit at the previously taken Travel
		/// This will only potentially return non-g_NoMOID if this object's Atom is set to
		/// hit MO's and the MO hit isn't marked to be ignored.
		/// @return The ID of the non-ignored MO, if any, that this object's Atom or AtomGroup is now
		/// intersecting because of the last Travel taken.
		MOID HitWhatMOID() const;

		/// Sets the ID of the MO hit at the previously taken Travel
		/// This will only potentially return non-g_NoMOID if this object's Atom is set to
		/// hit MO's and the MO hit isn't marked to be ignored.
		/// @param id The ID of the non-ignored MO, if any, that this object's Atom or AtomGroup is now
		/// intersecting because of the last Travel taken.
		void SetHitWhatMOID(MOID id);

		/// Returns the unique ID of the particle hit at the previously taken Travel
		/// @return Unique ID of the particle hit at the previously taken Travel
		long int HitWhatParticleUniqueID() const;

		/// Returns the unique ID of the particle hit at the previously taken Travel
		/// @param id Unique ID of the particle hit at the previously taken Travel.
		void SetHitWhatParticleUniqueID(long int id);

		/// Returns the terrain material the previously taken Tarvel
		/// hit, if any.
		/// @return The ID of the material, if any, that this MO hit during the last Travel.
		unsigned char HitWhatTerrMaterial() const;

		/// Sets the terrain material the previously taken Tarvel hit, if any.
		/// @param matID The ID of the material, if any, that this MO hit during the last Travel.
		void SetHitWhatTerrMaterial(unsigned char matID);

		/// Gets whether this MO's RootParent can GetHitByMOs and is currently traveling.
		/// @return Whether this MO's RootParent can GetHitByMOs and is currently traveling.
		bool GetTraveling() const { return GetRootParent()->m_IsTraveling; }

		/// Sets whether this MO's RootParent is currently traveling.
		/// @param newValue Whether this MO's RootParent is currently traveling.
		void SetTraveling(bool newValue) { GetRootParent()->m_IsTraveling = newValue; }

		/// Draws this MovableObject's graphical and material representations to the specified SLTerrain's respective layers.
		/// @param terrain The SLTerrain to draw this MovableObject to. Ownership is NOT transferred!
		/// @return Whether the object was successfully drawn to the terrain.
		bool DrawToTerrain(SLTerrain* terrain);

		/// Used to get the Lua state that handles our scripts.
		/// @return Our lua state. Can potentially be nullptr if we're not setup yet.
		LuaStateWrapper* GetLuaState() { return m_ThreadedLuaState; }

		/// Method to be run when the game is saved via ActivityMan::SaveCurrentGame. Not currently used in metagame or editor saving.
		virtual void OnSave() { RunScriptedFunctionInAppropriateScripts("OnSave"); }

		/// Requests a synced update for the MO this frame.
		virtual void RequestSyncedUpdate() { m_RequestedSyncedUpdate = true; }

		/// Resets the requested update flag.
		virtual void ResetRequestedSyncedUpdateFlag() { m_RequestedSyncedUpdate = false; }

		/// Returns whether this MO has requested a synced update this frame.
		/// @return Whether this MO has requested a synced update this frame.
		virtual bool HasRequestedSyncedUpdate() { return m_RequestedSyncedUpdate; }

		/// Protected member variable and method declarations
	protected:
		/// Does necessary work to setup a script object name for this object, allowing it to be accessed in Lua, then runs all of the MO's scripts' Create functions in Lua.
		/// @return 0 on success, -2 if it fails to setup the script object in Lua, and -3 if it fails to run any Create function.
		int InitializeObjectScripts();

		/// Runs the given function for the given script, with the given arguments. The first argument to the function will always be 'self'.
		/// If either argument list is not empty, its entries will be passed into the Lua function in order, with entity arguments first.
		/// @param scriptPath The path to the script to run.
		/// @param functionName The name of the function to run.
		/// @param functionEntityArguments Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.
		/// @param functionLiteralArguments Optional vector of strings, that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int RunFunctionOfScript(const std::string& scriptPath, const std::string& functionName, const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>());

		/// Makes this MO register itself and all its attached children in the
		/// MOID register and get ID:s for itself and its children for this frame.
		/// @param MOIDIndex The MOID index to register itself and its children in.
		/// @param rootMOID The MOID of the root MO of this MO, ie the highest parent of this MO. (default: g_NoMOID)
		/// 0 means that this MO is the root, ie it is owned by MovableMan.
		/// @param makeNewMOID Whether this MO should make a new MOID to use for itself, or to use (default: true)
		/// the same as the last one in the index (presumably its parent),
		virtual void UpdateChildMOIDs(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true) {}

		/// Makes this MO register itself in the MOID register and get ID:s for
		/// itself and its children for this frame.
		/// BITMAP of choice.
		/// @param MOIDIndex The MOID index to register itself and its children in.
		/// @param rootMOID The MOID of the root MO of this MO, ie the highest parent of this MO. (default: g_NoMOID)
		/// 0 means that this MO is the root, ie it is owned by MovableMan.
		/// @param makeNewMOID Whether this MO should make a new MOID to use for itself, or to use (default: true)
		/// the same as the last one in the index (presumably its parent),
		void RegMOID(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true);

		/// Copy constructor method used to instantiate a MovableObject object
		/// identical to an already existing one.
		/// @param A MovableObject object which is passed in by reference.
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Global counter with unique ID's
		static std::atomic<long> m_UniqueIDCounter;
		// The type of MO this is, either Actor, Item, or Particle
		int m_MOType;
		float m_Mass; // In metric kilograms (kg).
		Vector m_Vel; // In meters per second (m/s).
		Vector m_PrevPos; // Previous frame's position.
		Vector m_PrevVel; // Previous frame's velocity.
		float m_DistanceTravelled; //!< An estimate of how many pixels this MO has travelled since its creation.
		float m_Scale; // The scale that this MovableObject's representation will be drawn in. 1.0 being 1:1;
		// How this is affected by global effects, from +1.0 to -1.0. Something with a negative value will 'float' upward
		float m_GlobalAccScalar;
		// How much this is affected by air resistance when traveling over a second, 0 to 1.0, with 0 as default
		float m_AirResistance;
		// At which threshold of velocity, in m/s, the effect of AirResistance kicks in
		float m_AirThreshold;
		// The impulse force in kg * (m/s) needed to unpin this. Pinned MO's don't travel at all.
		float m_PinStrength;
		// The threshold in ms as to how long this MO should wait after being at rest
		// to get flagged to be copied to the terrain.
		int m_RestThreshold;
		// The forces acting on this MovableObject, the first vector being the force in
		// In kg * m/s^2 (Newtons), and the second one being the offset the force is being
		// applied from the m_Pos, IN METERS (not pixels!).
		std::deque<std::pair<Vector, Vector>> m_Forces;
		std::deque<std::pair<Vector, Vector>> m_ImpulseForces; // First in kg * m/s, second vector in meters.
		Timer m_AgeTimer;
		Timer m_RestTimer;

		unsigned long m_Lifetime;
		// The sharpness factor that gets added to single pixel hit impulses in
		// applicable situations.
		float m_Sharpness;
		// This is to be set each frame that this may be intersecting the terrain, like when it has been flipped
		bool m_CheckTerrIntersection;
		// Whether or not this MovableObject will test for collisions against other MOs.
		bool m_HitsMOs;
		// Another MovableObject that this should not be hitting even if it is set to hit MOs.
		MovableObject* m_pMOToNotHit;
		// For how long to not hit specific MO above
		Timer m_MOIgnoreTimer;
		// Whether or not this MovableObject can get hit by other MOs.
		bool m_GetsHitByMOs;
		// Whether this ignores collisions with other MOs of the same Team as this.
		bool m_IgnoresTeamHits;
		// This currently ignores hits with other AtomGroup MOs.
		bool m_IgnoresAtomGroupHits;
		// This will flip the IgnoreAtomGroupHits on or off depending on whether this MO is travelling slower than the threshold here, in m/s
		// This is disabled if set to negative value, and 0 means AG hits are never ignored
		float m_IgnoresAGHitsWhenSlowerThan;
		// Wehther this ignores collisions with actors
		bool m_IgnoresActorHits;
		// This is mission critical, which means it should NEVER be settled or destroyed by gibbing
		bool m_MissionCritical;
		// Whether this can be destroyed by being squished into the terrain
		bool m_CanBeSquished;
		// Whether or not this MovableObject has been updated yet this frame.
		bool m_IsUpdated;
		// Whether wrap drawing double across wrapping seams is enabled or not
		bool m_WrapDoubleDraw;
		// Whether the position of this object wrapped around the world this frame, or not.
		// This is just run-time data, don't need to be saved.
		bool m_DidWrap;
		// This is only valid the same frame it was assigned!
		MOID m_MOID;
		// This is only valid the same frame it was assigned!
		// MOID of the root MO, same as this' m_MOID if this is owned by MovableMan.
		MOID m_RootMOID;
		// How many total (subsequent) MOID's this MO and all its children are taking up this frame.
		// ie if this MO has no children, this will likely be 1.
		int m_MOIDFootprint;
		// Whether or not this object has ever been added to MovableMan. Does not take into account the object being removed from MovableMan, though in practice it usually will, cause objects are usually only removed when they're deleted.
		bool m_HasEverBeenAddedToMovableMan;
		// A set of ID:s of MO:s that already have collided with this MO during this frame.
		std::set<MOID> m_AlreadyHitBy;
		int m_VelOscillations; //!< A counter for oscillations in translational velocity, in order to detect settling.
		// Mark to have the MovableMan copy this the terrain layers at the end
		// of update.
		bool m_ToSettle;
		// Mark to delete at the end of MovableMan update
		bool m_ToDelete;
		// To draw this guy's HUD or not
		bool m_HUDVisible;

		bool m_IsTraveling; //!< Prevents self-intersection while traveling.

		LuaStateWrapper* m_ThreadedLuaState; //!< The lua state that will runs our lua scripts.
		bool m_ForceIntoMasterLuaState; //!< This is awful, and only exists for automovers because they mangle global state all over the place. TODO - change automovers to use messages.

		struct LuaFunction {
			bool m_ScriptIsEnabled; //!< Whether this function is in an enabled script.
			std::unique_ptr<LuabindObjectWrapper> m_LuaFunction; //!< The lua function itself.
		};

		std::string m_ScriptObjectName; //!< The name of this object for script usage.
		std::unordered_map<std::string, bool> m_AllLoadedScripts; //!< A map of script paths to the enabled state of the given script.
		std::unordered_map<std::string, std::vector<LuaFunction>> m_FunctionsAndScripts; //!< A map of function names to vectors of Lua functions. Used to maintain script execution order and avoid extraneous Lua calls.

		volatile bool m_RequestedSyncedUpdate; //!< For optimisation purposes, scripts explicitly request a synced update if they want one.

		std::unordered_map<std::string, std::string> m_StringValueMap; //<! Map to store any generic strings available from script
		std::unordered_map<std::string, double> m_NumberValueMap; //<! Map to store any generic numbers available from script
		std::unordered_map<std::string, Entity*> m_ObjectValueMap; //<! Map to store any generic object pointers available from script
		static std::string ms_EmptyString;

		// Special post processing flash effect file and Bitmap. Shuold be loaded from a 32bpp bitmap
		ContentFile m_ScreenEffectFile;
		// Not owned by this, owned by the contentfiles
		BITMAP* m_pScreenEffect;

		size_t m_ScreenEffectHash;

		// Effect's showing is delayed by this amount into this' lifetime, in ms
		int m_EffectStartTime;
		// Effect's ending this far into the lifetime of this, in ms. 0 means as long as this is alive
		int m_EffectStopTime;
		// The intensity of the effect blend at the start time, 0-255
		int m_EffectStartStrength;
		// The intensity of the effect blend at the stop time, 0-255
		int m_EffectStopStrength;
		// The effect can't be obscured
		bool m_EffectAlwaysShows;
		// The effect's rotation angle
		float m_EffectRotAngle;
		// Whether effect's rot angle should be inherited from parent
		bool m_InheritEffectRotAngle;
		// Whether effect's rot angle should be randomized on creation
		bool m_RandomizeEffectRotAngle;
		// Whether effects rot angle should be randomized every frame
		bool m_RandomizeEffectRotAngleEveryFrame;
		// Whether or not to draw the effect every frame; used for flashes
		bool m_PostEffectEnabled;

		// This object's unique persistent ID
		long m_UniqueID;
		// In which radis should we look to remove orphaned terrain on terrain penetration,
		// must not be greater than SceneMan::ORPHANSIZE, or will be truncated
		int m_RemoveOrphanTerrainRadius;
		// What is the max orphan area to trigger terrain removal
		int m_RemoveOrphanTerrainMaxArea;
		// Rate at which terrain removal checks will be triggered
		float m_RemoveOrphanTerrainRate;
		// Whether this should cause health-damage on collision
		float m_DamageOnCollision;
		// Whether this should cause health-damage on penetration
		float m_DamageOnPenetration;
		// Damage multiplier transferred to wound inflicted by this object on penetration
		float m_WoundDamageMultiplier;
		bool m_ApplyWoundDamageOnCollision; //!< Whether or not this should apply wound damage on collision, respecting WoundDamageMultiplier and without creating a wound.
		bool m_ApplyWoundBurstDamageOnCollision; //!< Whether or not this should apply wound burst damage on collision, respecting WoundDamageMultiplier and without creating a wound.
		// Whether this MO should ignore terrain when traveling
		bool m_IgnoreTerrain;
		// MOID hit during last Travel
		MOID m_MOIDHit;
		// Terrain material hit during last travel
		unsigned char m_TerrainMatHit;
		// Unique ID of particle hit this MO
		long int m_ParticleUniqueIDHit;
		// Number of sim update frame when last collision was detected
		int m_LastCollisionSimFrameNumber;
		int m_SimUpdatesBetweenScriptedUpdates; //!< The number of Sim updates between each scripted update for this MovableObject.
		int m_SimUpdatesSinceLastScriptedUpdate; //!< The counter for the current number of Sim updates since this MovableObject last ran a scripted update.

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this MovableObject, effectively resetting the members of this abstraction level only.
		void Clear();

		/// Handles reading for custom values, dealing with the various types of custom values.
		/// @param reader A Reader lined up to the custom value type to be read.
		void ReadCustomValueProperty(Reader& reader);

		/// Returns the script state to use for a given script path.
		/// This will be locked to our thread and safe to use - ensure that it'll be unlocked after use!
		/// @param scriptPath The path to the script to check for thread safety.
		/// @param function A LuaFunction, to use as an early-out check instead of redundantly hashing and checking the filepath string.
		/// @return A script state.
		LuaStateWrapper& GetAndLockStateForScript(const std::string& scriptPath, const LuaFunction* function = nullptr);

		// Disallow the use of some implicit methods.
		MovableObject(const MovableObject& reference) = delete;
		MovableObject& operator=(const MovableObject& ref) = delete;

		/// Sets the screen effect to draw at the final post-processing stage.
		void SetPostScreenEffectToDraw() const;
	};

} // namespace RTE
