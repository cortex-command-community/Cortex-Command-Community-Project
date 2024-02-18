#pragma once

/// Header file for the Scene class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Entity.h"
#include "Box.h"
#include "Activity.h"
#include "PathFinder.h"

#include <array>
#include <map>

namespace RTE {

	class ContentFile;
	class MovableObject;
	class PathFinder;
	class SLBackground;
	class SLTerrain;
	class SceneLayer;
	class BunkerAssembly;
	class SceneObject;
	class Deployment;

	/// Contains everything that defines a complete scene.
	class Scene : public Entity {

		friend struct EntityLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		// Available placed objects sets
		enum PlacedObjectSets {
			PLACEONLOAD = 0,
			BLUEPRINT,
			AIPLAN,
			PLACEDSETSCOUNT
		};

		/// Something to bundle the properties of scene areas together
		class Area :
		    public Serializable {

			friend class Scene;
			friend class AreaEditorGUI;
			friend class AreaPickerGUI;

			friend struct EntityLuaBindings;

			/// Public member variable, method and friend function declarations
		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a Area object in system
			/// memory. Create() should be called before using the object.
			Area() {
				Clear();
				Create();
			}
			Area(std::string name) {
				Clear();
				m_Name = name;
				Create();
			}
			Area(const Area& reference) {
				Clear();
				Create(reference);
			}

			/// Makes the Area object ready for use.
			/// @return An error return value signaling sucess or any particular failure.
			/// Anything below 0 is an error signal.
			int Create() override;

			/// Creates a Area to be identical to another, by deep copy.
			/// @param reference A reference to the Area to deep copy.
			/// @return An error return value signaling sucess or any particular failure.
			/// Anything below 0 is an error signal.
			int Create(const Area& reference);

			/// Resets the entire Serializable, including its inherited members, to their
			/// default settings or values.
			void Reset() override { Clear(); }

			/// Adds a Box to this' area coverage.
			/// @param newBox The Box to add. A copy will be made and added.
			/// @return Whether the Box was successfully added or not.
			bool AddBox(const Box& newBox);

			/// Removes the first Box in the Area that has the same Corner, Width and Height of the passed-in Box.
			/// @param boxToRemove A Box whose values are used to determine what Box to remove.
			/// @return Whether or not a Box was removed.
			bool RemoveBox(const Box& boxToRemove);

			/// Gets the first Box in this Area.
			/// @return The first Box in this Area.
			const Box* GetFirstBox() const { return m_BoxList.empty() ? nullptr : &m_BoxList[0]; }

			/// Gets the boxes for this area.
			/// @return The boxes in this Area.
			const std::vector<Box>& GetBoxes() const { return m_BoxList; }

			/// Shows whether this really has no Area at all, ie it doesn't have any
			/// Box:es with both width and height.
			/// @return Whether this Area actually covers any area.
			bool HasNoArea() const;

			/// Shows whether a point is anywhere inside this Area's coverage.
			/// @param point The point to check if it's inside the Area, in absolute scene coordinates.
			/// @return Whether the point is inside any of this Area's Box:es.
			bool IsInside(const Vector& point) const;

			/// Shows whether a coordinate is anywhere inside this Area's coverage, in the
			/// X-axis only.
			/// @param pointX The x coord to check if it's inside the Area, in absolute scene units.
			/// @return Whether the point is inside any of this Area's Box:es in the X axis.
			bool IsInsideX(float pointX) const;

			/// Shows whether a coordinate is anywhere inside this Area's coverage, in the
			/// Y-axis only.
			/// @param pointY The x coord to check if it's inside the Area, in absolute scene units.
			/// @return Whether the point is inside any of this Area's Box:es in the Y axis.
			bool IsInsideY(float pointY) const;

			/// Moves a coordinate to the closest value which is within any of this
			/// Area's Box:es, in the X axis only.
			/// @param pointX The x coord to transform to the closest inside poistion in the x-axis.
			/// @param direction Which direction to limit the search to. < 0 means can only look in the (default: 0)
			/// negative dir, 0 means can look in both directions.
			/// @return Whether the point was moved at all to get inside this' x-space.
			bool MovePointInsideX(float& pointX, int direction = 0) const;

			/// Gets the first Box encountered in this that contains a specific point.
			/// @param point The point to check for Box collision, in absolute scene coordinates.
			/// @return Pointer to the first Box which was found to contain the point. 0 if
			/// none was found. OWNERSHIP IS NOT TRANSFERRED!
			Box* GetBoxInside(const Vector& point);

			/// Removes the first Box encountered in this that contains a specific point.
			/// @param point The point to check for Box collision, in absolute scene coordinates.
			/// @return Copy of the Box that was removed. Will be  NoArea Box if none was found.
			Box RemoveBoxInside(const Vector& point);

			/// Gets a center point for this of all the boxes waeighted by their sizes.
			/// @return A center point of this area, can be outside the actual area though, if
			/// pulled apart by two separate boxes, for example. 0,0 if this has no Area
			Vector GetCenterPoint() const;

			/// Gets a random coordinate contained within any of this' Box:es.
			/// @return A random point that is within this Area. 0,0 if this has no Area
			Vector GetRandomPoint() const;

			/// Gets the name of the Area
			/// @return The name used to ID this Area.
			std::string GetName() const { return m_Name; }

			/// Protected member variable and method declarations
		protected:
			// The list of Box:es defining the Area in the owner Scene
			std::vector<Box> m_BoxList;
			// The name tag of this Area
			std::string m_Name;

			/// Private member variable and method declarations
		private:
			static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

			/// Clears all the member variables of this Exit, effectively
			/// resetting the members of this abstraction level only.
			void Clear();
		};

