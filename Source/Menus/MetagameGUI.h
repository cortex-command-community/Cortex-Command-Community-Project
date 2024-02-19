#pragma once

/// MetagameGUI class
/// @author Daniel Tabar
/// dtabar@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "ActivityMan.h"
#include "Timer.h"
#include "GUIBanner.h"

struct BITMAP;

namespace RTE {

	class GUIControl;
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class Entity;
	class Scene;
	class Activity;
	class GAScripted;

	/// A full menu system that represents the metagame GUI for Cortex Command
	class MetagameGUI : public Serializable {

		/// Public member variable, method and friend function declarations
		SerializableClassNameGetter
		SerializableOverrideMethods

	public:
		enum MenuScreens {
			ROOTBOX = 0,
			NEWDIALOG,
			LOADDIALOG,
			SAVEDIALOG,
			MENUDIALOG,
			STATSDIALOG,
			SCENEINFOBOX,
			SCREENCOUNT
		};

		// For storing lines to be drawn upon draw time
		struct SiteLine {
			int m_Player;
			float m_StartMeterAt;
			float m_MeterAmount;
			float m_FundsAmount;
			float m_FundsTarget;
			Vector m_PlanetPoint;
			std::string m_SiteName;
			// NOT owned here
			const Scene* m_pScene;
			int m_Color;
			int m_OnlyFirstSegments;
			int m_OnlyLastSegments;
			int m_ChannelHeight;
			float m_CircleSize;
			bool m_Square;

			SiteLine(int player,
			         float startMeterAt,
			         float meterAmount,
			         const Vector& planetPoint,
			         std::string siteName,
			         // Ownership NOT passed in
			         const Scene* pScene,
			         int color,
			         int onlyFirstSegments = -1,
			         int onlyLastSegments = -1,
			         int channelHeight = 60,
			         float circleSize = 1.0f,
			         bool squareSite = false) {
				m_Player = player;
				m_StartMeterAt = startMeterAt;
				m_MeterAmount = meterAmount;
				m_PlanetPoint = planetPoint;
				m_SiteName = siteName;
				m_pScene = pScene;
				m_Color = color;
				m_OnlyFirstSegments = onlyFirstSegments;
				m_OnlyLastSegments = onlyLastSegments;
				m_ChannelHeight = channelHeight;
				m_CircleSize = circleSize;
				m_Square = squareSite;
			}
		};

		// For storing info about target crosshairs over sites
		struct SiteTarget {
			enum SiteTargetStyle {
				CROSSHAIRSSHRINK = 0,
				CROSSHAIRSGROW,
				CIRCLESHRINK,
				CIRCLEGROW,
				SQUARESHRINK,
				SQUAREGROW,
				STYLECOUNT
			};

			Vector m_CenterPos;
			float m_AnimProgress;
			int m_Style;
			int m_Color;
			double m_StartTime;
			Timer m_AnimTimer;

			SiteTarget() {
				m_CenterPos.Reset();
				m_AnimProgress = 0;
				m_Style = 0;
				m_Color = 0;
				m_StartTime = 0;
				m_AnimTimer.Reset();
			}

			SiteTarget(const Vector& centerPos,
			           float animProgress,
			           int style,
			           int color,
			           double startTime = 0) {
				m_CenterPos = centerPos;
				m_AnimProgress = animProgress;
				m_Style = style;
				m_Color = color;
				m_StartTime = startTime;
				m_AnimTimer.Reset();
			}

			/// Draws this SiteTarget onto a bitmap of choice.
			/// @param drawBitmap The bitmap to draw to.
			void Draw(BITMAP* drawBitmap) const;
		};

		/// Constructor method used to instantiate a MetagameGUI object in system
		/// memory. Create() should be called before using the object.
		MetagameGUI();

		/// Destructor method used to clean up a MetagameGUI object before deletion
		/// from system memory.
		~MetagameGUI();

		/// Makes the MetagameGUI object ready for use.
		/// @param pController A poitner to a Controller which will control this Menu. Ownership is
		/// NOT TRANSFERRED!
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(Controller* pController);

		/// Resets the entire MetagameGUI, including its inherited members, to
		/// their default settings or values.
		void Reset() override { Clear(); }

		/// Destroys and resets (through Clear()) the MetagameGUI object.
		void Destroy();

