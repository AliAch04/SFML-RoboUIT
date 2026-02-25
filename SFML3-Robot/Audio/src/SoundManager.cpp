#include "SoundManager.h"
#include <iostream>
#include <iomanip>

SoundManager::SoundManager()
    : musicVolume(100.0f)
    , sfxVolume(100.0f)
    , musicMuted(false)
    , sfxMuted(false)
    , initialized(false)
    , currentBackgroundMusicId("test_music") {
}

SoundManager::~SoundManager() {
    // Sounds will be automatically destroyed
}

void SoundManager::initialize(float musicVol, float sfxVol) {
    if (!initialized) {
        musicVolume = musicVol;
        sfxVolume = sfxVol;

        // Load placeholder sounds
        bool musicLoaded = loadSound("test_music", "assets/sounds/background_music.wav", SoundType::BACKGROUND_MUSIC);
        bool sfxLoaded = loadSound("test_sfx", "assets/sounds/click.wav", SoundType::SOUND_EFFECT);

        // Load additional test sounds
        loadSound("success", "assets/sounds/success.wav", SoundType::SOUND_EFFECT);

        if (musicLoaded) {
            std::cout << "Background music loaded successfully" << std::endl;
            // Auto-start background music
            startBackgroundMusic();
        }
        else {
            std::cout << "Failed to load background music" << std::endl;
        }

        if (sfxLoaded) {
            std::cout << "SFX loaded successfully" << std::endl;
        }

        initialized = true;
        std::cout << "SoundManager initialized with SFML 2.x Audio" << std::endl;
    }
}

void SoundManager::startBackgroundMusic() {
    if (!initialized || currentBackgroundMusicId.empty()) return;

    auto it = soundBuffers.find(currentBackgroundMusicId);
    if (it == soundBuffers.end()) {
        std::cout << "Background music not loaded: " << currentBackgroundMusicId << std::endl;
        return;
    }

    if (it->second.type != SoundType::BACKGROUND_MUSIC) {
        std::cout << "Sound is not background music type: " << currentBackgroundMusicId << std::endl;
        return;
    }

    // Stop any existing background music
    stopBackgroundMusic();

    // Play with loop
    playSound(currentBackgroundMusicId, true);

    std::cout << "\nBACKGROUND MUSIC STARTED" << std::endl;
    std::cout << "  ID: " << currentBackgroundMusicId << std::endl;
    std::cout << "  Volume: " << calculateVolume(SoundType::BACKGROUND_MUSIC) << std::endl;
    std::cout << "  Duration: " << std::fixed << std::setprecision(2)
        << it->second.buffer.getDuration().asSeconds() << "s" << std::endl;
    std::cout << "  Loop: YES" << std::endl;
    std::cout << "  Muted: " << (musicMuted ? "YES" : "NO") << std::endl;
}

void SoundManager::stopBackgroundMusic() {
    if (!currentBackgroundMusicId.empty()) {
        auto it = activeSounds.find(currentBackgroundMusicId);
        if (it != activeSounds.end()) {
            it->second.stop();
            std::cout << "Background music stopped" << std::endl;
        }
    }
}

bool SoundManager::isBackgroundMusicPlaying() const {
    if (currentBackgroundMusicId.empty()) return false;

    auto it = activeSounds.find(currentBackgroundMusicId);
    if (it != activeSounds.end()) {
        return it->second.getStatus() == sf::Sound::Playing;
    }
    return false;
}

void SoundManager::setBackgroundMusic(const std::string& id) {
    currentBackgroundMusicId = id;
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
    if (!initialized) {
        std::cout << "SoundManager not initialized!" << std::endl;
        return;
    }

    auto it = soundBuffers.find(id);
    if (it == soundBuffers.end()) {
        std::cout << "Sound not loaded: " << id << std::endl;

        // List all loaded sounds for debugging
        std::cout << "Loaded sounds:" << std::endl;
        for (const auto& pair : soundBuffers) {
            std::cout << "  - " << pair.first
                << " (Type: " << (pair.second.type == SoundType::BACKGROUND_MUSIC ? "MUSIC" : "SFX")
                << ")" << std::endl;
        }
        return;
    }

    // Create and play the sound
    sf::Sound& sound = activeSounds[id];
    sound.setBuffer(it->second.buffer);
    sound.setLoop(loop);

    float volume = calculateVolume(it->second.type);
    sound.setVolume(volume);

    std::cout << "\nPlaying: " << id << std::endl;
    std::cout << "  Type: " << (it->second.type == SoundType::BACKGROUND_MUSIC ? "MUSIC" : "SFX") << std::endl;
    std::cout << "  Volume: " << volume << std::endl;
    std::cout << "  Loop: " << (loop ? "YES" : "NO") << std::endl;

    sound.play();
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
        baseVolume = sfxVolume;
        muted = sfxMuted;
        break;
    case SoundType::TEST:
        baseVolume = sfxVolume;
        muted = sfxMuted;
        break;
    default:
        baseVolume = 100.0f;
        muted = false;
    }

    float finalVolume = muted ? 0.0f : baseVolume;
    return finalVolume;
}

void SoundManager::playTestMusic() {
    std::cout << "\nTESTING BACKGROUND MUSIC" << std::endl;
    startBackgroundMusic();
}

void SoundManager::playTestSFX() {
    std::cout << "\nTESTING SFX" << std::endl;
    playSound("test_sfx");
}