		friend class AreaEditor;
		friend class AreaEditorGUI;
		friend class AreaPickerGUI;

#define METABASE_AREA_NAME "MetabaseServiceArea"

		enum NeighborDirection {
			NODIR = -1,
			E = 0,
			SE,
			S,
			SW,
			W,
			NW,
			N,
			NE
		};

		// Concrete allocation and cloning definitions
		EntityAllocation(Scene)

		/// Constructor method used to instantiate a Scene object in system
		/// memory. Create() should be called before using the object.
		Scene();

		/// Destructor method used to clean up a Scene object before deletion
		/// from system memory.
		~Scene() override;

		/// Makes the Scene object ready for use.
		/// @param pNewTerrain The Terrain to use. Ownership IS transferred!
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(SLTerrain* pNewTerrain);

		/// Creates a Scene to be identical to another, by deep copy.
		/// @param reference A reference to the Scene to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const Scene& reference);

		/// Actually loads previously specified/created data into memory. Has
		/// to be done before using this Scene.
		/// @param placeObjects Whetehr to actually place out all the sceneobjects associated with the (default: true)
		/// @param initPathfinding Scene's definition. If not, they still remain in the internal placed (default: true)
		/// objects list. This avoids messing with the MovableMan at all.
		/// @param placeUnits Whether to do pathfinding init, which should be avoided if we are only (default: true)
		/// loading and saving purposes of MetaMan, for example.
		/// Whether to place actors and deployments (doors not affected).
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadData(bool placeObjects = true, bool initPathfinding = true, bool placeUnits = true);

		/// Replace all assembly shemes by corresponding bunker assemblies in
		/// AI plan objects set.
		int ExpandAIPlanAssemblySchemes();

		/// Saves data currently in memory to disk.
		/// @param pathBase The filepath base to the where to save the Bitmap data. This means
		/// everything up to the extension. "FG" and "Mat" etc will be added.
		/// @param doAsyncSaves Whether or not to save asynchronously. (default: true)
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int SaveData(std::string pathBase, bool doAsyncSaves = true);

		/// Saves preview bitmap for this scene.
		/// @param bitmapPath The full filepath the where to save the Bitmap data.
		int SavePreview(const std::string& bitmapPath);

		/// Clears out any previously loaded bitmap data from memory.
		/// @return An error return value signaling success or any particular failure.
		/// Anything below 0 is an error signal.
		int ClearData();

		/// Resets the entire Scene, including its inherited members, to
		/// their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}

		/// Destroys and resets (through Clear()) the Scene object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Makes this an original Preset in a different module than it was before.
		/// It severs ties deeply to the old module it was saved in.
		/// @param whichModule The ID of the new module.
		/// @return Whether the migration was successful. If you tried to migrate to the
		/// same module it already was in, this would return false.
		bool MigrateToModule(int whichModule) override;

		/// Gets the specified location of this scene on the planet view
		/// @return A Vector showing the location of this scene on the planet view.
		Vector GetLocation() const { return m_Location; }

		/// Gets the specified temporary location offset of this scene on the planet view.
		/// @return A Vector showing the temporary location offset of this scene on the planet view.
		Vector GetLocationOffset() const { return m_LocationOffset; }

		/// Sets the specified temporary location offset of this scene on the planet view.
		/// @param newOffset A Vector showing the temporary location offset of this scene on the planet view.
		void SetLocationOffset(Vector newOffset) { m_LocationOffset = newOffset; }

		/// Shows whether this is compatible with metagame play at all.
		/// @return Whether this can be used on the metagame map.
		bool IsMetagamePlayable() const { return m_MetagamePlayable; }

