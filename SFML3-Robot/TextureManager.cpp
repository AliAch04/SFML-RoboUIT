#include "TextureManager.h"
#include <filesystem>
#include <cstdio>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#endif

TextureManager::TextureManager() {
    entries[(size_t)Id::Robot].label = "Robot";
    entries[(size_t)Id::Wall].label = "Wall";
    entries[(size_t)Id::Floor].label = "Floor";
    entries[(size_t)Id::Obstacle].label = "Obstacle";
}

void TextureManager::setDefaults(const std::string& robot,
    const std::string& wall,
    const std::string& floor,
    const std::string& obstacle)
{
    entries[(size_t)Id::Robot].defaultPath = robot;
    entries[(size_t)Id::Wall].defaultPath = wall;
    entries[(size_t)Id::Floor].defaultPath = floor;
    entries[(size_t)Id::Obstacle].defaultPath = obstacle;

    // Initialize current paths to defaults
    entries[(size_t)Id::Robot].currentPath = robot;
    entries[(size_t)Id::Wall].currentPath = wall;
    entries[(size_t)Id::Floor].currentPath = floor;
    entries[(size_t)Id::Obstacle].currentPath = obstacle;
}

const TextureManager::Entry& TextureManager::get(Id id) const { return entries[(size_t)id]; }
TextureManager::Entry& TextureManager::get(Id id) { return entries[(size_t)id]; }

const sf::Texture& TextureManager::texture(Id id) const {
    return entries[(size_t)id].texture;
}

bool TextureManager::load(Id id)
{
    auto& e = entries[(size_t)id];
    e.lastError.clear();
    e.loaded = false;

    if (e.currentPath.empty()) {
        e.lastError = "Empty path";
        return false;
    }

    if (!std::filesystem::exists(e.currentPath)) {
        e.lastError = "File not found";
        return false;
    }

    sf::Texture newTex;
    if (!newTex.loadFromFile(e.currentPath)) {
        e.lastError = "Invalid image or SFML failed to load";
        return false;
    }

    newTex.setSmooth(true);
    e.texture = std::move(newTex);
    e.loaded = true;

    e.thumbnailSprite.setTexture(e.texture, true);
    updateThumbnail(id, 72.f);
    return true;
}

void TextureManager::loadAll()
{
    load(Id::Robot);
    load(Id::Wall);
    load(Id::Floor);
    load(Id::Obstacle);
}

bool TextureManager::setPath(Id id, const std::string& path)
{
    auto& e = entries[(size_t)id];

    std::string oldPath = e.currentPath;
    bool oldLoaded = e.loaded;

    // Keep a copy of old texture by reloading it from oldPath if needed
    e.currentPath = path;

    if (load(id)) return true;

    // revert on failure
    e.currentPath = oldPath;
    if (oldLoaded) load(id); // attempt to restore texture state
    return false;
}

bool TextureManager::reset(Id id)
{
    auto& e = entries[(size_t)id];
    return setPath(id, e.defaultPath);
}

void TextureManager::updateThumbnail(Id id, float thumbSizePx)
{
    auto& e = entries[(size_t)id];
    if (!e.loaded) return;

    auto s = e.texture.getSize();
    if (s.x == 0 || s.y == 0) return;

    e.thumbnailSprite.setTexture(e.texture, true);

    float scaleX = thumbSizePx / (float)s.x;
    float scaleY = thumbSizePx / (float)s.y;
    float scale = (scaleX < scaleY) ? scaleX : scaleY; // preserve aspect

    e.thumbnailSprite.setScale(scale, scale);
}

void TextureManager::updateAllThumbnails(float thumbSizePx)
{
    updateThumbnail(Id::Robot, thumbSizePx);
    updateThumbnail(Id::Wall, thumbSizePx);
    updateThumbnail(Id::Floor, thumbSizePx);
    updateThumbnail(Id::Obstacle, thumbSizePx);
}

std::string TextureManager::openFileDialog(const char* title)
{
#ifdef _WIN32
    char fileName[MAX_PATH] = { 0 };

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = title;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(fileName);
    }
    return "";
#else
    // Minimal cross-platform fallback: return empty.
    // (We'll wire Linux/macOS later if you need it.)
    (void)title;
    return "";
#endif
}
