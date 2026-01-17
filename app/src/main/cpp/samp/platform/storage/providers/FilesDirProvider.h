#pragma once

#include "IAssetProvider.h"
#include "../jni/AssetJNIBridge.h"
#include <jni.h>

namespace samp {
namespace platform {
namespace storage {

class FilesDirProvider : public IAssetProvider {
public:
    FilesDirProvider(JavaVM* javaVM, jobject activity);
    ~FilesDirProvider() override = default;
    
    std::unique_ptr<Asset> Load(const std::string& path) override;
    bool Exists(const std::string& path) const override;
    std::vector<std::string> List(const std::string& directory) const override;
    AssetProviderType GetType() const override { return AssetProviderType::FilesDir; }
    int GetPriority() const override { return 1; }
    
private:
    bool ValidatePath(const std::string& path) const;
    
    JavaVM* m_javaVM;
    jobject m_activityObject;
};

} // namespace storage
} // namespace platform
} // namespace samp
