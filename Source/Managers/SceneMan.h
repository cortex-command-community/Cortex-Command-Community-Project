#pragma once

/// Header file for the SceneMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Serializable.h"
#include "Timer.h"
#include "Box.h"
#include "Singleton.h"
#include "SpatialPartitionGrid.h"

#include "ActivityMan.h"

#include <map>
#include <array>
#include <list>
#include <vector>
#include <string>
#include <utility>

#define g_SceneMan SceneMan::Instance()

namespace RTE {

	class Scene;
	class SceneLayer;
	class SceneLayerTracked;
	class SLTerrain;
	class SceneObject;
	class TerrainObject;
	class MovableObject;
	class MOPixel;
	class Material;
	class SoundContainer;
	struct PostEffect;

	// Different modes to draw the SceneLayers in
	enum LayerDrawMode {
		g_LayerNormal = 0,
		g_LayerTerrainMatter,
	};

#define SCENEGRIDSIZE 24
#define SCENESNAPSIZE 12
#define MAXORPHANRADIUS 11

	/// The singleton manager of all terrain and backgrounds in the RTE.
	class SceneMan : public Singleton<SceneMan>, public Serializable {
		friend class SettingsMan;

		/// Public member variable, method and friend function declarations
	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Constructor method used to instantiate a SceneMan object in system
		/// memory. Create() should be called before using the object.
		SceneMan();

		/// Destructor method used to clean up a SceneMan object before deletion
		/// from system memory.
		~SceneMan();

		/// Makes the SceneMan object ready for use.
		void Initialize() const;

		/// Makes the SceneMan object ready for use.
		/// @param readerFile A string with the filepath to a Reader file from screen this SceneMan's
		/// data should be created.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(std::string readerFile);

		/// Sets the instance name of the default Scene to be loaded if nothing
		/// else is available.
		/// @param defaultSceneName The default scene instance name.
		void SetDefaultSceneName(std::string defaultSceneName) { m_DefaultSceneName = defaultSceneName; }

		/// Gets the name of the default A to be loaded if nothing
		/// else is available.
		/// @return The default Scene instance name.
		std::string GetDefaultSceneName() const { return m_DefaultSceneName; }

		/// Actually loads a new Scene into memory. has to be done before using
		/// this object.
		/// @param pNewScene The instance of the Scene, ownership IS transferred!
		/// @param placeObjects Whether the scene should actually apply all its SceneObject:s placed (default: true)
		/// in its definition.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadScene(Scene* pNewScene, bool placeObjects = true, bool placeUnits = true);

		/// Stores a Scene reference to be loaded later into the SceneMan.
		/// @param pLoadScene The instance reference of the Scene, ownership IS NOT (!!) transferred!
		/// @param placeObjects Whether the scene should actually apply all its SceneObject:s placed (default: true)
		/// in its definition.
		void SetSceneToLoad(const Scene* pLoadScene, bool placeObjects = true, bool placeUnits = true) {
			m_pSceneToLoad = pLoadScene;
			m_PlaceObjects = placeObjects;
			m_PlaceUnits = placeUnits;
		}

		/// Sets a scene to load later, by preset name.
		/// @param sceneName The name of the Scene preset instance to load.
		/// @param placeObjects Whether the scene should actually apply all its SceneObject:s placed (default: true)
		/// in its definition.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int SetSceneToLoad(std::string sceneName, bool placeObjects = true, bool placeUnits = true);

		/// Gets the stored Scene reference to be loaded later into the SceneMan.
		/// @return The instance reference of the Scene, ownership IS NOT (!!) transferred!
		const Scene* GetSceneToLoad() { return m_pSceneToLoad; }

		/// Gets whether objects are placed when the Scene is initially started. Used for saving/loading games.
		/// @return Whether objects are placed when the Scene is initially started.
		bool GetPlaceObjectsOnLoad() const { return m_PlaceObjects; }

		/// Gets whether units are placed when the Scene is initially started. Used for saving/loading games.
		/// @return Whether units are placed when the Scene is initially started.
		bool GetPlaceUnitsOnLoad() const { return m_PlaceUnits; }

		/// Actually loads the Scene set to be loaded in SetSceneToLoad.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadScene();

		/// Loads a Scene right now, by preset name.
		/// @param sceneName The name of the Scene preset instance to load.
		/// @param placeObjects Whether the scene should actually apply all its SceneObject:s placed (default: true)
		/// in its definition.
		/// @param placeUnits Whether the scene should actually deploy all units placed in its definition. (default: true)
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadScene(std::string sceneName, bool placeObjects = true, bool placeUnits = true);

		/// Loads a Scene right now, by preset name.
		/// @param sceneName The name of the Scene preset instance to load.
		/// @param placeObjects Whether the scene should actually apply all its SceneObject:s placed (default: true) { return LoadScene(sceneName)
		/// in its definition.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int LoadScene(std::string sceneName, bool placeObjects = true) { return LoadScene(sceneName, placeObjects, true); }

		/// Resets the entire SceneMan, including its inherited members, to
		/// their default settings or values.
		void Reset() override { Clear(); }

		/// Destroys and resets (through Clear()) the SceneMan object.
		void Destroy();

		/// Gets the currently loaded scene, if any.
		/// @return The scene, ownership IS NOT TRANSFERRED!
		Scene* GetScene() const { return m_pCurrentScene; }

		/// Gets the total dimensions (width and height) of the scene, in pixels.
		/// @return A Vector describing the scene dimensions.
		Vector GetSceneDim() const;

		/// Gets the total width of the scene, in pixels.
		/// @return An int describing the scene width.
		int GetSceneWidth() const;

		/// Gets the total height of the scene, in pixels.
		/// @return An int describing the scene width.
		int GetSceneHeight() const;

		/// Gets access to the whole material palette array of 256 entries.
		/// @return A const reference to the material palette array.
		const std::array<Material*, c_PaletteEntriesNumber>& GetMaterialPalette() const { return m_apMatPalette; }

