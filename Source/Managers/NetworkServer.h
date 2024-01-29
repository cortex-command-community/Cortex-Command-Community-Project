#pragma once

// TODO: Figure out how to deal with anything that is defined by these and include them in implementation only to remove Windows.h macro pollution from our headers.
#include "RakPeerInterface.h"
#include "NatPunchthroughClient.h"

// RakNet includes Windows.h so we need to undefine macros that conflict with our method names.
#undef GetClassName

#include "Singleton.h"
#include "NetworkMessages.h"

#include <array>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

#define g_NetworkServer NetworkServer::Instance()

/////////////////////////////////////////////////////////////////////////
// TODO: Get Weegee to comment all these because I don't even.
/////////////////////////////////////////////////////////////////////////

namespace RTE {

	class Timer;

	/// The centralized singleton manager of the network multiplayer server.
	class NetworkServer : public Singleton<NetworkServer> {
		friend class SettingsMan;

	public:
		///
		enum NetworkServerStats {
			STAT_CURRENT = 0,
			STAT_SHOWN,
			MAX_STAT_RECORDS = 5
		};

		///
		enum ThreadExitReasons {
			NORMAL = 0,
			THREAD_FINISH,
			TOO_EARLY_TO_SEND,
			SEND_BUFFER_IS_FULL,
			SEND_BUFFER_IS_LIMITED_BY_CONGESTION,
			LOCKED
		};

		/// Struct for registering terrain change events for network transmission.
		struct NetworkTerrainChange {
			int x;
			int y;
			int w;
			int h;
			unsigned char color;
			bool back;
		};

#pragma region Creation
		/// Constructor method used to instantiate a NetworkServer object in system memory. This will call Create() so it shouldn't be called after.
		NetworkServer();

		/// Makes the NetworkServer object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a NetworkServer object before deletion from system memory.
		~NetworkServer();

		/// Destroys and resets (through Clear()) the NetworkServer object.
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether server mode is enabled or not.
		/// @return Whether server mode is enabled or not.
		bool IsServerModeEnabled() const { return m_IsInServerMode; }

		/// Enables server mode.
		void EnableServerMode() { m_IsInServerMode = true; }

		///
		/// @return
		bool ReadyForSimulation();

		/// Gets the network player's name.
		/// @param player The player to check for.
		/// @return A string with the network player's name.
		std::string& GetPlayerName(short player) { return m_ClientConnections[player].PlayerName; }

		/// Gets whether the specified player is connected to the server or not.
		/// @param player The player to check for.
		/// @return Whether the player is connected to the server or not.
		bool IsPlayerConnected(short player) const { return m_ClientConnections[player].IsActive; }

		/// Sets the port this server will be using.
		/// @param newPort The new port to set.
		void SetServerPort(const std::string& newPort);

		/// Sets whether interlacing is used to reduce bandwidth usage or not.
		/// @param newMode Whether to use interlacing or not.
		void SetInterlacingMode(bool newMode) { m_UseInterlacing = newMode; }

		/// Sets the duration this thread should be put to sleep for in milliseconds.
		/// @param player The player to set for.
		/// @param msecs Milliseconds to sleep for.
		void SetMSecsToSleep(short player, int msecs) { m_MSecsToSleep[player] = msecs; };

		/// Gets the ping time of the specified player.
		/// @param player The player to get for.
		/// @return The ping time of the player.
		unsigned short GetPing(short player) const { return m_Ping[player]; }

		/// Gets whether server puts threads to sleep if it didn't receive anything for 10 seconds to reduce CPU load.
		/// @return Whether threads will be put to sleep when server isn't receiving any data or not.
		bool GetServerSleepWhenIdle() const { return m_SleepWhenIdle; }

		/// Gets whether the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.
		/// @return Whether threads will be put to sleep if server completed frame faster than it normally should or not.
		bool GetServerSimSleepWhenIdle() const { return m_SimSleepWhenIdle; }
#pragma endregion

#pragma region Concrete Methods
		/// Start server, open ports etc.
		void Start();