		/// Gets the GUIControlManager owned and used by this.
		/// @return The GUIControlManager. Ownership is not transferred!
		GUIControlManager* GetGUIControlManager();

		/// Enables or disables the menu. This will animate it in and out of view.
		/// @param enable Whether to enable or disable the menu. (default: true)
		void SetEnabled(bool enable = true);

		/// Reports whether the menu is enabled or not.
		bool IsEnabled() { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }

		/// Switches to showing a specific menu screen/mode.
		/// @param newScreen The MenuScreen to switch to.
		void SwitchToScreen(int newScreen);

		/// Makes a round number into a nice friendly text string. "ONE" for 1 etc
		/// @param roundNumber The number of the round to convert to a string.
		/// @return The friendly text string for that round.
		std::string GetRoundName(int roundNumber);

		/// Sets where the planet is on the scren and its other data so the menu
		/// can overlay properly on it.
		/// @param center The absolute screen coordinates of the planet's center.
		/// @param radius The radius, in screen pixel units, of the planet.
		void SetPlanetInfo(const Vector& center, float radius) {
			m_PlanetCenter = center;
			m_PlanetRadius = radius;
		}

		/// Sets a specific scene as the currently selected one. OWNERSHIP IS NOT TRANSFERRED!
		/// @param pScene The Scene to set as selected. Ownership is NOT transferred.
		void SelectScene(Scene* pScene);

		/// Tries to select a specifically named scene on the metagame field.
		/// @param sceneName The name of the Scene to try to find and select.
		/// @return Whether mission was found and selected.
		bool SelectScene(std::string sceneName);

		/// Reports whether the player has decided to continue to next phase of the
		/// round of the current game.
		/// @return Whether the player just decided to continue this frame
		bool ContinuePhase() { return m_ContinuePhase; }

		/// Reports whether the player has decided to restart an activity this frame.
		/// All parameters for the new game has been fed into ActivityMan already.
		/// @return Whether the activity should be restarted.
		bool ActivityRestarted() { return m_ActivityRestarted; }

		/// Reports whether the player has decided to resume the current activity.
		/// @return Whether the activity should be resumed.
		bool ActivityResumed() { return m_ActivityResumed; }

		/// Reports whether the player has decided to go back to the main menu.
		/// @return Whether we should go back to main menu.
		bool BackToMain() { return m_BackToMain; }

		/// Reports whether the player has decided to quit the program.
		/// @return Whether the program has been commanded to shit down by the user.
		bool QuitProgram() { return m_Quit; }

		/// Attempts to start a new Metagame using the settings set in the
		/// New Game dialog box.
		/// @return Whether the game was able to be set up with the current settings.
		bool StartNewGame();

		/// Attempts to load a Metagame from disk using the settings set in the
		/// Load Game dialog box.
		/// @return Whether the game was able to be loaded with the current settings.
		bool LoadGame();

		/// Just saves out the MetaGame and all its Scene data as-is to a specific
		/// location.
		/// @param saveName The name of the save game to create or overwrite here.
		/// @param savePath The full path of the ini that we want to save the Metagame state to.
		/// @param resaveSceneData Whether to load all the scene data that is on disk first so it will (default: false)
		/// be re-saved to the new location here.
		/// @return Whether the game was able to be saved there.
		bool SaveGame(std::string saveName, std::string savePath, bool resaveSceneData = false);

		/// Attempts to save a Metagame to disk using the settings set in the
		/// Save Game dialog box.
		/// @return Whether the game was able to be saved with the current settings.
		bool SaveGameFromDialog();

		/// Updates the state of this Menu each frame
		void Update();

		/// Draws the menu
		/// @param drawBitmap The bitmap to draw on.
		void Draw(BITMAP* drawBitmap);

		/// Resets internal state of GUI to show 'Start new campaign' screen
		void SetToStartNewGame();

		/// Sets where the station is located on the planet orbit.
		/// @param newStationPos The position of the station on the planet orbit.
		void SetStationOrbitPos(const Vector& newStationPos) { m_StationPosOnOrbit = newStationPos; }

		/// Protected member variable and method declarations
	protected:
		/// Makes the MetaMan object ready for use -> this is acutally a light
		/// and not complete version of the one that takes a controller.
		/// It is only for init after reading stuff from file as a Serializable.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Updates the user input processing.
		void UpdateInput();

