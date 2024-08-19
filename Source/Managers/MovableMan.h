#pragma once

/// Header file for the MovableMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Serializable.h"
#include "Singleton.h"
#include "Activity.h"

#include <mutex>
#include <map>
#include <future>
#include <unordered_set>

#define g_MovableMan MovableMan::Instance()

namespace RTE {

	class MovableObject;
	class Actor;
	class HeldDevice;
	class MOPixel;
	class MOSprite;
	class AHuman;
	class SceneLayer;
	class SceneObject;
	class Box;
	class LuabindObjectWrapper;

	/// A struct to keep all data about a an alarming event for the AI Actors.
	struct AlarmEvent {
		AlarmEvent() {
			m_ScenePos.Reset();
			m_Team = Activity::NoTeam;
			m_Range = 1.0F;
		}
		// TODO: Stop relying on screen width for this shit!
		AlarmEvent(const Vector& pos, int team = Activity::NoTeam, float range = 1.0F);

		// Absolute position in the scene where this occurred
		Vector m_ScenePos;
		// The team of whatever object that caused this event
		Activity::Teams m_Team;
		// The range multiplier, that this alarming event can be heard
		float m_Range;
	};

	/// The singleton manager of all movable objects in the RTE.
	class MovableMan : public Singleton<MovableMan>, public Serializable {
		friend class SettingsMan;
		friend struct ManagerLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Constructor method used to instantiate a MovableMan object in system
		/// memory. Create() should be called before using the object.
		MovableMan();

		/// Destructor method used to clean up a MovableMan object before deletion
		/// from system memory.
		~MovableMan();

		/// Makes the MovableMan object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Initialize();

		/// Resets the entire MovableMan, including its inherited members, to
		/// their default settings or values.
		void Reset() override { Clear(); }

		/// Destroys and resets (through Clear()) the MovableMan object.
		void Destroy();

		/// Gets a MO from its MOID. Note that MOID's are only valid during the
		/// same frame as they were assigned to the MOs!
		/// @param whichID The MOID to get the matching MO from.
		/// @return A pointer to the requested MovableObject instance. 0 if no MO with that
		/// MOID was found. 0 if 0 was passed in as MOID (no MOID). Ownership is
		/// *NOT* transferred!!
		MovableObject* GetMOFromID(MOID whichID);

		/// Gets the number of MOID's currently in use this frame.
		/// @return The count of MOIDs in use this frame.
		int GetMOIDCount() { return m_MOIDIndex.size(); }

		/// Gets a MOID from pixel coordinates in the Scene.
		/// @param pixelX The X coordinate of the Scene pixel to get the MOID of.
		/// @param pixelY The Y coordinate of the Scene pixel to get the MOID of.
		/// @param moidList The collection of MOIDs to check the against the specified coordinates.
		/// @return The topmost MOID currently at the specified pixel coordinates.
		MOID GetMOIDPixel(int pixelX, int pixelY, const std::vector<int>& moidList);

		/// Returns MO count for specified team
		/// @param team Team to count MO's
		/// @return MO's count owned by this team
		int GetTeamMOIDCount(int team) const;

		/// Clears out all MovableObject:s out of this. Effectively empties the world
		/// of anything moving, without resetting all of this' settings.
		void PurgeAllMOs();

		/// Get a pointer to the first Actor in the internal Actor list that is
		/// of a specifc group, alternatively the first one AFTER a specific actor!
		/// @param group Which group to try to get an Actor for.
		/// @param pAfterThis A pointer to an Actor to use as starting point in the forward search. (default: 0)
		/// Ownership NOT xferred!
		/// @return An Actor pointer to the requested team's first Actor encountered
		/// in the list. 0 if there are no Actors of that team.
		Actor* GetNextActorInGroup(std::string group, Actor* pAfterThis = 0);

