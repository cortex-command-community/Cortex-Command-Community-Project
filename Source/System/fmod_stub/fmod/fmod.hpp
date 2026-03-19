/**
 * FMOD Core API stub for Emscripten / WebAssembly builds.
 *
 * Provides empty/no-op implementations of the FMOD types and methods used
 * by AudioMan so the codebase compiles without the proprietary FMOD SDK.
 *
 * AudioMan itself is compiled out on Emscripten and replaced by
 * AudioManWeb.cpp which uses the Web Audio API via OpenAL Soft or SDL_mixer.
 *
 * NOTE: Do NOT ship this header on native builds.  The real FMOD SDK must be
 * present in external/include/fmod/ for Windows/Linux/macOS targets.
 */

#pragma once

#include <cstdint>
#include <cstring>

// ---------------------------------------------------------------------------
// Additional FMOD structs and enums used in AudioMan.cpp
// ---------------------------------------------------------------------------

struct FMOD_ADVANCEDSETTINGS {
    int cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
    int maxMPEGCodecs = 0;
    int maxADPCMCodecs = 0;
    int maxXMACodecs = 0;
    int maxVorbisCodecs = 0;
    int maxAT9Codecs = 0;
    int maxFADPCMCodecs = 0;
    int maxPCMCodecs = 0;
    int ASIONumChannels = 0;
    int* ASIOChannelList = nullptr;
    int* ASIOSpeakerList = nullptr;
    float vol0virtualvol = 0.0f;
    unsigned int defaultDecodeBufferSize = 0;
    unsigned short profilePort = 0;
    unsigned int geometryMaxFadeTime = 0;
    float distanceFilterCenterFreq = 0.0f;
    int reverb3Dinstance = 0;
    int DSPBufferPoolSize = 0;
    unsigned int stackSizeStream = 0;
    unsigned int stackSizeNonBlocking = 0;
    unsigned int stackSizeMixer = 0;
    int resamplerMethod = 0;
    unsigned int commandQueueSize = 0;
    unsigned int randomSeed = 0;
};

enum FMOD_DSP_TYPE {
    FMOD_DSP_TYPE_MULTIBAND_EQ = 0,
    FMOD_DSP_TYPE_LIMITER = 1,
    FMOD_DSP_TYPE_COMPRESSOR = 2,
};

enum FMOD_OUTPUTTYPE {
    FMOD_OUTPUTTYPE_AUTODETECT = 0,
    FMOD_OUTPUTTYPE_NOSOUND    = 8,
};

enum FMOD_SPEAKERMODE {
    FMOD_SPEAKERMODE_DEFAULT = 0,
    FMOD_SPEAKERMODE_STEREO  = 3,
};

#define FMOD_CHANNEL_FREE (-1)
#define FMOD_ERR_HEADER_MISMATCH (-100)

// ---------------------------------------------------------------------------
// Additional FMOD mode flags used in AudioMan/SoundContainer
// ---------------------------------------------------------------------------
#define FMOD_LOOP_OFF           0x00000001
#define FMOD_3D_INVERSEROLLOFF  0x00100000
#define FMOD_3D_CUSTOMROLLOFF   0x04000000

// ---------------------------------------------------------------------------
// FMOD result codes
// ---------------------------------------------------------------------------
typedef int FMOD_RESULT;
#define FMOD_OK 0
#define FMOD_ERR_CHANNEL_ALLOC   (-1)
#define FMOD_ERR_INVALID_HANDLE  (-2)

// ---------------------------------------------------------------------------
// FMOD types
// ---------------------------------------------------------------------------
typedef unsigned int   FMOD_MODE;
typedef unsigned int   FMOD_TIMEUNIT;
typedef unsigned int   FMOD_INITFLAGS;
typedef unsigned int   FMOD_CHANNELMASK;
typedef float          FMOD_FLOAT;
typedef int            FMOD_BOOL;

#define FMOD_DEFAULT            0x00000000
#define FMOD_LOOP_NORMAL        0x00000002
#define FMOD_2D                 0x00000008
#define FMOD_3D                 0x00000010
#define FMOD_3D_LINEARROLLOFF   0x00400000
#define FMOD_NONBLOCKING        0x00010000
#define FMOD_TIMEUNIT_MS        0x00000001
#define FMOD_TIMEUNIT_PCM       0x00000002
#define FMOD_INIT_NORMAL        0x00000000
#define FMOD_CHANNELMASK_ALL    0xFFFFFFFF

