#pragma once

// TODO: Figure out how to deal with anything that is defined by these and include them in implementation only to remove Windows.h macro pollution from our headers.
#include "RakPeerInterface.h"
#include "NatPunchthroughClient.h"

// RakNet includes Windows.h so we need to undefine macros that conflict with our method names.
#undef GetClassName

#include "Singleton.h"
#include "NetworkMessages.h"
#include "PostProcessMan.h"
#include "Vector.h"

#define g_NetworkClient NetworkClient::Instance()

/////////////////////////////////////////////////////////////////////////
// TODO: Get Weegee to comment all these because I don't even.
/////////////////////////////////////////////////////////////////////////

namespace RTE {

	class SoundContainer;

	/// The centralized singleton manager of the network multiplayer client.
	class NetworkClient : public Singleton<NetworkClient> {
		friend class SettingsMan;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a NetworkClient object in system memory. Create() should be called before using the object.
		NetworkClient();

		/// Makes the NetworkClient object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a NetworkClient object before deletion from system memory.
		~NetworkClient();

		/// Destroys and resets (through Clear()) the NetworkClient object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters
		/// Gets whether the client is connected and registered to a server.
		/// @return Whether the client is connected and registered to a server.
		bool IsConnectedAndRegistered() const { return m_IsConnected && m_IsRegistered; }

		/// Gets scene width for network client.
		/// @return Current scene width.
		int GetSceneWidth() const { return m_SceneWidth; }

		/// Gets scene height for network client.
		/// @return Current scene height.
		int GetSceneHeight() const { return m_SceneHeight; }

		/// Indicates whether the scene wraps its scrolling around the X axis for network client.
		/// @return Whether the scene wraps around the X axis or not.
		bool SceneWrapsX() const { return m_SceneWrapsX; }

		/// Get the coordinates of the center of the current frame.
		/// @return A vector containing the X/Y coordinates of the frame target.
		const Vector& GetFrameTarget() const { return m_TargetPos[m_CurrentFrameNum]; }

#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this NetworkClient. Supposed to be done every frame.
		void Update();
#pragma endregion

#pragma region Connection Handling
		/// Connects the client to a server.
		/// @param serverName Server name (or address) to connect to.
		/// @param serverPort Server port.
		/// @param playerName Player name to be used in network game.
		void Connect(std::string serverName, unsigned short serverPort, std::string playerName);

		/// Connects the client to a server through a NAT service.
		/// @param address The NAT service address.
		void ConnectNAT(RakNet::SystemAddress address);

		/// Disconnects the client from the currently connected server.
		void Disconnect();

		/// Connects to a NAT service and performs punch-through.
		/// @param serviceServerName NAT service server name (or address) to use for punch-through.
		/// @param serviceServerPort NAT service server port.
		/// @param playerName Player name to be used in network game.
		/// @param serverName Server name (or address) to connect to.
		/// @param serverPassword Server password.
		void PerformNATPunchThrough(std::string serviceServerName, unsigned short serviceServerPort, std::string playerName, std::string serverName, std::string serverPassword);

		///
		/// @param rakPeer
		/// @param address
		/// @param port
		/// @return
		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface* rakPeer, const char* address, unsigned short port);
#pragma endregion

	protected:
		static constexpr unsigned short c_PlayerNameCharLimit = 15; //!< Maximum length of the player name.

		std::string m_PlayerName; //!< The player name the will be used by the client in network games.

		RakNet::RakPeerInterface* m_Client; //!< The client RakPeerInterface.
		RakNet::SystemAddress m_ClientID; //!< The client's identifier.

		RakNet::SystemAddress m_ServerID; //!< The server's identifier.
		RakNet::RakNetGUID m_ServerGUID; //!< The server's Globally Unique Identifier.

		RakNet::NatPunchthroughClient m_NATPunchthroughClient; //!< The NAT punch-through client.
		RakNet::SystemAddress m_NATServiceServerID; //!< The NAT server's identifier.

		bool m_UseNATPunchThroughService; //!< Whether to use NAT service for connecting to server.

		bool m_IsConnected; //!< Is client connected to server.
		bool m_IsRegistered; //!< Is client registered at server.
		bool m_IsNATPunched; //!< Is client connected through NAT service.

		unsigned char m_PixelLineBuffer[c_MaxPixelLineBufferSize]; //!<

		long int m_ReceivedData; //!<
		long int m_CompressedData; //!<

		int m_ClientInputFps; //!< The rate (in FPS) the client input is sent to the server.
		long long m_LastInputSentTime; //!< The last time input was sent in real time ticks.

