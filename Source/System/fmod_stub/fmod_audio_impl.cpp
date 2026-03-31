/**
 * fmod_audio_impl.cpp
 *
 * Implementation of the FMOD stub's real audio methods backed by SDL3.
 * Decodes FLAC/OGG/WAV files, mixes channels in software, and pushes
 * audio to an SDL3 audio stream (Web Audio API on Emscripten).
 */

#include "fmod/fmod.hpp"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Audio decoders (headers only — implementations in fmod_audio_decoders.cpp)
#include "dr_flac.h"

// stb_vorbis: include as header-only (implementation in fmod_audio_decoders.cpp)
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

using namespace FMOD;

// ---------------------------------------------------------------------------
// Global system singleton
// ---------------------------------------------------------------------------
static System* s_globalSystem = nullptr;

System* FMOD::GetGlobalSystem() { return s_globalSystem; }

// ---------------------------------------------------------------------------
// System
// ---------------------------------------------------------------------------

FMOD_RESULT System::create(System** sys) {
    static System s_instance;
    s_globalSystem = &s_instance;
    if (sys) *sys = &s_instance;
    return FMOD_OK;
}

FMOD_RESULT System::init(int maxChannels, FMOD_INITFLAGS, void*) {
    if (m_initialized) return FMOD_OK;

    // Ensure SDL audio subsystem is initialized
    if (!(SDL_WasInit(SDL_INIT_AUDIO))) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
#ifdef __EMSCRIPTEN__
            EM_ASM({ console.log('[Audio] Failed to init SDL audio subsystem: ' + UTF8ToString($0)); }, SDL_GetError());
#endif
            return FMOD_OK;  // Don't crash — game runs without audio
        }
    }

    // Open SDL3 audio device
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    spec.freq = OUTPUT_RATE;

    m_sdlStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!m_sdlStream) {
#ifdef __EMSCRIPTEN__
        EM_ASM({ console.log('[Audio] Failed to open SDL audio device: ' + UTF8ToString($0)); }, SDL_GetError());
#endif
        return FMOD_OK;  // Don't fail — game can run without audio
    }

    m_sdlDeviceId = SDL_GetAudioStreamDevice((SDL_AudioStream*)m_sdlStream);

    // Resume the audio device (browsers require user gesture, SDL handles this)
    SDL_ResumeAudioStreamDevice((SDL_AudioStream*)m_sdlStream);

    // Allocate channel pool
    int numChannels = std::min(maxChannels, MAX_CHANNELS);
    if (numChannels <= 0) numChannels = MAX_CHANNELS;
    m_channels.resize(numChannels);
    for (int i = 0; i < numChannels; i++) {
        m_channels[i] = std::make_unique<Channel>();
        m_channels[i]->impl.index = i;
    }

    // Setup master group
    m_masterGroup.impl.name = "Master";
    m_masterGroup.impl.volume = 1.0f;

    // Allocate mix buffer (stereo)
    m_mixBuffer.resize(MIX_FRAMES * 2, 0.0f);

    m_initialized = true;

#ifdef __EMSCRIPTEN__
    EM_ASM({ console.log('[Audio] SDL audio device opened: ' + $0 + ' channels, ' + $1 + 'Hz'); },
           numChannels, OUTPUT_RATE);
#endif

    return FMOD_OK;
}

FMOD_RESULT System::close() {
    if (m_sdlStream) {
        SDL_DestroyAudioStream((SDL_AudioStream*)m_sdlStream);
        m_sdlStream = nullptr;
    }
    m_initialized = false;
    return FMOD_OK;
}

// ---------------------------------------------------------------------------
// Sound loading — decode FLAC/OGG/WAV from filesystem
// ---------------------------------------------------------------------------

static bool endsWith(const std::string& str, const char* suffix) {
    size_t suffixLen = strlen(suffix);
    if (str.size() < suffixLen) return false;
    return str.compare(str.size() - suffixLen, suffixLen, suffix) == 0;
}

