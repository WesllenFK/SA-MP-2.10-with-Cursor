#include "AssetManager.h"
#include "providers/FilesDirProvider.h"
#include "providers/APKAssetProvider.h"
#include "../../log.h"
#include "../../java/jniutil.h"
#include <algorithm>

namespace samp {
namespace platform {
namespace storage {

AssetManager& AssetManager::Get() {
    static AssetManager instance;
    return instance;
}

bool AssetManager::Initialize(JavaVM* jvm, jobject activity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!jvm || !activity) {
        LOGE("AssetManager::Initialize: Invalid parameters");
        return false;
    }
    
    m_jvm = jvm;
    m_activityObject = activity;
    
    AssetJNIBridge& bridge = AssetJNIBridge::Get();
    if (!bridge.Initialize(nullptr, activity)) {
        LOGE("AssetManager::Initialize: Failed to initialize JNI bridge");
        return false;
    }
    
    m_basePath = bridge.GetBasePath();
    if (m_basePath.empty()) {
        LOGE("AssetManager::Initialize: Failed to get base path");
        return false;
    }
    
    m_cache = &AssetCache::Get();
    m_pathResolver = &AssetPathResolver::Get();
    m_jniBridge = &bridge;
    
    RegisterProviders(jvm, activity);
    
    if (m_providers.empty()) {
        LOGE("AssetManager::Initialize: No providers registered");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void AssetManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    if (m_jniBridge) {
        JNIEnv* env;
        bool needsDetach;
        if (GetJNIEnvSafe(&env, &needsDetach)) {
            m_jniBridge->Shutdown(env);
            DETACH_IF_NEEDED(needsDetach);
        }
    }
    
    m_providers.clear();
    m_cache = nullptr;
    m_pathResolver = nullptr;
    m_jniBridge = nullptr;
    m_jvm = nullptr;
    m_activityObject = nullptr;
    m_basePath.clear();
    m_initialized = false;
}

std::unique_ptr<Asset> AssetManager::LoadAsset(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        LOGE("AssetManager::LoadAsset: Not initialized");
        return nullptr;
    }
    
    std::string resolvedPath = m_pathResolver->Resolve(path);
    
    std::shared_ptr<Asset> cached = m_cache->Get(resolvedPath);
    if (cached) {
        FileAsset* fileAsset = dynamic_cast<FileAsset*>(cached.get());
        if (fileAsset) {
            size_t size = fileAsset->GetSize();
            auto data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
            std::memcpy(data.get(), fileAsset->GetData(), size);
            return std::make_unique<FileAsset>(
                fileAsset->GetPath(),
                std::move(data),
                size,
                fileAsset->GetProviderType()
            );
        }
    }
    
    for (auto& provider : m_providers) {
        if (provider->Exists(resolvedPath)) {
            std::unique_ptr<Asset> asset = provider->Load(resolvedPath);
            if (asset) {
                std::shared_ptr<Asset> shared = std::shared_ptr<Asset>(asset.release());
                m_cache->Put(resolvedPath, shared);
                
                FileAsset* fileAsset = dynamic_cast<FileAsset*>(shared.get());
                if (fileAsset) {
                    size_t size = fileAsset->GetSize();
                    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
                    std::memcpy(data.get(), fileAsset->GetData(), size);
                    return std::make_unique<FileAsset>(
                        fileAsset->GetPath(),
                        std::move(data),
                        size,
                        fileAsset->GetProviderType()
                    );
                }
            }
        }
    }
    
    return nullptr;
}

std::shared_ptr<Asset> AssetManager::LoadAssetShared(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        LOGE("AssetManager::LoadAssetShared: Not initialized");
        return nullptr;
    }
    
    std::string resolvedPath = m_pathResolver->Resolve(path);
    
    std::shared_ptr<Asset> cached = m_cache->Get(resolvedPath);
    if (cached) {
        return cached;
    }
    
    for (auto& provider : m_providers) {
        if (provider->Exists(resolvedPath)) {
            std::unique_ptr<Asset> asset = provider->Load(resolvedPath);
            if (asset) {
                std::shared_ptr<Asset> shared = std::shared_ptr<Asset>(asset.release());
                m_cache->Put(resolvedPath, shared);
                return shared;
            }
        }
    }
    
    return nullptr;
}

std::unique_ptr<FileAsset> AssetManager::LoadTexture(const std::string& path) {
    std::unique_ptr<Asset> asset = LoadAsset(path);
    if (!asset) {
        return nullptr;
    }
    
    FileAsset* fileAsset = dynamic_cast<FileAsset*>(asset.get());
    if (!fileAsset) {
        return nullptr;
    }
    
    return std::unique_ptr<FileAsset>(dynamic_cast<FileAsset*>(asset.release()));
}

std::unique_ptr<FileAsset> AssetManager::LoadAudio(const std::string& path) {
    return LoadTexture(path);
}

std::unique_ptr<FileAsset> AssetManager::LoadScript(const std::string& path) {
    return LoadTexture(path);
}

std::string AssetManager::ResolvePath(const std::string& logicalPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized || !m_pathResolver) {
        return logicalPath;
    }
    
    return m_pathResolver->Resolve(logicalPath);
}

void AssetManager::ClearCache() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_cache) {
        m_cache->Clear();
    }
}

size_t AssetManager::GetCacheSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_cache) {
        return 0;
    }
    
    return m_cache->GetSize();
}

IAssetProvider* AssetManager::FindProvider(const std::string& path) const {
    for (auto& provider : m_providers) {
        if (provider->Exists(path)) {
            return provider.get();
        }
    }
    return nullptr;
}

void AssetManager::RegisterProviders(JavaVM* jvm, jobject activity) {
    m_providers.clear();
    
    auto filesDirProvider = std::make_unique<FilesDirProvider>(jvm, activity);
    auto apkProvider = std::make_unique<APKAssetProvider>(jvm, activity);
    
    m_providers.push_back(std::move(filesDirProvider));
    m_providers.push_back(std::move(apkProvider));
    
    std::sort(m_providers.begin(), m_providers.end(),
              [](const std::unique_ptr<IAssetProvider>& a, const std::unique_ptr<IAssetProvider>& b) {
                  return a->GetPriority() > b->GetPriority();
              });
}

} // namespace storage
} // namespace platform
} // namespace samp