		/// Get a pointer to the last Actor in the internal Actor list that is
		/// of a specifc group, alternatively the last one BEFORE a specific actor!
		/// @param group Which group to try to get an Actor for.
		/// @param pBeforeThis A pointer to an Actor to use as starting point in the backward search. (default: 0)
		/// Ownership NOT xferred!
		/// @return An Actor pointer to the requested team's last Actor encountered
		/// in the list. 0 if there are no Actors of that team.
		Actor* GetPrevActorInGroup(std::string group, Actor* pBeforeThis = 0);

		/// Gets the list of all actors on one team, ordered by their X positions.
		/// @param team Which team to try to get the roster for. (default: 0) { return &(m_ActorRoster[team])
		/// @return A pointer to the list of all the actors on the specified team, sorted
		/// ascending by their X posistions. Ownership of the list or contained
		/// actors is NOT transferred!
		std::list<Actor*>* GetTeamRoster(int team = 0) { return &(m_ActorRoster[team]); }

		/// Get a pointer to the first Actor in the internal Actor list that is
		/// of a specifc team, alternatively the first one AFTER a specific actor!
		/// @param team Which team to try to get an Actor for. 0 means first team, 1 means 2nd. (default: 0)
		/// @param pAfterThis A pointer to an Actor to use as starting point in the forward search. (default: 0)
		/// Ownership NOT xferred!
		/// @return An Actor pointer to the requested team's first Actor encountered
		/// in the list. 0 if there are no Actors of that team.
		Actor* GetNextTeamActor(int team = 0, Actor* pAfterThis = 0);

		/// Get a pointer to the last Actor in the internal Actor list that is
		/// of a specifc team, alternatively the last one BEFORE a specific actor!
		/// @param team Which team to try to get an Actor for. 0 means first team, 1 means 2nd. (default: 0)
		/// @param pBeforeThis A pointer to an Actor to use as starting point in the backward search. (default: 0)
		/// Ownership NOT xferred!
		/// @return An Actor pointer to the requested team's last Actor encountered
		/// in the list. 0 if there are no Actors of that team.
		Actor* GetPrevTeamActor(int team = 0, Actor* pBeforeThis = 0);

		/// Get a pointer to an Actor in the internal Actor list that is of a specifc team and closest to a specific scene point.
		/// @param team Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
		/// @param player The player to get the Actor for. This affects which brain can be marked.
		/// @param scenePoint The Scene point to search for the closest to.
		/// @param maxRadius The maximum radius around that scene point to search.
		/// @param getDistance A Vector to be filled out with the distance of the returned closest to the search point. Will be unaltered if no object was found within radius.
		/// @param excludeThis An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!
		/// @return An Actor pointer to the requested team's Actor closest to the Scene point, but not outside the max radius. If no Actor other than the excluded one was found within the radius of the point, nullptr is returned.
		Actor* GetClosestTeamActor(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, const Actor* excludeThis = nullptr) { return GetClosestTeamActor(team, player, scenePoint, maxRadius, getDistance, false, excludeThis); }

		/// Get a pointer to an Actor in the internal Actor list that is of a specifc team and closest to a specific scene point.
		/// @param team Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
		/// @param player The player to get the Actor for. This affects which brain can be marked.
		/// @param scenePoint The Scene point to search for the closest to.
		/// @param maxRadius The maximum radius around that scene point to search.
		/// @param getDistance A Vector to be filled out with the distance of the returned closest to the search point. Will be unaltered if no object was found within radius.
		/// @param onlyPlayerControllableActors Whether to only get Actors that are flagged as player controllable.
		/// @param excludeThis An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!
		/// @return An Actor pointer to the requested team's Actor closest to the Scene point, but not outside the max radius. If no Actor other than the excluded one was found within the radius of the point, nullptr is returned.
		Actor* GetClosestTeamActor(int team, int player, const Vector& scenePoint, int maxRadius, Vector& getDistance, bool onlyPlayerControllableActors, const Actor* excludeThis = nullptr);