		/// Hides all menu screens, so one can easily be unhidden and shown only.
		void HideAllScreens();

		/// Makes sure a specific box doesn't end up moved completely off-screen.
		/// @param pBox The GUICollectionBox to adjust, if necessary.
		/// @param margin The amount of margin to allow the box to stay within. If negative, (default: 10)
		/// the width/height of the box itself are used.
		void KeepBoxOnScreen(GUICollectionBox* pBox, int margin = 10);

		/// Changes the animation mode
		void ChangeAnimMode(int newMode) {
			m_AnimMode = newMode;
			m_AnimModeChange = true;
		}

		/// Checks for and switches off the new animation mode flag
		bool NewAnimMode() {
			bool changed = m_AnimModeChange;
			m_AnimModeChange = false;
			return changed;
		}

		/// Handles what happens after an Activity within the Metagame was
		/// run and completed fully.
		void CompletedActivity();

		/// Automatically resolves an offensive fight without actually launching
		/// and going through an Activity. Will randomly determine who won and
		/// what the consequences are.
		/// @param pOffensive The Offsenive Activity to resolve and manipulate accordingly. OWNERSHIP IS NOT TRANSFERRED!
		/// @param pScene The Scene this Offensive is supposed to take place on. OWNERSHIP IS NOT TRANSFERRED!
		/// @param brainCheck Whether to check the validity of all players based on whether they (default: false)
		/// have brains remaining alive. If false, all active players will be
		/// instead be flagged as having had brains at some point.
		/// @return Whether the ownership of the relevant Scene changed due to this.
		bool AutoResolveOffensive(GAScripted* pOffensive, Scene* pScene, bool brainCheck = false);

		/// Updates the New Site Revealing animation
		void UpdateSiteRevealing();

		/// Updates team ownership change animations, if any.
		void UpdateSiteChangeAnim();

		/// Updates the Count Income animation
		/// @param initOverride Whether to just set up the lines and funds as if we had a new round. Also skips changing funds to avoid an income/cost duplication glitch when saving a game at the start of a round. (default: false)
		void UpdateIncomeCounting(bool initOverride = false);

		/// Updates a human player's turn.
		/// @param metaPlayer Which metaplayer' turn it is
		void UpdateHumanPlayerTurn(int metaPlayer);

		/// Updates the Base Building animation
		void UpdateBaseBuilding();

		/// Sets up the Activities that represent all the offensive actions of
		/// the teams this round.
		void SetupOffensives();

		/// Updates the offensive actions animation
		void UpdateOffensives();

		/// Finishes one battle in the UpdateOffensives and moves onto the next.
		/// @return If there are any more battles after the one that was just finalized.
		bool FinalizeOffensive();

		/// Hides and resets all battle info labels and panels
		void ResetBattleInfo();

		/// Updates which player get placed in which quad around a fought-over
		/// site.
		/// @param targetPos The absolutel screen position of the target site.
		void UpdateBattleQuads(Vector targetPos);

		/// Updates the animation and display of the info for the current
		/// offensive battle's attackers being next in line for this round.
		/// @param progress The normalized scalar which will set the desired progress of the
		/// total animation. 0 means nothing is shown, because it is at the start
		/// of the animation where brain icons start moving around.
		void UpdatePreBattleAttackers(float progress);

		/// Updates the animation and display of the info for the current
		/// offensive battle's defenders being next in line for this round.
		/// @param progress The normalized scalar which will set the desired progress of the
		/// total animation. 0 means nothing is shown, because it is at the start
		/// of the animation where brain icons start moving around.
		void UpdatePreBattleDefenders(float progress);

		/// Updates the animation and display of the info for the current
		/// offensive battle's retreating brains going back to their pools
		/// @param progress The normalized scalar which will set the desired progress of the
		/// total animation. 0 means nothing has happened, because it is at the
		/// start of the animation where brain icons start moving around.
		void UpdatePostBattleRetreaters(float progress);

		/// Updates the animation and display of the info for the current done
		/// offensive battle's winning brains going back into the site.
		/// @param progress The normalized scalar which will set the desired progress of the
		/// total animation. 0 means nothing has happened, because it is at the
		/// start of the animation where brain icons start moving around.
		void UpdatePostBattleResidents(float progress);