		/// Shows whether this is revealed on the metagame map.
		/// @return Whether this can be seen on the metagame map yet.
		bool IsRevealed() const { return m_Revealed; }

		/// Shows which team owns this Scene in a Metagame, if any
		/// @return The team that owns this site in a Metagame, if any
		int GetTeamOwnership() const { return m_OwnedByTeam; }

		/// Shows how much income this Scene pulls in for its owning team each
		/// round of a metagame.
		/// @return The income in oz that this generates each metagame round.
		float GetRoundIncome() const { return m_RoundIncome; }

		/// Shows how much gold this Scene is budgeted to be built for this round.
		/// @param player Which player's set budget to show.
		/// @return The budget in oz that this is allocated to have built for this round.
		float GetBuildBudget(int player) const { return m_BuildBudget[player]; }

		/// Shows how much of a player's budget this Scene is allocated to be
		/// built for this round.
		/// @param player Which player's set budget ratio to show.
		/// @return The budget in normalized ratio that this is allocated last round.
		float GetBuildBudgetRatio(int player) const { return m_BuildBudgetRatio[player]; }

		/// Sets whether this should be automatically designed by the AI plan
		/// even if owned by human players.
		/// @param autoDesigned What to set the setting to. (default: true)
		void SetAutoDesigned(bool autoDesigned = true) { m_AutoDesigned = autoDesigned; }

		/// Tells whether this should be automatically designed by the AI plan
		/// even if owned by human players.
		/// @return Whether this should be autodesigned or not.
		bool GetAutoDesigned() const { return m_AutoDesigned; }

		/// Sets the total defense investment this scene has experienced by all
		/// teams since the metagame started.
		/// @param totalInvestment What to set the total investment in gold oz) to.
		void SetTotalInvestment(float totalInvestment) { m_TotalInvestment = totalInvestment; }

		/// Gets the total defense investment this scene has experienced by all
		/// teams since the metagame started.
		/// @return The total investment in this scene.
		float GetTotalInvestment() const { return m_TotalInvestment; }

		/// Gets the SLTerrain.
		/// @return A pointer to the SLTerrain. Ownership is NOT transferred!
		SLTerrain* GetTerrain() { return m_pTerrain; }

		/// Gets access to the background layer list.
		/// @return A reference to the std::list containing all the background layers.
		/// Ownership is NOT transferred!
		std::list<SLBackground*>& GetBackLayers() { return m_BackLayerList; }

		/// Adds area to the list if this scene's areas.
		/// @param m_AreaList.push_back(newArea Area to add.
		void AddArea(Scene::Area& newArea) { m_AreaList.push_back(newArea); }

		/// Creates a new SceneLayer for a specific team and fills it with black
		/// pixels that end up being a specific size on the screen.
		/// @param pixelSize A Vector with the desired dimensions of the unseen layer's chunky pixels.
		/// @param team Which team to get the unseen layer for. (default: Activity::TeamOne)
		/// @param createNow Whether to create the unseen layers now, or wait until next time (default: true)
		/// LoadData is called on this.
		void FillUnseenLayer(Vector pixelSize, int team = Activity::TeamOne, bool createNow = true);

		/// Sets the unseen layer of a specific team.
		/// @param pNewLayer The new SceneLayer to use as the new unseen layer, Ownership IS XFERRED!
		/// @param team Which team to get the unseen layer for. (default: Activity::TeamOne)
		void SetUnseenLayer(SceneLayer* pNewLayer, int team = Activity::TeamOne);

		/// Gets the unseen layer of a specific team.
		/// @param team Which team to get the unseen layer for. (default: Activity::TeamOne)
		/// @return A pointer to the SceneLayer representing what hasn't been seen by a
		/// specific team yet. Ownership is NOT transferred!
		SceneLayer* GetUnseenLayer(int team = Activity::TeamOne) const { return team != Activity::NoTeam ? m_apUnseenLayer[team] : 0; }

		/// Gets the list of pixels that have been seen on a team's unseen layer.
		/// @param team Which team to get the unseen layer for. (default: Activity::TeamOne)
		/// @return The list of pixel coordinates in the unseen layer's scale.
		std::list<Vector>& GetSeenPixels(int team = Activity::TeamOne) { return m_SeenPixels[team]; }

		/// Clears the pixels that have been seen on a team's unseen layer.
		/// @param team Which team to get the unseen layer for. (default: Activity::TeamOne)
		void ClearSeenPixels(int team = Activity::TeamOne);

