#pragma once

#include "../Asset.h"
#include <memory>
#include <string>
#include <vector>

namespace samp {
namespace platform {
namespace storage {

class Asset;

class IAssetProvider {
public:
    virtual ~IAssetProvider() = default;

    virtual std::unique_ptr<Asset> Load(const std::string& path) = 0;
    virtual bool Exists(const std::string& path) const = 0;
    virtual std::vector<std::string> List(const std::string& directory) const = 0;
    virtual AssetProviderType GetType() const = 0;
    virtual int GetPriority() const = 0;

protected:
    IAssetProvider() = default;
    IAssetProvider(const IAssetProvider&) = delete;
    IAssetProvider& operator=(const IAssetProvider&) = delete;
    IAssetProvider(IAssetProvider&&) = default;
    IAssetProvider& operator=(IAssetProvider&&) = default;
};

} // namespace storage
} // namespace platform
} // namespace samp
