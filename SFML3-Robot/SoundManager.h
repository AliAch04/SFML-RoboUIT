#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class SoundManager {
public:
    enum class SoundType {
        BACKGROUND_MUSIC,
        SOUND_EFFECT,
        TEST
    };

    SoundManager();
    ~SoundManager();

    // Initialize with config values
    void initialize(float musicVolume = 100.0f, float sfxVolume = 100.0f);

    // Load sounds
    bool loadSound(const std::string& id, const std::string& filename, SoundType type);

    // Playback control
    void playSound(const std::string& id, bool loop = false);
    void stopSound(const std::string& id);
    void pauseSound(const std::string& id);
    void resumeSound(const std::string& id);

    // Volume control
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    void muteMusic(bool mute);
    void muteSFX(bool mute);

    // Getters
    float getMusicVolume() const { return musicVolume; }
    float getSFXVolume() const { return sfxVolume; }
    bool isMusicMuted() const { return musicMuted; }
    bool isSFXMuted() const { return sfxMuted; }

    // Test sounds
    void playTestMusic();
    void playTestSFX();

    // Status
    bool isInitialized() const { return initialized; }

private:
    struct SoundData {
        sf::SoundBuffer buffer;
        SoundType type;
    };

    struct PlayingSound {
        sf::Sound sound;
        SoundType type;
    };

    std::unordered_map<std::string, SoundData> soundBuffers;
    std::unordered_map<std::string, PlayingSound> activeSounds;

    float musicVolume;
    float sfxVolume;
    bool musicMuted;
    bool sfxMuted;
    bool initialized;

    void updateVolume(const std::string& id);
};