		/// Updates the action lines as per what the player has chosen to do
		/// during the current turn so far.
		/// @param player The metaplayer we want to update the lines for.
		/// Also add a line for the unallocated funds the player hasn't used for
		/// anyhting else yet. - NOPE, NOT IMPL YET
		/// @return The meter start that remains after all the lines are added.
		float UpdatePlayerActionLines(int player); //, bool addUnallocated = false);

		/// Updates the contents of the scene selection box.
		/// @param sceneChanged Whether the selected has changed and should refresh the box completely. (default: false)
		void UpdateScenesBox(bool sceneChanged = false);

		/// Updates the game size labels of the new game dialog
		void UpdateGameSizeLabels();

		/// Updates AI skill sliders and labels for all players.
		/// @param player Which player's slider was changed.
		void UpdateAISkillSliders(int player);

		/// Updates the player setup controls of the new game dialog
		void UpdatePlayerSetup();

		/// Updates the floating player bars with current funds, flag, etc.
		void UpdatePlayerBars();

		/// Updates the floating label over a planet site.
		/// @param visible Label is visible.
		/// @param text Text to show above the location. (default: "")
		/// @param location The location in planetary coords. (default: Vector())
		/// @param height How high above the location to show the text, adjustment from a good default. (default: 1.0)
		void UpdateSiteNameLabel(bool visible, std::string text = "", const Vector& location = Vector(), float height = 1.0);

		/// Starts an animation of a label showing a text string over a player bar
		/// @param player Which player the indication is relevant to
		/// @param text The string to display.
		/// @param screenPos Where, in screen coords the change should be indicated. The CENTER of
		/// the floating label will line up with this pos.
		/// @param animLengthMS How long, in MS, that the animation should linger
		void PlayerTextIndication(int player, std::string text, const Vector& screenPos, double animLengthMS);

		/// Starts an animation of a label showing funds changing for a player
		/// @param player Which player the change is relevant to
		/// @param change The change in funds to display.
		/// @param screenPos Where, in screen coords the change should be indicated. The RIGHTMOST
		/// @param animLengthMS UPPER CORNER of the floating label will line up with this pos.
		/// How long, in MS, that the animation should linger
		void FundsChangeIndication(int player, float change, const Vector& screenPos, double animLengthMS);

		/// Starts an animation of a label showing brains changing for a player
		/// @param player Which player the change is relevant to
		/// @param change The change in brains to display.
		/// @param screenPos Where, in screen coords the change should be indicated. The LEFTMOST
		/// @param fontAlignment UPPER CORNER of the floating label will line up with this pos.
		/// @param animLengthMS How long, in MS, that the animation should linger
		/// The horizontal font alignment of the change.
		void BrainsChangeIndication(int player, int change, const Vector& screenPos, int fontAlignment, double animLengthMS);

		/// Tells whether a SiteLine can be considered visible.
		/// @param sl The SiteLine to check.
		/// @return Whether visible.
		bool IsSiteLineVisible(SiteLine& sl) { return sl.m_OnlyFirstSegments != 0 && sl.m_OnlyLastSegments != 0; }

		/// Removes a specific index siteline out of a vector.
		/// @param lineList The vector of SiteLine:s to remove from.
		/// @param removeIndex The index of the siteline to remove
		/// @return Whether the line was removed or not.
		bool RemoveSiteLine(std::vector<SiteLine>& lineList, int removeIndex);

		/// Gets the total funds of all visible lines of a specific player.
		/// @param lineList A vector with SiteLine:s which may contain other players' lines too.
		/// @param player Which player's lines to check for.
		/// @param onlyVisible Only count the funds of visible lines. (default: true)
		/// @return The total funds, in oz.
		float GetPlayerLineFunds(std::vector<SiteLine>& lineList, int player, bool onlyVisible = true);

		/// Updates the site line meter ratios of a player based on their fund
		/// amounts and visibilty.
		/// @param lineList A vector with SiteLine:s which may contain other players' lines too.
		/// @param metaPlayer Which player's lines to update.
		/// @param onlyVisible Whetehr to only care about visible lines. (default: true)
		/// @param total The total funds to be calculating the ratios against. If negative, (default: -1)
		/// the total line amounts is what will be used.
		void UpdatePlayerLineRatios(std::vector<SiteLine>& lineList, int metaPlayer, bool onlyVisible = true, float total = -1);

