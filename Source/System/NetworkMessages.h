#pragma once

#include "Constants.h"
#include "MessageIdentifiers.h"

namespace RTE {

	/// Enumeration for the different types of network message IDs.
	enum CustomMessageIDTypes {
		ID_CUSTOM_START = ID_USER_PACKET_ENUM,
		ID_NAT_SERVER_REGISTER_SERVER,
		ID_NAT_SERVER_REGISTER_ACCEPTED,
		ID_NAT_SERVER_GET_SERVER_GUID,
		ID_NAT_SERVER_GUID,
		ID_NAT_SERVER_NO_GUID,
		ID_CLT_REGISTER,
		ID_CLT_INPUT,
		ID_SRV_ACCEPTED,
		ID_SRV_FRAME_SETUP,
		ID_SRV_FRAME_LINE,
		ID_SRV_FRAME_BOX_MO,
		ID_SRV_FRAME_BOX_UI,
		ID_SRV_FRAME_BOX_MO_DELTA,
		ID_SRV_FRAME_BOX_UI_DELTA,
		ID_SRV_SCENE_SETUP,
		ID_CLT_SCENE_SETUP_ACCEPTED,
		ID_SRV_SCENE,
		ID_SRV_SCENE_END,
		ID_CLT_SCENE_ACCEPTED,
		ID_CLT_DISCONNECT,
		ID_SRV_TERRAIN,
		ID_SRV_POST_EFFECTS,
		ID_SRV_SOUND_EVENTS,
		ID_SRV_MUSIC_EVENTS
	};

// Pack the structs so 1 byte members are exactly 1 byte in memory instead of being aligned by 4 bytes (padding) so the correct representation is sent over the network without empty bytes consumed by alignment.
#pragma pack(push, 1)

	// TODO: Figure out all these and add comments.

	///
	struct MsgRegisterServer {
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
		char ServerGuid[64];
	};

	///
	struct MsgRegisterServerAccepted {
		unsigned char Id;
	};

	///
	struct MsgGetServerRequest {
		unsigned char Id;

		char ServerName[64];
		char ServerPassword[64];
	};

	///
	struct MsgGetServerAnswer {
		unsigned char Id;

		char ServerGuid[64];
	};

	///
	struct MsgGetServerNoAnswer {
		unsigned char Id;
	};

	///
	struct MsgRegister {
		unsigned char Id;

		int ResolutionX;
		int ResolutionY;

		char Name[64];
	};

	///
	struct MsgFrameSetup {
		unsigned char Id;
		unsigned char FrameNumber;

		bool Interlaced;
		bool DeltaCompressed;

		unsigned short int BoxWidth;
		unsigned short int BoxHeight;

		short int TargetPosX;
		short int TargetPosY;

		float OffsetX[c_MaxLayersStoredForNetwork];
		float OffsetY[c_MaxLayersStoredForNetwork];
	};

	///
	struct MsgFrameLine {
		unsigned char Id;
		unsigned char FrameNumber;

		unsigned char Layer;
		unsigned short int LineNumber;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	///
	struct MsgFrameBox {
		unsigned char Id;

		unsigned char BoxX;
		unsigned char BoxY;

		unsigned short int DataSize;
	};

	///
	struct MsgDisconnect {
		unsigned char Id;
	};

	///
	struct MsgAccepted {
		unsigned char Id;
	};

	///
	struct LightweightSceneLayer {
		size_t BitmapHash;
		bool DrawTrans;
		float OffsetX;
		float OffsetY;
		float ScrollInfoX;
		float ScrollInfoY;
		float ScrollRatioX;
		float ScrollRatioY;
		float ScaleFactorX;
		float ScaleFactorY;
		float ScaleInverseX;
		float ScaleInverseY;
		float ScaledDimensionsX;
		float ScaledDimensionsY;

		bool WrapX;
		bool WrapY;

		unsigned char FillLeftColor;
		unsigned char FillRightColor;
		unsigned char FillUpColor;
		unsigned char FillDownColor;
	};

	///
	struct MsgSceneSetup {
		unsigned char Id;
		unsigned char SceneId;
		short int Width;
		short int Height;
		bool SceneWrapsX;

		short int BackgroundLayerCount;
		LightweightSceneLayer BackgroundLayers[c_MaxLayersStoredForNetwork];
	};

	///
	struct MsgSceneLine {
		unsigned char Id;
		unsigned char SceneId;
		unsigned short int X;
		unsigned short int Y;
		unsigned short int Width;
		unsigned char Layer;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	///
	struct MsgSceneEnd {
		unsigned char Id;
	};

	///
	struct MsgSceneAccepted {
		unsigned char Id;
	};

	///
	struct MsgTerrainChange {
		unsigned char Id;

		unsigned short int X;
		unsigned short int Y;
		unsigned short int W;
		unsigned short int H;
		bool Back;
		unsigned char Color;
		unsigned char SceneId;
		unsigned short int DataSize;
		unsigned short int UncompressedSize;
	};

	///
	struct PostEffectNetworkData {
		short int X;
		short int Y;
		size_t BitmapHash;
		short int Strength;
		float Angle;
	};

	///
	struct MsgPostEffects {
		unsigned char Id;
		unsigned char FrameNumber;
		int PostEffectsCount;
	};

	///
	struct MsgSoundEvents {
		unsigned char Id;
		unsigned char FrameNumber;
		int SoundEventsCount;
	};

	///
	struct MsgMusicEvents {
		unsigned char Id;
		unsigned char FrameNumber;
		int MusicEventsCount;
	};

	///
	struct MsgInput {
		unsigned char Id;

		int MouseX;
		int MouseY;
		bool padMouse1[MAX_MOUSE_BUTTONS]; //< Backwards compatibility with pre5-5.1
		bool padMouse2[MAX_MOUSE_BUTTONS];
		bool MouseButtonState[MAX_MOUSE_BUTTONS];
		bool ResetActivityVote;
		bool RestartActivityVote;

		int MouseWheelMoved;

		unsigned int padElement3;
		unsigned int padElement4;
		unsigned int InputElementState;
	};

// Disables the previously set pack pragma.
#pragma pack(pop)
} // namespace RTE