		int m_CurrentFrameNum; //!< The received frame number.
		int m_CurrentBoxWidth; //!< The received frame box width.
		int m_CurrentBoxHeight; //!< The received frame box height.
		bool m_CurrentFrameInterlaced; //!< Whether the received frame data is interlaced.
		bool m_CurrentFrameDeltaCompressed; //!< Whether the received frame data is delta compressed.

		bool m_ShowFillRate; //!<

		Vector m_TargetPos[c_FramesToRemember]; //!<
		std::list<PostEffect> m_PostEffects[c_FramesToRemember]; //!< List of post-effects received from server.

		std::unordered_map<int, SoundContainer*> m_ServerSounds; //!< Unordered map of SoundContainers received from server. OWNED!!!

		unsigned char m_SceneID; //!<
		int m_CurrentSceneLayerReceived; //!<

		BITMAP* m_SceneBackgroundBitmap; //!<
		BITMAP* m_SceneForegroundBitmap; //!<

		BITMAP* m_BackgroundBitmaps[c_MaxLayersStoredForNetwork]; //!<
		LightweightSceneLayer m_BackgroundLayers[c_FramesToRemember][c_MaxLayersStoredForNetwork]; //!<
		int m_ActiveBackgroundLayers; //!<
		bool m_SceneWrapsX; //!<
		int m_SceneWidth; //!<
		int m_SceneHeight; //!<

		int m_MouseButtonPressedState[MAX_MOUSE_BUTTONS]; //!<
		int m_MouseButtonReleasedState[MAX_MOUSE_BUTTONS]; //!<
		int m_MouseWheelMoved; //!< Whether the mouse wheel was moved this Update. Used to make mouse wheel detection better.

	private:
#pragma region Update Breakdown
		///
		void HandleNetworkPackets();
#pragma endregion

#pragma region Network Event Handling
		///
		/// @param packet
		/// @return
		unsigned char GetPacketIdentifier(RakNet::Packet* packet) const;

		///
		void SendRegisterMsg();

		///
		void ReceiveAcceptedMsg();

		///
		void SendDisconnectMsg();

		///
		/// @param address
		/// @param serverName
		/// @param serverPassword
		void SendServerGUIDRequest(RakNet::SystemAddress address, std::string serverName, std::string serverPassword);

		///
		/// @param packet
		void ReceiveServerGUIDAnswer(RakNet::Packet* packet);

		///
		void SendInputMsg();

		///
		/// @param packet
		void ReceiveFrameSetupMsg(RakNet::Packet* packet);

		///
		/// @param packet
		void ReceiveFrameLineMsg(RakNet::Packet* packet);

		///
		/// @param packet
		void ReceiveFrameBoxMsg(RakNet::Packet* packet);

		///
		void SendSceneAcceptedMsg();

		///
		/// @param packet
		void ReceiveSceneMsg(RakNet::Packet* packet);

		///
		void ReceiveSceneEndMsg();

		///
		/// @param packet
		void ReceiveSceneSetupMsg(RakNet::Packet* packet);

		///
		void SendSceneSetupAcceptedMsg();

		/// Receive and handle a packet of terrain change data.
		/// @param packet The packet to handle.
		void ReceiveTerrainChangeMsg(RakNet::Packet* packet);

		/// Receive and handle a packet of post-effect data.
		/// @param packet The packet to handle.
		void ReceivePostEffectsMsg(RakNet::Packet* packet);

		/// Receive and handle a packet of sound event data.
		/// @param packet The packet to handle.
		void ReceiveSoundEventsMsg(RakNet::Packet* packet);

		/// Receive and handle a packet of music event data.
		/// @param packet The packet to handle.
		void ReceiveMusicEventsMsg(RakNet::Packet* packet);
#pragma endregion

#pragma region Drawing
		///
		/// @param targetBitmap
		void DrawBackgrounds(BITMAP* targetBitmap);

		///
		/// @param frame
		void DrawPostEffects(int frame);

		///
		/// @param frameNumber
		/// @param useInterlacing
		/// @param clearFramebuffer
		void DrawFrame(int frameNumber, bool useInterlacing, bool clearFramebuffer);
#pragma endregion

		/// Gets the ping time between the client and the server.
		/// @return The ping time between the client and the server.
		int GetPing() const { return IsConnectedAndRegistered() ? m_Client->GetLastPing(m_ServerID) : 0; }

		/// Clears all the member variables of this NetworkClient, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		NetworkClient(const NetworkClient& reference) = delete;
		NetworkClient& operator=(const NetworkClient& rhs) = delete;
	};
} // namespace RTE
