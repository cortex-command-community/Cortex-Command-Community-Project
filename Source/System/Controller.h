#pragma once

#include "Timer.h"
#include "Vector.h"

#include <array>
#include <utility>

namespace RTE {

	class Actor;

	//!< Enumerate control states.
	enum ControlState {
		PRIMARY_ACTION = 0,
		SECONDARY_ACTION,
		MOVE_IDLE,
		MOVE_RIGHT,
		MOVE_LEFT,
		MOVE_UP,
		MOVE_DOWN,
		MOVE_FAST,
		BODY_JUMPSTART,
		BODY_JUMP,
		BODY_CROUCH,
		AIM_UP,
		AIM_DOWN,
		AIM_SHARP,
		WEAPON_FIRE,
		WEAPON_RELOAD,
		PIE_MENU_OPENED,
		PIE_MENU_ACTIVE,
		// When using a gamepad, the pie menu can be opened either by left trigger or the right secondary button
		// We want to differentiate between those, so that if the player is using their left hand to open the pie menu, then their right hand selects options
		// And vice versa
		PIE_MENU_ACTIVE_ANALOG,
		PIE_MENU_ACTIVE_DIGITAL,
		WEAPON_CHANGE_NEXT,
		WEAPON_CHANGE_PREV,
		WEAPON_PICKUP,
		WEAPON_DROP,
		ACTOR_NEXT,
		ACTOR_PREV,
		ACTOR_BRAIN,
		ACTOR_NEXT_PREP,
		ACTOR_PREV_PREP,
		HOLD_RIGHT,
		HOLD_LEFT,
		HOLD_UP,
		HOLD_DOWN,
		// These will only register once for key presses.
		PRESS_PRIMARY,
		PRESS_SECONDARY,
		PRESS_RIGHT,
		PRESS_LEFT,
		PRESS_UP,
		PRESS_DOWN,
		// When the buttons are released.
		RELEASE_PRIMARY,
		RELEASE_SECONDARY,
		// Either button of fire or aim
		PRESS_FACEBUTTON,
		RELEASE_FACEBUTTON,
		SCROLL_UP,
		SCROLL_DOWN,
		DEBUG_ONE,
		CONTROLSTATECOUNT
	};

	/// A class controlling MovableObjects through either player input, networking, scripting, AI, etc.
	class Controller {

	public:
		Vector m_AnalogMove; //!< Analog values for movement.
		Vector m_AnalogAim; //!< Analog values for aiming.
		Vector m_AnalogCursor; //!< Analog values for Pie Menu operation.

		//!< Enumerate different input modes.
		enum InputMode {
			CIM_DISABLED = 0,
			CIM_PLAYER,
			CIM_AI,
			CIM_NETWORK,
			CIM_INPUTMODECOUNT
		};

#pragma region Creation
		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		Controller() { Clear(); }

		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		/// @param mode The controller input mode, like AI, player etc.
		/// @param controlledActor The Actor this is supposed to control. Ownership is NOT transferred!
		Controller(InputMode mode, Actor* controlledActor) {
			Clear();
			Create(mode, controlledActor);
		}

		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		/// @param mode The controller input mode, like AI, player etc.
		/// @param player Which human player is controlling this.
		Controller(InputMode mode, int player = Players::PlayerOne) {
			Clear();
			Create(mode, player);
		}

		/// Copy constructor method used to instantiate a Controller object identical to an already existing one.
		/// @param reference A Controller object which is passed in by reference.
		Controller(const Controller& reference) {
			if (this != &reference) {
				Create(reference);
			}
		}

		/// Makes the Controller object ready for use.
		/// @param mode The controller input mode, like AI, player etc.
		/// @param controlledActor The Actor this is supposed to control. Ownership is NOT transferred!
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(InputMode mode, Actor* controlledActor);

		/// Makes the Controller object ready for use.
		/// @param mode The controller input mode, like AI, player etc.
		/// @param player Which player is controlling this.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(InputMode mode, int player) {
			m_InputMode = mode;
			m_Player = player;
			return 0;
		}

		/// Creates a Controller to be identical to another, by deep copy.
		/// @param reference A reference to the Controller to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Controller& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the entire Controller, including its inherited members, to their default settings or values.
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Shortcut to indicate if in player input mode.
		/// @param otherThanPlayer If you want to check if it's controlled by a player, AND that player is someone else than a specific one, pass in that player number here.
		/// @return Whether input mode is set to player input.
		bool IsPlayerControlled(int otherThanPlayer = Players::NoPlayer) const { return (m_InputMode == InputMode::CIM_PLAYER && (otherThanPlayer < Players::PlayerOne || m_Player != otherThanPlayer)); }