		/// Gets a specific material by name. Ownership is NOT transferred!
		/// @param matName The string name of the Material to get.
		/// @return A pointer to the requested material, or 0 if no material with that
		/// name was found.
		Material const* GetMaterial(const std::string& matName);

		/// Gets a specific material from the material palette. Ownership is NOT
		/// transferred!
		/// @param screen The unsigned char index specifying screen material to get (0-255).
		/// @return A reference to the requested material. OWNERSHIP IS NOT TRANSFERRED!
		Material const* GetMaterialFromID(unsigned char screen) { return screen >= 0 && screen < c_PaletteEntriesNumber && m_apMatPalette[screen] ? m_apMatPalette[screen] : m_apMatPalette[g_MaterialAir]; }

		/// Indicates whether the scene wraps its scrolling around the X axis.
		/// @return Whether the scene wraps around the X axis or not.
		bool SceneWrapsX() const;

		/// Indicates whether the scene wraps its scrolling around the Y axis.
		/// @return Whether the scene wraps around the Y axis or not.
		bool SceneWrapsY() const;

		/// Gets the orbit direction for the current scene.
		/// @return The orbit direction for the current scene.
		Directions GetSceneOrbitDirection() const;

		/// Gets the SLTerrain, or 0 if no scene is loaded.
		/// @return A pointer to the SLTerrain. Ownership is NOT transferred!
		SLTerrain* GetTerrain();

		/// Gets the bitmap of the intermediary collection SceneLayer that all
		/// MovableObject:s draw themselves onto before it itself gets drawn onto
		/// the screen back buffer.
		/// @return A BITMAP pointer to the MO bitmap. Ownership is NOT transferred!
		BITMAP* GetMOColorBitmap() const;

		/// Gets the bitmap of the SceneLayer that debug graphics is drawn onto.
		/// Will only return valid BITMAP if building with DEBUG_BUILD.
		/// @return A BITMAP pointer to the debug bitmap. Ownership is NOT transferred!
		BITMAP* GetDebugBitmap() const;

		/// Gets the current drawing mode of the SceneMan.
		/// @return The current layer draw mode, see the LayerDrawMode enumeration for the
		/// different possible mode settings.
		int GetLayerDrawMode() const { return m_LayerDrawMode; }

		/// Gets a specific pixel from the total material representation of
		/// this Scene. LockScene() must be called before using this method.
		/// @param pixelX The X and Y coordinates of screen material pixel to get.
		/// @return An unsigned char specifying the requested pixel's material index.
		unsigned char GetTerrMatter(int pixelX, int pixelY);

		/// Gets a MOID from pixel coordinates in the Scene. LockScene() must be called before using this method.
		/// @param pixelX The X coordinate of the Scene pixel to test.
		/// @param pixelY The Y coordinate of the Scene pixel to test.
		/// @param ignoreTeam The team to ignore.
		/// @return The MOID currently at the specified pixel coordinates.
		MOID GetMOIDPixel(int pixelX, int pixelY, int ignoreTeam);

		/// Gets a MOID from pixel coordinates in the Scene. LockScene() must be called before using this method.
		/// @param pixelX The X coordinate of the Scene pixel to test.
		/// @param pixelY The Y coordinate of the Scene pixel to test.
		/// @return The MOID currently at the specified pixel coordinates.
		MOID GetMOIDPixel(int pixelX, int pixelY) { return GetMOIDPixel(pixelX, pixelY, Activity::NoTeam); }

		/// Gets this Scene's MOID SpatialPartitionGrid.
		/// @return This Scene's MOID SpatialPartitionGrid.
		const SpatialPartitionGrid& GetMOIDGrid() const { return m_MOIDsGrid; }

		/// Gets the global acceleration (in m/s^2) that is applied to all movable
		/// objects' velocities during every frame. Typically models gravity.
		/// @return A Vector describing the global acceleration.
		Vector GetGlobalAcc() const;

		/// Gets how many Ounces there are in a metric Kilogram
		/// @return A float describing the Oz/Kg ratio.
		float GetOzPerKg() const { return 35.27396; }

		/// Gets how many metric Kilograms there are in an Ounce.
		/// @return A float describing the Kg/Oz ratio.
		float GetKgPerOz() const { return 0.02834952; }

		/// Sets the drawing mode of the SceneMan, to easily view what's going on
		/// in the different internal SceneLayer:s.
		/// @param mode The layer mode to draw in, see the LayerDrawMode enumeration for the
		/// different possible settings.
		void SetLayerDrawMode(int mode) { m_LayerDrawMode = mode; }

		/// Registers an area to be drawn upon, so it can be tracked and cleared later.
		/// @param bitmap The bitmap being drawn upon.
		/// @param moid The MOID, if we're drawing MOIDs.
		/// @param left The left boundary of the draw area.
		/// @param top The top boundary of the drawn area.
		/// @param right The right boundary of the draw area.
		/// @param bottom The bottom boundary of the draw area.
		void RegisterDrawing(const BITMAP* bitmap, int moid, int left, int top, int right, int bottom);

		/// Registers an area of to be drawn upon, so it can be tracked and cleared later.
		/// @param bitmap The bitmap being drawn upon.
		/// @param moid The MOID, if we're drawing MOIDs.
		/// @param center The centre position of the drawn area.
		/// @param radius The radius of the drawn area.
		void RegisterDrawing(const BITMAP* bitmap, int moid, const Vector& center, float radius);

		/// Clears all registered drawn areas of the MOID layer to the g_NoMOID color and clears the registrations too. Should be done each sim update.
		void ClearAllMOIDDrawings();

