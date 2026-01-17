#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

namespace samp {
namespace platform {
namespace storage {

class AssetPathResolver {
public:
    static AssetPathResolver& Get();
    
    std::string Resolve(const std::string& logicalPath);
    void RegisterMapping(const std::string& gamePath, const std::string& assetPath);
    void RegisterKnownPaths();
    
    bool IsMapped(const std::string& logicalPath) const;
    
private:
    AssetPathResolver() = default;
    ~AssetPathResolver() = default;
    AssetPathResolver(const AssetPathResolver&) = delete;
    AssetPathResolver& operator=(const AssetPathResolver&) = delete;
    
    bool ValidatePath(const std::string& path) const;
    
    std::unordered_map<std::string, std::string> m_pathMappings;
    mutable std::shared_mutex m_mutex;
};

} // namespace storage
} // namespace platform
} // namespace samp