		/// Shows whether this controller is disabled.
		/// @return Whether disabled.
		bool IsDisabled() const { return m_InputMode == InputMode::CIM_DISABLED || m_Disabled; }

		/// Sets whether this is a disabled controller that doesn't give any new output.
		/// @param disabled Disabled or not.
		void SetDisabled(bool disabled = true) {
			if (m_Disabled != disabled) {
				m_ReleaseTimer.Reset();
				ResetCommandState();
			}
			m_Disabled = disabled;
		}

		/// Shows whether the current controller is in a specific state.
		/// @param controlState What control state to check for.
		/// @return Whether the controller is in the specified state.
		bool IsState(ControlState controlState) const { return m_ControlStates[controlState]; };

		/// Sets one of this controller's states.
		/// @param controlStat Which state to set.
		/// @param setting Value of the state being set.
		void SetState(ControlState controlState, bool setting = true) {
			RTEAssert(controlState >= 0 && controlState < ControlState::CONTROLSTATECOUNT, "Control state out of whack");
			m_ControlStates[controlState] = setting;
		};

		/// Gets the current mode of input for this Controller.
		/// @return The InputMode that this controller is currently using.
		InputMode GetInputMode() const { return m_InputMode; }

		/// Sets the mode of input for this Controller.
		/// @param newMode The new InputMode for this controller to use.
		void SetInputMode(InputMode newMode) {
			if (m_InputMode != newMode) {
				m_ReleaseTimer.Reset();
			}
			m_InputMode = newMode;
		}

		/// Gets the analog movement input data.
		/// @return A vector with the analog movement data, both axes ranging form -1.0 to 1.0.
		Vector GetAnalogMove() const { return m_AnalogMove; }

		/// Sets the analog movement vector state of this.
		/// @param newMove The new analog movement vector.
		void SetAnalogMove(const Vector& newMove) { m_AnalogMove = newMove; }

		/// Gets the analog aiming input data.
		/// @return A vector with the analog aiming data, both axes ranging form -1.0 to 1.0.
		Vector GetAnalogAim() const { return m_AnalogAim; }

		/// Sets the analog aiming vector state of this.
		/// @param newAim The new analog aiming vector.
		void SetAnalogAim(const Vector& newAim) { m_AnalogAim = newAim; }

		/// Gets the analog menu input data.
		/// @return A vector with the analog menu data, both axes ranging form -1.0 to 1.0.
		Vector GetAnalogCursor() const { return m_AnalogCursor; }

		/// Sets the analog cursor to the specified position.
		/// @param newAnalogCursor The position the analog cursor should be set to.
		void SetAnalogCursor(const Vector& newAnalogCursor) { m_AnalogCursor = newAnalogCursor; }

		/// Sets the analog cursor angle limits for the given player (does nothing for player -1). The limit end is always CCW from the limit start.
		/// @param angleLimitStart The starting angle limit for the analog cursor.
		/// @param angleLimitEnd The ending angle limit for the analog cursor.
		void SetAnalogCursorAngleLimits(float angleLimitStart, float angleLimitEnd) { m_AnalogCursorAngleLimits = {{angleLimitStart, angleLimitEnd}, true}; }

		/// Clears the analog cursor aim limits for the given player (does nothing for player -1).
		void ClearAnalogCursorAngleLimits() { m_AnalogCursorAngleLimits.second = false; }

		/// Adds relative movement to a passed-in vector. Uses the appropriate input method currently of this.
		/// @param cursorPos The vector to alter.
		/// @param moveScale The scale of the input. 1.0 is 'normal'.
		/// @return Whether the vector was altered or not.
		bool RelativeCursorMovement(Vector& cursorPos, float moveScale = 1.0F) const;

		/// Indicates whether this is listening to mouse input at all.
		/// @return Whether this is using mouse input at all.
		bool IsMouseControlled() const;

		/// Indicates whether this is only listening to keyboard input.
		/// @return Whether this is only using keyboard input.
		bool IsKeyboardOnlyControlled() const;

		/// Indicates whether this is listening to gamepad at all.
		/// @return Whether this is using gamepad input at all.
		bool IsGamepadControlled() const;

		/// Gets the relative movement of the mouse since last update.
		/// @return The relative mouse movements, in both axes.
		const Vector& GetMouseMovement() const { return m_MouseMovement; }

		/// Get the digital aim speed multiplier of the scheme associated with this Controller.
		/// @return The digital aim speed set to the scheme of this Controller.
		float GetDigitalAimSpeed() const;

		/// Gets the player this is listening. Unlike GetPlayer, this ignores the input mode.
		/// @return The player this is listening to, regardless of input mode.
		int GetPlayerRaw() const { return m_Player; }

