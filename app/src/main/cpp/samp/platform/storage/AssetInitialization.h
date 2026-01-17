#pragma once

#include <string>
#include <jni.h>

namespace samp {
namespace platform {
namespace storage {

class AssetInitialization {
public:
    static AssetInitialization& Get();
    
    bool InitializeAssets(JNIEnv* env, jobject activity);
    bool IsInitialized(JNIEnv* env, jobject activity) const;
    
private:
    AssetInitialization() = default;
    ~AssetInitialization() = default;
    AssetInitialization(const AssetInitialization&) = delete;
    AssetInitialization& operator=(const AssetInitialization&) = delete;
};

} // namespace storage
} // namespace platform
} // namespace samp