		/// Updates the state of this NetworkServer. Supposed to be done every frame before drawing.
		/// @param processInput Whether to process packets of player input data or not.
		void Update(bool processInput = false);
#pragma endregion

#pragma region Network Scene Handling
		///
		/// @param isLocked
		void LockScene(bool isLocked);

		///
		void ResetScene();

		///
		/// @param terrainChange
		void RegisterTerrainChange(NetworkTerrainChange terrainChange);
#pragma endregion

	protected:
		///
		struct ClientConnection {
			bool IsActive; //!<
			RakNet::SystemAddress ClientId; //!<
			RakNet::SystemAddress InternalId; //!<
			int ResX; //!<
			int ResY; //!<
			std::thread* SendThread; //!<
			std::string PlayerName; //!<
		};

		bool m_IsInServerMode = false; //!<

		bool m_SleepWhenIdle; //!< If true puts thread to sleep if it didn't receive anything for 10 seconds to avoid melting the CPU at 100% even if there are no connections.
		bool m_SimSleepWhenIdle; //!< If true the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.

		int m_ThreadExitReason[c_MaxClients]; //!<

		long m_MSecsSinceLastUpdate[c_MaxClients]; //!<
		long m_MSecsToSleep[c_MaxClients]; //!<

		RakNet::RakPeerInterface* m_Server; //!<

		std::string m_ServerPort; //!<

		std::array<ClientConnection, c_MaxClients> m_ClientConnections; //!<

		bool m_UseNATService; //!< Whether a NAT service is used for punch-through.
		RakNet::NatPunchthroughClient m_NATPunchthroughClient; //!<
		RakNet::SystemAddress m_NATServiceServerID; //!<
		bool m_NatServerConnected; //!<

		BITMAP* m_BackBuffer8[c_MaxClients]; //!< Buffers to store client screens before compression.
		BITMAP* m_BackBufferGUI8[c_MaxClients]; //!< Buffers to store client GUI screens before compression.

		void* m_LZ4CompressionState[c_MaxClients]; //!<
		void* m_LZ4FastCompressionState[c_MaxClients]; //!<

		int m_MouseState1[c_MaxClients]; //!<
		int m_MouseState2[c_MaxClients]; //!<
		int m_MouseState3[c_MaxClients]; //!<

		int m_MouseEvent1[c_MaxClients]; //!<
		int m_MouseEvent2[c_MaxClients]; //!<
		int m_MouseEvent3[c_MaxClients]; //!<

		bool m_UseHighCompression; //!< Whether to use higher compression methods (default).
		bool m_UseFastCompression; //!< Whether to use faster compression methods and conserve CPU.
		bool m_UseDeltaCompression; //!< Whether to use delta compression methods and conserve bandwidth.
		int m_HighCompressionLevel; //!< Compression level. 10 is optimal, 12 is highest.

		/// Acceleration factor, higher values consume more bandwidth but less CPU.
		/// The larger the acceleration value, the faster the algorithm, but also lesser the compression. It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
		/// An acceleration value of "1" is the same as regular LZ4_compress_default(). Values <= 0 will be replaced by ACCELERATION_DEFAULT(currently == 1, see lz4 documentation).
		int m_FastAccelerationFactor;

		bool m_UseInterlacing; //!< Use interlacing to heavily reduce bandwidth usage at the cost of visual degradation (unusable at 30 fps, but may be suitable at 60 fps).
		int m_EncodingFps; //!< Frame transmission rate. Higher value equals more CPU and bandwidth consumption.

		bool m_SendEven[c_MaxClients]; //!<

		bool m_SendSceneSetupData[c_MaxClients]; //!<
		bool m_SendSceneData[c_MaxClients]; //!<
		bool m_SceneAvailable[c_MaxClients]; //!<
		bool m_SendFrameData[c_MaxClients]; //!<

		std::mutex m_SceneLock[c_MaxClients]; //!<