		/// Draws a fancy thick flickering line to point out scene points on the
		/// planet.
		/// @param drawBitmap The bitmap to draw to.
		/// @param start The start and end Vector:s for the line, in absolute screen coordinates.
		/// @param end The color to draw the line in. Use makecol(r, g, b) to create the color
		static void DrawGlowLine(BITMAP* drawBitmap, const Vector& start, const Vector& end, int color);

		/// Draws a fancy thick flickering lines to point out scene points on the
		/// planet, FROM an arbitrary screen point.
		/// @param drawBitmap The bitmap to draw to.
		/// @param screenPoint The point on the screen to point from, in screen coordinates.
		/// @param planetPoint The point on the planet to point at, in planet coordinates.
		/// @param color The color of the line.
		/// @param onlyFirstSegments How many of the segments from the start (the start of the line) to draw. (default: -1)
		/// @param onlyLastSegments How many of the segments from the end (site circle) to draw. -1 is all. (default: -1)
		/// @param channelHeight The height of the 'channel' above and below that the lines will go around (default: 80)
		/// the player bar.
		/// @param circleSize What size factor from 'normal' should the circle's diameter be drawn. (default: 1.0)
		/// @return Whether all segments of the line were drawn with the segment params.
		bool DrawScreenLineToSitePoint(BITMAP* drawBitmap,
		                               const Vector& screenPoint,
		                               const Vector& planetPoint,
		                               int color,
		                               int onlyFirstSegments = -1,
		                               int onlyLastSegments = -1,
		                               int channelHeight = 80,
		                               float circleSize = 1.0,
		                               bool squareSite = false) const;

		/// Draws a fancy thick flickering lines to point out scene points on the
		/// planet, FROM a floating player bar, showing a certain ratio.
		/// @param drawBitmap The bitmap to draw to.
		/// @param player The player whose floating bar we draw from.
		/// @param startMeterAt The start percentage of the meter to indicate, from 0 to 1.0
		/// @param meterAmount The actual percentage of the meter to indicate, from 0 to 1.0
		/// @param planetPoint The point on the planet to point at, in planet coordinates.
		/// @param color The color of the line.
		/// @param onlyFirstSegments How many of the segments from the start (the player floater) to draw. (default: -1)
		/// @param onlyLastSegments How many of the segments from the end (site circle) to draw. -1 is all. (default: -1)
		/// @param channelHeight The height of the 'channel' above and below that the lines will go around (default: 60)
		/// the player bar.
		/// @param circleSize What size factor from 'normal' should the circle's diameter be drawn. (default: 1.0)
		/// @param squareSite Whether the circle should instead be a squareSite! (default: false)
		/// @param drawMeterOverride Whether to draw the meter (FirstSegment == 1) no matter what (default: false)
		/// @return Whether all segments of the line were drawn with the segment params.
		bool DrawPlayerLineToSitePoint(BITMAP* drawBitmap,
		                               int player,
		                               float startMeterAt,
		                               float meterAmount,
		                               const Vector& planetPoint,
		                               int color,
		                               int onlyFirstSegments = -1,
		                               int onlyLastSegments = -1,
		                               int channelHeight = 60,
		                               float circleSize = 1.0,
		                               bool squareSite = false,
		                               bool drawMeterOverride = false) const;

		/// Draws a fancy thick flickering lines to point out scene points on the
		/// planet, FROM a floating player bar, showing a certain ratio.
		/// @param drawBitmap The bitmap to draw to.
		/// @param sl The SiteLine struct with all the parameters this needs.
		/// @param drawMeterOverride Whether to draw the meter (FirstSegment == 1) no matter what (default: false) const { return DrawPlayerLineToSitePoint(drawBitmap)
		/// @return Whether all segments of the line were drawn with the segment params.
		bool DrawPlayerLineToSitePoint(BITMAP* drawBitmap, const SiteLine& sl, bool drawMeterOverride = false) const { return DrawPlayerLineToSitePoint(drawBitmap, sl.m_Player, sl.m_StartMeterAt, sl.m_MeterAmount, sl.m_PlanetPoint.GetFloored(), sl.m_Color, sl.m_OnlyFirstSegments, sl.m_OnlyLastSegments, sl.m_ChannelHeight, sl.m_CircleSize, sl.m_Square, drawMeterOverride); }