		/// Gets which player's input this is listening to, if in player input mode.
		/// @return The player number, or -1 if not in player input mode.
		int GetPlayer() const { return (m_InputMode == InputMode::CIM_PLAYER) ? m_Player : Players::NoPlayer; }

		/// Sets which player's input this is listening to, and will enable player input mode.
		/// @param player The player number.
		void SetPlayer(int player) {
			m_Player = player;
			if (m_Player >= Players::PlayerOne) {
				m_InputMode = InputMode::CIM_PLAYER;
			}
		}

		/// Gets the Team number using this controller.
		/// @return An int representing the team which this Controller belongs to. 0 is the first team. 0 if no team is using it.
		int GetTeam() const;

		/// Sets the team which is controlling this Controller's controlled Actor.
		/// @param team The team number. 0 is the first team.
		void SetTeam(short team);

		/// Gets which Actor is being controlled by this. 0 if none.
		/// @return A pointer to the Actor which is being controlled by this. Ownership is NOT transferred!
		Actor* GetControlledActor() const { return m_ControlledActor; }

		/// Sets which Actor is supposed to be controlled by this.
		/// @param controlledActor A pointer to a an Actor which is being controlled by this. Ownership is NOT transferred!
		void SetControlledActor(Actor* controlledActor = nullptr) { m_ControlledActor = controlledActor; }

		/// Returns whether the AI should be updated this frame.
		/// @return Whether the AI should be updated this frame.
		bool ShouldUpdateAIThisFrame() const;
#pragma endregion

#pragma region Virtual Override Methods
		/// Updates this Controller. Supposed to be done every frame.
		void Update();
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one Controller equal to another.
		/// @param rhs A Controller reference.
		/// @return A reference to the changed Controller.
		Controller& operator=(const Controller& rhs);
#pragma endregion

#pragma region Misc
		/// Overrides this controller, setting it to match another controller. This is useful for multithreading, where the Lua script can use a copied controller in a multi-threaded context, before overriding the controller in a single-threaded context.
		/// This is exposed to Lua API to be clear, whereas ownership relies on operator overloading is rather temperamental :)
		/// @param otherController The other controller's state to copy. Ownership is not transferred
		void Override(const Controller& otherController);
#pragma endregion

	protected:
		static constexpr int m_ReleaseDelay = 250; //!< The delay between releasing a menu button and activating the regular controls, to avoid accidental input.

		std::array<bool, ControlState::CONTROLSTATECOUNT> m_ControlStates; //!< Control states.
		bool m_Disabled; //!< Quick and easy disable to prevent updates from being made.

		InputMode m_InputMode; //!< The current controller input mode, like AI, player etc.

		Actor* m_ControlledActor; //!< The actor controlled by this.

		/// The last player this controlled. This is necessary so we still have some control after controlled's death.
		/// If this is -1, no player is controlling/ed, even if in player control input mode.
		int m_Player;

		int m_Team; //!< The last team this controlled. This is necessary so we still have some control after controlled's death.

		/// These are hacks to make the switch to brain shortcut work without immediately switching away by
		/// detecting the release of the previous and next buttons after pressing them both down to get to the brain.
		bool m_NextIgnore;
		bool m_PrevIgnore;

		//!< These are used to track just a single press on  shortcut button.
		bool m_WeaponChangeNextIgnore;
		bool m_WeaponChangePrevIgnore;
		bool m_WeaponPickupIgnore;
		bool m_WeaponDropIgnore;
		bool m_WeaponReloadIgnore;

		Timer m_ReleaseTimer; //!< Timer for measuring release delays.
		Timer m_JoyAccelTimer; //!< Timer for measuring analog joystick-controlled cursor acceleration.
		Timer m_KeyAccelTimer; //!< Timer for measuring keyboard-controlled cursor acceleration.

		Vector m_MouseMovement; //!< Relative mouse movement, if this player uses the mouse.

		std::pair<std::pair<float, float>, bool> m_AnalogCursorAngleLimits; //!< Analog aim value limits, as well as whether or not the limit is actually enabled.

	private:
#pragma region Update Breakdown
		/// Updates the player's inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		/// This method will call both UpdatePlayerPieMenuInput and UpdatePlayerAnalogInput.
		void UpdatePlayerInput();

		/// Updates the player's PieMenu inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		void UpdatePlayerPieMenuInput();

		/// Updates the player's analog inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		void UpdatePlayerAnalogInput();

		/// Clears the command state, meaning no input is given and our actor will be idle.
		void ResetCommandState();

		/// Requests and applies input from the player.
		void GetInputFromPlayer();
#pragma endregion

		/// Clears all the member variables of this Controller, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
