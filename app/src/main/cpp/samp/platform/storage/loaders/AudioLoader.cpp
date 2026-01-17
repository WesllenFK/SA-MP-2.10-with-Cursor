#include "AudioLoader.h"
#include "../AssetLoader.h"
#include "../../../log.h"

namespace samp {
namespace platform {
namespace storage {

std::unique_ptr<AudioAsset> AudioLoader::Load(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadAudio(path);
}

bool AudioLoader::Exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadAudio(path) != nullptr;
}

} // namespace storage
} // namespace platform
} // namespace samp