		/// Moves any locations closer to the ceonter of the planet if they were left out
		/// of the screen due to low display resolution.
		/// Arguments:		None.
		/// Return value:	None.
		void MoveLocationsIntoTheScreen();

		enum MenuEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		enum MetaButtons {
			CONFIRM = 0,
			P1CONTROL,
			P2CONTROL,
			P3CONTROL,
			P4CONTROL,
			STARTNEW,
			LOADNOW,
			SAVENOW,
			CONTINUE,
			SCENEACTION,
			DESIGNBASE,
			SCANNOW,
			SCANLATER,
			METABUTTONCOUNT
		};

		enum BlinkMode {
			NOBLINK = 0,
			NOFUNDS,
			NOCRAFT,
			BLINKMODECOUNT
		};

		enum LineAnimMode {
			PAUSEANIM = 0,
			TARGETZEROING,
			BLINKCIRCLE,
			SHRINKCIRCLE,
			LINECONNECTFW,
			LINECONNECTBW,
			LINEDISCONNECTFW,
			BLINKMETER,
			GROWMETER,
			SHRINKMETER,
			RETRACTLINES,
			SHOWDEFENDERS,
			SHOWPOSTBATTLEPAUSE,
			SHOWPOSTBATTLEBALANCE,
			SHOWPOSTBATTLEBRAINS,
			SHOWNEWRESIDENTS,
			ANIMMODECOUNT
		};

		int m_RootBoxMaxWidth; //!< The maximum width the root CollectionBox that holds all this menu's GUI elements. This is to constrain this menu to the primary window's display (left-most) while in multi-display fullscreen, otherwise positioning can get stupid.

		// Controller which controls this menu. Not owned
		Controller* m_pController;
		// GUI Screen for use by the in-game GUI
		GUIScreen* m_pGUIScreen;
		// Input controller
		GUIInput* m_pGUIInput;
		// The control manager which holds all the controls
		GUIControlManager* m_pGUIController;
		// Visibility state of the menu
		int m_MenuEnabled;
		// Screen selection state
		int m_MenuScreen;
		// Change in menu screens detected
		bool m_ScreenChange;
		// Focus state on selecting scenes
		int m_SceneFocus;
		// Focus change direction - 0 is none, negative is back, positive forward
		int m_FocusChange;
		// Speed at which the menus appear and disappear
		float m_MenuSpeed;
		// Notification blink timer
		Timer m_BlinkTimer;
		// What we're blinking
		int m_BlinkMode;

		// GUI Banners
		GUIBanner* m_pBannerRedTop;
		GUIBanner* m_pBannerRedBottom;
		GUIBanner* m_pBannerYellowTop;
		GUIBanner* m_pBannerYellowBottom;

		// General-purpose animation timers
		Timer m_AnimTimer1;
		Timer m_AnimTimer2;
		Timer m_AnimTimer3;

		// Currently animated things
		int m_AnimMode;
		bool m_AnimModeChange;
		float m_AnimModeDuration;
		int m_AnimMetaPlayer;
		int m_AnimDefenseTeam;
		bool m_AnimActivityChange;
		const Scene* m_pAnimScene;
		float m_AnimRatio;
		float m_AnimProgress;
		float m_AnimTotalFunds;
		float m_AnimFundsMax;
		float m_AnimFundsMin;
		int m_AnimBuildCount;
		int m_AnimIncomeLineIndex;
		bool m_AnimIncomeLineChange;
		int m_AnimActionLine;
		bool m_AnimActionLineChange;
		int m_AnimSegment;
		int m_AnimCountStart;
		int m_AnimCountCurrent;
		int m_AnimCountEnd;
		// Whether the line we're animating managed to connect with the current params
		bool m_LineConnected;
		// The income-related lines to keep drawing each frame
		std::vector<SiteLine> m_IncomeSiteLines;
		// Indices to the player sitelines that point at the moving station
		int m_aStationIncomeLineIndices[Players::MaxPlayerCount];
		// Indices to the player sitelines that point at the their own brain pool counters
		int m_aBrainSaleIncomeLineIndices[Players::MaxPlayerCount];
		// Which player are currently showing their player lines. -1 means none
		int m_ActivePlayerIncomeLines;
		// The action-related lines to keep drawing each frame
		std::vector<SiteLine> m_ActionSiteLines[Players::MaxPlayerCount];
		// Override to alwasys draw the player action meters, no matter what
		bool m_ActionMeterDrawOverride;
		// The attack target crosshair info
		SiteTarget m_SiteAttackTarget;
		// The crosshairs showing new sites
		std::vector<SiteTarget> m_NewSiteIndicators;
		// The indicators of sites that just changed ownership
		std::vector<SiteTarget> m_SiteSwitchIndicators;

