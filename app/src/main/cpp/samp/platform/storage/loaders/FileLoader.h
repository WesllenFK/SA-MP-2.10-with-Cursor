#pragma once

#include "../Asset.h"
#include "../AssetManager.h"
#include <memory>
#include <string>

namespace samp {
namespace platform {
namespace storage {

class FileLoader {
public:
    static std::unique_ptr<FileAsset> Load(const std::string& path);
    static bool Exists(const std::string& path);
};

} // namespace storage
} // namespace platform
} // namespace samp