		/// Test whether a pixel of the scene would be knocked loose and turned into a MO by another particle of a certain material going at a certain velocity.
		/// @param posX The X and Y coords of the scene pixel that is collided with.
		/// @param posY The velocity of the incoming particle.
		/// @param velocity The mass of the incoming particle.
		/// @return A bool indicating wether the scene pixel would be knocked loose or
		/// not. If the pixel location specified happens to be of the air
		/// material (0) false will be returned here.
		bool WillPenetrate(const int posX,
		                   const int posY,
		                   const Vector& velocity,
		                   const float mass) { return WillPenetrate(posX, posY, velocity * mass); }

		/// Test whether a pixel of the scene would be knocked loose and turned into a MO by a certain impulse force.
		/// @param posX The X and Y coords of the scene pixel that is collided with.
		/// @param posY The impulse force vector, in Kg * m/s.
		/// @return A bool indicating wether the scene pixel would be knocked loose or
		/// not. If the pixel location specified happens to be of the air
		/// material (0) false will be returned here.
		bool WillPenetrate(const int posX,
		                   const int posY,
		                   const Vector& impulse);

		/// Calculate whether a pixel of the scene would be knocked loose and turned into a MO by another particle of a certain material going at a certain velocity.
		/// If so, the incoming particle will knock loose the specified pixel in the scene and momentarily take its place.
		/// @param posX The X and Y coord of the scene pixel that is to be collided with.
		/// @param posY The impulse force exerted on the terrain pixel. If this magnitude
		/// exceeds the strength threshold of the material of the terrain pixel
		/// hit, the terrain pixel will be knocked loose an turned into an MO.
		/// @param impulse The velocity of the the point hitting the terrain here.
		/// @param velocity A float reference screen will be set to the factor with screen to
		/// multiply the collision velocity to get the resulting retardation
		/// (negative acceleration) that occurs when a penetration happens.
		/// @param retardation The normalized probability ratio between 0.0 and 1.0 that determines
		/// the chance of a penetration to remove a pixel from the scene and
		/// thus replace it with and air pixel. 1.0 = always, 0.0 = never.
		/// @param airRatio How many consecutive penetrations in a row immediately before this try.
		/// @param numPenetrations The size of the area to look for orphaned terrain elements. (default: 0)
		/// @param removeOrphansRadius Max area or orphaned area to remove. (default: 0)
		/// @param removeOrphansMaxArea Orphan area removal trigger rate. (default: 0)
		/// @return A bool indicating wether the scene pixel was knocked loose or not.
		/// If the pixel location specified happens to be of the air material (0)
		/// false will be returned here.
		bool TryPenetrate(int posX,
		                  int posY,
		                  const Vector& impulse,
		                  const Vector& velocity,
		                  float& retardation,
		                  const float airRatio,
		                  const int numPenetrations = 0,
		                  const int removeOrphansRadius = 0,
		                  const int removeOrphansMaxArea = 0,
		                  const float removeOrphansRate = 0.0);

		/// Returns the area of an orphaned region at specified coordinates.
		/// @param posX Coordinates to check for region, whether the orphaned region should be converted into MOPixels and region removed.
		/// @param posY Area of orphaned object calculated during recursve function call to check if we're out of limits
		/// @param radius Size of the are to look for orphaned objects
		/// @param maxArea Max area of orphaned object to remove
		/// @param remove Whether to actually remove orphaned pixels or not (default: false)
		/// Whether to clear internal terrain tracking bitmap or not
		/// @return The area of orphaned region at posX,posY
		int RemoveOrphans(int posX, int posY, int radius, int maxArea, bool remove = false);

		/// Returns the area of an orphaned region at specified coordinates.
		/// @param posX Coordinates to check for region, whether the orphaned region should be converted into MOPixels and region removed.
		/// @param posY Coordinates of initial terrain penetration to check, which serves as a center of orphaned object detection.
		/// @param centerPosX Area of orphaned object calculated during recursve function call to check if we're out of limits
		/// @param centerPosY Size of the are to look for orphaned objects
		/// @param accumulatedArea Max area of orphaned object to remove
		/// @param radius Whether to actually remove orphaned pixels or not
		/// @param maxArea Whether to clear internal terrain tracking bitmap or not
		/// @return The area of orphaned region at posX,posY
		int RemoveOrphans(int posX,
		                  int posY,
		                  int centerPosX,
		                  int centerPosY,
		                  int accumulatedArea,
		                  int radius,
		                  int maxArea,
		                  bool remove = false);

		/// Removes a pixel from the terrain and adds it to MovableMan.
		/// @param posX The X coordinate of the terrain pixel.
		/// @param posX The Y coordinate of the terrain pixel.
		/// @return The newly dislodged pixel, if one was found.
		MOPixel* DislodgePixel(int posX, int posY);

		/// Removes a pixel from the terrain and adds it to MovableMan.
		/// @param posX The X coordinate of the terrain pixel.
		/// @param posX The Y coordinate of the terrain pixel.
		/// @param deletePixel Whether or not to immediately mark the pixel for deletion.
		/// @return The newly dislodged pixel, if one was found.
		MOPixel* DislodgePixelBool(int posX, int posY, bool deletePixel);

		/// Removes a circle of pixels from the terrain and adds them to MovableMan.
		/// @param centre The vector position of the centre of the circle.
		/// @param radius The radius of the circle of pixels to remove.
		/// @param deletePixels Whether or not to immediately mark all found pixels for deletion.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelCircle(const Vector& centre, float radius, bool deletePixels);

		/// Removes a circle of pixels from the terrain and adds them to MovableMan.
		/// @param centre The vector position of the centre of the circle.
		/// @param radius The radius of the circle of pixels to remove.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelCircleNoBool(const Vector& centre, float radius);

		/// Removes a ring of pixels from the terrain and adds them to MovableMan.
		/// @param centre The vector position of the centre of the ring.
		/// @param innerRadius The inner radius of the ring of pixels to remove.
		/// @param outerRadius The outer radius of the ring of pixels to remove.
		/// @param deletePixels Whether or not to immediately mark all found pixels for deletion.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelRing(const Vector& centre, float innerRadius, float outerRadius, bool deletePixels);