		// The absolute screen position of the planet center
		Vector m_PlanetCenter;
		// The screen radius of the planet
		float m_PlanetRadius;

		// General game message label
		GUILabel* m_pGameMessageLabel;

		// Tooltip box
		GUICollectionBox* m_pToolTipBox;
		// Label displaying the ToolTip info
		GUILabel* m_pToolTipText;
		// Timer for detemining when it's time to actually show the tt
		Timer m_ToolTipTimer;
		// The control that the cursor has hovered over
		GUIControl* m_pHoveredControl;

		// Collection boxes of the main screens of the GUI
		GUICollectionBox* m_apScreenBox[SCREENCOUNT];
		// The metagame menu buttons
		GUIButton* m_apMetaButton[METABUTTONCOUNT];

		// The confirmation box and its controls
		GUICollectionBox* m_pConfirmationBox;
		GUILabel* m_pConfirmationLabel;
		GUIButton* m_pConfirmationButton;

		// The player floating bars
		GUICollectionBox* m_apPlayerBox[Players::MaxPlayerCount];
		// The player flag icon in the floating bars
		GUICollectionBox* m_apPlayerTeamBox[Players::MaxPlayerCount];
		// Funds label in the floating bars
		GUILabel* m_apPlayerBarLabel[Players::MaxPlayerCount];
		// Brain Pool label next to the floating bars
		GUILabel* m_apBrainPoolLabel[Players::MaxPlayerCount];
		// The animated label that shows a message to the player over his bar
		//    GUILabel *m_apPlayerMessageLabel[Players::MaxPlayerCount];
		// The animated label that shows a change in the funds of a player, animating up or downward
		GUILabel* m_apFundsChangeLabel[Players::MaxPlayerCount];
		// The animated label that shows a change in the brain pool of a player, animating up or downward
		GUILabel* m_apBrainChangeLabel[Players::MaxPlayerCount];

		// Timer for animating the message labels going northward
		//    Timer m_apPlayerMessageTimer[Players::MaxPlayerCount];
		// Timer for animating the change labels going northward
		Timer m_apFundsChangeTimer[Players::MaxPlayerCount];
		// Timer for animating the change labels going northward
		Timer m_apBrainsChangeTimer[Players::MaxPlayerCount];
		// Previous pos of mouse to calculate dragging
		Vector m_PrevMousePos;

		// Battle site display
		// The player flag icon that surrrounds the battle sites
		GUICollectionBox* m_apPlayerTeamActionBox[Players::MaxPlayerCount];
		// Traveling brain label that ends up around battle sites, showing info
		GUILabel* m_apPlayerBrainTravelLabel[Players::MaxPlayerCount];
		// How much funds have been allocated to each player's battle chest for the next battle
		float m_aBattleFunds[Players::MaxPlayerCount];
		// Which of the players are currently attacking a place - just used for icons
		bool m_aBattleAttacker[Players::MaxPlayerCount];
		// Which of the battling brains are yet graphically destroyed
		bool m_aAnimDestroyed[Players::MaxPlayerCount];
		// Where the center of the brain icon is on the traveling brain label
		Vector m_aBrainIconPos[Players::MaxPlayerCount];
		// Which quadrant positions of the battle matrix that have been taken by which metaplayer. NOPLAYER means no player (duh)
		int m_aQuadTakenBy[4];

