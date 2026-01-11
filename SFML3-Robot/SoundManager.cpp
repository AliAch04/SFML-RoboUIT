#include "SoundManager.h"
#include <iostream>

SoundManager::SoundManager()
    : musicVolume(100.0f)
    , sfxVolume(100.0f)
    , musicMuted(false)
    , sfxMuted(false)
    , initialized(false) {
}

SoundManager::~SoundManager() {
    // Sounds will be automatically destroyed
}

void SoundManager::initialize(float musicVol, float sfxVol) {
    if (!initialized) {
        musicVolume = musicVol;
        sfxVolume = sfxVol;

        // Try to load placeholder sounds
        bool musicLoaded = loadSound("test_music", "assets/sounds/background_music.wav", SoundType::BACKGROUND_MUSIC);
        bool sfxLoaded = loadSound("test_sfx", "assets/sounds/click.wav", SoundType::SOUND_EFFECT);

        if (!musicLoaded || !sfxLoaded) {
            std::cout << "Warning: Could not load all sound files." << std::endl;
        }

        initialized = true;
        std::cout << "SoundManager initialized with SFML 2.x Audio" << std::endl;
    }
}

bool SoundManager::loadSound(const std::string& id, const std::string& filename, SoundType type) {
    SoundData data;
    data.type = type;

    if (data.buffer.loadFromFile(filename)) {
        soundBuffers[id] = data;
        std::cout << "Loaded sound: " << id << " from " << filename << std::endl;
        return true;
    }

    std::cout << "Failed to load sound: " << filename << std::endl;
    return false;
}

void SoundManager::playSound(const std::string& id, bool loop) {
    if (!initialized) return;

    auto it = soundBuffers.find(id);
    if (it == soundBuffers.end()) {
        std::cout << "Sound not loaded: " << id << std::endl;
        return;
    }

    // Create and play the sound
    sf::Sound& sound = activeSounds[id];
    sound.setBuffer(it->second.buffer);
    sound.setLoop(loop);
    sound.setVolume(calculateVolume(it->second.type));
    sound.play();

    std::cout << "Playing sound: " << id << (loop ? " (looping)" : "") << std::endl;
}

void SoundManager::stopSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.stop();
        activeSounds.erase(it);
    }
}

void SoundManager::pauseSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.pause();
    }
}

void SoundManager::resumeSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.play();
    }
}

void SoundManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(100.0f, volume));
    updateAllVolumes();
}

void SoundManager::setSFXVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(100.0f, volume));
    updateAllVolumes();
}

void SoundManager::muteMusic(bool mute) {
    musicMuted = mute;
    updateAllVolumes();
}

void SoundManager::muteSFX(bool mute) {
    sfxMuted = mute;
    updateAllVolumes();
}

void SoundManager::updateAllVolumes() {
    for (auto& pair : activeSounds) {
        // Find the sound type
        auto bufferIt = soundBuffers.find(pair.first);
        if (bufferIt != soundBuffers.end()) {
            pair.second.setVolume(calculateVolume(bufferIt->second.type));
        }
    }
}

float SoundManager::calculateVolume(SoundType type) const {
    float baseVolume = 0.0f;
    bool muted = false;

    switch (type) {
    case SoundType::BACKGROUND_MUSIC:
        baseVolume = musicVolume;
        muted = musicMuted;
        break;
    case SoundType::SOUND_EFFECT:
    case SoundType::TEST:
        baseVolume = sfxVolume;
        muted = sfxMuted;
        break;
    }

    return muted ? 0.0f : baseVolume;
}

void SoundManager::playTestMusic() {
    playSound("test_music", true);
}

void SoundManager::playTestSFX() {
    playSound("test_sfx");
}