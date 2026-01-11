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
    // Clean up active sounds
    for (auto& pair : activeSounds) {
        pair.second.sound.stop();
    }
    activeSounds.clear();
    soundBuffers.clear();
}

void SoundManager::initialize(float musicVol, float sfxVol) {
    if (!initialized) {
        musicVolume = musicVol;
        sfxVolume = sfxVol;

        // Load placeholder sounds
        if (!loadSound("test_music", "assets/sounds/background_music.wav", SoundType::BACKGROUND_MUSIC)) {
            std::cout << "Warning: Could not load test music. Create placeholder file." << std::endl;
        }

        if (!loadSound("test_sfx", "assets/sounds/click.wav", SoundType::SOUND_EFFECT)) {
            std::cout << "Warning: Could not load test SFX. Create placeholder file." << std::endl;
        }

        initialized = true;
        std::cout << "SoundManager initialized" << std::endl;
    }
}

bool SoundManager::loadSound(const std::string& id, const std::string& filename, SoundType type) {
    // Check if already loaded
    if (soundBuffers.find(id) != soundBuffers.end()) {
        return true;
    }

    SoundData data;
    data.type = type;

    if (data.buffer.loadFromFile(filename)) {
        soundBuffers[id] = std::move(data);
        return true;
    }

    std::cout << "Failed to load sound: " << filename << std::endl;
    return false;
}

void SoundManager::playSound(const std::string& id, bool loop) {
    if (!initialized) return;

    auto it = soundBuffers.find(id);
    if (it == soundBuffers.end()) {
        std::cout << "Sound not found: " << id << std::endl;
        return;
    }

    // Stop if already playing
    stopSound(id);

    PlayingSound playing;
    playing.sound.setBuffer(it->second.buffer);
    playing.type = it->second.type;
    playing.sound.setLoop(loop);

    // Set initial volume
    updateVolume(id);

    playing.sound.play();
    activeSounds[id] = std::move(playing);
}

void SoundManager::stopSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.sound.stop();
        activeSounds.erase(it);
    }
}

void SoundManager::pauseSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.sound.pause();
    }
}

void SoundManager::resumeSound(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        it->second.sound.play();
    }
}

void SoundManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(100.0f, volume));

    // Update volume of all active music sounds
    for (auto& pair : activeSounds) {
        if (pair.second.type == SoundType::BACKGROUND_MUSIC) {
            updateVolume(pair.first);
        }
    }
}

void SoundManager::setSFXVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(100.0f, volume));

    // Update volume of all active SFX sounds
    for (auto& pair : activeSounds) {
        if (pair.second.type == SoundType::SOUND_EFFECT) {
            updateVolume(pair.first);
        }
    }
}

void SoundManager::muteMusic(bool mute) {
    musicMuted = mute;
    for (auto& pair : activeSounds) {
        if (pair.second.type == SoundType::BACKGROUND_MUSIC) {
            updateVolume(pair.first);
        }
    }
}

void SoundManager::muteSFX(bool mute) {
    sfxMuted = mute;
    for (auto& pair : activeSounds) {
        if (pair.second.type == SoundType::SOUND_EFFECT) {
            updateVolume(pair.first);
        }
    }
}

void SoundManager::updateVolume(const std::string& id) {
    auto it = activeSounds.find(id);
    if (it != activeSounds.end()) {
        float baseVolume = 0.0f;
        bool muted = false;

        switch (it->second.type) {
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

        it->second.sound.setVolume(muted ? 0.0f : baseVolume);
    }
}

void SoundManager::playTestMusic() {
    playSound("test_music", true);
}

void SoundManager::playTestSFX() {
    playSound("test_sfx");
}