struct FMOD_VECTOR { float x, y, z; };
struct FMOD_3D_ATTRIBUTES { FMOD_VECTOR pos, vel, forward, up; };

#define F_CALLBACK
typedef FMOD_RESULT (F_CALLBACK *FMOD_CHANNEL_CALLBACK)(void*, int, int, void*, void*);

// ---------------------------------------------------------------------------
// Forward declarations so AudioMan.h can include fmod.hpp without errors
// ---------------------------------------------------------------------------
enum FMOD_CHANNELCONTROL_TYPE { FMOD_CHANNELCONTROL_CHANNEL = 0, FMOD_CHANNELCONTROL_CHANNELGROUP = 1 };
enum FMOD_CHANNELCONTROL_CALLBACK_TYPE { FMOD_CHANNELCONTROL_CALLBACK_END = 0 };
// FMOD_CHANNELCONTROL is the opaque base type for Channel/ChannelGroup callbacks
typedef void FMOD_CHANNELCONTROL;
typedef FMOD_RESULT (F_CALLBACK *FMOD_CHANNELCONTROL_CALLBACK)(FMOD_CHANNELCONTROL*, FMOD_CHANNELCONTROL_TYPE, FMOD_CHANNELCONTROL_CALLBACK_TYPE, void*, void*);

// ---------------------------------------------------------------------------
// FMOD namespace stubs
// ---------------------------------------------------------------------------
namespace FMOD {

    class Sound;
    class Channel;
    class ChannelGroup;
    class DSP;