		/// Removes a ring of pixels from the terrain and adds them to MovableMan.
		/// @param centre The vector position of the centre of the ring.
		/// @param innerRadius The inner radius of the ring of pixels to remove.
		/// @param outerRadius The outer radius of the ring of pixels to remove.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelRingNoBool(const Vector& centre, float innerRadius, float outerRadius);

		/// Removes a box of pixels from the terrain and adds them to MovableMan.
		/// @param upperLeftCorner The vector position of the upper left corner of the box.
		/// @param lowerRightCorner The vector position of the lower right corner of the box.
		/// @param deletePixels Whether or not to immediately mark all found pixels for deletion.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelBox(const Vector& upperLeftCorner, const Vector& lowerRightCorner, bool deletePixels);

		/// Removes a box of pixels from the terrain and adds them to MovableMan.
		/// @param upperLeftCorner The vector position of the upper left corner of the box.
		/// @param lowerRightCorner The vector position of the lower right corner of the box.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelBoxNoBool(const Vector& upperLeftCorner, const Vector& lowerRightCorner);

		/// Removes a line of pixels from the terrain and adds them to MovableMan.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param skip For every pixel checked along the line, how many to skip between them.
		/// @param deletePixels Whether or not to immediately mark all found pixels for deletion.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelLine(const Vector& start, const Vector& ray, int skip, bool deletePixels);

		/// Removes a line of pixels from the terrain and adds them to MovableMan.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param skip For every pixel checked along the line, how many to skip between them.
		/// @return A list of the removed pixels, if any.
		std::vector<MOPixel*>* DislodgePixelLineNoBool(const Vector& start, const Vector& ray, int skip);

		/// Sets one team's view of the scene to be unseen, using a generated map
		/// of a specific resolution chunkiness.
		/// @param pixelSize The dimensions of the pixels that should make up the unseen layer.
		/// @param team The team we're talking about.
		void MakeAllUnseen(Vector pixelSize, const int team);

		/// Sets one team's view of the scene to be all seen.
		/// @param team The team we're talking about.
		void MakeAllSeen(const int team);

		/// Loads a bitmap from file and use it as the unseen layer for a team.
		/// @param bitmapPath The path to the bitmap to use as the unseen layer.
		/// @param team Which team we're talking about.
		/// @return Whether the loading was successful or not.
		bool LoadUnseenLayer(std::string bitmapPath, const int team);

		/// Tells whether a team has anything still unseen on the scene.
		/// @param team The team we're talking about.
		/// @return A bool indicating whether that team has anyhting yet unseen.
		bool AnythingUnseen(const int team);

		/// Shows what the resolution factor of the unseen map to the entire Scene
		/// is, in both axes.
		/// @param team The team we're talking about.
		/// @return A vector witht he factors in each element representing the factors.
		Vector GetUnseenResolution(const int team) const;

		/// Checks whether a pixel is in an unseen area on of a specific team.
		/// @param posX The X and Y coords of the scene pixel that is to be checked.
		/// @param posY The team we're talking about.
		/// @return A bool indicating whether that point is yet unseen.
		bool IsUnseen(const int posX, const int posY, const int team);

		/// Reveals a pixel on the unseen map for a specific team, if there is any.
		/// @param posX The X and Y coord of the scene pixel that is to be revealed.
		/// @param posY The team to reveal for.
		/// @return A bool indicating whether there was an unseen pixel revealed there.
		bool RevealUnseen(const int posX, const int posY, const int team);

		/// Hides a pixel on the unseen map for a specific team, if there is any.
		/// @param posX The X and Y coord of the scene pixel that is to be revealed.
		/// @param posY The team to hide for.
		/// @return A bool indicating whether there was a seen pixel hidden there.
		bool RestoreUnseen(const int posX, const int posY, const int team);

		/// Reveals a box on the unseen map for a specific team, if there is any.
		/// @param posX The X and Y coords of the upper left corner of the box to be revealed.
		/// @param posY The width and height of the box to be revealed, in scene units (pixels)
		/// @param width The team to reveal for.
		void RevealUnseenBox(const int posX, const int posY, const int width, const int height, const int team);

		/// Restores a box on the unseen map for a specific team, if there is any.
		/// @param posX The X and Y coords of the upper left corner of the box to be revealed.
		/// @param posY The width and height of the box to be restored, in scene units (pixels)
		/// @param width The team to restore for.
		void RestoreUnseenBox(const int posX, const int posY, const int width, const int height, const int team);

		/// Traces along a vector and stops when the accumulated material strengths of the
		/// traced-through terrain meets or exceeds a given value.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param endPos A Vector that will be set to the position of where the sight ray was
		/// terminated. If it reached the end, it will be set to the end of the ray.
		/// @param strengthLimit The accumulated material strength limit where the ray stops.
		/// @param skip For every pixel checked along the line, how many to skip between them
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return Whether the ray was stopped prematurely or not.
		bool CastTerrainPenetrationRay(const Vector& start, const Vector& ray, Vector& endPos, int strengthLimit, int skip);

		/// Traces along a vector and reveals or hides pixels on the unseen layer of a team
		/// as long as the accumulated material strengths traced through the terrain
		/// don't exceed a specific value.
		/// @param team The team to see for.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param endPos A Vector that will be set to the position of where the sight ray was
		/// terminated. If it reached the end, it will be set to the end of the ray.
		/// @param strengthLimit The material strength limit where
		/// @param skip For every pixel checked along the line, how many to skip between them
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param reveal Whether the ray should reveal or restore unseen layer
		/// @return Whether any unseen pixels were revealed as a result of this seeing.
		bool CastUnseenRay(int team, const Vector& start, const Vector& ray, Vector& endPos, int strengthLimit, int skip, bool reveal);

