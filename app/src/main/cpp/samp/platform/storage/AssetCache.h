#pragma once

#include "Asset.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace samp {
namespace platform {
namespace storage {

class AssetCache {
public:
    static AssetCache& Get();
    
    void Put(const std::string& path, std::shared_ptr<Asset> asset);
    std::shared_ptr<Asset> Get(const std::string& path);
    void Remove(const std::string& path);
    void Clear();
    size_t GetSize() const;
    
private:
    AssetCache() = default;
    ~AssetCache() = default;
    AssetCache(const AssetCache&) = delete;
    AssetCache& operator=(const AssetCache&) = delete;
    
    std::unordered_map<std::string, std::weak_ptr<Asset>> m_cache;
    mutable std::mutex m_mutex;
};

} // namespace storage
} // namespace platform
} // namespace samp