// Browser-native audio decoding via Web Audio API decodeAudioData.
// Falls back to C++ decoders (stb_vorbis, dr_flac) if browser decode fails.
static bool browserDecodeAudio(const char* path, SoundImpl& impl) {
#ifdef __EMSCRIPTEN__
    FILE* fp = fopen(path, "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize <= 0) { fclose(fp); return false; }
    fseek(fp, 0, SEEK_SET);
    std::vector<uint8_t> fileData(fileSize);
    fread(fileData.data(), 1, fileSize, fp);
    fclose(fp);

    // Use EM_ASM to decode via browser's AudioContext.decodeAudioData.
    // This is synchronous from C++ perspective — we poll a flag via emscripten_sleep.
    struct DecodeResult {
        float* pcm = nullptr;
        int frames = 0;
        int channels = 0;
        int sampleRate = 0;
        int done = 0;  // 0=pending, 1=success, -1=error
    };
    static DecodeResult result;
    result = {};

    EM_ASM({
        var data = Module.HEAPU8.slice($0, $0 + $1);
        var resultPtr = $2;
        var ctx = window._ccAudioCtx;
        if (!ctx) {
            ctx = new (window.AudioContext || window.webkitAudioContext)();
            window._ccAudioCtx = ctx;
        }
        ctx.decodeAudioData(data.buffer).then(function(audioBuffer) {
            var frames = audioBuffer.length;
            var channels = audioBuffer.numberOfChannels;
            var pcmSize = frames * channels * 4;
            var pcmPtr = Module._malloc(pcmSize);
            // Interleave channel data into the WASM heap
            for (var c = 0; c < channels; c++) {
                var chanData = audioBuffer.getChannelData(c);
                for (var i = 0; i < frames; i++) {
                    Module.HEAPF32[(pcmPtr >> 2) + i * channels + c] = chanData[i];
                }
            }
            Module.HEAP32[(resultPtr >> 2)] = pcmPtr;
            Module.HEAP32[(resultPtr >> 2) + 1] = frames;
            Module.HEAP32[(resultPtr >> 2) + 2] = channels;
            Module.HEAP32[(resultPtr >> 2) + 3] = audioBuffer.sampleRate;
            Module.HEAP32[(resultPtr >> 2) + 4] = 1; // done=success
        }).catch(function(err) {
            console.warn('[Audio] Browser decode failed:', err.message);
            Module.HEAP32[(resultPtr >> 2) + 4] = -1; // done=error
        });
    }, fileData.data(), (int)fileSize, &result);

    // Wait for async decode to complete (Asyncify yields to browser)
    while (result.done == 0) {
        emscripten_sleep(1);
    }

    if (result.done == 1 && result.pcm && result.frames > 0) {
        impl.channels = result.channels;
        impl.sampleRate = result.sampleRate;
        impl.totalFrames = result.frames;
        impl.pcmData.assign(result.pcm, result.pcm + result.frames * result.channels);
        free(result.pcm);
        return true;
    }
    if (result.pcm) free(result.pcm);
#endif
    return false;
}

FMOD_RESULT System::createSound(const char* path, FMOD_MODE mode, void*, Sound** outSound) {
    if (!outSound) return FMOD_OK;

    auto sound = std::make_unique<Sound>();
    sound->impl.mode = mode;

    std::string filePath(path);
    bool decoded = false;

#ifdef __EMSCRIPTEN__
    // Web Audio path: just store the compressed file data. No decoding here.
    // playSound() will pass the bytes to JS for on-demand decoding + playback.
    // This uses almost no memory (stores compressed OGG, not decoded PCM).
    if (!decoded && (endsWith(filePath, ".ogg") || endsWith(filePath, ".flac") ||
                     endsWith(filePath, ".wav") || endsWith(filePath, ".mp3"))) {
        FILE* fp = fopen(path, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long fileSize = ftell(fp);
            if (fileSize > 0) {
                fseek(fp, 0, SEEK_SET);
                sound->impl.fileData.resize(fileSize);
                fread(sound->impl.fileData.data(), 1, fileSize, fp);
                sound->impl.filePath = path;
                sound->impl.useWebAudio = true;
                sound->impl.sampleRate = 44100;
                sound->impl.channels = 2;
                // Estimate duration from file size (~112kbps OGG)
                sound->impl.totalFrames = (uint32_t)((float)fileSize / 14000.0f * 44100.0f);
                decoded = true;
            }
            fclose(fp);
        }
    }
#endif

    // Fallback: C++ decoders for FLAC
    if (!decoded && endsWith(filePath, ".flac")) {
        unsigned int channels = 0, sampleRate = 0;
        drflac_uint64 totalFrames = 0;
        float* samples = drflac_open_file_and_read_pcm_frames_f32(path, &channels, &sampleRate, &totalFrames, nullptr);
        if (samples && totalFrames > 0) {
            sound->impl.channels = channels;
            sound->impl.sampleRate = sampleRate;
            sound->impl.totalFrames = (uint32_t)totalFrames;
            sound->impl.pcmData.assign(samples, samples + totalFrames * channels);
            drflac_free(samples, nullptr);
            decoded = true;
        }
    }

    // Fallback: C++ decoder for OGG
    if (!decoded && endsWith(filePath, ".ogg")) {
        int channels = 0, sampleRate = 0;
        short* samples = nullptr;
        int totalFrames = stb_vorbis_decode_filename(path, &channels, &sampleRate, &samples);
        if (samples && totalFrames > 0) {
            sound->impl.channels = channels;
            sound->impl.sampleRate = sampleRate;
            sound->impl.totalFrames = (uint32_t)totalFrames;
            sound->impl.pcmData.resize(totalFrames * channels);
            for (int i = 0; i < totalFrames * channels; i++) {
                sound->impl.pcmData[i] = samples[i] / 32768.0f;
            }
            free(samples);
            decoded = true;
        }
    }

    if (!decoded && endsWith(filePath, ".wav")) {
        // Decode WAV via SDL3
        SDL_AudioSpec spec;
        Uint8* buf = nullptr;
        Uint32 bufLen = 0;
        if (SDL_LoadWAV(path, &spec, &buf, &bufLen) && buf) {
            // Convert to float32 stereo
            SDL_AudioSpec dstSpec;
            dstSpec.format = SDL_AUDIO_F32;
            dstSpec.channels = spec.channels;
            dstSpec.freq = spec.freq;

            sound->impl.channels = spec.channels;
            sound->impl.sampleRate = spec.freq;
            int bytesPerFrame = spec.channels * sizeof(float);
            sound->impl.totalFrames = bufLen / (SDL_AUDIO_BYTESIZE(spec.format) * spec.channels);

            // If already float32, just copy
            if (spec.format == SDL_AUDIO_F32) {
                sound->impl.pcmData.resize(bufLen / sizeof(float));
                memcpy(sound->impl.pcmData.data(), buf, bufLen);
            } else {
                // Use SDL audio stream for conversion
                SDL_AudioStream* conv = SDL_CreateAudioStream(&spec, &dstSpec);
                if (conv) {
                    SDL_PutAudioStreamData(conv, buf, bufLen);
                    SDL_FlushAudioStream(conv);
                    int outBytes = SDL_GetAudioStreamAvailable(conv);
                    sound->impl.pcmData.resize(outBytes / sizeof(float));
                    SDL_GetAudioStreamData(conv, sound->impl.pcmData.data(), outBytes);
                    sound->impl.totalFrames = outBytes / bytesPerFrame;
                    SDL_DestroyAudioStream(conv);
                }
            }
            SDL_free(buf);
            decoded = true;
        }
    }

    if (!decoded) {
        // Return an empty sound — playback will be silent but won't crash
        *outSound = sound.release();
        m_sounds.emplace_back(*outSound);
        return FMOD_OK;
    }

    // Set loop mode from flags
    if (mode & FMOD_LOOP_NORMAL) {
        sound->impl.loopCount = -1;  // Infinite loop
    }

    *outSound = sound.release();
    m_sounds.emplace_back(*outSound);
    return FMOD_OK;
}

// ---------------------------------------------------------------------------
// Channel group management
// ---------------------------------------------------------------------------

FMOD_RESULT System::createChannelGroup(const char* name, ChannelGroup** g) {
    if (!g) return FMOD_OK;
    auto group = std::make_unique<ChannelGroup>();
    group->impl.name = name ? name : "";
    *g = group.get();
    m_channelGroups.push_back(std::move(group));
    return FMOD_OK;
}

FMOD_RESULT System::getMasterChannelGroup(ChannelGroup** g) {
    if (g) *g = &m_masterGroup;
    return FMOD_OK;
}

// ---------------------------------------------------------------------------
// Playback
// ---------------------------------------------------------------------------

FMOD_RESULT System::playSound(Sound* sound, ChannelGroup* group, bool paused, Channel** outChannel) {
    if (!sound || !m_initialized) {
        if (outChannel) *outChannel = nullptr;
        return FMOD_OK;
    }

    // Find a free channel
    Channel* ch = nullptr;
    for (auto& c : m_channels) {
        if (!c->impl.playing && !c->impl.paused) {
            ch = c.get();
            break;
        }
    }

    if (!ch) {
        // All channels busy — steal the oldest one
        if (!m_channels.empty()) {
            ch = m_channels[0].get();
            ch->impl.playing = false;
        }
    }

    if (!ch) {
        if (outChannel) *outChannel = nullptr;
        return FMOD_ERR_CHANNEL_ALLOC;
    }

    ch->impl.sound = &sound->impl;
    ch->impl.playPosition = 0.0;
    ch->impl.volume = 1.0f;
    ch->impl.pitch = 1.0f;
    ch->impl.pan = 0.0f;
    ch->impl.paused = paused;
    ch->impl.playing = true;
    ch->impl.loopCount = sound->impl.loopCount;
    ch->impl.callback = nullptr;
    ch->impl.userData = nullptr;
    ch->impl.group = group ? &group->impl : &m_masterGroup.impl;
    ch->impl.webAudioId = -1;

#ifdef __EMSCRIPTEN__
    // Web Audio path: pass compressed bytes to JS for native decode+play.
    // The browser decodes and plays without storing full PCM in WASM memory.
    if (sound->impl.useWebAudio && !sound->impl.fileData.empty() && !paused) {
        float groupVol = ch->impl.group ? ch->impl.group->volume : 1.0f;
        float vol = ch->impl.volume * groupVol * m_masterGroup.impl.volume;
        bool loop = (ch->impl.loopCount != 0);
        ch->impl.webAudioId = EM_ASM_INT({
            var data = Module.HEAPU8.slice($0, $0 + $1);
            var vol = $2;
            var loop = $3;
            var chanIdx = $4;
            if (!window._ccAudioCtx) {
                window._ccAudioCtx = new (window.AudioContext || window.webkitAudioContext)();
            }
            var ctx = window._ccAudioCtx;
            if (!window._ccAudioNodes) window._ccAudioNodes = {};

            ctx.decodeAudioData(data.buffer.slice(data.byteOffset, data.byteOffset + data.byteLength)).then(function(audioBuffer) {
                var source = ctx.createBufferSource();
                source.buffer = audioBuffer;
                source.loop = loop;

                var gainNode = ctx.createGain();
                gainNode.gain.value = vol;

                source.connect(gainNode);
                gainNode.connect(ctx.destination);
                source.start(0);

                window._ccAudioNodes[chanIdx] = { source: source, gain: gainNode };
                source.onended = function() {
                    delete window._ccAudioNodes[chanIdx];
                };
            }).catch(function(err) {
                // Silent fail — sound just won't play
            });
            return chanIdx;
        }, sound->impl.fileData.data(), (int)sound->impl.fileData.size(),
           (double)ch->impl.volume, (int)(ch->impl.loopCount != 0), ch->impl.index);
    }
#endif

    if (outChannel) *outChannel = ch;
    return FMOD_OK;
}

Channel* System::getChannelByIndex(int idx) {
    if (idx >= 0 && idx < (int)m_channels.size())
        return m_channels[idx].get();
    return nullptr;
}

FMOD_RESULT System::getChannelsPlaying(int* v, int* r) {
    int count = 0;
    for (auto& c : m_channels) {
        if (c->impl.playing) count++;
    }
    if (v) *v = count;
    if (r) *r = count;
    return FMOD_OK;
}

// ---------------------------------------------------------------------------
// Mixer — called every frame from AudioMan::Update -> system->update()
// ---------------------------------------------------------------------------

FMOD_RESULT System::update() {
    if (!m_initialized || !m_sdlStream) return FMOD_OK;

    // Check how much audio the stream needs
    int queued = SDL_GetAudioStreamQueued((SDL_AudioStream*)m_sdlStream);
    int bytesPerFrame = 2 * sizeof(float);  // stereo float32
    int targetBytes = MIX_FRAMES * bytesPerFrame;

    // Only mix if the stream needs more data (avoid building up latency)
    if (queued > targetBytes * 4) return FMOD_OK;

    // Clear mix buffer
    std::fill(m_mixBuffer.begin(), m_mixBuffer.end(), 0.0f);
    int framesToMix = MIX_FRAMES;

    // Collect channels that finished this frame (fire callbacks after mixing)
    std::vector<Channel*> endedChannels;

    // Mix all active channels (skip Web Audio channels — handled by browser)
    for (auto& ch : m_channels) {
        ChannelImpl& ci = ch->impl;
        if (!ci.playing || ci.paused || !ci.sound)
            continue;
        if (ci.sound->useWebAudio || ci.sound->pcmData.empty())
            continue;

        SoundImpl& snd = *ci.sound;
        float groupVol = ci.group ? ci.group->volume : 1.0f;
        bool groupMuted = ci.group ? ci.group->muted : false;
        float effectiveVol = ci.volume * groupVol * m_masterGroup.impl.volume;
        if (groupMuted || m_masterGroup.impl.muted) effectiveVol = 0.0f;

        // Panning: equal-power pan law
        float panL = std::cos((ci.pan + 1.0f) * 0.25f * 3.14159265f);
        float panR = std::sin((ci.pan + 1.0f) * 0.25f * 3.14159265f);

        for (int f = 0; f < framesToMix; f++) {
            int pos = (int)ci.playPosition;
            if (pos >= (int)snd.totalFrames) {
                // Handle loop or end
                if (ci.loopCount != 0) {
                    ci.playPosition = 0.0;
                    pos = 0;
                    if (ci.loopCount > 0) ci.loopCount--;
                } else {
                    ci.playing = false;
                    endedChannels.push_back(ch.get());
                    break;
                }
            }

            // Read sample (mono or stereo source)
            float sL = 0.0f, sR = 0.0f;
            if (snd.channels == 1) {
                sL = sR = snd.pcmData[pos];
            } else if (snd.channels >= 2) {
                sL = snd.pcmData[pos * snd.channels];
                sR = snd.pcmData[pos * snd.channels + 1];
            }

            // Apply volume and pan, accumulate into stereo mix
            m_mixBuffer[f * 2]     += sL * effectiveVol * panL;
            m_mixBuffer[f * 2 + 1] += sR * effectiveVol * panR;

            ci.playPosition += ci.pitch;
        }
    }

    // Clamp output
    for (auto& s : m_mixBuffer) {
        s = std::clamp(s, -1.0f, 1.0f);
    }

    // Push to SDL audio stream
    SDL_PutAudioStreamData((SDL_AudioStream*)m_sdlStream, m_mixBuffer.data(),
                           framesToMix * bytesPerFrame);

    // Fire end callbacks
    for (Channel* ch : endedChannels) {
        fireEndCallback(ch);
    }

    return FMOD_OK;
}

void System::fireEndCallback(Channel* ch) {
    if (ch->impl.callback) {
        ch->impl.callback(
            (FMOD_CHANNELCONTROL*)ch,
            FMOD_CHANNELCONTROL_CHANNEL,
            FMOD_CHANNELCONTROL_CALLBACK_END,
            nullptr, nullptr
        );
    }
    ch->impl.callback = nullptr;
    ch->impl.userData = nullptr;
}

// ---------------------------------------------------------------------------
// Channel method implementations that need System access
// ---------------------------------------------------------------------------

FMOD_RESULT Channel::stop() {
    impl.playing = false;
    impl.paused = false;
#ifdef __EMSCRIPTEN__
    // Stop Web Audio source node if active
    if (impl.webAudioId >= 0) {
        EM_ASM({
            var nodes = window._ccAudioNodes;
            if (nodes && nodes[$0]) {
                try { nodes[$0].source.stop(); } catch(e) {}
                delete nodes[$0];
            }
        }, impl.webAudioId);
        impl.webAudioId = -1;
    }
#endif
    // Fire end callback
    if (impl.callback) {
        System* sys = GetGlobalSystem();
        if (sys) sys->fireEndCallback(this);
    }
    return FMOD_OK;
}

FMOD_RESULT Channel::getCurrentSound(Sound** s) {
    if (!s) return FMOD_OK;
    // We need to find the Sound object that owns this SoundImpl.
    // For simplicity, return nullptr — AudioMan doesn't rely heavily on this.
    *s = nullptr;
    return FMOD_OK;
}

FMOD_RESULT Channel::setChannelGroup(ChannelGroup* g) {
    if (g) impl.group = &g->impl;
    return FMOD_OK;
}

FMOD_RESULT Channel::getPosition(unsigned int* p, FMOD_TIMEUNIT u) {
    if (!p) return FMOD_OK;
    if (u & FMOD_TIMEUNIT_MS) {
        *p = impl.sound ? (unsigned int)((uint64_t)impl.playPosition * 1000 / impl.sound->sampleRate) : 0;
    } else {
        *p = (unsigned int)impl.playPosition;
    }
    return FMOD_OK;
}
