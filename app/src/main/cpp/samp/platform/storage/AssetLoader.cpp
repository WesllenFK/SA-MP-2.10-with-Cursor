#include "AssetLoader.h"
#include "../../log.h"
#include <cstring>

namespace samp {
namespace platform {
namespace storage {

AssetLoader& AssetLoader::Get() {
    static AssetLoader instance;
    return instance;
}

std::unique_ptr<FileAsset> AssetLoader::LoadFile(const std::string& path) {
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

std::unique_ptr<TextureAsset> AssetLoader::LoadTexture(const std::string& path) {
    AssetManager& manager = AssetManager::Get();
    std::unique_ptr<FileAsset> fileAsset = manager.LoadTexture(path);
    
    if (!fileAsset) {
        return nullptr;
    }
    
    size_t size = fileAsset->GetSize();
    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
    std::memcpy(data.get(), fileAsset->GetData(), size);
    
    return std::make_unique<TextureAsset>(
        fileAsset->GetPath(),
        std::move(data),
        size,
        fileAsset->GetProviderType(),
        0, 0, 0
    );
}

std::unique_ptr<AudioAsset> AssetLoader::LoadAudio(const std::string& path) {
    AssetManager& manager = AssetManager::Get();
    std::unique_ptr<FileAsset> fileAsset = manager.LoadAudio(path);
    
    if (!fileAsset) {
        return nullptr;
    }
    
    size_t size = fileAsset->GetSize();
    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
    std::memcpy(data.get(), fileAsset->GetData(), size);
    
    return std::make_unique<AudioAsset>(
        fileAsset->GetPath(),
        std::move(data),
        size,
        fileAsset->GetProviderType(),
        0, 0, 0
    );
}

std::unique_ptr<ScriptAsset> AssetLoader::LoadScript(const std::string& path) {
    AssetManager& manager = AssetManager::Get();
    std::unique_ptr<FileAsset> fileAsset = manager.LoadScript(path);
    
    if (!fileAsset) {
        return nullptr;
    }
    
    size_t size = fileAsset->GetSize();
    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
    std::memcpy(data.get(), fileAsset->GetData(), size);
    
    return std::make_unique<ScriptAsset>(
        fileAsset->GetPath(),
        std::move(data),
        size,
        fileAsset->GetProviderType()
    );
}

} // namespace storage
} // namespace platform
} // namespace samp