		/// Checks a specific unseen pixel for only having two or less unseen
		/// neighbors, and if so, makes it seen.
		/// @param posX Coordinates to the pixel to check for orphaness.
		/// @param posY The direction we might be checking 'from', ie the neighbor we can
		/// already assume is seen without poking at the unseen map.
		/// @param checkingFrom Which team's unseen layer to check the pixel on. (default: NODIR)
		/// @return Whether the pixel was deemed to be orphan and thus cleaned up.
		bool CleanOrphanPixel(int posX, int posY, NeighborDirection checkingFrom = NODIR, int team = Activity::TeamOne);

		/// Gets the total dimensions (width and height) of the scene, in pixels.
		/// @return A Vector describing the scene dimensions.
		Vector GetDimensions() const;

		/// Gets the total width of the scene, in pixels.
		/// @return An int describing the scene width.
		int GetWidth() const;

		/// Gets the total height of the scene, in pixels.
		/// @return An int describing the scene height.
		int GetHeight() const;

		/// Indicates whether the scene wraps its scrolling around the X axis.
		/// @return Whether the scene wraps around the X axis or not.
		bool WrapsX() const;

		/// Indicates whether the scene wraps its scrolling around the Y axis.
		/// @return Whether the scene wraps around the Y axis or not.
		bool WrapsY() const;

		/// Places the individual brain of a single player which may be stationed
		/// on this Scene, and registers them as such in an Activity.
		/// @param player The player's brain to place.
		/// @param newActivity The Activity to register the placed brains with. OWNERSHIP IS NOT TRANSFERRED!
		/// @return If the brain was successfully found as resident and placed.
		bool PlaceResidentBrain(int player, Activity& newActivity);

		/// Places the individual brains of the various players which may be
		/// stationed on this Scene, and registers them as such in an Activity.
		/// @param newActivity The Activity to register the placed brains with. OWNERSHIP IS NOT TRANSFERRED!
		/// @return How many brains were finally placed.
		int PlaceResidentBrains(Activity& newActivity);

		/// Looks at the Activity and its players' registered brain Actors, and
		/// saves them as resident brains for this Scene. Done when a fight is over
		/// and the survivors remain!
		/// @param oldActivity The Activity to check for registered brains. OWNERSHIP IS NOT TRANSFERRED!
		/// @return How many brains were found registered with the passed in Activity.
		int RetrieveResidentBrains(Activity& oldActivity);

		/// Sucks up all the Actors, Items and Particles currently active in MovableMan and
		/// puts them into this' list of objects to place on next load.
		/// @param transferOwnership The team to only retrieve Actors of. If NoTeam, then all will be grabbed.
		/// @param onlyTeam Whether to not get any brains at all. (default: -1)
		/// @return How many objects were found knocking about in the world, and stored.
		int RetrieveSceneObjects(bool transferOwnership, int onlyTeam = -1, bool noBrains = false);

		/// Gets the list of SceneObject:s which are placed in this scene on loading.
		/// @param whichSet Which set of placed objects to get. See the PlacedObjectSets enum.
		/// @return The list of of placed objects. Ownership is NOT transferred!
		const std::list<SceneObject*>* GetPlacedObjects(int whichSet) const { return &m_PlacedObjects[whichSet]; }

		/// Adds a SceneObject to be placed in this scene. Ownership IS transferred!
		/// @param whichSet Which set of placed objects to add to. See the PlacedObjectSets enum.
		/// @param pObjectToAdd The SceneObject instance to add, OIT!
		/// @param listOrder Where in the list the object should be inserted. -1 means at the end (default: -1)
		/// of the list.
		void AddPlacedObject(int whichSet, SceneObject* pObjectToAdd, int listOrder = -1);

		/// Removes a SceneObject placed in this scene.
		/// @param whichSet Which set of placed objects to rem from. See the PlacedObjectSets enum.
		/// @param whichToRemove The list order number of the object to remove. If -1, the last one is removed. (default: -1)
		void RemovePlacedObject(int whichSet, int whichToRemove = -1);

		/// Returns the last placed object that graphically overlaps an absolute
		/// point in the scene.
		/// @param whichSet Which set of placed objects to pick from. See the PlacedObjectSets enum.
		/// @param scenePoint The point in absolute scene coordinates that will be used to pick the
		/// last placed SceneObject which overlaps it.
		/// @param pListOrderPlace An int which will be filled out with the order place of any found object (default: 0)
		/// in the list. if nothing is found, it will get a value of -1.
		/// @return The last hit SceneObject, if any. Ownership is NOT transferred!
		const SceneObject* PickPlacedObject(int whichSet, Vector& scenePoint, int* pListOrderPlace = 0) const;

