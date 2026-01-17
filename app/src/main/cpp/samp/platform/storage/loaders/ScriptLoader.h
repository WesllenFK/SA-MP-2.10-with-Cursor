#pragma once

#include "../Asset.h"
#include <memory>
#include <string>

namespace samp {
namespace platform {
namespace storage {

class ScriptLoader {
public:
    static std::unique_ptr<ScriptAsset> Load(const std::string& path);
    static bool Exists(const std::string& path);
};

} // namespace storage
} // namespace platform
} // namespace samp