		/// Get a pointer to an Actor in the internal Actor list that is is not of
		/// the specified team and closest to a specific scene point.
		/// @param team Which team to try to get an enemy Actor for. NoTeam means all teams.
		/// @param scenePoint The Scene point to search for the closest to.
		/// @param maxRadius The maximum radius around that scene point to search.
		/// @param getDistance A Vector to be filled out with the distance of the returned closest to
		/// the search point. Will be unaltered if no object was found within radius.
		/// @return An Actor pointer to the enemy closest to the Scene
		/// point, but not outside the max radius. If no Actor
		/// was found within the radius of the point, 0 is returned.
		Actor* GetClosestEnemyActor(int team, const Vector& scenePoint, int maxRadius, Vector& getDistance);

		/// Get a pointer to first best Actor in the internal Actor list that is
		/// of a specifc team.
		/// @param team Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
		/// @param player The player to get the Actor for. This affects which brain can be marked.
		/// @return An Actor pointer to the first one of the requested team. If no Actor
		/// is in that team, 0 is returned.
		Actor* GetFirstTeamActor(int team, int player) {
			Vector temp;
			return GetClosestTeamActor(team, player, Vector(), 10000000, temp);
		}

		/// Get a pointer to an Actor in the internal Actor list that is closest
		/// to a specific scene point.
		/// @param scenePoint Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
		/// @param maxRadius The Scene point to search for the closest to.
		/// @param getDistance The maximum radius around that scene point to search.
		/// @param pExcludeThis A Vector to be filled out with the distance of the returned closest to (default: 0)
		/// the search point. Will be unaltered if no object was found within radius.
		/// An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!
		/// @return An Actor pointer to the requested Actor closest to the Scene
		/// point, but not outside the max radius. If no Actor other than the
		/// excluded one was found within the radius of the point, 0 is returned.
		Actor* GetClosestActor(const Vector& scenePoint, int maxRadius, Vector& getDistance, const Actor* pExcludeThis = 0);

		/// Get a pointer to the brain actor of a specific team that is closest to
		/// a scene point. OWNERSHIP IS NOT TRANSFERRED!
		/// @param team Which team to try to get the brain for. 0 means first team, 1 means 2nd.
		/// @param scenePoint The point in the scene where to look for the closest opposite team brain.
		/// @return An Actor pointer to the requested team's brain closest to the point.
		/// 0 if there are no brains of that team. OWNERSHIP IS NOT TRANSFERRED!
		Actor* GetClosestBrainActor(int team, const Vector& scenePoint) const;

		/// Get a pointer to the brain actor of a specific team that is closest to
		/// a scene point. OWNERSHIP IS NOT TRANSFERRED!
		/// @param GetClosestBrainActor(team Which team to try to get the brain for. 0 means first team, 1 means 2nd.
		/// @param Vector() The point in the scene where to look for the closest opposite team brain.
		/// @return An Actor pointer to the requested team's brain closest to the point.
		/// 0 if there are no brains of that team. OWNERSHIP IS NOT TRANSFERRED!
		Actor* GetFirstBrainActor(int team) const { return GetClosestBrainActor(team, Vector()); }

		/// Get a pointer to the brain actor NOT of a specific team that is closest
		/// to a scene point. OWNERSHIP IS NOT TRANSFERRED!
		/// @param notOfTeam Which team to NOT get the brain for. 0 means first team, 1 means 2nd.
		/// @param scenePoint The point where to look for the closest brain not of this team.
		/// @return An Actor pointer to the requested brain closest to the point.
		/// 0 if there are no brains not on that team. OWNERSHIP IS NOT TRANSFERRED!
		Actor* GetClosestOtherBrainActor(int notOfTeam, const Vector& scenePoint) const;

		/// Get a pointer to the brain actor NOT of a specific team. OWNERSHIP IS NOT TRANSFERRED!
		/// @param GetClosestOtherBrainActor(notOfTeam Which team to NOT get the brain for. 0 means first team, 1 means 2nd.
		/// @return An Actor pointer to the requested brain of that team.
		/// 0 if there are no brains not on that team. OWNERSHIP IS NOT TRANSFERRED!
		Actor* GetFirstOtherBrainActor(int notOfTeam) const { return GetClosestOtherBrainActor(notOfTeam, Vector()); }