		/// Returns the last placed actor object that is closer than range to scenePoint
		/// @param whichSet Which set of placed objects to pick from. See the PlacedObjectSets enum.
		/// @param scenePoint The point in absolute scene coordinates that will be used to pick the
		/// closest placed SceneObject near it.
		/// @param range The range to check for nearby objects.
		/// @param pListOrderPlace An int which will be filled out with the order place of any found object
		/// in the list. if nothing is found, it will get a value of -1.
		/// @return The closest actor SceneObject, if any. Ownership is NOT transferred!
		const SceneObject* PickPlacedActorInRange(int whichSet, Vector& scenePoint, int range, int* pListOrderPlace) const;

		/// Updated the objects in the placed scene objects list of this. This is
		/// mostly for the editor to represent the items correctly.
		/// @param whichSet Which set of placed objects to update. See the PlacedObjectSets enum.
		void UpdatePlacedObjects(int whichSet);

		/// Removes all entries in a specific set of placed Objects.
		/// @param whichSet Which set of placed objects to clear. See the PlacedObjectSets enum.
		/// @param weHaveOwnership Whether or not we have ownership of these items, and should delete them. (default: true)
		/// @return How many things were removed in teh process of clearing that set.
		int ClearPlacedObjectSet(int whichSet, bool weHaveOwnership = true);

		/// Gets the resident brain Actor of a specific player from this scene,
		/// if there is any. OWNERSHIP IS NOT TRANSFERRED!
		/// @param player Which player to get the resident brain of.
		/// @return The SO containing the brain, or 0 if there aren't any of that player.
		SceneObject* GetResidentBrain(int player) const;

		/// Sets the resident brain Actor of a specific player from this scene,
		/// if there is any. Ownership IS transferred!
		/// @param player Which player to set the resident brain of.
		/// @param pNewBrain The Actor to set as the resident brain of the specified player.
		void SetResidentBrain(int player, SceneObject* pNewBrain);

		/// Gets the number of brains currently residing in this scene.
		/// @return The number of resident brains who are installed here.
		int GetResidentBrainCount() const;

		/// Adds one or modifies an existing area of this Scene.
		/// @param newArea The area to add or modify of the same name in this Scene. Ownership is
		/// NOT transferred!
		/// @return Whether the specified area was previously defined in this scene.
		bool SetArea(Area& newArea);

		/// Checks for the existence of a specific Area identified by a name.
		/// This won't throw any errors to the console if the Area isn't found.
		/// @param areaName The name of the Area to try to find in this Scene.
		/// @return Whether the specified area is defined in this Scene.
		bool HasArea(std::string areaName);

		/// Gets a specified Area identified by name. Ownership is NOT transferred!
		/// @param areaName The name of the Area to try to get.
		/// @param required Whether the area is required, and should throw an error if not found.
		/// @return A pointer to the Area asked for, or nullptr if no Area of that name was found.
		Area* GetArea(const std::string_view& areaName, bool required);

		/// Gets a specified Area identified by name. Ownership is NOT transferred!
		/// @param areaName The name of the Area to try to get.
		/// @return A pointer to the Area asked for, or nullptr if no Area of that name was found.
		Area* GetArea(const std::string& areaName) { return GetArea(areaName, true); }

		/// Gets a specified Area identified by name, showing a Lua warning if it's not found. Ownership is NOT transferred!
		/// Using this function will not add the area to the list of required areas which Scenario GUI uses to show compatible areas.
		/// @param areaName The name of the Area to try to get.
		/// @return A pointer to the Area asked for, or nullptr if no Area of that name was found.
		Area* GetOptionalArea(const std::string& areaName) { return GetArea(areaName, false); }

		void AddNavigatableArea(const std::string& areaName) {
			m_NavigatableAreas.push_back(areaName);
			m_NavigatableAreasUpToDate = false;
		}
		void ClearNavigatableAreas(const std::string& areaName) {
			m_NavigatableAreas.clear();
			m_NavigatableAreasUpToDate = false;
		}

		/// Removes a specific Area identified by a name.
		/// @param areaName The name of the Area to try to remove.
		/// @return Whether an Area of that name was found, and subsequently removed.
		bool RemoveArea(std::string areaName);

		/// Checks if a point is within a specific named Area of this Scene. If
		/// no Area of the name is found, this just returns false without error.
		/// @param areaName The name of the Area to try to check against.
		/// @param point The point to see if it's within the specified Area.
		/// @return Whether any Area of that name was found, AND the point falls within it.
		bool WithinArea(std::string areaName, const Vector& point) const;

