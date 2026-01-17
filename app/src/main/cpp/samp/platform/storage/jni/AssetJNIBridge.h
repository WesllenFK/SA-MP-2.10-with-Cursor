#pragma once

#include <jni.h>
#include <string>
#include <mutex>

namespace samp {
namespace platform {
namespace storage {

class AssetJNIBridge {
public:
    static AssetJNIBridge& Get();
    
    bool Initialize(JNIEnv* env, jobject activity);
    void Shutdown(JNIEnv* env);
    
    JNIEnv* GetEnv();
    bool FileExists(const std::string& path);
    std::unique_ptr<uint8_t[]> ReadFile(const std::string& path, size_t& outSize);
    std::string GetBasePath();
    
private:
    AssetJNIBridge() = default;
    ~AssetJNIBridge() = default;
    AssetJNIBridge(const AssetJNIBridge&) = delete;
    AssetJNIBridge& operator=(const AssetJNIBridge&) = delete;
    
    JavaVM* m_javaVM;
    jobject m_activityObject;
    jclass m_storageManagerClass;
    jmethodID m_getBasePathMethod;
    jmethodID m_fileExistsMethod;
    jmethodID m_readFileMethod;
    
    std::mutex m_mutex;
    bool m_initialized;
    
    void InitializeMembers() {
        m_javaVM = nullptr;
        m_activityObject = nullptr;
        m_storageManagerClass = nullptr;
        m_getBasePathMethod = nullptr;
        m_fileExistsMethod = nullptr;
        m_readFileMethod = nullptr;
        m_initialized = false;
    }
};

} // namespace storage
} // namespace platform
} // namespace samp