		/// Traces along a vector and reveals pixels on the unseen layer of a team
		/// as long as the accumulated material strengths traced through the terrain
		/// don't exceed a specific value.
		/// @param team The team to see for.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param endPos A Vector that will be set to the position of where the sight ray was
		/// terminated. If it reached the end, it will be set to the end of the ray.
		/// @param strengthLimit The material strength limit where
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return Whether any unseen pixels were revealed as a result of this seeing.
		bool CastSeeRay(int team, const Vector& start, const Vector& ray, Vector& endPos, int strengthLimit, int skip = 0);

		/// Traces along a vector and hides pixels on the unseen layer of a team
		/// as long as the accumulated material strengths traced through the terrain
		/// don't exceed a specific value.
		/// @param team The team to see for.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param endPos A Vector that will be set to the position of where the sight ray was
		/// terminated. If it reached the end, it will be set to the end of the ray.
		/// @param strengthLimit The material strength limit where
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return Whether any unseen pixels were revealed as a result of this seeing.
		bool CastUnseeRay(int team, const Vector& start, const Vector& ray, Vector& endPos, int strengthLimit, int skip = 0);

		/// Traces along a vector and gets the location of the first encountered
		/// pixel of a specific material in the terrain.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param material The material ID to look for.
		/// @param result A reference to the vector screen will be filled out with the absolute
		/// location of the found terrain pixel of the above material.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param wrap Whetehr the ray should wrap around the scene if it crosses a seam. (default: true)
		/// @return Whether the material was found along the ray. If not, the fourth
		/// parameter will not have been altered (and may still not be 0!)
		bool CastMaterialRay(const Vector& start, const Vector& ray, unsigned char material, Vector& result, int skip = 0, bool wrap = true);

		/// Traces along a vector and returns how far along that ray there is an
		/// encounter with a pixel of a specific material in the terrain.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param material The material ID to look for.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return How far along, in pixel units, the ray the material pixel was encountered.
		/// If no pixel of the right material was found, < 0 is returned.
		float CastMaterialRay(const Vector& start, const Vector& ray, unsigned char material, int skip = 0);

		/// Traces along a vector and gets the location of the first encountered
		/// pixel that is NOT of a specific material in the scene's terrain.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param material The material ID to find something OTHER than.
		/// @param result A reference to the vector screen will be filled out with the absolute
		/// location of the found terrain pixel of the above material.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param checkMOs Whether to check for MO layer collisions as well, not just terrain. (default: false)
		/// @return Whether the a pixel other than the material was found along the ray.
		/// If not, the fourth parameter will not have been altered (and may still not be 0!)
		bool CastNotMaterialRay(const Vector& start, const Vector& ray, unsigned char material, Vector& result, int skip = 0, bool checkMOs = false);

		/// Traces along a vector and returns how far along that ray there is an
		/// encounter with a pixel of OTHER than a specific material in the terrain.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param material The material ID to find something OTHER than.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param checkMOs Whether to check for MO layer collisions as well, not just terrain. (default: false)
		/// @return How far along, in pixel units, the ray the pixel of any other material
		/// was encountered. If no pixel of the right material was found, < 0 is returned.
		float CastNotMaterialRay(const Vector& start, const Vector& ray, unsigned char material, int skip = 0, bool checkMOs = false);

		/// Traces along a vector and returns how the sum of all encountered pixels'
		/// material strength values. This will take wrapping into account.
		/// @param start The starting position.
		/// @param end The ending position.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param ignoreMaterial A material ID to ignore, IN ADDITION to Air. (default: g_MaterialAir)
		/// @return The sum of all encountered pixels' material strength vales. So if it was
		/// all Air, then 0 is returned (Air's strength value is 0).
		float CastStrengthSumRay(const Vector& start, const Vector& end, int skip = 0, unsigned char ignoreMaterial = g_MaterialAir);

		/// Traces along a vector and returns the strongest of all encountered pixels'
		/// material strength values.
		/// This will take wrapping into account.
		/// @param start The starting position.
		/// @param end The ending position.
		/// @param skip For every pixel checked along the line, how many to skip between them
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param ignoreMaterial A material ID to ignore, IN ADDITION to Air. This defaults to doors, for legacy script purposes
		/// @return The max of all encountered pixels' material strength vales. So if it was
		/// all Air, then 0 is returned (Air's strength value is 0).
		// We use two accessors instead of default parameters, for lua compat
		float CastMaxStrengthRay(const Vector& start, const Vector& end, int skip, unsigned char ignoreMaterial);
		float CastMaxStrengthRay(const Vector& start, const Vector& end, int skip) { return CastMaxStrengthRay(start, end, skip, g_MaterialDoor); };

		/// Traces along a vector and returns the strongest of all encountered pixels' materials
		/// This will take wrapping into account.
		/// @param start The starting position.
		/// @param end The ending position.
		/// @param skip For every pixel checked along the line, how many to skip between them
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param ignoreMaterial A material ID to ignore, IN ADDITION to Air.
		/// @return The strongest material encountered
		const Material* CastMaxStrengthRayMaterial(const Vector& start, const Vector& end, int skip, unsigned char ignoreMaterial);

		/// Traces along a vector and shows where along that ray there is an
		/// encounter with a pixel of a material with strength more than or equal
		/// to a specific value.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param strength The strength value of screen any found to be equal or more than will
		/// terminate the ray.
		/// @param result A reference to the vector screen will be filled out with the absolute
		/// location of the found terrain pixel of less than or equal to above strength.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param ignoreMaterial A material ID to ignore, IN ADDITION to Air. (default: g_MaterialAir)
		/// @param wrap Whetehr the ray should wrap around the scene if it crosses a seam. (default: true)
		/// @return Whether a material of equal or more strength was found along the ray.
		/// If not, the fourth parameter have been set to last position of the ray.
		bool CastStrengthRay(const Vector& start, const Vector& ray, float strength, Vector& result, int skip = 0, unsigned char ignoreMaterial = g_MaterialAir, bool wrap = true);