		/// Gets the global acceleration (in m/s^2) that is applied to all movable
		/// objects' velocities during every frame. Typically models gravity.
		/// @return A Vector describing the global acceleration.
		Vector GetGlobalAcc() const { return m_GlobalAcc; }

		/// Sets the global acceleration (in m/s^2) that is applied to all movable
		/// objects' velocities during every frame. Typically models gravity.
		/// @param newValue A Vector describing the global acceleration.
		void SetGlobalAcc(Vector newValue) { m_GlobalAcc = newValue; }

		/// Returns parent scene name of this metascene.
		/// @return Name of a parent scene.
		std::string GetMetasceneParent() const { return m_MetasceneParent; }

		/// Sets the specified location of this Scene in the scene
		/// @param newLocation A Vector with the desired location of this Scene in the scene.
		void SetLocation(const Vector& newLocation) { m_Location = newLocation; }

		/// Sets whether this can be played in the Metagame map at all.
		/// @param isPlayable Whether this is compatible with metagame play at all.
		void SetMetagamePlayable(bool isPlayable) { m_MetagamePlayable = isPlayable; }

		/// Sets whether this should show up on the Metagame map yet.
		/// @param isRevealed Whether to reveal this on the metagame map or not.
		void SetRevealed(bool isRevealed) { m_Revealed = isRevealed; }

		/// Sets the team who owns this Scene in a Metagame
		/// @param newTeam The team who should now own this Scene
		void SetTeamOwnership(int newTeam);

		/// Sets how much gold this Scene is budgeted to be built for this round.
		/// @param player The player whom is setting the budget.
		/// @param budget The budget in oz that this is allocated to have built for this round.
		void SetBuildBudget(int player, float budget) { m_BuildBudget[player] = budget; }

		/// Sets how much of a player's budget this Scene is budgeted to be build
		/// for each turn.
		/// @param player The player whom is setting the budget ratio.
		/// @param budgetRatio The budget in normalized ratio that this is allocated of the total.
		void SetBuildBudgetRatio(int player, float budgetRatio) { m_BuildBudgetRatio[player] = budgetRatio; }

		/// Figure out exactly how much of the build budget would be used if
		/// as many blueprint objects as can be afforded and exists would be built.
		/// @param player The player for whom we are calculating this budget use.
		/// @param pAffordCount An optional int that will be filled with number of objects that can (default: 0)
		/// acutally be built.
		/// @param pAffordAIPlanCount An optional int that will be filled with number of objects that can (default: 0)
		/// built out of the AI plan set, AFTER the blueprints are built.
		/// @return The amount of funds that would be applied to the building of objects.
		float CalcBuildBudgetUse(int player, int* pAffordCount = 0, int* pAffordAIPlanCount = 0) const;

		/// Puts the pre-built AI base plan into effect by transferring as many
		/// pieces as the current base budget allows from the AI plan to the actual
		/// blueprints to be built at this Scene.
		/// @param player The AI player whom is putting his plans into motion.
		/// @param pObjectsApplied An optional int that will be filled with number of objects that were (default: 0)
		/// acutally moved from the AI plan to the blueprints.
		/// @return The value of the AI plan objects that were put onto the blueprints.
		float ApplyAIPlan(int player, int* pObjectsApplied = 0);

		/// Actually builds as many objects in the specific player's Blueprint
		/// list as can be afforded by his build budget. The budget is deducted
		/// accordingly.
		/// @param player The player whom is using his budget.
		/// @param pObjectsBuilt An optional int that will be filled with number of objects that were (default: 0)
		/// acutally built.
		/// @return The amount of funds that were applied to the building of objects.
		float ApplyBuildBudget(int player, int* pObjectsBuilt = 0);

		/// Remove all actors that are in the placed set of objects to load for
		/// this scene. All except for an optionally specified team, that is.
		/// @param exceptTeam Remove all actors but of this team. (default: -1)
		/// @return How many actors were actually removed.
		int RemoveAllPlacedActors(int exceptTeam = -1);

		/// Sets the ownership of all doors placed in this scene to a specific team
		/// @param team The team to change the ownership to
		/// @param player The player which placed these doors.
		/// @return How many doors were actually affected.
		int SetOwnerOfAllDoors(int team, int player);

		/// Tells whether a specific team has scheduled an orbital Scan of this.
		/// @param team The team to check for.
		/// @return Whether the scan has been scheduled and paid for.
		bool IsScanScheduled(int team) const { return m_ScanScheduled[team]; }

		/// Sets this to be orbitally scanned by a specific team on next load.
		/// @param team The team to schedule the scan for.
		/// @param scan Whether to actually schedule the scan or clear it. (default: true)
		void SetScheduledScan(int team, bool scan = true) { m_ScanScheduled[team] = scan; }

