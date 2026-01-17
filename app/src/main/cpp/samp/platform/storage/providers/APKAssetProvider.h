#pragma once

#include "IAssetProvider.h"
#include <jni.h>

namespace samp {
namespace platform {
namespace storage {

class APKAssetProvider : public IAssetProvider {
public:
    APKAssetProvider(JavaVM* javaVM, jobject activity);
    ~APKAssetProvider() override = default;
    
    std::unique_ptr<Asset> Load(const std::string& path) override;
    bool Exists(const std::string& path) const override;
    std::vector<std::string> List(const std::string& directory) const override;
    AssetProviderType GetType() const override { return AssetProviderType::APK; }
    int GetPriority() const override { return 0; }
    
private:
    bool ValidatePath(const std::string& path) const;
    
    JavaVM* m_javaVM;
    jobject m_activityObject;
};

} // namespace storage
} // namespace platform
} // namespace samp