		/// Get a pointer to the first brain actor of a specific team which hasn't
		/// been assigned to a player yet.
		/// @param team Which team to try to get the brain for. 0 means first team, 1 means 2nd. (default: 0)
		/// @return An Actor pointer to the requested team's first brain encountered
		/// in the list that hasn't been assigned to a player. 0 if there are no
		/// unassigned brains of that team.
		Actor* GetUnassignedBrain(int team = 0) const;

		/// Gets the number of actors currently held.
		/// @return The number of actors.
		long GetActorCount() const { return m_Actors.size(); }

		/// Gets the number of particles (MOPixel:s) currently held.
		/// @return The number of particles.
		long GetParticleCount() const { return m_Particles.size(); }

		/// Gets the global setting for how much splash MOPixels should be created
		/// an MO penetrates the terrain deeply.
		/// @return A float with the global splash amount setting, form 1.0 to 0.0.
		float GetSplashRatio() const { return m_SplashRatio; }

		/// Sets the max number of dropped items that will be reached before the
		/// first dropped with be copied to the terrain.
		/// @param newLimit An int spefifying the limit.
		void SetMaxDroppedItems(int newLimit) { m_MaxDroppedItems = newLimit; }

		/// Gets the max number of dropped items that will be reached before the
		/// first dropped with be copied to the terrain.
		/// @return An int spefifying the limit.
		int GetMaxDroppedItems() const { return m_MaxDroppedItems; }

		/// Sets this to draw HUD lines for a specific team's roster this frame.
		/// @param team Which team to have lines drawn of.
		void SortTeamRoster(int team) { m_SortTeamRoster[team] = true; }

		/// Adds a MovableObject to this, after it is determined what it is and the best way to add it is. E.g. if it's an Actor, it will be added as such. Ownership IS transferred!
		/// @param movableObjectToAdd A pointer to the MovableObject to add. Ownership IS transferred!
		/// @return Whether the MovableObject was successfully added or not. Note that Ownership IS transferred either way, but the MovableObject will be deleted if this is not successful.
		bool AddMO(MovableObject* movableObjectToAdd);

		/// Adds an Actor to the internal list of Actors. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
		/// @param actorToAdd A pointer to the Actor to add. Ownership IS transferred!
		void AddActor(Actor* actorToAdd);

		/// Adds a pickup-able item to the internal list of items. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
		/// @param itemToAdd A pointer to the item to add. Ownership IS transferred!
		void AddItem(HeldDevice* itemToAdd);

		/// Adds a MovableObject to the internal list of particles. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
		/// @param particleToAdd A pointer to the MovableObject to add. Ownership is transferred!
		void AddParticle(MovableObject* particleToAdd);

		/// Removes an Actor from the internal list of MO:s. After the Actor is
		/// removed, ownership is effectively released and transferred to whatever
		/// client called this method.
		/// @param pActorToRem A pointer to the MovableObject to remove.
		/// @return Whether the object was found in the particle list, and consequently
		/// removed. If the particle entry wasn't found, false is returned.
		Actor* RemoveActor(MovableObject* pActorToRem);

		/// Removes a pickup-able MovableObject item from the internal list of
		/// MO:s. After the item is removed, ownership is effectively released and
		/// transferred to whatever client called this method.
		/// @param pItemToRem A pointer to the MovableObject to remove.
		/// @return Whether the object was found in the particle list, and consequently
		/// removed. If the particle entry wasn't found, false is returned.
		MovableObject* RemoveItem(MovableObject* pItemToRem);

		/// Removes a MovableObject from the internal list of MO:s. After the
		/// MO is removed, ownership is effectively released and transferred to
		/// whatever client called this method.
		/// @param pMOToRem A pointer to the MovableObject to remove.
		/// @return Whether the object was found in the particle list, and consequently
		/// removed. If the particle entry wasn't found, false is returned.
		MovableObject* RemoveParticle(MovableObject* pMOToRem);

		/// Changes actor team and updates team rosters.
		/// @param pActor Pointer to actor, new team value
		void ChangeActorTeam(Actor* pActor, int team);

		/// Adds actor to internal team roster
		/// @param pActorToAdd Pointer to actor
		void AddActorToTeamRoster(Actor* pActorToAdd);