		/// Recalculates all of the pathfinding data. This is very expensive, so
		/// do very rarely!
		void ResetPathFinding();

		/// Blocks this thread until all pathing requests are completed.
		void BlockUntilAllPathingRequestsComplete();

		/// Recalculates only the areas of the pathfinding data that have been
		/// marked as outdated.
		void UpdatePathFinding();

		/// Tells whether the pathfinding data has been updated in the last frame.
		/// @return Whether the pathfinding data was recalculated fully or partially.
		bool PathFindingUpdated() { return m_PathfindingUpdated; }

		/// Calculates and returns the least difficult path between two points on
		/// the current scene. Takes both distance and materials into account.
		/// When pathing using the NoTeam pathFinder, no doors are considered passable.
		/// @param start Start and end positions on the scene to find the path between.
		/// @param end A list which will be filled out with waypoints between the start and end.
		/// @param pathResult The maximum material strength any actor traveling along the path can dig through.
		/// @param digStrength The team we're pathing for (doors for this team will be considered passable) (default: c_PathFindingDefaultDigStrength)
		/// @return The total minimum difficulty cost calculated between the two points on
		/// the scene.
		float CalculatePath(const Vector& start, const Vector& end, std::list<Vector>& pathResult, float digStrength = c_PathFindingDefaultDigStrength, Activity::Teams team = Activity::Teams::NoTeam);

		/// Asynchronously calculates the least difficult path between two points on the current Scene. Takes both distance and materials into account.
		/// When pathing using the NoTeam pathFinder, no doors are considered passable.
		/// @param start Start position of the pathfinding request.
		/// @param end End position of the pathfinding request.
		/// @param digStrength The maximum material strength any actor traveling along the path can dig through.
		/// @param team The team we're pathing for (doors for this team will be considered passable)
		/// @return A shared pointer to the volatile PathRequest to be used to track whehter the asynchrnous path calculation has been completed, and check its results.
		std::shared_ptr<volatile PathRequest> CalculatePathAsync(const Vector& start, const Vector& end, float digStrength = c_PathFindingDefaultDigStrength, Activity::Teams team = Activity::Teams::NoTeam, PathCompleteCallback callback = nullptr);

		/// Gets how many waypoints there are in the ScenePath currently
		/// @return The number of waypoints in the ScenePath.
		int GetScenePathSize() const;

		std::list<Vector>& GetScenePath();

		/// Returns whether two position represent the same path nodes.
		/// @param pos1 First coordinates to compare.
		/// @param pos2 Second coordinates to compare.
		/// @return Whether both coordinates represent the same path node.
		bool PositionsAreTheSamePathNode(const Vector& pos1, const Vector& pos2) const;

		/// Updates the state of this Scene. Supposed to be done every frame
		/// before drawing.
		void Update();

		/// Whether this scene is a temprorary metagame scene and should
		/// not be used anywhere except in metagame.
		/// @return Whether scene belongs to metagame or not.
		bool IsMetagameInternal() const { return m_IsMetagameInternal; }

		/// Sets whether this scene is a temprorary metagame scene and should
		/// not be used anywhere except in metagame.
		/// @param newValue New value.
		void SetMetagameInternal(bool newValue) { m_IsMetagameInternal = newValue; }

		/// Gets whether this Scene is a saved game Scene copy and should not be used anywhere except for game saving and loading.
		/// @return Whether this Scene is a saved game Scene copy.
		bool IsSavedGameInternal() const { return m_IsSavedGameInternal; }

		/// Sets whether this Scene is a saved game Scene copy and should not be used anywhere except for game saving and loading.
		/// @param newValue Whether this Scene is a saved game Scene copy.
		void SetSavedGameInternal(bool newValue) { m_IsSavedGameInternal = newValue; }

		/// Returns preview bitmap pointer for this scene.
		/// @return Pointer to preview bitmap.
		BITMAP* GetPreviewBitmap() const { return m_pPreviewBitmap; };

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		// Position of the site/scene on the planet in the site selection menu view, relative to the center of the planet
		Vector m_Location;
		// Temporary location offset used to correct scene position when scene dots overlap.
		Vector m_LocationOffset;
		// Whether at all eligible for the Metagame
		bool m_MetagamePlayable;
		// Whether this is revealed on the metagame planet map yet or not
		bool m_Revealed;
		// Owned by which Team, if any (<0 if none)
		int m_OwnedByTeam;
		// Total income this place generates per Metagame round for its owner team
		float m_RoundIncome;
		// The special placed brain actors of each player that inhabit this Scene, OWNED here
		SceneObject* m_ResidentBrains[Players::MaxPlayerCount];
		// Budget in oz this place is allocated per player for a metagame round for building (applying) blueprint objects.
		float m_BuildBudget[Players::MaxPlayerCount];
		// Budget in ratio of the player for a metagame round. This is used to re-set the BuildBudget to match the ratio
		// that a player budgeted to this site in the previous turn.
		float m_BuildBudgetRatio[Players::MaxPlayerCount];
		// Whether this should be automatically designed by the AI Plan even if it's owned by a human player
		bool m_AutoDesigned;
		// The total amount of gold (in oz) that has been invested in the defenses of this site, by all teams
		float m_TotalInvestment;
		// Terrain definition
		SLTerrain* m_pTerrain;

