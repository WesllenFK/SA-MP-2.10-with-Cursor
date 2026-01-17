#include "TextureLoader.h"
#include "../AssetLoader.h"
#include "../../../log.h"

namespace samp {
namespace platform {
namespace storage {

std::unique_ptr<TextureAsset> TextureLoader::Load(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadTexture(path);
}

bool TextureLoader::Exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadTexture(path) != nullptr;
}

} // namespace storage
} // namespace platform
} // namespace samp
