#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace samp {
namespace platform {
namespace storage {

enum class AssetProviderType {
    FilesDir,
    APK,
    Network,
    External
};

class Asset {
public:
    virtual ~Asset() = default;

    virtual const std::string& GetPath() const = 0;
    virtual size_t GetSize() const = 0;
    virtual const uint8_t* GetData() const = 0;
    virtual AssetProviderType GetProviderType() const = 0;

protected:
    Asset() = default;
    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;
    Asset(Asset&&) = default;
    Asset& operator=(Asset&&) = default;
};

class FileAsset : public Asset {
public:
    FileAsset(std::string path, std::unique_ptr<uint8_t[]> data, size_t size, AssetProviderType provider)
        : m_path(std::move(path))
        , m_data(std::move(data))
        , m_size(size)
        , m_providerType(provider)
    {}

    const std::string& GetPath() const override { return m_path; }
    size_t GetSize() const override { return m_size; }
    const uint8_t* GetData() const override { return m_data.get(); }
    AssetProviderType GetProviderType() const override { return m_providerType; }

private:
    std::string m_path;
    std::unique_ptr<uint8_t[]> m_data;
    size_t m_size;
    AssetProviderType m_providerType;
};

class TextureAsset : public FileAsset {
public:
    TextureAsset(std::string path, std::unique_ptr<uint8_t[]> data, size_t size, AssetProviderType provider,
                 int width, int height, int format)
        : FileAsset(std::move(path), std::move(data), size, provider)
        , m_width(width)
        , m_height(height)
        , m_format(format)
    {}

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetFormat() const { return m_format; }

private:
    int m_width;
    int m_height;
    int m_format;
};

class AudioAsset : public FileAsset {
public:
    AudioAsset(std::string path, std::unique_ptr<uint8_t[]> data, size_t size, AssetProviderType provider,
               int sampleRate, int channels, int bitDepth)
        : FileAsset(std::move(path), std::move(data), size, provider)
        , m_sampleRate(sampleRate)
        , m_channels(channels)
        , m_bitDepth(bitDepth)
    {}

    int GetSampleRate() const { return m_sampleRate; }
    int GetChannels() const { return m_channels; }
    int GetBitDepth() const { return m_bitDepth; }

private:
    int m_sampleRate;
    int m_channels;
    int m_bitDepth;
};

class ScriptAsset : public FileAsset {
public:
    ScriptAsset(std::string path, std::unique_ptr<uint8_t[]> data, size_t size, AssetProviderType provider)
        : FileAsset(std::move(path), std::move(data), size, provider)
    {}
};

} // namespace storage
} // namespace platform
} // namespace samp