		// Pathfinding graph and logic. Owned by this
		// The array of PathFinders for each team. Because we also have a shared pathfinder using index 0, we need to use MaxTeamCount + 1 to handle all the Teams' PathFinders.
		std::array<std::unique_ptr<PathFinder>, Activity::Teams::MaxTeamCount + 1> m_pPathFinders;
		// Is set to true on any frame the pathfinding data has been updated
		bool m_PathfindingUpdated;
		// Timer for when to do an update of the pathfinding data
		Timer m_PartialPathUpdateTimer;

		// SceneObject:s to be placed in the scene, divided up by different sets - OWNED HERE
		std::list<SceneObject*> m_PlacedObjects[PLACEDSETSCOUNT];
		// List of background layers, first is the closest to the terrain, last is closest to the back
		std::list<SLBackground*> m_BackLayerList;
		// Dimensions of the pixels of the unseen layers, when they are dynamically generated. If 0, the layer was not generated
		Vector m_UnseenPixelSize[Activity::MaxTeamCount];
		// Layers representing the unknown areas for each team
		SceneLayer* m_apUnseenLayer[Activity::MaxTeamCount];
		// Which pixels of the unseen map have just been revealed this frame, in the coordinates of the unseen map
		std::list<Vector> m_SeenPixels[Activity::MaxTeamCount];
		// Pixels on the unseen map deemed to be orphans and cleaned up, will be moved to seen pixels next update
		std::list<Vector> m_CleanedPixels[Activity::MaxTeamCount];
		// Whether this Scene is scheduled to be orbitally scanned by any team
		bool m_ScanScheduled[Activity::MaxTeamCount];

		// List of all the specified Area's of the scene
		std::list<Area> m_AreaList;

		// List of navigatable areas in the scene. If this list is empty, the entire scene is assumed to be navigatable
		std::vector<std::string> m_NavigatableAreas;
		bool m_NavigatableAreasUpToDate;

		// The global acceleration vector in m/s^2. (think gravity/wind)
		Vector m_GlobalAcc;
		// Names of all Schemes and selected assemblies for them
		std::map<std::string, const BunkerAssembly*> m_SelectedAssemblies;
		// Amounts of limited assemblies
		std::map<std::string, int> m_AssembliesCounts;
		// Scene preview bitmap
		BITMAP* m_pPreviewBitmap;
		// Scene preview source file
		ContentFile m_PreviewBitmapFile;
		// Name of a scene which can be replaced by this scene in MetaGame
		// Scenes with m_MetaSceneParent field set will be invisible for editors and activities unless
		// ShowMetaScenes flag in settings.ini is set
		std::string m_MetasceneParent;

		// Whether this scene must be shown anywhere in UIs
		bool m_IsMetagameInternal;
		bool m_IsSavedGameInternal;

		std::list<Deployment*> m_Deployments;

		/// Private member variable and method declarations
	private:
		/// Gets the pathfinder for a given team.
		/// @param team The team to get the pathfinder for. NoTeam is valid, and will give a shared pathfinder.
		/// @return A reference to the pathfinder for the given team.
		PathFinder& GetPathFinder(Activity::Teams team);

		/// Serializes the SceneObject via the Writer. Necessary because full serialization doesn't know how to deal with duplicate properties.
		/// @param writer The Writer being used for serialization.
		/// @param sceneObjectToSave The SceneObject to save.
		/// @param isChildAttachable Convenience flag for whether or not this SceneObject is a child Attachable, and certain properties shouldn't be saved.
		/// @param saveFullData Whether or not to save most data. Turned off for stuff like SceneEditor saves.
		void SaveSceneObject(Writer& writer, const SceneObject* sceneObjectToSave, bool isChildAttachable, bool saveFullData) const;

		/// Clears all the member variables of this Scene, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Scene(const Scene& reference) = delete;
		void operator=(const Scene& rhs) = delete;
	};

} // namespace RTE
