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
6. [Asset Initialization](#6-asset-initialization)
7. [JNI Integration](#7-jni-integration)
8. [Android 16 Compatibility](#8-android-16-compatibility)
9. [Complete Replacement of Legacy Systems](#9-complete-replacement-of-legacy-systems)
10. [Integration Guide](#10-integration-guide)
11. [Future Extensibility](#11-future-extensibility)

---

## 1. Overview

### 1.1 Objective

Unified modular asset loading system that:

- Centralizes all file access in the project
- Copies assets from `assets/` (APK) to `data/data/` on first run (initialization)
- Provides unified API for Java and C++ via JNI
- Completely replaces all legacy loading systems
- Modular structure prepared for future extensions (download, import, etc.)
- Strictly follows the modular monolith architecture

### 1.2 Design Principles

| Principle | Description |
|-----------|-------------|
| **Modularity** | Follows modular monolith pattern, integrated via `SAMPCore` |
| **Security** | Uses only `data/data/com.samp.mobile/` (protected by UID) |
| **Unification** | Single entry point for all assets (Java and C++) |
| **Complete Replacement** | No legacy systems will be maintained - immediate and total replacement |
| **Extensibility** | Structure prepared for future extensions (download, import, etc.) |
| **Compatibility** | Compatible with Android 16+ and privacy policies |

### 1.3 Requirements

**Client-Side:**

- Android NDK 26.2+
- C++17
- Java/Kotlin
- Android 16 (API 36)
- Modular Monolith Architecture

**Initialization:**

- Assets from `app/src/main/assets/` (APK)
- Destination: `data/data/com.samp.mobile/files/`
- One-time copy on first run

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
│  │  • LoadScript() • ResolvePath()  • ClearCache()         │     │
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
│  │ SAFProvider  │   │FilesDirProv. │   │APKProvider   │         │
│  │ (SAF URIs)   │   │(data/data/)  │   │(assets/)     │         │
│  │✅ IMPLEMENT  │   │✅ IMPLEMENT  │   │✅ IMPLEMENT  │         │
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
│  │  • loadAsset()  • getBasePath()  • initializeAssets()   │     │
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
    ├── AssetInitialization.h/cpp    # Initialization (APK copy)
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
    │   ├── SAFProvider.h/cpp        # Storage Access Framework ✅
    │   ├── APKAssetProvider.h/cpp   # APK assets (fallback) ✅
    │   ├── FilesDirProvider.h/cpp   # data/data/ ✅
    │   ├── NetworkProvider.h        # Future: interface only ⏳
    │   └── ExternalProvider.h       # Future: interface only ⏳
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
    SAF,        // Storage Access Framework (priority 150) - See SAF_INTEGRATION_SPECIFICATION.md
    FilesDir,   // data/data/com.samp.mobile/files/ (priority 100)
    APK,        // assets/ (priority 50)
    External,   // Future: external storage
    Network     // Future: download
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

1. **SAFProvider** (priority 150)
   - Path: Storage Access Framework URIs (`content://...`)
   - Usage: User-selected files via SAF
   - Access: Via JNI (`SAFStorageManager`) and `DocumentFile`
   - **Complete documentation:** See `SAF_INTEGRATION_SPECIFICATION.md`

2. **FilesDirProvider** (priority 100)
   - Path: `data/data/com.samp.mobile/files/`
   - Usage: Migrated and user-modified assets
   - Access: Via JNI (`AssetStorageManager`)

3. **APKAssetProvider** (priority 50)
   - Path: `assets/` inside APK
   - Usage: Fallback for default game assets
   - Access: Via Android `AssetManager`

**Note:** NetworkProvider and ExternalProvider will be added in the future through the `IAssetProvider` interface. The current system supports SAF, FilesDir and APK.

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

## 6. Asset Initialization

### 6.1 Initialization Process

The `AssetInitialization` service performs the initial copy of assets from APK:

1. **Verification**
   - Checks if assets have already been initialized
   - If yes, skips the copy

2. **Scanning**
   - Scans `assets/` in APK
   - Lists all files to copy

3. **Validation**
   - Checks available disk space
   - Validates APK file integrity

4. **Copy**
   - Copies files from `assets/` (APK) to `data/data/com.samp.mobile/files/`
   - Maintains directory structure
   - Preserves permissions

5. **Verification**
   - Confirms copied files
   - Marks initialization as complete

**Important Note:** There is no migration from `data/android/` because the legacy system will be completely removed. Only APK assets are copied on first run.

### 6.2 AssetInitialization

```cpp
class AssetInitialization {
public:
    struct InitResult {
        bool success;
        size_t filesCopied;
        size_t bytesCopied;
        std::vector<std::string> errors;
    };
    
    // Copy only APK assets (does not copy from legacy system)
    InitResult InitializeFromAPK(JavaVM* jvm, jobject context);
    
private:
    bool ShouldInitialize(); // Check if already initialized
    bool CopyAssetFromAPK(const std::string& assetPath, const std::string& destPath);
    bool ValidateFile(const std::string& path);
};
```

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
    
    // Note: Callbacks for download/import will be added in the future
    
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

## 9. Complete Replacement of Legacy Systems

### 9.1 Principle: No Legacy Systems Will Be Maintained

**Important:** There will be no transition period or compatibility. The legacy system will be completely removed and all usage points will be immediately updated to the new system.

### 9.2 Complete Replacement Table

| Legacy System | Status | New System | Action |
|---------------|--------|------------|--------|
| `g_pszStorage` | ❌ **REMOVE** | `AssetManager::Get()->GetBasePath()` | Completely eliminate global variable |
| `NvFOpen` hook | ❌ **REMOVE** | `AssetManager::LoadAsset()` | Remove hook, use AssetManager directly |
| `NvAPKFileHelper` | ❌ **REMOVE** | `APKAssetProvider` | Remove Java class, refactor to provider |
| `NvEventQueueActivity.loadFile()` | ❌ **REMOVE** | `AssetStorageManager.loadAsset()` | Remove method, use AssetStorageManager |
| `NvEventQueueActivity.loadTexture()` | ❌ **REMOVE** | `AssetStorageManager.loadTexture()` | Remove method, use AssetStorageManager |
| `TextureDatabaseRuntime::Load()` | ❌ **REMOVE** | `AssetManager::LoadTexture()` | Remove legacy system, use TextureLoader |
| `CFileMgr::OpenFile()` | ❌ **REMOVE** | `AssetManager::LoadAsset()` | Refactor to use AssetManager |
| All uses of `g_pszStorage` | ❌ **UPDATE** | `AssetManager::GetBasePath()` | Replace all references |

### 9.3 Direct Replacement Example

**Before (legacy system):**

```cpp
// hooks.cpp - NvFOpen hook (COMPLETELY REMOVE)
stFile* NvFOpen(const char* r0, const char* r1, int r2, int r3) {
    static char path[255];
    sprintf(path, "%s%s", g_pszStorage, r1); // ❌ g_pszStorage no longer exists
    FILE* f = fopen(path, "rb");
    // ...
}

// main.cpp - g_pszStorage (COMPLETELY REMOVE)
char* g_pszStorage = nullptr; // ❌ REMOVE
```

**After (new system):**

```cpp
// Where NvFOpen was used, now use AssetManager directly
auto asset = AssetManager::Get()->LoadAsset(logicalPath);
if (asset) {
    const void* data = asset->GetData();
    size_t size = asset->GetSize();
    // Use data directly - no need for FILE*
}
```

### 9.4 Files That Will Be Completely Removed

- ❌ `app/src/main/cpp/samp/game/hooks.cpp` - `NvFOpen` function (completely remove)
- ❌ `app/src/main/java/com/nvidia/devtech/NvAPKFileHelper.java` (completely remove)
- ❌ `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java` - `loadFile()` and `loadTexture()` methods (remove)
- ❌ `app/src/main/cpp/samp/main.cpp` - `g_pszStorage` variable and `setStoragePath` (remove)
- ❌ `TextureDatabaseRuntime` system (will be replaced by `TextureLoader`)

### 9.5 All Systems Will Be Updated Immediately

There will be no legacy system support. All systems that depend on file loading will be updated in the same implementation to use `AssetManager`.

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
    
    // Initialize assets (APK copy) on first run
    auto initialization = AssetInitialization();
    auto result = initialization.InitializeFromAPK(jvm, context);
    if (!result.success) {
        FLog("Asset initialization failed: %zu files copied", result.filesCopied);
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

The system has been designed to be modular and extensible. Although we are not implementing download, import, modification, and verification features now, the structure allows these functionalities to be added in the future without changing the main public API.

### 11.1 How to Extend the System

**Adding a New Provider:**

1. Create class implementing `IAssetProvider`
2. Register in `AssetManager::RegisterProviders()`
3. System will automatically use when needed (based on priority)

**Future Example (NetworkProvider):**

```cpp
// providers/NetworkProvider.h - Interface only for now
class NetworkProvider : public IAssetProvider {
public:
    NetworkProvider() = default;
    AssetProviderType GetType() const override { return AssetProviderType::Network; }
    int GetPriority() const override { return 25; }
    
    // Methods return nullptr/empty for now
    std::unique_ptr<Asset> Load(const std::string& path) override {
        // TODO: Implement when needed
        return nullptr;
    }
    
    bool Exists(const std::string& path) const override {
        // TODO: Implement when needed
        return false;
    }
    
    std::vector<std::string> List(const std::string& directory) const override {
        // TODO: Implement when needed
        return {};
    }
};
```

**Implemented Extensions:**

- ✅ **Asset import (via SAF - Storage Access Framework)** - See `SAF_INTEGRATION_SPECIFICATION.md`

**Future Extensions (not implemented now):**

- Asset download (via NetworkProvider)
- Asset modification (write to FilesDirProvider)
- Integrity verification (checksums)
- Asset deletion

The modular structure allows adding these functionalities without modifying the `AssetManager` core.

---

**Last update:** January 2026  
**Document version:** 1.0