		unsigned char m_PixelLineBuffer[c_MaxClients][c_MaxPixelLineBufferSize]; //!< Buffer to store currently transferred pixel data line.
		unsigned char m_PixelLineBufferDelta[c_MaxClients][c_MaxPixelLineBufferSize]; //!< Buffer to store currently transferred pixel data line.
		unsigned char m_CompressedLineBuffer[c_MaxClients][c_MaxPixelLineBufferSize]; //!< Buffer to store compressed pixel data line.

		unsigned char* m_PixelLineBuffersPrev[c_MaxClients]; //!<
		unsigned char* m_PixelLineBuffersGUIPrev[c_MaxClients]; //!<

		std::queue<NetworkTerrainChange> m_PendingTerrainChanges[c_MaxClients]; //!<
		std::queue<NetworkTerrainChange> m_CurrentTerrainChanges[c_MaxClients]; //!<

		std::mutex m_Mutex[c_MaxClients]; //!<

		std::vector<MsgInput> m_InputMessages[c_MaxClients]; //!< Message Queue for received input events to be processed this frame.

		unsigned char m_SceneID; //!<

		std::array<bool, c_MaxClients> m_EndActivityVotes; //!< Votes from each player required to return to the Multiplayer Lobby.
		std::array<bool, c_MaxClients> m_RestartActivityVotes; //!< Votes from each player required to restart the current activity.

		long long m_LatestRestartTime; //!< The time, in ticks, that the last activity restart took place on the server.

		int m_FrameNumbers[c_MaxClients]; //!<

		unsigned short m_Ping[c_MaxClients]; //!<
		std::array<std::unique_ptr<Timer>, c_MaxClients> m_PingTimer; //!<

		std::unique_ptr<Timer> m_LastPackedReceived; //!<

		/// Transmit frames as blocks instead of lines. Provides better compression at the cost of higher CPU usage.
		/// Though the compression is quite high it is recommended that Width * Height are less than MTU size or about 1500 bytes or packets may be fragmented by network hardware or dropped completely.
		bool m_TransmitAsBoxes;
		int m_BoxWidth; //!< Width of the transmitted CPU block. Different values may improve bandwidth usage.
		int m_BoxHeight; //!< Height of the transmitted CPU block. Different values may improve bandwidth usage.

		int m_EmptyBlocks[MAX_STAT_RECORDS]; //!<
		int m_FullBlocks[MAX_STAT_RECORDS]; //!<
		int m_SendBufferBytes[MAX_STAT_RECORDS]; //!<
		int m_SendBufferMessages[MAX_STAT_RECORDS]; //!<
		int m_DelayedFrames[c_MaxClients]; //!<
		int m_MsecPerFrame[c_MaxClients]; //!<
		int m_MsecPerSendCall[c_MaxClients]; //!<

		long long m_LastFrameSentTime[c_MaxClients]; //!<
		long long m_LastStatResetTime[c_MaxClients]; //!<

		unsigned int m_FramesSent[MAX_STAT_RECORDS]; //!< Number of frames sent by the server to each client and total.
		unsigned int m_FramesSkipped[MAX_STAT_RECORDS]; //!< Numbers of frames skipped by the server for each client and total.

		unsigned long m_DataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_DataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_FullBlocksSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long m_EmptyBlocksSentCurrent[MAX_STAT_RECORDS][2];

		unsigned long m_FullBlocksDataSentCurrent[MAX_STAT_RECORDS][2];
		unsigned long m_EmptyBlocksDataSentCurrent[MAX_STAT_RECORDS][2];

		unsigned long m_DataUncompressedTotal[MAX_STAT_RECORDS]; //!<
		unsigned long m_DataUncompressedCurrent[MAX_STAT_RECORDS][2]; //!<

		unsigned long m_FrameDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_FrameDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_PostEffectDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_PostEffectDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_SoundDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_SoundDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_TerrainDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_TerrainDataSentTotal[MAX_STAT_RECORDS]; //!<

		unsigned long m_OtherDataSentCurrent[MAX_STAT_RECORDS][2]; //!<
		unsigned long m_OtherDataSentTotal[MAX_STAT_RECORDS]; //!<

	private:
#pragma region Thread Handling
		///
		/// @param server
		/// @param player
		static void BackgroundSendThreadFunction(NetworkServer* server, short player);

