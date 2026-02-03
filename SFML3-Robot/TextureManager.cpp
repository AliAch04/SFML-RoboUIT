#include "TextureManager.h"
#include <filesystem>
#include <cstdio>
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "Comdlg32.lib")
#endif

namespace fs = std::filesystem;

// Helper: get a stable base dir (where config.txt is, or fallback to cwd)
static fs::path getDefaultBaseDir()
{
    try {
        fs::path cfg = fs::absolute("config.txt");
        return cfg.parent_path();
    }
    catch (...) {
        return fs::current_path();
    }
}

TextureManager::TextureManager()
{
    entries[(size_t)Id::Robot].label = "Robot";
    entries[(size_t)Id::Wall].label = "Wall";
    entries[(size_t)Id::Floor].label = "Floor";
    entries[(size_t)Id::Obstacle].label = "Obstacle";

    // NEW: stable base dir for resolving relative paths
    baseDir = getDefaultBaseDir();

    std::cout << "[TEXTURE] baseDir = " << baseDir.string() << "\n";
}

// NEW: resolve relative path against baseDir
fs::path TextureManager::resolvePath(const std::string& p) const
{
    if (p.empty()) return fs::path();

    fs::path pp(p);

    // If config accidentally contains quotes, trim them quickly
    if (!p.empty() && (p.front() == '"' || p.front() == '\'')) {
        std::string q = p;
        if (!q.empty() && (q.back() == '"' || q.back() == '\'')) q.pop_back();
        q.erase(q.begin());
        pp = fs::path(q);
    }

    if (pp.is_absolute()) return pp;
    return baseDir / pp;
}

// Optional (but useful): allow GameEngine to set this explicitly
void TextureManager::setBaseDir(const std::string& dir)
{
    try {
        baseDir = fs::path(dir);
    }
    catch (...) {
        baseDir = getDefaultBaseDir();
    }
    std::cout << "[TEXTURE] baseDir overridden = " << baseDir.string() << "\n";
}

void TextureManager::setDefaults(const std::string& robot,
    const std::string& wall,
    const std::string& floor,
    const std::string& obstacle)
{
    auto makeAbs = [&](const std::string& p) -> std::string {
        try {
            return resolvePath(p).string();  // <-- always baseDir-absolute now
        }
        catch (...) {
            return p;
        }
        };

    entries[(size_t)Id::Robot].defaultPath = makeAbs(robot);
    entries[(size_t)Id::Wall].defaultPath = makeAbs(wall);
    entries[(size_t)Id::Floor].defaultPath = makeAbs(floor);
    entries[(size_t)Id::Obstacle].defaultPath = makeAbs(obstacle);

    // Initialize current paths to defaults
    entries[(size_t)Id::Robot].currentPath = entries[(size_t)Id::Robot].defaultPath;
    entries[(size_t)Id::Wall].currentPath = entries[(size_t)Id::Wall].defaultPath;
    entries[(size_t)Id::Floor].currentPath = entries[(size_t)Id::Floor].defaultPath;
    entries[(size_t)Id::Obstacle].currentPath = entries[(size_t)Id::Obstacle].defaultPath;

    std::cout << "[TEXTURE] setDefaults() resolved:\n"
        << "  robot=" << entries[(size_t)Id::Robot].defaultPath << "\n"
        << "  wall =" << entries[(size_t)Id::Wall].defaultPath << "\n"
        << "  floor=" << entries[(size_t)Id::Floor].defaultPath << "\n"
        << "  obst =" << entries[(size_t)Id::Obstacle].defaultPath << "\n";
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

    fs::path resolved = resolvePath(e.currentPath);

    // DEBUG: show the REAL path being checked
    std::cout << "[TEXTURE] load(" << e.label << ") currentPath=" << e.currentPath
        << " | resolved=" << resolved.string() << "\n";

    if (!fs::exists(resolved)) {
        e.lastError = "File not found";
        return false;
    }

    sf::Texture newTex;
    if (!newTex.loadFromFile(resolved.string())) {
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

    e.currentPath = path;

    if (load(id)) return true;

    // revert on failure
    e.currentPath = oldPath;
    if (oldLoaded) load(id);
    return false;
}

bool TextureManager::reset(Id id)
{
    auto& e = entries[(size_t)id];

    // defaultPath is stored as resolved absolute now
    e.currentPath = e.defaultPath;

    std::cout << "[TEXTURE] reset(" << e.label << ") defaultPath=" << e.defaultPath << "\n";

    if (load(id)) return true;

    e.loaded = false;
    if (e.lastError.empty()) e.lastError = "Failed to load default texture";
    return false;
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
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

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
    (void)title;
    return "";
#endif
}

