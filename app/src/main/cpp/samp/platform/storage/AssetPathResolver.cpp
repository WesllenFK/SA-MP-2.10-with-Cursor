#include "AssetPathResolver.h"
#include "../../log.h"
#include <algorithm>
#include <cstring>

namespace samp {
namespace platform {
namespace storage {

AssetPathResolver& AssetPathResolver::Get() {
    static AssetPathResolver instance;
    static bool initialized = false;
    if (!initialized) {
        instance.RegisterKnownPaths();
        initialized = true;
    }
    return instance;
}

std::string AssetPathResolver::Resolve(const std::string& logicalPath) {
    if (logicalPath.empty()) {
        return "";
    }
    
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    auto it = m_pathMappings.find(logicalPath);
    if (it != m_pathMappings.end()) {
        return it->second;
    }
    
    return logicalPath;
}

void AssetPathResolver::RegisterMapping(const std::string& gamePath, const std::string& assetPath) {
    if (gamePath.empty() || assetPath.empty()) {
        LOGE("AssetPathResolver::RegisterMapping: Empty path");
        return;
    }
    
    if (!ValidatePath(gamePath) || !ValidatePath(assetPath)) {
        LOGE("AssetPathResolver::RegisterMapping: Invalid path");
        return;
    }
    
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_pathMappings[gamePath] = assetPath;
}

void AssetPathResolver::RegisterKnownPaths() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    m_pathMappings.clear();
    
    m_pathMappings["DATA/GTA.DAT"] = "data/gta.dat";
    m_pathMappings["DATA/DEFAULT.DAT"] = "data/default.dat";
    m_pathMappings["DATA/OBJECT.DAT"] = "data/object.dat";
    m_pathMappings["DATA/IDE/"] = "data/ide/";
    m_pathMappings["DATA/IMG/"] = "data/img/";
    m_pathMappings["DATA/MAPS/"] = "data/maps/";
    m_pathMappings["DATA/TEXT/"] = "data/text/";
    m_pathMappings["DATA/ANIM/"] = "data/anim/";
    m_pathMappings["mainV1.scm"] = "main.scm";
    m_pathMappings["main.scm"] = "main.scm";
    
    m_pathMappings["MODELS/"] = "models/";
    m_pathMappings["TEXTURES/"] = "textures/";
    m_pathMappings["AUDIO/"] = "audio/";
    m_pathMappings["SCRIPTS/"] = "scripts/";
}

bool AssetPathResolver::IsMapped(const std::string& logicalPath) const {
    if (logicalPath.empty()) {
        return false;
    }
    
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_pathMappings.find(logicalPath) != m_pathMappings.end();
}

bool AssetPathResolver::ValidatePath(const std::string& path) const {
    if (path.empty()) {
        return false;
    }
    
    if (path.find("../") != std::string::npos || path.find("..\\") != std::string::npos) {
        return false;
    }
    
    return true;
}

} // namespace storage
} // namespace platform
} // namespace samp