    // ------------------------------------------------------------------
    // ChannelControl — shared interface for Channel and ChannelGroup
    // ------------------------------------------------------------------
    class ChannelControl {
    public:
        FMOD_RESULT setVolume(float)          { return FMOD_OK; }
        FMOD_RESULT getVolume(float* v)       { if (v) *v = 1.0f; return FMOD_OK; }
        FMOD_RESULT setPaused(bool)           { return FMOD_OK; }
        FMOD_RESULT setMute(bool)             { return FMOD_OK; }
        FMOD_RESULT getMute(bool* m)          { if (m) *m = false; return FMOD_OK; }
        FMOD_RESULT setPitch(float)           { return FMOD_OK; }
        FMOD_RESULT isPlaying(bool* p)        { if (p) *p = false; return FMOD_OK; }
        FMOD_RESULT stop()                    { return FMOD_OK; }
        FMOD_RESULT addDSP(int, DSP*)         { return FMOD_OK; }
        FMOD_RESULT removeDSP(DSP*)           { return FMOD_OK; }
        FMOD_RESULT setCallback(FMOD_CHANNELCONTROL_CALLBACK) { return FMOD_OK; }
        FMOD_RESULT setUserData(void*)        { return FMOD_OK; }
        FMOD_RESULT getUserData(void** d)     { if (d) *d = nullptr; return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // Channel
    // ------------------------------------------------------------------
    class Channel : public ChannelControl {
    public:
        FMOD_RESULT set3DAttributes(const FMOD_VECTOR*, const FMOD_VECTOR*) { return FMOD_OK; }
        FMOD_RESULT get3DAttributes(FMOD_VECTOR*, FMOD_VECTOR*)             { return FMOD_OK; }
        FMOD_RESULT set3DMinMaxDistance(float, float)                       { return FMOD_OK; }
        FMOD_RESULT setPan(float)                                           { return FMOD_OK; }
        FMOD_RESULT setFrequency(float)                                     { return FMOD_OK; }
        FMOD_RESULT getFrequency(float* f)    { if (f) *f = 44100.0f; return FMOD_OK; }
        FMOD_RESULT setLoopCount(int)                                       { return FMOD_OK; }
        FMOD_RESULT getCurrentSound(Sound** s){ if (s) *s = nullptr; return FMOD_OK; }
        FMOD_RESULT setChannelGroup(ChannelGroup*)                          { return FMOD_OK; }
        FMOD_RESULT addFadePoint(unsigned long long, float)                 { return FMOD_OK; }
        FMOD_RESULT setFadePointRamp(unsigned long long, float)             { return FMOD_OK; }
        FMOD_RESULT getPosition(unsigned int* p, FMOD_TIMEUNIT)             { if (p) *p = 0; return FMOD_OK; }
        FMOD_RESULT getIndex(int* i)          { if (i) *i = 0; return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // ChannelGroup
    // ------------------------------------------------------------------
    class ChannelGroup : public ChannelControl {
    public:
        FMOD_RESULT getNumChannels(int* n)    { if (n) *n = 0; return FMOD_OK; }
        FMOD_RESULT getChannel(int, Channel** c) { if (c) *c = nullptr; return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // Sound
    // ------------------------------------------------------------------
    class Sound {
    public:
        FMOD_RESULT getLength(unsigned int* l, FMOD_TIMEUNIT) { if (l) *l = 0; return FMOD_OK; }
        FMOD_RESULT release()                 { return FMOD_OK; }
        FMOD_RESULT setMode(FMOD_MODE)        { return FMOD_OK; }
        FMOD_RESULT set3DMinMaxDistance(float, float) { return FMOD_OK; }
        FMOD_RESULT getDefaults(float* f, int*) { if (f) *f = 44100.0f; return FMOD_OK; }
        FMOD_RESULT setDefaults(float, int)   { return FMOD_OK; }
        FMOD_RESULT getNumSubSounds(int* n)   { if (n) *n = 0; return FMOD_OK; }
        FMOD_RESULT getSubSound(int, Sound** s){ if (s) *s = nullptr; return FMOD_OK; }
        FMOD_RESULT getUserData(void** d)     { if (d) *d = nullptr; return FMOD_OK; }
        FMOD_RESULT setUserData(void*)        { return FMOD_OK; }
        FMOD_RESULT setLoopCount(int)         { return FMOD_OK; }
        FMOD_RESULT getLoopCount(int* n)      { if (n) *n = 0; return FMOD_OK; }
        FMOD_RESULT setLoopPoints(unsigned int, FMOD_TIMEUNIT, unsigned int, FMOD_TIMEUNIT) { return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // DSP
    // ------------------------------------------------------------------
    class DSP {
    public:
        FMOD_RESULT release()                              { return FMOD_OK; }
        FMOD_RESULT setParameterFloat(int, float)          { return FMOD_OK; }
        FMOD_RESULT getParameterFloat(int, float* v, char*, int) { if (v) *v = 0.0f; return FMOD_OK; }
        FMOD_RESULT setBypass(bool)                        { return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // System — the main FMOD context
    // ------------------------------------------------------------------
    class System {
    public:
        // Factory — returns a stub System pointer (allocated once, never freed)
        static FMOD_RESULT create(System** sys) {
            static System s_instance;
            if (sys) *sys = &s_instance;
            return FMOD_OK;
        }

        FMOD_RESULT init(int, FMOD_INITFLAGS, void*)                       { return FMOD_OK; }
        FMOD_RESULT close()                                               { return FMOD_OK; }
        FMOD_RESULT release()                                             { return FMOD_OK; }
        FMOD_RESULT update()                                              { return FMOD_OK; }
        FMOD_RESULT getAdvancedSettings(FMOD_ADVANCEDSETTINGS*)           { return FMOD_OK; }
        FMOD_RESULT setAdvancedSettings(FMOD_ADVANCEDSETTINGS*)           { return FMOD_OK; }
        FMOD_RESULT setOutput(FMOD_OUTPUTTYPE)                            { return FMOD_OK; }
        FMOD_RESULT setSpeakerMode(FMOD_SPEAKERMODE)                      { return FMOD_OK; }
        FMOD_RESULT getSpeakerMode(FMOD_SPEAKERMODE* m)                   { if (m) *m = FMOD_SPEAKERMODE_STEREO; return FMOD_OK; }
        FMOD_RESULT getVersion(unsigned int* v)                           { if (v) *v = 0x00020213; return FMOD_OK; }

        FMOD_RESULT createStream(const char*, FMOD_MODE, void*, Sound** s) {
            static Sound stub; if (s) *s = &stub; return FMOD_OK;
        }
        FMOD_RESULT createSound(const char*, FMOD_MODE, void*, Sound** s) {
            static Sound stub; if (s) *s = &stub; return FMOD_OK;
        }
        FMOD_RESULT createChannelGroup(const char*, ChannelGroup** g) {
            static ChannelGroup stub; if (g) *g = &stub; return FMOD_OK;
        }
        FMOD_RESULT createDSPByType(int, DSP** d) {
            static DSP stub; if (d) *d = &stub; return FMOD_OK;
        }

        FMOD_RESULT getMasterChannelGroup(ChannelGroup** g) {
            static ChannelGroup stub; if (g) *g = &stub; return FMOD_OK;
        }
        FMOD_RESULT playSound(Sound*, ChannelGroup*, bool, Channel** c) {
            static Channel stub; if (c) *c = &stub; return FMOD_OK;
        }
        FMOD_RESULT set3DNumListeners(int)                     { return FMOD_OK; }
        FMOD_RESULT set3DListenerAttributes(int, const FMOD_VECTOR*, const FMOD_VECTOR*, const FMOD_VECTOR*, const FMOD_VECTOR*) { return FMOD_OK; }
        FMOD_RESULT set3DSettings(float, float, float)         { return FMOD_OK; }
        FMOD_RESULT setSoftwareChannels(int)                   { return FMOD_OK; }
        FMOD_RESULT getSoftwareChannels(int* n)                { if (n) *n = 128; return FMOD_OK; }
        FMOD_RESULT getChannelsPlaying(int* v, int* r)         { if (v) *v = 0; if (r) *r = 0; return FMOD_OK; }
        FMOD_RESULT setDSPBufferSize(unsigned int, int)        { return FMOD_OK; }
    };

} // namespace FMOD

// ---------------------------------------------------------------------------
// FMOD free functions (C-style API used by AudioMan)
// ---------------------------------------------------------------------------
inline FMOD_RESULT FMOD_System_Create(FMOD::System** system, unsigned int) {
    return FMOD::System::create(system);
}
// AudioMan calls FMOD::System_Create (C++ namespace version)
namespace FMOD {
    inline FMOD_RESULT System_Create(System** sys, unsigned int = 0x00020213) {
        return System::create(sys);
    }
}

// ---------------------------------------------------------------------------
// FMOD_RESULT constant used as FMOD_VECTOR type in some callbacks
// ---------------------------------------------------------------------------
typedef void* FMOD_SOUND_PCMREAD_CALLBACK;
typedef void* FMOD_SOUND_PCMSETPOS_CALLBACK;
typedef void* FMOD_SOUND_NONBLOCK_CALLBACK;

struct FMOD_CREATESOUNDEXINFO {
    int             cbsize              = sizeof(FMOD_CREATESOUNDEXINFO);
    unsigned int    length              = 0;
    unsigned int    fileoffset          = 0;
    int             numchannels         = 0;
    int             defaultfrequency    = 0;
    int             format              = 0;
    unsigned int    decodebuffersize    = 0;
    int             initialsubsound     = 0;
    int             numsubsounds        = 0;
    int*            inclusionlist       = nullptr;
    int             inclusionlistnum    = 0;
    FMOD_SOUND_PCMREAD_CALLBACK   pcmreadcallback   = nullptr;
    FMOD_SOUND_PCMSETPOS_CALLBACK pcmsetposcallback = nullptr;
    FMOD_SOUND_NONBLOCK_CALLBACK  nonblockcallback  = nullptr;
    const char*     dlsname             = nullptr;
    const char*     encryptionkey       = nullptr;
    int             maxpolyphony        = 0;
    void*           userdata            = nullptr;
    int             suggestedsoundtype  = 0;
    void*           fileuseropen        = nullptr;
    void*           fileuserclose       = nullptr;
    void*           fileuserread        = nullptr;
    void*           fileuserseek        = nullptr;
    void*           fileuserasyncread   = nullptr;
    void*           fileuserasynccancel = nullptr;
    void*           speakermap          = nullptr;
    void*           initialsoundgroup   = nullptr;
    unsigned int    initialseekposition = 0;
    FMOD_TIMEUNIT   initialseekpostype  = FMOD_TIMEUNIT_MS;
    int             ignoresetfilesystem = 0;
    unsigned int    audioqueuepolicy    = 0;
    unsigned int    minmidigranularity  = 0;
    int             nonblockthreadid    = 0;
    void*           fsbguid             = nullptr;
};