		/// Traces along a vector and shows where along that ray there is an
		/// encounter with a pixel of a material with strength less than or equal
		/// to a specific value.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param strength The strength value of screen any found to be equal or less than will
		/// terminate the ray.
		/// @param result A reference to the vector screen will be filled out with the absolute
		/// location of the found terrain pixel of less than or equal to above strength.
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @param wrap Whetehr the ray should wrap around the scene if it crosses a seam. (default: true)
		/// @return Whether a material of equal or less strength was found along the ray.
		/// If not, the fourth parameter have been set to last position of the ray.
		bool CastWeaknessRay(const Vector& start, const Vector& ray, float strength, Vector& result, int skip = 0, bool wrap = true);

		/// Traces along a vector and returns MOID of the first non-ignored
		/// non-NoMOID MO encountered. If a non-air terrain pixel is encountered
		/// first, g_NoMOID will be returned.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param ignoreMOID An MOID to ignore. Any child MO's of this MOID will also be ignored. (default: g_NoMOID)
		/// @param ignoreTeam To enable ignoring of all MOIDs associated with an object of a specific (default: Activity::NoTeam)
		/// team which also has team ignoring enabled itself.
		/// @param ignoreMaterial A specific material ID to ignore hits with. (default: 0)
		/// @param ignoreAllTerrain Whether to ignore all terrain hits or not. (default: false)
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return The MOID of the hit non-ignored MO, or g_NoMOID if terrain or no MO was hit.
		MOID CastMORay(const Vector& start, const Vector& ray, MOID ignoreMOID = g_NoMOID, int ignoreTeam = Activity::NoTeam, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false, int skip = 0);

		/// Traces along a vector and shows where a specific MOID has been found.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param targetMOID An MOID to find. Any child MO's of this MOID will also be found. ------------ ???
		/// @param resultPos A reference to the vector screen will be filled out with the absolute
		/// location of the found MO pixel of the above MOID.
		/// @param ignoreMaterial A specific material ID to ignore hits with. (default: 0)
		/// @param ignoreAllTerrain Whether to ignore all terrain hits or not. (default: false)
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return Whether the target MOID was found along the ray or not.
		bool CastFindMORay(const Vector& start, const Vector& ray, MOID targetMOID, Vector& resultPos, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false, int skip = 0);

		/// Traces along a vector and returns the length of how far the trace went
		/// without hitting any non-ignored terrain material or MOID at all.
		/// @param start The starting position.
		/// @param ray The vector to trace along.
		/// @param obstaclePos A reference to the vector screen will be filled out with the absolute
		/// location of the first obstacle, or the end of the ray if none was hit.
		/// @param freePos A reference to the vector screen will be filled out with the absolute
		/// location of the last free position before hitting an obstacle, or the
		/// end of the ray if none was hit. This is only altered if thre are any
		/// free pixels encountered.
		/// @param ignoreMOID An MOID to ignore. Any child MO's of this MOID will also be ignored. (default: g_NoMOID)
		/// @param ignoreTeam To enable ignoring of all MOIDs associated with an object of a specific (default: Activity::NoTeam)
		/// team which also has team ignoring enabled itself.
		/// @param ignoreMaterial A specific material ID to ignore hits with. (default: 0)
		/// @param skip For every pixel checked along the line, how many to skip between them (default: 0)
		/// for optimization reasons. 0 = every pixel is checked.
		/// @return How far along, in pixel units, the ray the pixel of any obstacle was
		/// encountered. If no pixel of the right material was found, < 0 is returned.
		/// If an obstacle on the starting position was encountered, 0 is returned.
		float CastObstacleRay(const Vector& start, const Vector& ray, Vector& obstaclePos, Vector& freePos, MOID ignoreMOID = g_NoMOID, int ignoreTeam = Activity::NoTeam, unsigned char ignoreMaterial = 0, int skip = 0);

		/// Gets the abosulte pos of where the last cast ray hit somehting.
		/// @return A vector with the absolute pos of where the last ray cast hit somehting.
		const Vector& GetLastRayHitPos();

		/// Calculates the altitide of a certain point above the terrain, measured
		/// in pixels.
		/// @param from The max altitude you care to check for. 0 Means check the whole scene's height.
		/// @param max The accuracy within screen measurement is acceptable. Higher number
		/// here means less calculation.
		/// @return The altitude over the terrain, in pixels.
		float FindAltitude(const Vector& from, int max, int accuracy, bool fromSceneOrbitDirection);
		float FindAltitude(const Vector& from, int max, int accuracy) { return FindAltitude(from, max, accuracy, false); }

		/// Calculates the altitide of a certain point above the terrain, measured
		/// in pixels, and then tells if that point is over a certain value.
		/// @param point The altitude threshold you want to check for.
		/// @param threshold The accuracy within screen measurement is acceptable. Higher number
		/// here means less costly.
		/// @return Whether the point is over the threshold altitude or not.
		bool OverAltitude(const Vector& point, int threshold, int accuracy = 0);

		/// Takes an arbitrary point in the air and calculates it to be straight
		/// down at a certain maximum distance from the ground.
		/// @param from The point to start from. Should be in the air, or the same point will
		/// be returned (null operation)
		/// @param maxAltitude The max altitude in px you want the point to be above the ground. (default: 0)
		/// @param accuracy The accuracy within screen measurement is acceptable. Higher number (default: 0)
		/// here means less calculation.
		/// @return The new point screen is no higher than accuracy + max altitude over
		/// the terrain.
		Vector MovePointToGround(const Vector& from, int maxAltitude = 0, int accuracy = 0);

		/// Returns whether the integer coordinates passed in are within the
		/// bounds of the current Scene, considering its wrapping.
		/// @param pixelX Int coordinates.
		/// @param pixelY A margin
		/// @return Whether within bounds or not, considering wrapping.
		bool IsWithinBounds(const int pixelX, const int pixelY, const int margin = 0) const;

