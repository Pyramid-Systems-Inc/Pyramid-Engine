# Audio System API Reference

## Overview

The Pyramid Engine's Audio System provides comprehensive audio management for games and interactive applications. The system is designed to be modular, extensible, and performance-oriented.

**Note**: The Audio System is currently in development. This documentation represents the planned API and architecture.

## Planned Features

### Core Audio Features
- **3D Spatial Audio**: Positional audio with distance attenuation and doppler effects
- **Multi-Channel Support**: Stereo, 5.1, and 7.1 surround sound support
- **Format Support**: WAV, OGG, MP3, FLAC audio format loading
- **Streaming Audio**: Large audio files streamed from disk
- **Audio Compression**: Real-time audio compression and decompression
- **Effects Processing**: Reverb, echo, filters, and custom effect chains

### Audio Engine Architecture
- **Hardware Abstraction**: Support for multiple audio APIs (DirectSound, WASAPI, OpenAL)
- **Thread-Safe Design**: Concurrent audio processing and main thread updates
- **Resource Management**: Efficient audio resource loading and caching
- **Performance Optimized**: Low-latency audio processing and minimal CPU overhead

## Planned API Structure

### AudioEngine Class

The main audio engine management class:

```cpp
class AudioEngine {
public:
    static AudioEngine& GetInstance();
    
    bool Initialize(const AudioConfig& config);
    void Shutdown();
    void Update(float deltaTime);
    
    // Audio source management
    AudioSourceID CreateAudioSource();
    void DestroyAudioSource(AudioSourceID sourceId);
    
    // Audio clip management
    AudioClipID LoadAudioClip(const std::string& path);
    void UnloadAudioClip(AudioClipID clipId);
    
    // Playback control
    void PlayAudio(AudioSourceID sourceId, AudioClipID clipId);
    void StopAudio(AudioSourceID sourceId);
    void PauseAudio(AudioSourceID sourceId);
    void ResumeAudio(AudioSourceID sourceId);
    
    // 3D Audio
    void SetListenerPosition(const Math::Vec3& position);
    void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up);
    void SetSourcePosition(AudioSourceID sourceId, const Math::Vec3& position);
    void SetSourceVelocity(AudioSourceID sourceId, const Math::Vec3& velocity);
    
    // Volume and effects
    void SetMasterVolume(float volume);
    void SetSourceVolume(AudioSourceID sourceId, float volume);
    void SetSourcePitch(AudioSourceID sourceId, float pitch);
    
    // Audio effects
    void AttachEffect(AudioSourceID sourceId, AudioEffectID effectId);
    void DetachEffect(AudioSourceID sourceId, AudioEffectID effectId);
    
private:
    AudioEngine() = default;
    ~AudioEngine() = default;
};
```

### AudioSource Class

Individual audio source for playing sounds:

```cpp
class AudioSource {
public:
    // Playback control
    void Play(AudioClipID clipId);
    void Stop();
    void Pause();
    void Resume();
    
    // Properties
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool loop);
    
    // 3D Audio properties
    void SetPosition(const Math::Vec3& position);
    void SetVelocity(const Math::Vec3& velocity);
    void SetMinDistance(float distance);
    void SetMaxDistance(float distance);
    void SetAttenuationMode(AttenuationMode mode);
    
    // Status queries
    bool IsPlaying() const;
    bool IsPaused() const;
    float GetPlaybackPosition() const;
    
private:
    AudioSourceID m_sourceId;
    Math::Vec3 m_position;
    Math::Vec3 m_velocity;
    float m_volume = 1.0f;
    float m_pitch = 1.0f;
    bool m_looping = false;
};
```

### AudioClip Class

Audio data container:

```cpp
class AudioClip {
public:
    // Loading
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const u8* data, size_t size);
    
    // Properties
    float GetDuration() const;
    u32 GetSampleRate() const;
    u32 GetChannelCount() const;
    u32 GetBitDepth() const;
    AudioFormat GetFormat() const;
    
    // Data access
    const u8* GetData() const;
    size_t GetSizeInBytes() const;
    
private:
    std::unique_ptr<u8[]> m_audioData;
    AudioFormat m_format;
    u32 m_sampleRate;
    u32 m_channels;
    u32 m_bitDepth;
    float m_duration;
};
```

### Audio Effects System

