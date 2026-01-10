#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <array>

class TextureManager {
public:
    enum class Id { Robot = 0, Wall, Floor, Obstacle, Count };

    struct Entry {
        std::string label;
        std::string defaultPath;
        std::string currentPath;

        sf::Texture texture;          // in-game texture
        sf::Sprite  thumbnailSprite;  // UI preview sprite
        bool loaded = false;
        std::string lastError;
    };

    TextureManager();

    // Set default paths (also initializes current paths to defaults)
    void setDefaults(const std::string& robot,
        const std::string& wall,
        const std::string& floor,
        const std::string& obstacle);

    // Load single / all textures using currentPath
    bool load(Id id);
    void loadAll();

    // Change path and load immediately (reverts if invalid)
    bool setPath(Id id, const std::string& path);

    // Reset path to default and reload
    bool reset(Id id);

    // Access
    const Entry& get(Id id) const;
    Entry& get(Id id);

    const sf::Texture& texture(Id id) const;

    // Thumbnail helpers
    void updateThumbnail(Id id, float thumbSizePx = 72.f);
    void updateAllThumbnails(float thumbSizePx = 72.f);

    // File dialog (platform-specific). Returns "" if canceled or failed.
    static std::string openFileDialog(const char* title = "Select texture");

private:
    std::array<Entry, (size_t)Id::Count> entries;
};
#pragma once
