#pragma once

#include "Singleton.h"
#include "Box.h"
#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "SceneMan.h"
#include "Shader.h"

#include <array>
#include <atomic>
#include <memory>
#include <list>
#include <vector>

#define g_PostProcessMan PostProcessMan::Instance()

namespace RTE {
	/// Struct for storing GL information in the BITMAP->extra field.
	struct GLBitmapInfo {
		GLuint m_Texture;
	};

	/// Structure for storing a post-process screen effect to be applied at the last stage of 32bpp rendering.
	struct PostEffect {
		BITMAP* m_Bitmap = nullptr; //!< The bitmap to blend, not owned.
		size_t m_BitmapHash = 0; //!< Hash used to transmit glow events over the network.
		float m_Angle = 0.0F; // Post effect angle in radians.
		int m_Strength = 128; //!< Scalar float for how hard to blend it in, 0 - 255.
		Vector m_Pos; //!< Post effect position. Can be relative to the scene, or to the screen, depending on context.

		/// Constructor method used to instantiate a PostEffect object in system memory.
		PostEffect(const Vector& pos, BITMAP* bitmap, size_t bitmapHash, int strength, float angle) :
		    m_Bitmap(bitmap), m_BitmapHash(bitmapHash), m_Angle(angle), m_Strength(strength), m_Pos(pos) {}
	};

	/// Singleton manager responsible for all 32bpp post-process effect drawing.
	class PostProcessMan : public Singleton<PostProcessMan> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a PostProcessMan object in system memory. Create() should be called before using the object.
		PostProcessMan();

		/// Makes the PostProcessMan object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize();

		/// (Re-)Initializes the GL backbuffers to the current render resolution for post-processing.
		void CreateGLBackBuffers();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a PostProcessMan object before deletion from system memory.
		~PostProcessMan();

		/// Destroys and resets (through Clear()) the PostProcessMan object.
		void Destroy();

		/// Clears the list of registered post-processing screen effects and glow boxes.
		void ClearScreenPostEffects() {
			m_PostScreenEffects.clear();
			m_PostScreenGlowBoxes.clear();
		}

		/// Clears the list of registered post-processing scene effects and glow areas.
		void ClearScenePostEffects() {
			m_PostSceneEffects.clear();
			m_GlowAreas.clear();
		}
#pragma endregion

#pragma region Concrete Methods
		/// Takes the current state of the 8bpp back-buffer, copies it, and adds post-processing effects on top like glows etc.
		void PostProcess();

		/// Adjusts the offsets of all effects relative to the specified player screen and adds them to the total screen effects list so they can be drawn in PostProcess().
		/// @param playerScreen Player screen to adjust effect offsets for.
		/// @param targetBitmap Bitmap representing the player screen.
		/// @param targetBitmapOffset The position of the specified player's draw screen on the backbuffer.
		/// @param screenRelativeEffectsList List of the specified player's accumulated post effects for this frame.
		/// @param screenRelativeGlowBoxesList List of the specified player's accumulated glow boxes for this frame.
		void AdjustEffectsPosToPlayerScreen(int playerScreen, BITMAP* targetBitmap, const Vector& targetBitmapOffset, std::list<PostEffect>& screenRelativeEffectsList, std::list<Box>& screenRelativeGlowBoxesList);
#pragma endregion

#pragma region Post Effect Handling
		/// Registers a post effect to be added at the very last stage of 32bpp rendering by the FrameMan.
		/// @param effectPos The absolute scene coordinates of the center of the effect.
		/// @param effect A 32bpp BITMAP screen should be drawn centered on the above scene location in the final frame buffer. Ownership is NOT transferred!
		/// @param hash Hash value of the effect for transmitting over the network.
		/// @param strength The intensity level this effect should have when blended in post. 0 - 255.
		/// @param angle The angle this effect should be rotated at in radians.
		void RegisterPostEffect(const Vector& effectPos, BITMAP* effect, size_t hash, int strength = 255, float angle = 0);

		/// Gets all screen effects that are located within a box in the scene.
		/// Their coordinates will be returned relative to the upper left corner of the box passed in here. Wrapping of the box will be taken care of.
		/// @param boxPos The top left coordinates of the box to get post effects for.
		/// @param boxWidth The width of the box.
		/// @param boxHeight The height of the box.
		/// @param effectsList The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.
		/// @param team The team whose unseen layer should obscure the screen effects here.
		/// @return Whether any active post effects were found in that box.
		bool GetPostScreenEffectsWrapped(const Vector& boxPos, int boxWidth, int boxHeight, std::list<PostEffect>& effectsList, int team = -1);