		/// Removes actor from internal team roster
		/// @param pActorToRem Pointer to actor
		void RemoveActorFromTeamRoster(Actor* pActorToRem);

		/// Goes through and checks that all MOID's have valid MO pointers
		/// associated with them. This shuold only be used for testing, as it will
		/// crash the app if validation fails.
		/// @return All MOIDs valid.
		bool ValidateMOIDs();

		/// Indicates whether the passed in MovableObject pointer points to an
		/// MO that's currently active in the simulation, and kept by this
		/// MovableMan. Internal optimization is made so that the same MO can
		/// efficiently be checked many times during the same frame.
		/// @param pMOToCheck A pointer to the MovableObject to check for being actively kept by
		/// this MovableMan.
		/// @return Whether the MO instance was found in the active list or not.
		bool ValidMO(const MovableObject* pMOToCheck);

		/// Indicates whether the passed in MovableObject is an active Actor kept
		/// by this MovableMan or not.
		/// @param pMOToCheck A pointer to the MovableObject to check for Actorness.
		/// @return Whether the object was found in the Actor list or not.
		bool IsActor(const MovableObject* pMOToCheck);

		/// Indicates whether the passed in MovableObject is an active Item kept
		/// by this MovableMan or not.
		/// @param pMOToCheck A pointer to the MovableObject to check for Itemness.
		/// @return Whether the object was found in the Item list or not.
		bool IsDevice(const MovableObject* pMOToCheck);

		/// Indicates whether the passed in MovableObject is an active Item kept
		/// by this MovableMan or not.
		/// @param pMOToCheck A pointer to the MovableObject to check for Itemness.
		/// @return Whether the object was found in the Particle list or not.
		bool IsParticle(const MovableObject* pMOToCheck);

		/// Indicates whether the passed in MOID is that of an MO which either is
		/// or is parented to an active Actor by this MovableMan, or not.
		/// @param checkMOID An MOID to check for Actorness.
		/// @return Whether the object was found or owned by an MO in the Actor list or not.
		bool IsOfActor(MOID checkMOID);

		/// Gives a unique, contiguous id per-actor. This is regenerated every frame.
		/// @param actor The actor to get a contiguous id for.
		/// @return A contiguous id for the actor. Returns -1 if the actor doesn't exist in MovableMan.
		/// @remark This function is used for AI throttling.
		int GetContiguousActorID(const Actor* actor) const;

		/// Produces the root MOID of the MOID of a potential child MO to another MO.
		/// @param checkMOID An MOID to get the root MOID of.
		/// @return The MOID of the root MO of the MO the passed-in MOID represents. This
		/// will be the same as the MOID passed in if the MO is a root itself. It will
		/// be equal to g_NoMOID if the MOID isn't allocated to an MO.
		MOID GetRootMOID(MOID checkMOID);

		/// Removes a MovableObject from the any and all internal lists of MO:s.
		/// After the MO is removed, ownership is effectively released and
		/// transferred to whatever client called this method.
		/// @param pMOToRem A pointer to the MovableObject to remove.
		/// @return Whether the object was found in MovableMan's custody, and consequently
		/// removed. If the MO entry wasn't found, false is returned.
		bool RemoveMO(MovableObject* pMOToRem);

		/// Kills and destroys all Actors of a specific Team.
		/// @param teamToKill The team to annihilate. If NoTeam is passed in, then NO Actors die.
		/// @return How many Actors were killed.
		int KillAllTeamActors(int teamToKill) const;

		/// Kills and destroys all enemy Actors of a specific Team.
		/// @param teamNotToKill The team to NOT annihilate. If NoTeam is passed in, then ALL Actors die.
		/// @return How many Actors were killed.
		int KillAllEnemyActors(int teamNotToKill = Activity::NoTeam) const;

