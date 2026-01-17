#pragma once

#include "Asset.h"
#include "AssetCache.h"
#include "AssetPathResolver.h"
#include "providers/IAssetProvider.h"
#include "jni/AssetJNIBridge.h"
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <jni.h>

namespace samp {
namespace platform {
namespace storage {

class AssetManager {
public:
    static AssetManager& Get();
    
    bool Initialize(JavaVM* jvm, jobject activity);
    void Shutdown();
    
    std::unique_ptr<Asset> LoadAsset(const std::string& path);
    std::shared_ptr<Asset> LoadAssetShared(const std::string& path);
    
    std::unique_ptr<FileAsset> LoadTexture(const std::string& path);
    std::unique_ptr<FileAsset> LoadAudio(const std::string& path);
    std::unique_ptr<FileAsset> LoadScript(const std::string& path);
    
    std::string ResolvePath(const std::string& logicalPath);
    
    void ClearCache();
    size_t GetCacheSize() const;
    
    const std::string& GetBasePath() const { return m_basePath; }
    
private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    
    IAssetProvider* FindProvider(const std::string& path) const;
    void RegisterProviders(JavaVM* jvm, jobject activity);
    
    std::vector<std::unique_ptr<IAssetProvider>> m_providers;
    AssetCache* m_cache;
    AssetPathResolver* m_pathResolver;
    AssetJNIBridge* m_jniBridge;
    JavaVM* m_jvm;
    jobject m_activityObject;
    std::string m_basePath;
    std::mutex m_mutex;
    bool m_initialized;
};

} // namespace storage
} // namespace platform
} // namespace samp
