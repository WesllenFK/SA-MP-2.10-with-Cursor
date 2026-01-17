# Asset Loading System - Complete Technical Specification

**Version**: 1.0  
**Last update**: January 2026  
**Compatibility**: Android 16 (API 36), NDK 26.2, Modular Monolith

---

## Table of Contents

1. [Overview](#1-overview)
2. [System Architecture](#2-system-architecture)
3. [Data Structures](#3-data-structures)
4. [Asset Providers](#4-asset-providers)
5. [Loading System](#5-loading-system)
6. [Asset Migration](#6-asset-migration)
7. [JNI Integration](#7-jni-integration)
8. [Android 16 Compatibility](#8-android-16-compatibility)
9. [Replacement of Legacy Systems](#9-replacement-of-legacy-systems)
10. [Integration Guide](#10-integration-guide)
11. [Future Extensibility](#11-future-extensibility)

---

## 1. Overview

### 1.1 Objective

Unified modular asset loading system that:
- Centralizes all file access in the project
- Migrates assets from `assets/` (APK) and `data/android/` to `data/data/` (more secure)
- Provides unified API for Java and C++ via JNI
- Extensible for download, import, modification, and deletion of assets
- Strictly follows the modular monolith architecture

### 1.2 Design Principles

| Principle | Description |
|-----------|-------------|
| **Modularity** | Follows modular monolith pattern, integrated via `SAMPCore` |
| **Security** | Uses only `data/data/com.samp.mobile/` (protected by UID) |
| **Unification** | Single entry point for all assets (Java and C++) |
| **Extensibility** | Prepared for future systems (download, import, mod, delete, check) |
| **Compatibility** | Compatible with Android 16+ and privacy policies |

### 1.3 Requirements

**Client-Side:**
- Android NDK 26.2+
- C++17
- Java/Kotlin
- Android 16 (API 36)
- Modular Monolith Architecture

**Migration:**
- Assets from `app/src/main/assets/`
- Data from `data/android/com.samp.mobile/files/`
- Destination: `data/data/com.samp.mobile/files/`

---

## 2. System Architecture

### 2.1 General Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        ASSET SYSTEM                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │                   AssetManager (Core)                    │     │
│  │  • LoadAsset()  • LoadTexture()  • LoadAudio()          │     │
│  │  • Download()   • Import()       • Modify()             │     │
│  │  • Delete()     • Check()        • ResolvePath()        │     │
│  └─────────────────────────────────────────────────────────┘     │
│                              │                                     │
│          ┌───────────────────┼───────────────────┐                │
│          │                   │                   │                │
│          ▼                   ▼                   ▼                │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │PathResolver  │   │AssetCache    │   │AssetLoader   │         │
│  │              │   │              │   │              │         │
│  │ • Resolve()  │   │ • Get()      │   │ • LoadFile() │         │
│  │ • Map()      │   │ • Put()      │   │ • LoadTex()  │         │
│  └──────────────┘   │ • Clear()    │   │ • LoadAud()  │         │
│                     └──────────────┘   └──────────────┘         │
│                              │                                     │
│          ┌───────────────────┼───────────────────┐                │
│          │                   │                   │                │
│          ▼                   ▼                   ▼                │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │FilesDirProv. │   │APKProvider   │   │NetworkProv.  │         │
│  │(data/data/)  │   │(assets/)     │   │(future)      │         │
│  └──────────────┘   └──────────────┘   └──────────────┘         │
│                              │                                     │
│                              ▼                                     │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │              AssetJNIBridge (JNI)                        │     │
│  │  • FileExists()  • ReadFile()  • WriteFile()            │     │
│  └─────────────────────────────────────────────────────────┘     │
│                              │                                     │
│                              ▼                                     │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │          AssetStorageManager (Java/Kotlin)               │     │
│  │  • loadAsset()  • getBasePath()  • migrateAssets()      │     │
│  └─────────────────────────────────────────────────────────┘     │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Directory Structure

```
platform/
└── storage/
    ├── AssetManager.h/cpp           # Main unified system
    ├── AssetLoader.h/cpp            # Specialized loaders
    ├── AssetPathResolver.h/cpp      # Path resolution
    ├── AssetCache.h/cpp             # Cache and validation
    ├── AssetMigration.h/cpp         # Migration of old assets
    │
    ├── loaders/
    │   ├── FileLoader.h/cpp         # Generic files
    │   ├── TextureLoader.h/cpp      # Textures
    │   ├── AudioLoader.h/cpp        # Audio
    │   ├── ScriptLoader.h/cpp       # SCM/IMG scripts
    │   └── DataLoader.h/cpp         # Game data
    │
    ├── providers/
    │   ├── IAssetProvider.h         # Base interface
    │   ├── APKAssetProvider.h/cpp   # APK assets (fallback)
    │   ├── FilesDirProvider.h/cpp   # data/data/
    │   ├── ExternalProvider.h/cpp   # Future: external assets
    │   └── NetworkProvider.h/cpp    # Future: download
    │
    └── jni/
        └── AssetJNIBridge.h/cpp     # JNI bridge
```

---

## 3. Data Structures

### 3.1 Asset (Base Abstract)

```cpp
namespace platform::storage {

class Asset {
public:
    virtual ~Asset() = default;
    
    const std::string& GetPath() const { return m_path; }
    size_t GetSize() const { return m_size; }
    const void* GetData() const { return m_data.get(); }
    AssetProviderType GetProvider() const { return m_provider; }
    
protected:
    std::string m_path;
    size_t m_size;
    std::unique_ptr<uint8_t[]> m_data;
    AssetProviderType m_provider;
};

}
```

### 3.2 Specialized Assets

```cpp
// Textures
class TextureAsset : public Asset {
public:
    RwTexture* GetTexture() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
private:
    RwTexture* m_texture;
    int m_width, m_height;
};

// Audio
class AudioAsset : public Asset {
public:
    const void* GetAudioData() const { return GetData(); }
    int GetSampleRate() const { return m_sampleRate; }
    int GetChannels() const { return m_channels; }
    
private:
    int m_sampleRate;
    int m_channels;
};

// Scripts
class ScriptAsset : public Asset {
public:
    const void* GetScriptData() const { return GetData(); }
    size_t GetScriptSize() const { return GetSize(); }
};
```

### 3.3 AssetProviderType

```cpp
enum class AssetProviderType {
    FilesDir,      // data/data/com.samp.mobile/files/ (priority)
    APK,           // assets/ (fallback)
    External,      // Future: external storage
    Network        // Future: download
};
```

---

## 4. Asset Providers

### 4.1 Base Interface

```cpp
class IAssetProvider {
public:
    virtual ~IAssetProvider() = default;
    virtual AssetProviderType GetType() const = 0;
    virtual std::unique_ptr<Asset> Load(const std::string& path) = 0;
    virtual bool Exists(const std::string& path) const = 0;
    virtual std::vector<std::string> List(const std::string& directory) const = 0;
    virtual int GetPriority() const = 0;
};
```

### 4.2 Search Order

Provider search order follows priority:

1. **FilesDirProvider** (priority 100)
   - Path: `data/data/com.samp.mobile/files/`
   - Usage: Migrated and user-modified assets
   - Access: Via JNI (`AssetStorageManager`)

2. **APKAssetProvider** (priority 50)
   - Path: `assets/` inside APK
   - Usage: Fallback for default game assets
   - Access: Via Android `AssetManager`

3. **NetworkProvider** (priority 25) - Future
   - Usage: Download assets from server

4. **ExternalProvider** (priority 10) - Future
   - Usage: Externally imported assets

---

## 5. Loading System

### 5.1 AssetManager

```cpp
namespace platform::storage {

class AssetManager {
public:
    // Singleton via SAMPCore
    static AssetManager* Get();
    
    // Initialization (called in Bootstrap)
    bool Initialize(JavaVM* jvm, jobject context);
    void Shutdown();
    
    // Main loading API
    std::unique_ptr<Asset> LoadAsset(const std::string& path);
    std::shared_ptr<Asset> LoadAssetShared(const std::string& path);
    
    // Specialized loading
    std::unique_ptr<TextureAsset> LoadTexture(const std::string& path);
    std::unique_ptr<AudioAsset> LoadAudio(const std::string& path);
    std::unique_ptr<ScriptAsset> LoadScript(const std::string& path);
    
    // Future operations
    bool DownloadAsset(const std::string& url, const std::string& path);
    bool ImportAsset(const std::string& sourcePath, const std::string& destPath);
    bool ModifyAsset(const std::string& path, const void* data, size_t size);
    bool DeleteAsset(const std::string& path);
    bool CheckAsset(const std::string& path, AssetChecksum& checksum);
    
    // Path resolution
    std::string ResolvePath(const std::string& logicalPath);
    
    // Cache
    void ClearCache();
    size_t GetCacheSize() const;
    
    // Base path
    const std::string& GetBasePath() const { return m_basePath; }
    
private:
    std::vector<std::unique_ptr<IAssetProvider>> m_providers;
    std::unique_ptr<AssetCache> m_cache;
    std::unique_ptr<AssetPathResolver> m_pathResolver;
    JavaVM* m_jvm;
    jobject m_context;
    std::string m_basePath;
    
    IAssetProvider* FindProvider(const std::string& path) const;
    void RegisterProviders(JavaVM* jvm, jobject context);
};

}
```

### 5.2 Path Mappings

**Main Mappings:**

| GTASA Path | Asset Path |
|------------|------------|
| `DATA/GTA.DAT` | `SAMP/gta.dat` |
| `mainV1.scm` | `SAMP/main.scm` |
| `SCRIPTV1.IMG` | `SAMP/script.img` |
| `DATA/PEDS.IDE` | `SAMP/peds.ide` |
| `DATA/VEHICLES.IDE` | `SAMP/vehicles.ide` |
| `DATA/HANDLING.CFG` | `SAMP/handling.cfg` |
| `DATA/WEAPON.DAT` | `SAMP/weapon.dat` |
| `DATA/FONTS.DAT` | `data/fonts.dat` |
| `DATA/PEDSTATS.DAT` | `data/pedstats.dat` |
| `DATA/TIMECYC.DAT` | `data/timecyc.dat` |
| `DATA/POPCYCLE.DAT` | `data/popcycle.dat` |

---

## 6. Asset Migration

### 6.1 Migration Process

The `AssetMigration` service performs migration in stages:

1. **Detection**
   - Scans `assets/` in APK
   - Scans `data/android/com.samp.mobile/files/` (if exists)
   - Lists all files to migrate

2. **Validation**
   - Checks available disk space
   - Validates file integrity
   - Creates migration list

3. **Copy**
   - Copies files to `data/data/com.samp.mobile/files/`
   - Maintains directory structure
   - Preserves permissions

4. **Verification**
   - Validates checksums
   - Confirms copied files
   - Updates metadata

5. **Cleanup** (optional)
   - Removes old files from `data/android/` (if desired)

---

## 7. JNI Integration

### 7.1 AssetJNIBridge

```cpp
namespace platform::storage::jni {

class AssetJNIBridge {
public:
    static void Initialize(JNIEnv* env, jobject context);
    static void Shutdown();
    
    // Calls from C++ to Java
    static bool FileExists(const std::string& path);
    static std::vector<uint8_t> ReadFile(const std::string& path);
    static bool WriteFile(const std::string& path, const void* data, size_t size);
    static bool DeleteFile(const std::string& path);
    static std::vector<std::string> ListFiles(const std::string& directory);
    static std::string GetBasePath();
    
    // Calls from Java to C++
    static void OnAssetDownloaded(const std::string& path);
    static void OnAssetImported(const std::string& path);
    
private:
    static JavaVM* s_jvm;
    static jobject s_context;
    // ...
    
    static JNIEnv* GetEnv();
};

}
```

---

## 8. Android 16 Compatibility

### 8.1 Privacy Policies

- **No direct access to `/data/`**: Use only `data/data/com.samp.mobile/`
- **Storage Scoped**: Respect storage scope of Android 14+
- **Permissions**: Does not require external storage permissions

### 8.2 Safe Paths

| Type | Path | Access |
|------|------|--------|
| **App Private** | `data/data/com.samp.mobile/files/` | ✅ Via `Context.getFilesDir()` |
| **APK Assets** | `assets/` | ✅ Via `AssetManager` |
| **External (Old)** | `data/android/com.samp.mobile/files/` | ❌ Deprecated |
| **System Root** | `/data/` | ❌ Prohibited in Android 16+ |

---

## 9. Replacement of Legacy Systems

### 9.1 Mapping Table

| Legacy System | New System | Notes |
|---------------|------------|-------|
| `g_pszStorage` | `AssetManager::Get()->GetBasePath()` | Remove global variable |
| `NvFOpen` hook | `AssetManager::LoadAsset()` | Replace hook with direct call |
| `NvAPKFileHelper` | `APKAssetProvider` | Refactor to provider |
| `TextureDatabaseRuntime::Load()` | `AssetManager::LoadTexture()` | Wrapper in `TextureLoader` |
| `CFileMgr::OpenFile()` | `AssetManager::LoadAsset()` | Use AssetManager internally |
| `loadFile()` Java | `AssetStorageManager.loadAsset()` | Unify Java API |
| `loadTexture()` Java | `AssetStorageManager.loadTexture()` | Specialize |

---

## 10. Integration Guide

### 10.1 Initialization in Bootstrap

```cpp
// core/bootstrap/Bootstrap.cpp
#include "platform/storage/AssetManager.h"

bool Bootstrap::Initialize() {
    // ... other initializations
    
    // Initialize AssetManager
    JavaVM* jvm = GetJVM();
    jobject context = GetContext();
    
    if (!platform::storage::AssetManager::Get()->Initialize(jvm, context)) {
        FLog("Failed to initialize AssetManager");
        return false;
    }
    
    // Migrate assets on first run
    auto migration = AssetMigration();
    auto result = migration.MigrateAll(jvm, context);
    if (!result.success) {
        FLog("Asset migration failed: %zu files migrated", result.filesMigrated);
    }
    
    return true;
}
```

### 10.2 Usage in Modules

```cpp
// In any project module
#include "core/services/ServiceLocator.h"
#include "platform/storage/AssetManager.h"

// Load texture
auto texture = ServiceLocator::Assets()->LoadTexture("samp/icon");
if (texture) {
    RwTexture* tex = texture->GetTexture();
    // Use texture
}

// Load script
auto script = ServiceLocator::Assets()->LoadScript("SAMP/main.scm");
if (script) {
    const void* data = script->GetScriptData();
    size_t size = script->GetScriptSize();
    // Process script
}
```

---

## 11. Future Extensibility

### 11.1 Asset Download

```cpp
// Future: NetworkProvider
class NetworkProvider : public IAssetProvider {
public:
    bool DownloadAsset(const std::string& url, const std::string& localPath) {
        // Implement HTTP/HTTPS download
        // Save to FilesDirProvider
        // Notify via JNI callback
    }
};
```

### 11.2 Asset Import

```cpp
// Future: Import via SAF (Storage Access Framework)
bool AssetManager::ImportAsset(const std::string& sourceUri) {
    // Open file selector via JNI
    // Copy to FilesDirProvider
    // Validate integrity
}
```

### 11.3 Asset Modification

```cpp
// Future: Asset editing
bool AssetManager::ModifyAsset(const std::string& path, const void* data, size_t size) {
    // Validate write permission
    // Invalidate cache
    // Save to FilesDirProvider
}
```

### 11.4 Integrity Verification

```cpp
// Future: Checksums and validation
struct AssetChecksum {
    std::string md5;
    std::string sha256;
    size_t size;
    time_t modified;
};

bool AssetManager::CheckAsset(const std::string& path, AssetChecksum& checksum) {
    // Calculate checksums
    // Compare with database
    // Return integrity status
}
```

---

**Last update:** January 2026  
**Document version:** 1.0
