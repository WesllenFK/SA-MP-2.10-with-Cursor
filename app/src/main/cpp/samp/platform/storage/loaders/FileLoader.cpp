#include "FileLoader.h"
#include "../AssetManager.h"
#include "../../../log.h"

namespace samp {
namespace platform {
namespace storage {

std::unique_ptr<FileAsset> FileLoader::Load(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    AssetManager& manager = AssetManager::Get();
    std::unique_ptr<Asset> asset = manager.LoadAsset(path);
    
    if (!asset) {
        return nullptr;
    }
    
    FileAsset* fileAsset = dynamic_cast<FileAsset*>(asset.get());
    if (!fileAsset) {
        return nullptr;
    }
    
    asset.release();
    return std::unique_ptr<FileAsset>(fileAsset);
}

bool FileLoader::Exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    AssetManager& manager = AssetManager::Get();
    std::unique_ptr<Asset> asset = manager.LoadAsset(path);
    return asset != nullptr;
}

} // namespace storage
} // namespace platform
} // namespace samp
