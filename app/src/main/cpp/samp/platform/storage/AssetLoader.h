#pragma once

#include "Asset.h"
#include "AssetManager.h"
#include <memory>
#include <string>

namespace samp {
namespace platform {
namespace storage {

class AssetLoader {
public:
    static AssetLoader& Get();
    
    std::unique_ptr<FileAsset> LoadFile(const std::string& path);
    std::unique_ptr<TextureAsset> LoadTexture(const std::string& path);
    std::unique_ptr<AudioAsset> LoadAudio(const std::string& path);
    std::unique_ptr<ScriptAsset> LoadScript(const std::string& path);
    
private:
    AssetLoader() = default;
    ~AssetLoader() = default;
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader& operator=(const AssetLoader&) = delete;
};

} // namespace storage
} // namespace platform
} // namespace samp