		/// Wraps or bounds a position coordinate if it is off bounds of the
		/// Scene, depending on the wrap settings of this Scene.
		/// @param posX The X and Y coordinates of the position to wrap, if needed.
		/// @return Whether wrapping was performed or not. (Does not report on bounding)
		bool ForceBounds(int& posX, int& posY) const;

		/// Wraps or bounds a position coordinate if it is off bounds of the
		/// Scene, depending on the wrap settings of this Scene.
		/// @param pos The vector coordinates of the position to wrap, if needed.
		/// @return Whether wrapping was performed or not. (Does not report on bounding)
		bool ForceBounds(Vector& pos) const;

		/// Only wraps a position coordinate if it is off bounds of the Scene
		/// and wrapping in the corresponding axes are turned on.
		/// @param posX The X and Y coordinates of the position to wrap, if needed.
		/// @return Whether wrapping was performed or not.
		bool WrapPosition(int& posX, int& posY) const;

		/// Only wraps a position coordinate if it is off bounds of the Scene
		/// and wrapping in the corresponding axes are turned on.
		/// @param pos The vector coordinates of the position to wrap, if needed.
		/// @return Whether wrapping was performed or not.
		bool WrapPosition(Vector& pos) const;

		/// Returns a position snapped to the current scene grid.
		/// @param pos The vector coordinates of the position to snap.
		/// @param snap Whether to actually snap or not. This is useful for cleaner toggle code. (default: true)
		/// @return The new snapped position.
		Vector SnapPosition(const Vector& pos, bool snap = true) const;

		/// Calculates the shortest distance between two points in scene
		/// coordinates, taking into account all wrapping and out of bounds of the
		/// two points.
		/// @param pos1 The two Vector coordinates of the two positions to find the shortest
		/// distance between.
		/// @param pos2 Whether to check if the passed in points are outside the scene, and to
		/// wrap them if they are.
		/// @return The resulting vector screen shows the shortest distance, spanning over
		/// wrapping borders etc. Basically the ideal pos2 - pos1.
		Vector ShortestDistance(Vector pos1, Vector pos2, bool checkBounds = false) const;

		/// Calculates the shortest distance between two x values in scene
		/// coordinates, taking into account all wrapping and out of bounds of the
		/// two values.
		/// @param val1 The X coordinates of the two values to find the shortest distance between.
		/// @param val2 Whether to check if the passed in points are outside the scene, and to
		/// wrap them if they are.
		/// @param checkBounds Whether to constrain the distance to only be in a certain direction: (default: false)
		/// 0 means no constraint, < 0 means only look in the negative dir, etc.
		/// @param direction If the scene doesn't wrap in the constraint's direction, the constraint (default: 0)
		/// will be ignored.
		/// @return The resulting X value screen shows the shortest distance, spanning over
		/// wrapping borders etc. Basically the ideal val2 - val1.
		float ShortestDistanceX(float val1, float val2, bool checkBounds = false, int direction = 0) const;

		/// Calculates the shortest distance between two Y values in scene
		/// coordinates, taking into account all wrapping and out of bounds of the
		/// two values.
		/// @param val1 The Y coordinates of the two values to find the shortest distance between.
		/// @param val2 Whether to check if the passed in points are outside the scene, and to
		/// wrap them if they are.
		/// @param checkBounds Whether to constrain the distance to only be in a certain direction: (default: false)
		/// 0 means no constraint, < 0 means only look in the negative dir, etc.
		/// @param direction If the scene doesn't wrap in the constraint's direction, the constraint (default: 0)
		/// will be ignored.
		/// @return The resulting Y value screen shows the shortest distance, spanning over
		/// wrapping borders etc. Basically the ideal val2 - val1.
		float ShortestDistanceY(float val1, float val2, bool checkBounds = false, int direction = 0) const;

		/// Tells whether a point on the scene is obscured by MOID or Terrain
		/// non-air material.
		/// @param point The point on the scene to check.
		/// @param team Wheter to also check for unseen areas of a specific team. -1 means (default: -1) { return ObscuredPoint(point.GetFloorIntX())
		/// don't check this.
		/// @return Whether that point is obscured/obstructed or not.
		bool ObscuredPoint(Vector& point, int team = -1) { return ObscuredPoint(point.GetFloorIntX(), point.GetFloorIntY()); }

		/// Tells whether a point on the scene is obscured by MOID or Terrain
		/// non-air material.
		/// @param x The point on the scene to check.
		/// @param y Wheter to also check for unseen areas of a specific team. -1 means
		/// don't check this.
		/// @return Whether that point is obscured/obstructed or not.
		bool ObscuredPoint(int x, int y, int team = -1);

		/*
		/// Tells whether two IntRect:s in the scene intersect, while taking
		/// wrapping into account.
		/// @param x The point on the scene to check.
		/// @return Whether that point is obscured/obstructed or not.
		    bool SceneRectsIntersect(int x, int y);
		*/

		/// Takes a rect and adds all possible wrapped appearances of that rect
		/// to a passed-in list. IF if a passed in rect straddles the seam of a
		/// wrapped scene axis, it will be added twice to the output list. If it
		/// doesn't straddle any seam, it will be only added once.
		/// @param wrapRect The IntRect to check for wrapping of and add to the output list below.
		/// @param outputList A reference to a list of IntRect:s that will only be added to, never
		/// cleared.
		/// @return How many additions of the passed in rect was added to the list. 1 if
		/// no wrapping need was detected, up to 4 possible (if straddling both seams)
		int WrapRect(const IntRect& wrapRect, std::list<IntRect>& outputList);

		/// Takes a Box and adds all possible scenewrapped appearances of that Box
		/// to a passed-in list. IF if a passed in rect straddles the seam of a
		/// wrapped scene axis, it will be added twice to the output list. If it
		/// doesn't straddle any seam, it will be only added once.
		/// @param wrapBox The IntRect to check for wrapping of and add to the output list below.
		/// @param outputList A reference to a list of IntRect:s that will only be added to, never
		/// cleared.
		/// @return How many additions of the passed in Box was added to the list. 1 if
		/// no wrapping need was detected, up to 4 possible (if straddling both seams)
		int WrapBox(const Box& wrapBox, std::list<Box>& outputList);