		/// Gets a temporary bitmap of specified size to rotate post effects in.
		/// @param bitmapSize Size of bitmap to get.
		/// @return Pointer to the temporary bitmap.
		BITMAP* GetTempEffectBitmap(BITMAP* bitmap) const;
#pragma endregion

#pragma region Post Pixel Glow Handling

		/// Registers a specific IntRect to be post-processed and have special pixel colors lit up by glow effects in it.
		/// @param glowArea The IntRect to have special color pixels glow in, in scene coordinates.
		void RegisterGlowArea(const IntRect& glowArea) {
			if (g_TimerMan.DrawnSimUpdate() && g_TimerMan.SimUpdatesSinceDrawn() >= 0) {
				m_GlowAreas.push_back(glowArea);
			}
		}

		/// Creates an IntRect and registers it to be post-processed and have special pixel colors lit up by glow effects in it.
		/// @param center The center of the IntRect.
		/// @param radius The radius around it to add as an area.
		void RegisterGlowArea(const Vector& center, float radius) {
			RegisterGlowArea(IntRect(static_cast<int>(center.m_X - radius), static_cast<int>(center.m_Y - radius), static_cast<int>(center.m_X + radius), static_cast<int>(center.m_Y + radius)));
		}

		/// Registers a specific glow dot effect to be added at the very last stage of 32bpp rendering by the FrameMan.
		/// @param effectPos The absolute scene coordinates of the center of the effect.
		/// @param color Which glow dot color to register, see the DotGlowColor enumerator.
		/// @param strength The intensity level this effect should have when blended in post. 0 - 255.
		void RegisterGlowDotEffect(const Vector& effectPos, DotGlowColor color, int strength = 255);

		/// Gets all glow areas that affect anything within a box in the scene.
		/// Their coordinates will be returned relative to the upper left corner of the box passed in here. Wrapping of the box will be taken care of.
		/// @param boxPos The top left coordinates of the box to get post effects for.
		/// @param boxWidth The width of the box.
		/// @param boxHeight The height of the box.
		/// @param areaList The list to add the glow Boxes that intersect to. The coordinates of the Boxes returned here will be relative to the boxPos passed in above.
		/// @return Whether any active post effects were found in that box.
		bool GetGlowAreasWrapped(const Vector& boxPos, int boxWidth, int boxHeight, std::list<Box>& areaList) const;
#pragma endregion

#pragma region Network Post Effect Handling
		/// Copies the specified player's screen relative post effects list of this PostProcessMan to the referenced list. Used for sending post effect data over the network.
		/// @param whichScreen Which player screen to get list for.
		/// @param outputList Reference to the list of post effects to copy into.
		void GetNetworkPostEffectsList(int whichScreen, std::list<PostEffect>& outputList);

		/// Copies the player's screen relative post effects from the referenced list to the list of this PostProcessMan. Used for receiving post effect data over the network.
		/// @param whichScreen Which player screen to set list for.
		/// @param inputList Reference to the list of post effects to copy from.
		void SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect>& inputList);
#pragma endregion

		/// Gets the backbuffer texture for indexed drawings.
		/// @return The opengl backbuffer texture for indexed drawings.
		GLuint GetPostProcessColorBuffer() { return m_BackBuffer32; }

	protected:
		std::list<PostEffect> m_PostScreenEffects; //!< List of effects to apply at the end of each frame. This list gets cleared out and re-filled each frame.
		std::list<PostEffect> m_PostSceneEffects; //!< All post-processing effects registered for this draw frame in the scene.

		std::list<Box> m_PostScreenGlowBoxes; //!< List of areas that will be processed with glow.
		std::list<IntRect> m_GlowAreas; //!< All the areas to do post glow pixel effects on, in scene coordinates.

		std::array<std::list<PostEffect>, c_MaxScreenCount> m_ScreenRelativeEffects; //!< List of screen relative effects for each player in online multiplayer.
		std::array<std::mutex, c_MaxScreenCount> ScreenRelativeEffectsMutex; //!< Mutex for the ScreenRelativeEffects list when accessed by multiple threads in online multiplayer.

		BITMAP* m_YellowGlow; //!< Bitmap for the yellow dot glow effect.
		BITMAP* m_RedGlow; //!< Bitmap for the red dot glow effect.
		BITMAP* m_BlueGlow; //!< Bitmap for the blue dot glow effect.

		size_t m_YellowGlowHash; //!< Hash value for the yellow dot glow effect bitmap.
		size_t m_RedGlowHash; //!< Hash value for the red dot glow effect bitmap.
		size_t m_BlueGlowHash; //!< Hash value for the blue dot glow effect bitmap.

		std::unordered_map<int, BITMAP*> m_TempEffectBitmaps; //!< Stores temporary bitmaps to rotate post effects in for quick access.

	private:
		GLuint m_BackBuffer8; //!< Backbuffer texture for incoming indexed drawings.
		GLuint m_BackBuffer32; //!< Backbuffer texture for the final 32bpp frame.
		GLuint m_Palette8Texture; //!< Palette texture for incoming indexed drawings.
		std::vector<std::unique_ptr<GLBitmapInfo>> m_BitmapTextures; //!< Vector of all the GL textures for the bitmaps that have been uploaded so far.
		GLuint m_BlitFramebuffer; //!< Framebuffer for blitting the 8bpp backbuffer to the 32bpp backbuffer.
		GLuint m_PostProcessFramebuffer; //!< Framebuffer for post-processing effects.
		GLuint m_PostProcessDepthBuffer; //!< Depth buffer for post-processing effects.
		std::unique_ptr<glm::mat4> m_ProjectionMatrix; //!< Projection matrix for post-processing effects.
		GLuint m_VertexBuffer; //!< Vertex buffer for post-processing effects.
		GLuint m_VertexArray; //!< Vertex array for post-processing effects.
		std::unique_ptr<Shader> m_Blit8; //!< Shader for blitting the 8bpp backbuffer to the 32bpp backbuffer.
		std::unique_ptr<Shader> m_PostProcessShader; //!< Shader for drawing bitmap post effects.