		/// Adds all Actors in MovableMan to the given list.
		/// @param transferOwnership Whether or not ownership of the Actors should be transferred from MovableMan to the list.
		/// @param actorList The list to be filled with Actors.
		/// @param onlyTeam The team to get Actors of. If NoTeam, then all teams will be used.
		/// @param noBrains Whether or not to get brain Actors.
		/// @return The number of Actors added to the list.
		int GetAllActors(bool transferOwnership, std::list<SceneObject*>& actorList, int onlyTeam = -1, bool noBrains = false);

		/// @param transferOwnership Whether or not ownershp of the items shoudl be transferred from MovableMan to the list.
		/// @param itemList The list to be filled with items.
		/// @return The number of items added to the list.
		int GetAllItems(bool transferOwnership, std::list<SceneObject*>& itemList);

		/// Adds all particles in MovableMan to the given list.
		/// @param transferOwnership Whether or not ownership of the particles should be transferred from MovableMan to the list.
		/// @param particleList The list to be filled with particles.
		/// @return The number of particles added to the list.
		int GetAllParticles(bool transferOwnership, std::list<SceneObject*>& particleList);

		/// Opens all doors and keeps them open until this is called again with false.
		/// @param open Whether to open all doors (true), or close all doors (false).
		/// @param team Which team to open doors for. NoTeam means all teams.
		void OpenAllDoors(bool open = true, int team = Activity::NoTeam) const;

		/// Temporarily erases or redraws any material door representations of a specific team.
		/// Used to make pathfinding work better, allowing Actors to navigate through firendly bases despite the door material layer.
		/// @param eraseDoorMaterial Whether to erase door material, thereby overriding it, or redraw it and undo the override.
		/// @param team Which team to do this for, NoTeam means all teams.
		void OverrideMaterialDoors(bool eraseDoorMaterial, int team = Activity::NoTeam) const;

		/// Registers an AlarmEvent to notify things around that somehting alarming
		/// like a gunshot or explosion just happened.
		/// @param newEvent The AlarmEvent to register.
		void RegisterAlarmEvent(const AlarmEvent& newEvent);

		/// Gets the list of AlarmEvent:s from last frame's update.
		/// @return The const list of AlarmEvent:s.
		const std::vector<AlarmEvent>& GetAlarmEvents() const { return m_AlarmEvents; }

		/// Shows whetehr particles are set to get copied to the terrain upon
		/// settling
		/// @return Whether enabled or not.
		bool IsParticleSettlingEnabled() { return m_SettlingEnabled; }

		/// Sets whether particles will get copied into the terrain upon them settling down.
		/// @param enable Whether to enable or not. (default: true)
		void EnableParticleSettling(bool enable = true) { m_SettlingEnabled = enable; }

		/// Shows whether MO's sihouettes can get subtracted from the terrain at all.
		/// @return Whether enabled or not.
		bool IsMOSubtractionEnabled() { return m_MOSubtractionEnabled; }

		/// Updates the state of this MovableMan. Supposed to be done every frame.
		void Update();

		/// Draws this MovableMan's all MO's current material representations to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene.
		void DrawMatter(BITMAP* pTargetBitmap, Vector& targetPos);

		/// Updates the MOIDs of all current MOs.
		void UpdateDrawMOIDs();

		// Forces MOID drawing to complete (should be done before any physics sim or collision detection etc)
		void CompleteQueuedMOIDDrawings();

		/// Draws this MovableMan's current graphical representation to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector());

		/// Draws the HUDs of all MovableObject:s of this MovableMan to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		/// @param which Which player's screen is being drawn. Tis affects which actor's HUDs (default: 0)
		/// get drawn.
		void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int which = 0, bool playerControlled = false);

		/// Verifieis whether all elements of MOID index has correct ID. Should be used in Debug mode only.
		void VerifyMOIDIndex();

		/// Registers an object in a global Map collection so it could be found later with FindObjectByUniqueId
		/// @param mo MO to register.
		void RegisterObject(MovableObject* mo);

		/// Removes an object from the global lookup collection
		/// @param mo MO to remove.
		void UnregisterObject(MovableObject* mo);

