#pragma once

#include "../Asset.h"
#include <memory>
#include <string>

namespace samp {
namespace platform {
namespace storage {

class TextureLoader {
public:
    static std::unique_ptr<TextureAsset> Load(const std::string& path);
    static bool Exists(const std::string& path);
};

} // namespace storage
} // namespace platform
} // namespace samp