		// Game Phase Box and info
		GUICollectionBox* m_pPhaseBox;
		GUILabel* m_pPhaseLabel;
		// Pre-player-turn hold so player's privacy is protected
		bool m_PreTurn;
		// Have an incompleted offensive battle to resume
		bool m_BattleToResume;
		// Still showing the aftermath of a battle before moving onto the next
		bool m_PostBattleReview;
		// Whether the last battle caused a change in team
		bool m_BattleCausedOwnershipChange;
		// The previous ownership status of a battled scene.
		// It's used to for a period during battle review still show the old ownership until the new one is dramaticlaly revealed
		int m_PreBattleTeamOwnership;

		// Hover name label over Scene:s
		GUILabel* m_pScenePlanetLabel;

		// Scene info popup mouseover box and controls
		GUICollectionBox* m_pSceneInfoPopup;
		GUIButton* m_pSceneCloseButton;
		GUILabel* m_pSceneNameLabel;
		GUICollectionBox* m_pSceneOwnerTeam;
		GUILabel* m_pSceneResidentsLabel;
		GUILabel* m_pSceneInfoLabel;
		GUILabel* m_pSceneBudgetLabel;
		GUISlider* m_pSceneBudgetSlider;
		GUICollectionBox* m_pSceneBudgetBar;
		GUICheckbox* m_pAutoDesignCheckbox;
		GUILabel* m_pScanInfoLabel;

		// Currently dragged GUI box
		GUICollectionBox* m_pDraggedBox;
		// New potential drag is starting
		bool m_EngageDrag;
		// The scene currently hovered, NOT OWNED
		Scene* m_pHoveredScene;
		// The scene currently selected, NOT OWNED
		Scene* m_pSelectedScene;
		// The scene currently being played, NOT OWNED
		Scene* m_pPlayingScene;

		bool m_TechAndFlagListFetched; //!< Whether the tech list was fetched and each team's ComboBox was populated with it, even if no valid tech modules were added. The team flags are also populated at the same time.

		// NEW GAME DIALOG
		// Game size label and slider
		GUILabel* m_pSizeLabel;
		GUISlider* m_pSizeSlider;
		GUILabel* m_pDifficultyLabel;
		GUISlider* m_pDifficultySlider;
		GUILabel* m_pGoldLabel;
		GUISlider* m_pGoldSlider;
		GUILabel* m_pLengthLabel;
		GUISlider* m_pLengthSlider;
		GUILabel* m_pErrorLabel;
		GUIButton* m_apPlayerControlButton[Players::MaxPlayerCount];
		GUIComboBox* m_apPlayerTeamSelect[Players::MaxPlayerCount];
		GUIComboBox* m_apPlayerTechSelect[Players::MaxPlayerCount];
		GUIComboBox* m_apPlayerHandicap[Players::MaxPlayerCount];
		GUITextBox* m_apPlayerNameBox[Players::MaxPlayerCount];
		GUISlider* m_apPlayerAISkillSlider[Players::MaxPlayerCount];
		GUILabel* m_apPlayerAISkillLabel[Players::MaxPlayerCount];

		// SAVING/LOADING GAME DIALOGS
		GUITextBox* m_NewSaveBox;
		// The combobox which lists all the games that can be saved over
		GUIComboBox* m_pSavesToOverwriteCombo;
		GUIComboBox* m_pSavesToLoadCombo;
		// Info boxes showing some basic data on a save game
		GUILabel* m_pSaveInfoLabel;
		GUILabel* m_pLoadInfoLabel;
		// Hack to keep the MetaSave Entity that has been selected for load even though confirmation dlg pups up and clears the selection combo
		const Entity* m_pSelectedGameToLoad;

		// Whether player decided to continue to the next phase of the game
		bool m_ContinuePhase;
		// Whether the game was restarted this frame or not
		bool m_ActivityRestarted;
		// Whether the game was resumed this frame or not
		bool m_ActivityResumed;
		// How much money both players start with in the new game
		int m_StartFunds;
		// Which player is CPU managed, if any (-1)
		int m_CPUPlayer;
		// Difficulty setting
		int m_StartDifficulty;
		// Whether user has chosen to go back to the main menu
		bool m_BackToMain;
		// Player selected to quit the program
		bool m_Quit;

		Vector m_StationPosOnOrbit; //!< The position of the station on the planet orbit.

		/// Private member variable and method declarations
	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this MetagameGUI, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MetagameGUI(const MetagameGUI& reference) = delete;
		MetagameGUI& operator=(const MetagameGUI& rhs) = delete;
	};

} // namespace RTE