```cpp
// Audio effect base class
class AudioEffect {
public:
    virtual ~AudioEffect() = default;
    virtual void ProcessAudio(float* samples, u32 sampleCount, u32 channels) = 0;
    virtual void SetParameter(const std::string& name, float value) = 0;
};

// Reverb effect
class ReverbEffect : public AudioEffect {
public:
    void ProcessAudio(float* samples, u32 sampleCount, u32 channels) override;
    void SetParameter(const std::string& name, float value) override;
    
    // Reverb-specific parameters
    void SetRoomSize(float size);
    void SetDamping(float damping);
    void SetWetness(float wetness);
    void SetDryness(float dryness);
};

// Echo effect
class EchoEffect : public AudioEffect {
public:
    void ProcessAudio(float* samples, u32 sampleCount, u32 channels) override;
    void SetParameter(const std::string& name, float value) override;
    
    // Echo-specific parameters
    void SetDelay(float delay);
    void SetFeedback(float feedback);
    void SetMix(float mix);
};
```

## Planned Configuration

### AudioConfig Structure

```cpp
struct AudioConfig {
    // Hardware settings
    u32 sampleRate = 44100;
    u32 bufferSize = 1024;
    u32 channelCount = 2;  // Stereo by default
    AudioAPI preferredAPI = AudioAPI::Default;
    
    // Performance settings
    u32 maxAudioSources = 64;
    u32 maxStreamingSources = 8;
    bool enableEffectProcessing = true;
    
    // 3D Audio settings
    float speedOfSound = 343.0f;  // m/s
    float dopplerFactor = 1.0f;
    AttenuationModel attenuationModel = AttenuationModel::InverseDistance;
};
```

### AudioAPI Enumeration

```cpp
enum class AudioAPI {
    Default,      // Platform default
    DirectSound,  // Windows DirectSound
    WASAPI,       // Windows WASAPI
    OpenAL,       // Cross-platform OpenAL
    XAudio2       // Windows XAudio2
};
```

## Planned Usage Examples

### Basic Audio Playback

```cpp
#include <Pyramid/Audio/AudioEngine.hpp>

// Initialize audio system
AudioConfig config;
config.sampleRate = 44100;
config.bufferSize = 1024;

auto& audioEngine = AudioEngine::GetInstance();
if (!audioEngine.Initialize(config)) {
    PYRAMID_LOG_ERROR("Failed to initialize audio engine");
    return false;
}

// Load audio clip
auto bulletSoundId = audioEngine.LoadAudioClip("sounds/bullet_shot.wav");
auto musicId = audioEngine.LoadAudioClip("music/background_music.ogg");

// Create audio sources
auto sfxSourceId = audioEngine.CreateAudioSource();
auto musicSourceId = audioEngine.CreateAudioSource();

// Play sounds
audioEngine.PlayAudio(sfxSourceId, bulletSoundId);
audioEngine.PlayAudio(musicSourceId, musicId);

// Set looping for background music
audioEngine.SetSourceLooping(musicSourceId, true);
```

### 3D Spatial Audio

```cpp
// Setup 3D listener (camera position)
audioEngine.SetListenerPosition(camera.GetPosition());
audioEngine.SetListenerOrientation(camera.GetForward(), camera.GetUp());

// Create 3D positioned audio source
auto enemySourceId = audioEngine.CreateAudioSource();
audioEngine.SetSourcePosition(enemySourceId, enemy.GetPosition());
audioEngine.SetSourceVelocity(enemySourceId, enemy.GetVelocity());

// Configure 3D audio properties
audioEngine.SetSourceMinDistance(enemySourceId, 5.0f);  // Full volume within 5 units
audioEngine.SetSourceMaxDistance(enemySourceId, 100.0f); // No audio beyond 100 units
audioEngine.SetSourceAttenuationMode(enemySourceId, AttenuationMode::Realistic);

// Play positioned sound
auto enemyFootstepsId = audioEngine.LoadAudioClip("sounds/footsteps.wav");
audioEngine.PlayAudio(enemySourceId, enemyFootstepsId);
```

### Audio Effects Processing