		/// Uses a global lookup map to find an object by it's unique id.
		/// @param id Unique Id to look for.
		/// @return Object found or 0 if not found any.
		MovableObject* FindObjectByUniqueID(long int id) {
			if (m_KnownObjects.count(id) > 0)
				return m_KnownObjects[id];
			else
				return 0;
		}

		/// Returns the size of the object registry collection
		/// @return Size of the objects registry.
		unsigned int GetKnownObjectsCount() { return m_KnownObjects.size(); }

		/// Returns the current sim update frame number
		/// @return Current sim update frame number.
		unsigned int GetSimUpdateFrameNumber() const { return m_SimUpdateFrameNumber; }

		/// Gets pointers to the MOs that are within the given Box, and whose team is not ignored.
		/// @param box The Box to get MOs within.
		/// @param ignoreTeam The team to ignore.
		/// @param getsHitByMOsOnly Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.
		/// @return Pointers to the MOs that are within the given Box, and whose team is not ignored.
		const std::vector<MovableObject*>* GetMOsInBox(const Box& box, int ignoreTeam, bool getsHitByMOsOnly) const;

		/// Gets pointers to the MOs that are within the given Box, and whose team is not ignored.
		/// @param box The Box to get MOs within.
		/// @param ignoreTeam The team to ignore.
		/// @return Pointers to the MOs that are within the given Box, and whose team is not ignored.
		const std::vector<MovableObject*>* GetMOsInBox(const Box& box, int ignoreTeam) const { return GetMOsInBox(box, ignoreTeam, false); }

		/// Gets pointers to the MOs that are within the given Box.
		/// @param box The Box to get MOs within.
		/// @return Pointers to the MOs that are within the given Box.
		const std::vector<MovableObject*>* GetMOsInBox(const Box& box) const { return GetMOsInBox(box, Activity::NoTeam); }

		/// Gets pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
		/// @param centre The position to check for MOs in.
		/// @param radius The radius to check for MOs within.
		/// @param ignoreTeam The team to ignore.
		/// @param getsHitByMOsOnly Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.
		/// @return Pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
		const std::vector<MovableObject*>* GetMOsInRadius(const Vector& centre, float radius, int ignoreTeam, bool getsHitByMOsOnly) const;

		/// Gets pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
		/// @param centre The position to check for MOs in.
		/// @param radius The radius to check for MOs within.
		/// @param ignoreTeam The team to ignore.
		/// @return Pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
		const std::vector<MovableObject*>* GetMOsInRadius(const Vector& centre, float radius, int ignoreTeam) const { return GetMOsInRadius(centre, radius, ignoreTeam, false); }

		/// Gets pointers to the MOs that are within the specified radius of the given centre position.
		/// @param centre The position to check for MOs in.
		/// @param radius The radius to check for MOs within.
		/// @return Pointers to the MOs that are within the specified radius of the given centre position.
		const std::vector<MovableObject*>* GetMOsInRadius(const Vector& centre, float radius) const { return GetMOsInRadius(centre, radius, Activity::NoTeam); }

		/// Runs a lua function on all MOs in the simulation, including owned child MOs.
		void RunLuaFunctionOnAllMOs(const std::string& functionName, bool includeAdded, const std::vector<const Entity*>& functionEntityArguments = std::vector<const Entity*>(), const std::vector<std::string_view>& functionLiteralArguments = std::vector<std::string_view>(), const std::vector<LuabindObjectWrapper*>& functionObjectArguments = std::vector<LuabindObjectWrapper*>());

		/// Clears all cached lua functions on all MOs, including owned child MOs.
		void ReloadLuaScripts();

		/// Protected member variable and method declarations
	protected:
		// All actors in the scene
		std::deque<Actor*> m_Actors;
		// A map to give a unique contiguous identifier per-actor. This is re-created per frame.
		std::unordered_map<const Actor*, int> m_ContiguousActorIDs;
		// List of items that are pickup-able by actors
		std::deque<MovableObject*> m_Items;
		// List of free, dead particles flying around
		std::deque<MovableObject*> m_Particles;
		// These are the actors/items/particles which were added during a frame.
		// They are moved to the containers above at the end of the frame.
		std::deque<Actor*> m_AddedActors;
		std::deque<MovableObject*> m_AddedItems;
		std::deque<MovableObject*> m_AddedParticles;