#pragma region Post Effect Handling
		/// Gets all screen effects that are located within a box in the scene. Their coordinates will be returned relative to the upper left corner of the box passed in here.
		/// @param boxPos The top left coordinates of the box to get post effects for.
		/// @param boxWidth The width of the box.
		/// @param boxHeight The height of the box.
		/// @param effectsList The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.
		/// @param team The team whose unseen area should block the glows.
		/// @return Whether any active post effects were found in that box.
		bool GetPostScreenEffects(Vector boxPos, int boxWidth, int boxHeight, std::list<PostEffect>& effectsList, int team = -1);

		/// Gets all screen effects that are located within a box in the scene. Their coordinates will be returned relative to the upper left corner of the box passed in here.
		/// @param left Position of box left plane (X start).
		/// @param top Position of box top plane (Y start).
		/// @param right Position of box right plane (X end).
		/// @param bottom Position of box bottom plane (Y end).
		/// @param effectsList The list to add the screen effects that fall within the box to. The coordinates of the effects returned here will be relative to the boxPos passed in above.
		/// @param team The team whose unseen area should block the glows.
		/// @return Whether any active post effects were found in that box.
		bool GetPostScreenEffects(int left, int top, int right, int bottom, std::list<PostEffect>& effectsList, int team = -1);
#pragma endregion

#pragma region Post Pixel Glow Handling
		/// Gets a specific standard dot glow effect for making pixels glow.
		/// @param which Which of the dot glow colors to get, see the DotGlowColor enumerator.
		/// @return The requested glow dot BITMAP.
		BITMAP* GetDotGlowEffect(DotGlowColor whichColor) const;

		/// Gets the hash value of a specific standard dot glow effect for making pixels glow.
		/// @param which Which of the dot glow colors to get, see the DotGlowColor enumerator.
		/// @return The hash value of the requested glow dot BITMAP.
		size_t GetDotGlowEffectHash(DotGlowColor whichColor) const;
#pragma endregion

#pragma region PostProcess Breakdown
		/// Draws all the glow dot effects on pixels registered inside glow boxes for this frame. This is called from PostProcess().
		void DrawDotGlowEffects();

		/// Draws all the glow effects registered for this frame. This is called from PostProcess().
		void DrawPostScreenEffects();
#pragma endregion

		/// Clears all the member variables of this PostProcessMan, effectively resetting the members of this abstraction level only.
		void Clear();

		/// Initializes all the GL pointers used by this PostProcessMan.
		void InitializeGLPointers();

		/// Destroys all the GL pointers used by this PostProcessMan.
		void DestroyGLPointers();

		/// Updates the palette texture with the current palette.
		void UpdatePalette();

		/// Creates and upload a new GL texture. The texture pointer is stored in the BITMAP->extra field.
		/// @param bitmap The bitmap to create a texture for.
		void LazyInitBitmap(BITMAP* bitmap);

		// Disallow the use of some implicit methods.
		PostProcessMan(const PostProcessMan& reference) = delete;
		PostProcessMan& operator=(const PostProcessMan& rhs) = delete;
	};
} // namespace RTE