		///
		/// @param player
		/// @param reason
		void SetThreadExitReason(short player, int reason) { m_ThreadExitReason[player] = reason; };
#pragma endregion

#pragma region Network Event Handling
		///
		/// @param packet
		/// @return
		unsigned char GetPacketIdentifier(RakNet::Packet* packet) const;

		///
		/// @param packet
		void ReceiveNewIncomingConnection(RakNet::Packet* packet);

		///
		/// @param player
		void SendAcceptedMsg(short player);

		///
		/// @param packet
		void ReceiveDisconnection(RakNet::Packet* packet);

		///
		/// @param packet
		void ReceiveRegisterMsg(RakNet::Packet* packet);

		///
		/// @param addr
		void SendNATServerRegistrationMsg(RakNet::SystemAddress address);

		///
		/// @param packet
		void ReceiveInputMsg(RakNet::Packet* packet);

		///
		/// @param player
		/// @param msg
		void ProcessInputMsg(short player, MsgInput msg);

		///
		/// @param player
		void ClearInputMessages(short player);

		///
		/// @param player
		void SendSoundData(short player);

		///
		/// @param player
		void SendMusicData(short player);
#pragma endregion

#pragma region Network Scene Handling
		///
		/// @param player
		/// @return
		bool IsSceneAvailable(short player) const { return m_SceneAvailable[player]; }

		///
		/// @param player
		/// @return
		bool NeedToSendSceneSetupData(short player) const { return m_SendSceneSetupData[player]; }

		///
		/// @param player
		void SendSceneSetupData(short player);

		///
		/// @param packet
		void ReceiveSceneSetupDataAccepted(RakNet::Packet* packet);

		///
		/// @param player
		/// @return
		bool NeedToSendSceneData(short player) const { return m_SendSceneData[player]; }

		///
		/// @param player
		void SendSceneData(short player);

		///
		/// @param player
		void ClearTerrainChangeQueue(short player);

		///
		/// @param player
		/// @return
		bool NeedToProcessTerrainChanges(short player);

		///
		/// @param player
		void ProcessTerrainChanges(short player);

		///
		/// @param player
		/// @param terrainChange
		void SendTerrainChangeMsg(short player, NetworkTerrainChange terrainChange);

		///
		/// @param packet
		void ReceiveSceneAcceptedMsg(RakNet::Packet* packet);

		///
		/// @param player
		void SendSceneEndMsg(short player);
#pragma endregion

#pragma region Network Frame Handling and Drawing
		///
		/// @param player
		/// @param w
		/// @param h
		void CreateBackBuffer(short player, int w, int h);

		///
		/// @param player
		/// @param w
		/// @param h
		void ClearBackBuffer(int player, int w, int h);

		///
		/// @param player
		void DestroyBackBuffer(short player);

		///
		/// @param player
		void SendFrameSetupMsg(short player);

		///
		/// @param player
		/// @return
		bool SendFrameData(short player) const { return m_SendFrameData[player]; }

		///
		/// @param player
		void SendPostEffectData(short player);

		///
		/// @param player
		/// @return
		int SendFrame(short player);
#pragma endregion

#pragma region Network Stats Handling
		///
		/// @param player
		void UpdateStats(short player);

		///
		void DrawStatisticsData();
#pragma endregion

#pragma region Update Breakdown
		///
		void HandleNetworkPackets();
#pragma endregion

		/// Gets the Globally Unique Identifier of the server.
		/// @return The GUID of the server.
		RakNet::RakNetGUID GetServerGUID() const { return m_Server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS); }

		///
		/// @param rakPeer
		/// @param address
		/// @param port
		/// @return
		RakNet::SystemAddress ConnectBlocking(RakNet::RakPeerInterface* rakPeer, const char* address, unsigned short port);

		/// Clears all the member variables of this NetworkServer, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		NetworkServer(const NetworkServer& reference) = delete;
		NetworkServer& operator=(const NetworkServer& rhs) = delete;
	};
} // namespace RTE
