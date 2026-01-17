#include "AssetCache.h"
#include "../../log.h"
#include <algorithm>

namespace samp {
namespace platform {
namespace storage {

AssetCache& AssetCache::Get() {
    static AssetCache instance;
    return instance;
}

void AssetCache::Put(const std::string& path, std::shared_ptr<Asset> asset) {
    if (path.empty() || !asset) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache[path] = std::weak_ptr<Asset>(asset);
}

std::shared_ptr<Asset> AssetCache::Get(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        std::shared_ptr<Asset> asset = it->second.lock();
        if (!asset) {
            m_cache.erase(it);
            return nullptr;
        }
        return asset;
    }
    
    return nullptr;
}

void AssetCache::Remove(const std::string& path) {
    if (path.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.erase(path);
}

void AssetCache::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.clear();
}

size_t AssetCache::GetSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t count = 0;
    for (auto it = m_cache.begin(); it != m_cache.end();) {
        if (it->second.lock()) {
            ++count;
            ++it;
        } else {
            it = m_cache.erase(it);
        }
    }
    
    return count;
}

} // namespace storage
} // namespace platform
} // namespace samp
