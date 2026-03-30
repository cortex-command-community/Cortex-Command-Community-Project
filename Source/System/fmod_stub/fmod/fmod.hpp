/**
 * FMOD Core API stub for Emscripten / WebAssembly builds.
 *
 * Provides REAL audio playback backed by SDL3's audio API (which uses the
 * Web Audio API on Emscripten).  Audio files are decoded from FLAC/OGG/WAV
 * via dr_flac and stb_vorbis into float32 PCM, then mixed in software and
 * pushed to an SDL3 audio stream.
 *
 * The public API surface matches the subset of FMOD used by AudioMan,
 * MusicMan, SoundContainer, and ContentFile — so no game code changes
 * are needed.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>

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
#define FMOD_CREATESAMPLE       0x00000100
#define FMOD_3D_LINEARROLLOFF   0x00400000
#define FMOD_NONBLOCKING        0x00010000
#define FMOD_TIMEUNIT_MS        0x00000001
#define FMOD_TIMEUNIT_PCM       0x00000002
#define FMOD_INIT_NORMAL                  0x00000000
#define FMOD_INIT_VOL0_BECOMES_VIRTUAL    0x00000200
#define FMOD_INIT_PROFILE_ENABLE          0x00010000
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
typedef void FMOD_CHANNELCONTROL;
typedef FMOD_RESULT (F_CALLBACK *FMOD_CHANNELCONTROL_CALLBACK)(FMOD_CHANNELCONTROL*, FMOD_CHANNELCONTROL_TYPE, FMOD_CHANNELCONTROL_CALLBACK_TYPE, void*, void*);

// ---------------------------------------------------------------------------
// Internal implementation types (opaque to game code)
// ---------------------------------------------------------------------------
struct SoundImpl {
    std::vector<float> pcmData;     // Interleaved float32 PCM samples
    uint32_t totalFrames = 0;       // Number of sample frames
    uint32_t channels = 0;          // 1 = mono, 2 = stereo
    uint32_t sampleRate = 44100;    // Original sample rate
    int loopCount = 0;              // -1 = infinite, 0 = no loop
    FMOD_MODE mode = 0;
};

struct ChannelGroupImpl;

struct ChannelImpl {
    SoundImpl* sound = nullptr;
    double playPosition = 0.0;       // Current frame position (double for pitch interpolation)
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;                // -1 left, 0 center, +1 right
    bool paused = false;
    bool playing = false;
    int loopCount = 0;               // Remaining loops
    void* userData = nullptr;
    FMOD_CHANNELCONTROL_CALLBACK callback = nullptr;
    ChannelGroupImpl* group = nullptr;
    int index = -1;                  // Stable index in the channel pool
};

struct ChannelGroupImpl {
    float volume = 1.0f;
    bool muted = false;
    bool paused = false;
    std::string name;
};

// ---------------------------------------------------------------------------
// FMOD namespace — real implementations backed by SDL3 audio
// ---------------------------------------------------------------------------
namespace FMOD {

    class Sound;
    class Channel;
    class ChannelGroup;
    class DSP;
    class System;

    // Global system pointer for access from Channel/Sound methods
    System* GetGlobalSystem();

    // ------------------------------------------------------------------
    // ChannelControl — non-virtual base to avoid WASM call_indirect
    // signature mismatches on WebKit/iOS. Methods are re-declared in
    // both Channel and ChannelGroup (no virtual dispatch).
    // ------------------------------------------------------------------
    class ChannelControl {
    public:
        FMOD_RESULT addDSP(int, DSP*)         { return FMOD_OK; }
        FMOD_RESULT removeDSP(DSP*)           { return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // Channel
    // ------------------------------------------------------------------
    class Channel : public ChannelControl {
    public:
        ChannelImpl impl;

        FMOD_RESULT setVolume(float v)        { impl.volume = v; return FMOD_OK; }
        FMOD_RESULT getVolume(float* v)       { if (v) *v = impl.volume; return FMOD_OK; }
        FMOD_RESULT setPaused(bool p)         { impl.paused = p; return FMOD_OK; }
        FMOD_RESULT setMute(bool)             { return FMOD_OK; }
        FMOD_RESULT getMute(bool* m)          { if (m) *m = false; return FMOD_OK; }
        FMOD_RESULT setPitch(float p)         { impl.pitch = std::max(0.01f, p); return FMOD_OK; }
        FMOD_RESULT isPlaying(bool* p)        { if (p) *p = impl.playing; return FMOD_OK; }
        FMOD_RESULT stop();
        FMOD_RESULT setCallback(FMOD_CHANNELCONTROL_CALLBACK cb) { impl.callback = cb; return FMOD_OK; }
        FMOD_RESULT setUserData(void* d)      { impl.userData = d; return FMOD_OK; }
        FMOD_RESULT getUserData(void** d)     { if (d) *d = impl.userData; return FMOD_OK; }

        // 3D audio — no-op for now, store values for future use
        FMOD_RESULT set3DAttributes(const FMOD_VECTOR*, const FMOD_VECTOR*) { return FMOD_OK; }
        FMOD_RESULT get3DAttributes(FMOD_VECTOR* p, FMOD_VECTOR* v)        { if (p) *p = {0,0,0}; if (v) *v = {0,0,0}; return FMOD_OK; }
        FMOD_RESULT set3DMinMaxDistance(float, float)                       { return FMOD_OK; }
        FMOD_RESULT get3DMinMaxDistance(float* mn, float* mx)               { if (mn) *mn = 1.0f; if (mx) *mx = 10000.0f; return FMOD_OK; }
        FMOD_RESULT set3DLevel(float)                                       { return FMOD_OK; }
        FMOD_RESULT get3DLevel(float* l)                                    { if (l) *l = 1.0f; return FMOD_OK; }
        FMOD_RESULT getDSP(int, DSP** d); // Defined after DSP class
        FMOD_RESULT setPan(float p)                    { impl.pan = std::clamp(p, -1.0f, 1.0f); return FMOD_OK; }
        FMOD_RESULT setFrequency(float f)              { if (impl.sound && impl.sound->sampleRate > 0) impl.pitch = f / (float)impl.sound->sampleRate; return FMOD_OK; }
        FMOD_RESULT getFrequency(float* f)             { if (f) *f = impl.sound ? (float)impl.sound->sampleRate * impl.pitch : 44100.0f; return FMOD_OK; }
        FMOD_RESULT setLoopCount(int n)                { impl.loopCount = n; return FMOD_OK; }
        FMOD_RESULT getCurrentSound(Sound** s);
        FMOD_RESULT setChannelGroup(ChannelGroup* g);
        FMOD_RESULT addFadePoint(unsigned long long, float) { return FMOD_OK; }  // Fade: no-op
        FMOD_RESULT setFadePointRamp(unsigned long long, float) { return FMOD_OK; }
        FMOD_RESULT getPosition(unsigned int* p, FMOD_TIMEUNIT u);
        FMOD_RESULT getIndex(int* i)                   { if (i) *i = impl.index; return FMOD_OK; }
        FMOD_RESULT getDSPClock(unsigned long long* c, unsigned long long* p) { if (c) *c = 0; if (p) *p = 0; return FMOD_OK; }
        FMOD_RESULT setPriority(int)                                        { return FMOD_OK; }
        FMOD_RESULT getAudibility(float* a)                                 { if (a) *a = impl.playing ? 1.0f : 0.0f; return FMOD_OK; }
        FMOD_RESULT getMode(FMOD_MODE* m)                                   { if (m) *m = impl.sound ? impl.sound->mode : 0; return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // ChannelGroup
    // ------------------------------------------------------------------
    class ChannelGroup : public ChannelControl {
    public:
        ChannelGroupImpl impl;

        FMOD_RESULT setVolume(float v)        { impl.volume = v; return FMOD_OK; }
        FMOD_RESULT getVolume(float* v)       { if (v) *v = impl.volume; return FMOD_OK; }
        FMOD_RESULT setPaused(bool p)         { impl.paused = p; return FMOD_OK; }
        FMOD_RESULT setMute(bool m)           { impl.muted = m; return FMOD_OK; }
        FMOD_RESULT getMute(bool* m)          { if (m) *m = impl.muted; return FMOD_OK; }
        FMOD_RESULT setPitch(float)           { return FMOD_OK; }
        FMOD_RESULT isPlaying(bool* p)        { if (p) *p = false; return FMOD_OK; }
        FMOD_RESULT stop()                    { return FMOD_OK; }
        FMOD_RESULT setCallback(FMOD_CHANNELCONTROL_CALLBACK) { return FMOD_OK; }
        FMOD_RESULT setUserData(void*)        { return FMOD_OK; }
        FMOD_RESULT getUserData(void** d)     { if (d) *d = nullptr; return FMOD_OK; }

        FMOD_RESULT getNumChannels(int* n)             { if (n) *n = 0; return FMOD_OK; }
        FMOD_RESULT getChannel(int, Channel** c)       { if (c) *c = nullptr; return FMOD_OK; }
        FMOD_RESULT addGroup(ChannelGroup*, bool = true, void* = nullptr) { return FMOD_OK; }
        FMOD_RESULT getDSP(int, DSP**); // Defined after DSP class
    };

    // ------------------------------------------------------------------
    // Sound
    // ------------------------------------------------------------------
    class Sound {
    public:
        SoundImpl impl;

        FMOD_RESULT getLength(unsigned int* l, FMOD_TIMEUNIT u) {
            if (!l) return FMOD_OK;
            if (u & FMOD_TIMEUNIT_MS)
                *l = impl.sampleRate > 0 ? (unsigned int)((uint64_t)impl.totalFrames * 1000 / impl.sampleRate) : 0;
            else
                *l = impl.totalFrames;
            return FMOD_OK;
        }
        FMOD_RESULT release()                          { return FMOD_OK; }
        FMOD_RESULT setMode(FMOD_MODE m)               { impl.mode = m; return FMOD_OK; }
        FMOD_RESULT set3DMinMaxDistance(float, float)   { return FMOD_OK; }
        FMOD_RESULT getDefaults(float* f, int*)         { if (f) *f = (float)impl.sampleRate; return FMOD_OK; }
        FMOD_RESULT setDefaults(float f, int)           { impl.sampleRate = (uint32_t)f; return FMOD_OK; }
        FMOD_RESULT getNumSubSounds(int* n)             { if (n) *n = 0; return FMOD_OK; }
        FMOD_RESULT getSubSound(int, Sound** s)         { if (s) *s = nullptr; return FMOD_OK; }
        FMOD_RESULT getUserData(void** d)               { if (d) *d = nullptr; return FMOD_OK; }
        FMOD_RESULT setUserData(void*)                  { return FMOD_OK; }
        FMOD_RESULT setLoopCount(int n)                 { impl.loopCount = n; return FMOD_OK; }
        FMOD_RESULT getLoopCount(int* n)                { if (n) *n = impl.loopCount; return FMOD_OK; }
        FMOD_RESULT setLoopPoints(unsigned int, FMOD_TIMEUNIT, unsigned int, FMOD_TIMEUNIT) { return FMOD_OK; }
    };

    // ------------------------------------------------------------------
    // DSP — no-op (effects not implemented)
    // ------------------------------------------------------------------
    class DSP {
    public:
        FMOD_RESULT release()                              { return FMOD_OK; }
        FMOD_RESULT setParameterFloat(int, float)          { return FMOD_OK; }
        FMOD_RESULT getParameterFloat(int, float* v, char*, int) { if (v) *v = 0.0f; return FMOD_OK; }
        FMOD_RESULT setBypass(bool)                        { return FMOD_OK; }
    };

    // Deferred inline definitions (need complete types)
    inline FMOD_RESULT Channel::getDSP(int, DSP** d) { static DSP stub; if (d) *d = &stub; return FMOD_OK; }
    inline FMOD_RESULT ChannelGroup::getDSP(int, DSP** d) { static DSP stub; if (d) *d = &stub; return FMOD_OK; }

    // ------------------------------------------------------------------
    // System — the main audio context backed by SDL3
    // ------------------------------------------------------------------
    class System {
    public:
        static constexpr int MAX_CHANNELS = 128;
        static constexpr int OUTPUT_RATE = 48000;
        static constexpr int MIX_FRAMES = 1024;   // Frames per mix pass

        // Factory
        static FMOD_RESULT create(System** sys);

        FMOD_RESULT init(int maxChannels, FMOD_INITFLAGS flags, void* extraDriverData);
        FMOD_RESULT close();
        FMOD_RESULT release()                                             { return close(); }
        FMOD_RESULT update();
        FMOD_RESULT getAdvancedSettings(FMOD_ADVANCEDSETTINGS*)           { return FMOD_OK; }
        FMOD_RESULT setAdvancedSettings(FMOD_ADVANCEDSETTINGS*)           { return FMOD_OK; }
        FMOD_RESULT setOutput(FMOD_OUTPUTTYPE)                            { return FMOD_OK; }
        FMOD_RESULT setSpeakerMode(FMOD_SPEAKERMODE)                      { return FMOD_OK; }
        FMOD_RESULT getSpeakerMode(FMOD_SPEAKERMODE* m)                   { if (m) *m = FMOD_SPEAKERMODE_STEREO; return FMOD_OK; }
        FMOD_RESULT getVersion(unsigned int* v)                           { if (v) *v = 0x00020213; return FMOD_OK; }
        FMOD_RESULT getSoftwareFormat(int* rate, void*, int*) { if (rate) *rate = OUTPUT_RATE; return FMOD_OK; }

        FMOD_RESULT createStream(const char* path, FMOD_MODE mode, void*, Sound** s) {
            return createSound(path, mode, nullptr, s);
        }
        FMOD_RESULT createSound(const char* path, FMOD_MODE mode, void* exinfo, Sound** outSound);
        FMOD_RESULT createChannelGroup(const char* name, ChannelGroup** g);
        FMOD_RESULT createDSPByType(int, DSP** d) {
            static DSP stub; if (d) *d = &stub; return FMOD_OK;
        }

        FMOD_RESULT getMasterChannelGroup(ChannelGroup** g);
        FMOD_RESULT playSound(Sound* sound, ChannelGroup* group, bool paused, Channel** outChannel);
        FMOD_RESULT set3DNumListeners(int)                     { return FMOD_OK; }
        FMOD_RESULT set3DListenerAttributes(int, const FMOD_VECTOR*, const FMOD_VECTOR*, const FMOD_VECTOR*, const FMOD_VECTOR*) { return FMOD_OK; }
        FMOD_RESULT set3DSettings(float, float, float)         { return FMOD_OK; }
        FMOD_RESULT setSoftwareChannels(int)                   { return FMOD_OK; }
        FMOD_RESULT getSoftwareChannels(int* n)                { if (n) *n = MAX_CHANNELS; return FMOD_OK; }
        FMOD_RESULT getChannelsPlaying(int* v, int* r);
        FMOD_RESULT setDSPBufferSize(unsigned int, int)        { return FMOD_OK; }
        FMOD_RESULT getChannel(int idx, Channel** c) {
            Channel* ch = getChannelByIndex(idx);
            if (c) *c = ch;
            return ch ? FMOD_OK : FMOD_ERR_INVALID_HANDLE;
        }

        // Internal: get channel by index
        Channel* getChannelByIndex(int idx);

    // Made public so Channel::stop() can access it from fmod_audio_impl.cpp
    public:
        bool m_initialized = false;
        uint32_t m_sdlDeviceId = 0;
        void* m_sdlStream = nullptr;  // SDL_AudioStream*

        ChannelGroup m_masterGroup;
        std::vector<std::unique_ptr<ChannelGroup>> m_channelGroups;
        std::vector<std::unique_ptr<Channel>> m_channels;
        std::vector<std::unique_ptr<Sound>> m_sounds;

        std::vector<float> m_mixBuffer;  // Stereo interleaved mix output

        // Fire end callback for a channel
        void fireEndCallback(Channel* ch);
    };

} // namespace FMOD

// ---------------------------------------------------------------------------
// FMOD free functions
// ---------------------------------------------------------------------------
inline FMOD_RESULT FMOD_System_Create(FMOD::System** system, unsigned int) {
    return FMOD::System::create(system);
}
namespace FMOD {
    inline FMOD_RESULT System_Create(System** sys, unsigned int = 0x00020213) {
        return System::create(sys);
    }
}

// ---------------------------------------------------------------------------
// FMOD_CREATESOUNDEXINFO and callback types
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