```cpp
// Create and configure reverb effect
auto reverbEffect = std::make_unique<ReverbEffect>();
reverbEffect->SetRoomSize(0.8f);
reverbEffect->SetDamping(0.5f);
reverbEffect->SetWetness(0.3f);

auto reverbEffectId = audioEngine.RegisterEffect(std::move(reverbEffect));

// Apply reverb to music source
audioEngine.AttachEffect(musicSourceId, reverbEffectId);

// Create echo effect for special sounds
auto echoEffect = std::make_unique<EchoEffect>();
echoEffect->SetDelay(0.3f);
echoEffect->SetFeedback(0.4f);
echoEffect->SetMix(0.5f);

auto echoEffectId = audioEngine.RegisterEffect(std::move(echoEffect));
audioEngine.AttachEffect(specialSoundSourceId, echoEffectId);
```

### Game Integration Example

```cpp
class GameAudioManager {
public:
    bool Initialize() {
        AudioConfig config;
        config.maxAudioSources = 32;
        config.enableEffectProcessing = true;
        
        auto& audioEngine = AudioEngine::GetInstance();
        if (!audioEngine.Initialize(config)) {
            return false;
        }
        
        // Load common game sounds
        LoadGameSounds();
        CreateAudioSources();
        
        return true;
    }
    
    void Update(float deltaTime, const Camera& camera) {
        auto& audioEngine = AudioEngine::GetInstance();
        
        // Update listener position
        audioEngine.SetListenerPosition(camera.GetPosition());
        audioEngine.SetListenerOrientation(camera.GetForward(), camera.GetUp());
        
        // Update audio engine
        audioEngine.Update(deltaTime);
    }
    
    void PlayPlayerShoot() {
        auto& audioEngine = AudioEngine::GetInstance();
        audioEngine.PlayAudio(m_playerSfxSource, m_shootSoundId);
    }
    
    void PlayEnemyFootsteps(const Math::Vec3& position, const Math::Vec3& velocity) {
        auto& audioEngine = AudioEngine::GetInstance();
        audioEngine.SetSourcePosition(m_enemySfxSource, position);
        audioEngine.SetSourceVelocity(m_enemySfxSource, velocity);
        audioEngine.PlayAudio(m_enemySfxSource, m_footstepsSoundId);
    }
    
private:
    AudioSourceID m_playerSfxSource;
    AudioSourceID m_enemySfxSource;
    AudioSourceID m_musicSource;
    
    AudioClipID m_shootSoundId;
    AudioClipID m_footstepsSoundId;
    AudioClipID m_backgroundMusicId;
};
```

## Implementation Status

**Current Status**: Planning and Architecture Phase

### Completed
- ✅ Architecture design
- ✅ API specification
- ✅ Integration planning

### In Progress
- 🔄 Core audio engine implementation
- 🔄 Audio format loading (WAV, OGG)
- 🔄 Basic playback functionality

### Planned
- ⏳ 3D spatial audio implementation
- ⏳ Audio effects system
- ⏳ Streaming audio support
- ⏳ Multi-platform audio API support
- ⏳ Performance optimization
- ⏳ Audio compression support

## Performance Considerations

### Memory Management
- **Audio Streaming**: Large audio files streamed from disk to reduce memory usage
- **Resource Pooling**: Reusable audio source and buffer pools
- **Smart Caching**: Frequently used sounds cached in memory

### Threading Model
- **Audio Thread**: Dedicated thread for audio processing to minimize latency
- **Lock-Free Queues**: Thread-safe communication between game and audio threads
- **Real-Time Processing**: Audio processing designed for real-time constraints

### CPU Optimization
- **SIMD Audio Processing**: Vectorized audio processing for effects and mixing
- **Efficient Mixing**: Optimized audio mixing algorithms
- **Culling**: Spatial audio culling to reduce processing overhead

## See Also

- [Core Game Class](../Core/Game.md) - Main game loop integration
- [Scene Management](../Graphics/SceneManager.md) - 3D positioning integration
- [Resource Management](../Core/ResourceManager.md) - Audio asset loading
- [Performance Profiling](../Utils/Profiling.md) - Audio performance monitoring

## Future Extensions

- **Audio Scripting**: Lua/C# scripting for complex audio behaviors
- **Audio Occlusion**: Environmental audio occlusion and obstruction
- **Dynamic Music**: Adaptive music system based on game state
- **Voice Chat**: Real-time voice communication support
- **Audio Capture**: Microphone input and recording capabilities
