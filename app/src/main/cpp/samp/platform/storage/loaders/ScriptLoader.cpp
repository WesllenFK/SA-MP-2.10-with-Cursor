#include "ScriptLoader.h"
#include "../AssetLoader.h"
#include "../../../log.h"

namespace samp {
namespace platform {
namespace storage {

std::unique_ptr<ScriptAsset> ScriptLoader::Load(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadScript(path);
}

bool ScriptLoader::Exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    AssetLoader& loader = AssetLoader::Get();
    return loader.LoadScript(path) != nullptr;
}

} // namespace storage
} // namespace platform
} // namespace samp