		/// Takes any scene object and adds it to the scene in the appropriate way.
		/// If it's a TerrainObject, then it gets applied to the terrain, if it's
		/// an MO, it gets added to the correct type group in MovableMan.
		/// @param pObject The SceneObject to add. Ownership IS transferred!
		/// @return Whether the SceneObject was successfully added or not. Either way,
		/// ownership was transferred. If no success, the object was deleted.
		bool AddSceneObject(SceneObject* pObject);

		/// Updates the state of this SceneMan. Supposed to be done every frame
		/// before drawing.
		/// @param screenId Which screen to update for. (default: 0)
		void Update(int screenId = 0);

		/// Draws this SceneMan's current graphical representation to a
		/// BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on, appropriately sized for the split
		/// screen segment.
		/// @param targetGUIBitmap The offset into the scene where the target bitmap's upper left corner
		/// is located.
		void Draw(BITMAP* targetBitmap, BITMAP* targetGUIBitmap, const Vector& targetPos = Vector(), bool skipBackgroundLayers = false, bool skipTerrain = false);

		/// Clears the color MO layer. Should be done every frame.
		void ClearMOColorLayer();

		/// Clears the list of pixels on the unseen map that have been revealed.
		void ClearSeenPixels();

		/// Creates a copy of passed material and stores it into internal vector
		/// to make sure there's only one material owner
		/// @param mat Material to add.
		/// @return Pointer to stored material.
		Material* AddMaterialCopy(Material* mat);

		/// Registers terrain change event for the network server to be then sent to clients.
		/// @param x,y - scene coordinates of change, w,h - size of the changed region,
		/// color - changed color for one-pixel events,
		/// back - if true, then background bitmap was changed if false then foreground.
		void RegisterTerrainChange(int x, int y, int w, int h, unsigned char color, bool back);

		/// Gets an intermediate bitmap that is used for drawing a settled MovableObject into the terrain.
		/// @param moDiameter The diameter of the MovableObject to calculate the required bitmap size.
		/// @return Pointer to the temp BITMAP of the appropriate size. Ownership is NOT transferred!
		BITMAP* GetIntermediateBitmapForSettlingIntoTerrain(int moDiameter) const;

		/// Sets the current scene pointer to null
		void ClearCurrentScene();

		/// Gets the maximum height of a column of scrap terrain to collapse, when the bottom pixel is knocked loose.
		/// @return The compacting height of scrap terrain.
		int GetScrapCompactingHeight() const { return m_ScrapCompactingHeight; }

		/// Sets the maximum height of a column of scrap terrain to collapse, when the bottom pixel is knocked loose.
		/// @param newHeight The new compacting height, in pixels.
		void SetScrapCompactingHeight(int newHeight) { m_ScrapCompactingHeight = newHeight; }

		/// Protected member variable and method declarations
	protected:
		static std::vector<std::pair<int, BITMAP*>> m_IntermediateSettlingBitmaps; //!< Intermediate bitmaps of different sizes that are used to draw settled MovableObjects into the terrain.

		// Default Scene name to load if nothing else is specified
		std::string m_DefaultSceneName;
		// Scene reference to load from, if any. NOT OWNED - a clone of this actually gets loaded
		const Scene* m_pSceneToLoad;
		// Whether to place objects later when loading a clone of the above scene
		bool m_PlaceObjects;
		// Whether to place units and deployments when loading scence
		bool m_PlaceUnits;

		// Current scene being used
		Scene* m_pCurrentScene;
		// Color MO layer
		SceneLayerTracked* m_pMOColorLayer;
		// A spatial partitioning grid of MOIDs, used to optimize collision and distance queries
		SpatialPartitionGrid m_MOIDsGrid;

		// Debug layer for seeing cast rays etc
		SceneLayer* m_pDebugLayer;

		// The mode we're drawing layers in to the screen
		int m_LayerDrawMode;

		// Material palette stuff
		std::map<std::string, unsigned char> m_MatNameMap;
		// This gets filled with holes, not contigous from 0 onward, but whatever the ini specifies. The Material objects are owned here
		std::array<Material*, c_PaletteEntriesNumber> m_apMatPalette;
		// The total number of added materials so far
		int m_MaterialCount;

		// Non original materials added by inheritance
		std::vector<Material*> m_MaterialCopiesVector;

		// Sound of an unseen pixel on an unseen layer being revealed.
		SoundContainer* m_pUnseenRevealSound;

		bool m_DrawRayCastVisualizations; //!< Whether to visibly draw RayCasts to the Scene debug Bitmap.
		bool m_DrawPixelCheckVisualizations; //!< Whether to visibly draw pixel checks (GetTerrMatter and GetMOIDPixel) to the Scene debug Bitmap.

		// The last screen everything has been updated to
		int m_LastUpdatedScreen;
		// Whether we're in second pass of the structural computations.
		// Second pass is where structurally unsound areas of the Terrain are turned into
		// MovableObject:s.
		bool m_SecondStructPass;

		// The Timer that keeps track of how much time there is left for
		// structural calculations each frame.
		Timer m_CalcTimer;

		// The Timer to measure time between cleanings of the color layer of the Terrain.
		Timer m_CleanTimer;
		// Bitmap to look for orphaned regions
		BITMAP* m_pOrphanSearchBitmap;

		int m_ScrapCompactingHeight; //!< The maximum height of a column of scrap terrain to collapse, when the bottom pixel is knocked loose.

		/// Private member variable and method declarations
	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this SceneMan, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		SceneMan(const SceneMan& reference) = delete;
		SceneMan& operator=(const SceneMan& rhs) = delete;
	};

} // namespace RTE