		// Currently active MOs in the simulation. This is required because the code is awful and ownership isn't transported to/from lua in any sensible way.
		// It's entirely possible that stuff is deleted in the game but a reference to it is kept in Lua. Which is awful. Obviously.
		// Or perhaps even more concerningly, stuff can be deleted, re-allocated over the same space, and then readded to movableman. Which even this solution does nothing to fix.
		// Anyways, until we fix up ownership semantics... this is the best we can do.
		std::unordered_set<const MovableObject*> m_ValidActors;
		std::unordered_set<const MovableObject*> m_ValidItems;
		std::unordered_set<const MovableObject*> m_ValidParticles;

		// Mutexes to ensure MOs aren't being removed from separate threads at the same time
		std::mutex m_ActorsMutex;
		std::mutex m_ItemsMutex;
		std::mutex m_ParticlesMutex;

		// Mutexes to ensure MOs aren't being added from separate threads at the same time
		std::mutex m_AddedActorsMutex;
		std::mutex m_AddedItemsMutex;
		std::mutex m_AddedParticlesMutex;

		// Mutex to ensure objects aren't registered/deregistered from separate threads at the same time
		std::mutex m_ObjectRegisteredMutex;

		// Mutex to ensure actors don't change team roster from seperate threads at the same time
		std::mutex m_ActorRosterMutex;

		// Async to draw MOIDs while rendering
		std::future<void> m_DrawMOIDsTask;

		// Roster of each team's actors, sorted by their X positions in the scene. Actors not owned here
		std::list<Actor*> m_ActorRoster[Activity::MaxTeamCount];
		// Whether to draw HUD lines between the actors of a specific team
		bool m_SortTeamRoster[Activity::MaxTeamCount];
		// Every team's MO footprint
		int m_TeamMOIDCount[Activity::MaxTeamCount];

		// The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
		// This is the last frame's events, is the one for Actors to poll for events, should be cleaned out and refilled each frame.
		std::vector<AlarmEvent> m_AlarmEvents;
		// The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
		// This is the current frame's events, will be filled up during MovableMan Updates, should be transferred to Last Frame at end of update.
		std::vector<AlarmEvent> m_AddedAlarmEvents;

		// Mutexes to ensure alarm events aren't being added from separate threads at the same time
		std::mutex m_AddedAlarmEventsMutex;

		// The list created each frame to register all the current MO's
		std::vector<MovableObject*> m_MOIDIndex;

		// The ration of terrain pixels to be converted into MOPixel:s upon
		// deep impact of MO.
		float m_SplashRatio;
		// The maximum number of loose items allowed.
		int m_MaxDroppedItems;

		// Whether settling of particles is enabled or not
		bool m_SettlingEnabled;
		// Whtehr MO's vcanng et subtracted form the terrain at all
		bool m_MOSubtractionEnabled;

		unsigned int m_SimUpdateFrameNumber;

		// Global map which stores all objects so they could be foud by their unique ID
		std::map<long int, MovableObject*> m_KnownObjects;

		/// Private member variable and method declarations
	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this MovableMan, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		/// Travels all of our MOs, updating their location/velocity/physical characteristics.
		void Travel();

		/// Updates the controllers of all the actors we own.
		/// This is needed for a tricky reason - we want the controller from the activity to override the normal controller state
		/// So we need to update the controller state prior to activity, so the changes from activity are layered on top.
		void UpdateControllers();

		/// Updates all things that need to be done before we update the controllers.
		/// This is needed because of a very awkward and ugly old code path where controllers were updated in the middle of update, and various mods relied of this behaviour for actions that were therefore delayed by a frame
		/// Ideally we wouldn't need this, but this is all very fragile code and I'd prefer to avoid breaking things.
		void PreControllerUpdate();

		// Disallow the use of some implicit methods.
		MovableMan(const MovableMan& reference) = delete;
		MovableMan& operator=(const MovableMan& rhs) = delete;
	};

} // namespace RTE
