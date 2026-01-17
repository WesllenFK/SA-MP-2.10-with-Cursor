# Storage Access Framework (SAF) Integration - Complete Technical Specification

**Version**: 1.0  
**Last update**: January 2026  
**Compatibility**: Android 16 (API 36), NDK 26.2, Modular Monolith

---

## Table of Contents

1. [Overview](#1-overview)
2. [System Architecture](#2-system-architecture)
3. [Storage Access Framework](#3-storage-access-framework)
4. [Data Structures](#4-data-structures)
5. [SAFProvider](#5-safprovider)
6. [JNI Integration](#6-jni-integration)
7. [SAFStorageManager (Java)](#7-safstoragemanager-java)
8. [Usage Flow](#8-usage-flow)
9. [Search Order](#9-search-order)
10. [URI Management](#10-uri-management)
11. [Android 16 Compatibility](#11-android-16-compatibility)
12. [Performance Considerations](#12-performance-considerations)
13. [Implementation Guide](#13-implementation-guide)
14. [Usage Examples](#14-usage-examples)
15. [Troubleshooting](#15-troubleshooting)

---

## 1. Overview

### 1.1 Objective

Integration of Android's Storage Access Framework (SAF) into the asset loading system, enabling:

- Users to select files/folders from any location (Google Drive, Dropbox, local storage, etc.)
- Native code (C++) to access SAF-selected files transparently
- System to maintain compatibility with Android 16+ and privacy policies
- Integration to be modular and extensible, following the modular monolith architecture

### 1.2 Design Principles

| Principle | Description |
|-----------|-------------|
| **Modularity** | SAFProvider as separate provider, following `IAssetProvider` pattern |
| **Transparency** | Native code uses same API, unaware of file origin |
| **Compatibility** | Works with Android 16+ and privacy policies |
| **Security** | Uses official Android APIs, no hacks or workarounds |
| **Performance** | Cache and optimizations to minimize SAF overhead |
| **Extensibility** | Structure prepared for future extensions |

### 1.3 Requirements

**Client-Side:**

- Android NDK 26.2+
- C++17
- Java/Kotlin
- Android 16 (API 36)
- Modular Monolith Architecture
- DocumentFile API (androidx.documentfile.provider)

**SAF:**

- Persistent permissions via `takePersistableUriPermission()`
- URI management via `SAFStorageManager`
- Access via `ContentResolver` and `DocumentFile`

---

## 2. System Architecture

### 2.1 General Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│              ASSET SYSTEM WITH SAF                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │                   AssetManager (Core)                    │     │
│  │  • LoadAsset() searches by priority order                │     │
│  │  • ResolvePath() • RegisterSAFUri()                     │     │
│  └─────────────────────────────────────────────────────────┘     │
│                              │                                     │
│          ┌───────────────────┼───────────────────┐                │
│          │                   │                   │                │
│          ▼                   ▼                   ▼                │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │ SAFProvider  │   │FilesDirProv. │   │APKProvider   │         │
│  │ (Priority    │   │ (Priority    │   │ (Priority     │         │
│  │    150)      │   │    100)      │   │     50)       │         │
│  └─────┬────────┘   └──────────────┘   └──────────────┘         │
│        │                                                          │
│        ▼                                                          │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │              AssetJNIBridge (JNI)                        │     │
│  │  • ReadSAFFile()  • SAFFileExists()                    │     │
│  │  • ListSAFDirectory()  • GetSAFDisplayName()          │     │
│  └─────┬───────────────────────────────────────────────────┘     │
│        │                                                          │
│        ▼                                                          │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │      SAFStorageManager (Java/Kotlin)                     │     │
│  │  • registerUri()  • readFile()                          │     │
│  │  • fileExists()  • listDirectory()                     │     │
│  │  • takePersistableUriPermission()                      │     │
│  └─────────────────────────────────────────────────────────┘     │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Directory Structure

```
platform/storage/
├── providers/
│   ├── SAFProvider.h/cpp          # SAF Provider ✅
│   ├── FilesDirProvider.h/cpp      # FilesDir Provider ✅
│   ├── APKAssetProvider.h/cpp      # APK Provider ✅
│   └── IAssetProvider.h            # Base interface ✅
│
├── jni/
│   └── AssetJNIBridge.h/cpp        # Extended with SAF ✅
│
└── ...

java/com/samp/mobile/platform/storage/
├── SAFStorageManager.java          # SAF Manager ✅
└── AssetStorageManager.java        # Existing manager ✅
```

---

## 3. Storage Access Framework

### 3.1 What is SAF?

The Storage Access Framework (SAF) is an Android API that allows users to:

- Select files/folders from any storage provider
- Access Google Drive, Dropbox, local storage, etc.
- Maintain persistent permissions after app restart
- Work without explicit storage permissions

### 3.2 How It Works

1. **User Selection:**
   - App launches `Intent.ACTION_OPEN_DOCUMENT` or `ACTION_OPEN_DOCUMENT_TREE`
   - System shows file picker
   - User selects file/folder
   - App receives URI of selected file

2. **Persistent Permissions:**
   - App calls `takePersistableUriPermission()` on URI
   - Permission persists after app restart
   - URI can be saved and reused

3. **File Access:**
   - Uses `ContentResolver` and `DocumentFile` to access files
   - Cannot use `File` or `fopen()` directly
   - Everything via URIs and streams

### 3.3 SAF Limitations

| Limitation | Description | Solution |
|------------|-------------|----------|
| **No file paths** | No physical file path | Use URIs and `DocumentFile` |
| **No direct fopen()** | Cannot use standard `FILE*` | Read via `ContentResolver` and copy to buffer |
| **Performance** | Slower than local files | Implement cache for frequent files |
| **Thread Safety** | `ContentResolver` is not thread-safe | Use mutex and synchronization |

---

## 4. Data Structures

### 4.1 AssetProviderType (Updated)

```cpp
// Asset.h
enum class AssetProviderType {
    SAF,        // Storage Access Framework (priority 150)
    FilesDir,   // data/data/com.samp.mobile/files/ (priority 100)
    APK,        // assets/ (priority 50)
    Network,    // Future: download
    External    // Future: external storage
};
```

### 4.2 SAFUriMapping

```cpp
// SAFProvider.h
struct SAFUriMapping {
    std::string logicalPath;    // Logical path: "samp/custom/main.scm"
    std::string uriString;      // SAF URI: "content://..."
    std::string displayName;    // File name for display
    time_t registeredAt;        // Registration timestamp
};
```

### 4.3 SAFProvider Interface

```cpp
// providers/SAFProvider.h
namespace samp {
namespace platform {
namespace storage {

class SAFProvider : public IAssetProvider {
public:
    SAFProvider(JavaVM* jvm, jobject context);
    ~SAFProvider() = default;

    // IAssetProvider interface
    std::unique_ptr<Asset> Load(const std::string& path) override;
    bool Exists(const std::string& path) const override;
    std::vector<std::string> List(const std::string& directory) const override;
    AssetProviderType GetType() const override { return AssetProviderType::SAF; }
    int GetPriority() const override { return 150; }

    // SAF-specific methods
    bool RegisterSAFUri(const std::string& logicalPath, const std::string& uriString);
    void UnregisterSAFUri(const std::string& logicalPath);
    bool IsRegistered(const std::string& logicalPath) const;
    std::vector<std::string> GetRegisteredPaths() const;
    void ClearAllMappings();

private:
    JavaVM* m_jvm;
    jobject m_context;
    
    std::string ResolveSAFUri(const std::string& logicalPath) const;
    std::unique_ptr<uint8_t[]> ReadFromSAF(const std::string& uriString, size_t& outSize) const;
    bool SAFExists(const std::string& uriString) const;
    std::vector<std::string> ListSAFDirectory(const std::string& uriString) const;
    
    std::unordered_map<std::string, std::string> m_uriMappings;
    mutable std::mutex m_mutex;
};

} // namespace storage
} // namespace platform
} // namespace samp
```

---

## 5. SAFProvider

### 5.1 Implementation

```cpp
// providers/SAFProvider.cpp
#include "SAFProvider.h"
#include "../jni/AssetJNIBridge.h"
#include "../Asset.h"

namespace samp {
namespace platform {
namespace storage {

SAFProvider::SAFProvider(JavaVM* jvm, jobject context)
    : m_jvm(jvm)
    , m_context(context)
{
}

std::unique_ptr<Asset> SAFProvider::Load(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string uriString = ResolveSAFUri(path);
    if (uriString.empty()) {
        return nullptr;
    }
    
    size_t size = 0;
    auto data = ReadFromSAF(uriString, size);
    if (!data || size == 0) {
        return nullptr;
    }
    
    return std::make_unique<FileAsset>(
        path,
        std::move(data),
        size,
        AssetProviderType::SAF
    );
}

bool SAFProvider::Exists(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string uriString = ResolveSAFUri(path);
    if (uriString.empty()) {
        return false;
    }
    
    return SAFExists(uriString);
}

std::vector<std::string> SAFProvider::List(const std::string& directory) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string uriString = ResolveSAFUri(directory);
    if (uriString.empty()) {
        return {};
    }
    
    return ListSAFDirectory(uriString);
}

bool SAFProvider::RegisterSAFUri(const std::string& logicalPath, const std::string& uriString) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (logicalPath.empty() || uriString.empty()) {
        return false;
    }
    
    m_uriMappings[logicalPath] = uriString;
    return true;
}

void SAFProvider::UnregisterSAFUri(const std::string& logicalPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_uriMappings.erase(logicalPath);
}

std::string SAFProvider::ResolveSAFUri(const std::string& logicalPath) const {
    auto it = m_uriMappings.find(logicalPath);
    if (it != m_uriMappings.end()) {
        return it->second;
    }
    
    for (const auto& pair : m_uriMappings) {
        if (logicalPath.find(pair.first) == 0) {
            std::string relativePath = logicalPath.substr(pair.first.length());
            if (!relativePath.empty() && relativePath[0] == '/') {
                relativePath = relativePath.substr(1);
            }
            return pair.second + "/" + relativePath;
        }
    }
    
    return "";
}

std::unique_ptr<uint8_t[]> SAFProvider::ReadFromSAF(const std::string& uriString, size_t& outSize) const {
    return AssetJNIBridge::Get().ReadSAFFile(uriString, outSize);
}

bool SAFProvider::SAFExists(const std::string& uriString) const {
    return AssetJNIBridge::Get().SAFFileExists(uriString);
}

std::vector<std::string> SAFProvider::ListSAFDirectory(const std::string& uriString) const {
    return AssetJNIBridge::Get().ListSAFDirectory(uriString);
}

} // namespace storage
} // namespace platform
} // namespace samp
```

---

## 6. JNI Integration

### 6.1 AssetJNIBridge (Extended)

```cpp
// jni/AssetJNIBridge.h - Add SAF methods
namespace samp {
namespace platform {
namespace storage {

class AssetJNIBridge {
public:
    // ... existing methods ...
    
    // SAF methods
    std::unique_ptr<uint8_t[]> ReadSAFFile(const std::string& uriString, size_t& outSize);
    bool SAFFileExists(const std::string& uriString);
    std::vector<std::string> ListSAFDirectory(const std::string& uriString);
    std::string GetSAFDisplayName(const std::string& uriString);
    
private:
    // ... existing fields ...
    jclass m_safStorageManagerClass;
    jmethodID m_readSAFFileMethod;
    jmethodID m_safFileExistsMethod;
    jmethodID m_listSAFDirectoryMethod;
    jmethodID m_getSAFDisplayNameMethod;
};

} // namespace storage
} // namespace platform
} // namespace samp
```

### 6.2 JNI Implementation

```cpp
// jni/AssetJNIBridge.cpp
std::unique_ptr<uint8_t[]> AssetJNIBridge::ReadSAFFile(const std::string& uriString, size_t& outSize) {
    JNIEnv* env = GetEnv();
    if (!env || !m_safStorageManagerClass) {
        return nullptr;
    }
    
    jstring jUriString = env->NewStringUTF(uriString.c_str());
    jbyteArray jData = (jbyteArray)env->CallStaticObjectMethod(
        m_safStorageManagerClass,
        m_readSAFFileMethod,
        jUriString
    );
    
    if (!jData) {
        env->DeleteLocalRef(jUriString);
        return nullptr;
    }
    
    jsize length = env->GetArrayLength(jData);
    auto data = std::make_unique<uint8_t[]>(length);
    env->GetByteArrayRegion(jData, 0, length, reinterpret_cast<jbyte*>(data.get()));
    
    outSize = length;
    
    env->DeleteLocalRef(jData);
    env->DeleteLocalRef(jUriString);
    
    return data;
}

bool AssetJNIBridge::SAFFileExists(const std::string& uriString) {
    JNIEnv* env = GetEnv();
    if (!env || !m_safStorageManagerClass) {
        return false;
    }
    
    jstring jUriString = env->NewStringUTF(uriString.c_str());
    jboolean result = env->CallStaticBooleanMethod(
        m_safStorageManagerClass,
        m_safFileExistsMethod,
        jUriString
    );
    
    env->DeleteLocalRef(jUriString);
    
    return result == JNI_TRUE;
}
```

---

## 7. SAFStorageManager (Java)

### 7.1 Main Class

```java
// SAFStorageManager.java
package com.samp.mobile.platform.storage;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.DocumentsContract;
import androidx.documentfile.provider.DocumentFile;
import java.io.InputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class SAFStorageManager {
    private static SAFStorageManager instance;
    private Context context;
    private Map<String, Uri> uriMappings;
    
    public static SAFStorageManager getInstance(Context context) {
        if (instance == null) {
            instance = new SAFStorageManager(context);
        }
        return instance;
    }
    
    private SAFStorageManager(Context context) {
        this.context = context;
        this.uriMappings = new HashMap<>();
        loadPersistedUris();
    }
    
    public void registerUri(String logicalPath, Uri uri) {
        try {
            context.getContentResolver().takePersistableUriPermission(
                uri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION |
                Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            );
            uriMappings.put(logicalPath, uri);
            persistUri(logicalPath, uri);
        } catch (SecurityException e) {
            // URI does not support persistent permissions
        }
    }
    
    public byte[] readFile(String uriString) throws IOException {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
        
        if (docFile == null || !docFile.exists() || !docFile.canRead()) {
            return null;
        }
        
        InputStream is = context.getContentResolver().openInputStream(uri);
        if (is == null) {
            return null;
        }
        
        long fileSize = docFile.length();
        if (fileSize > Integer.MAX_VALUE) {
            is.close();
            throw new IOException("File too large");
        }
        
        byte[] buffer = new byte[(int) fileSize];
        int bytesRead = is.read(buffer);
        is.close();
        
        if (bytesRead != fileSize) {
            throw new IOException("Failed to read complete file");
        }
        
        return buffer;
    }
    
    public boolean fileExists(String uriString) {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
        return docFile != null && docFile.exists() && docFile.isFile();
    }
    
    public String[] listDirectory(String uriString) {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromTreeUri(context, uri);
        
        if (docFile == null || !docFile.exists() || !docFile.isDirectory()) {
            return new String[0];
        }
        
        DocumentFile[] files = docFile.listFiles();
        String[] names = new String[files.length];
        for (int i = 0; i < files.length; i++) {
            names[i] = files[i].getName();
        }
        return names;
    }
    
    public String getDisplayName(String uriString) {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
        return docFile != null && docFile.getName() != null 
            ? docFile.getName() 
            : "";
    }
    
    private void persistUri(String logicalPath, Uri uri) {
        // Save to SharedPreferences or database
        // Project-specific implementation
    }
    
    private void loadPersistedUris() {
        // Load persisted URIs on initialization
        // Project-specific implementation
    }
}
```

---

## 8. Usage Flow

### 8.1 User File Selection

```java
// In Activity/Fragment
private static final int REQUEST_CODE_OPEN_DOCUMENT = 1001;
private static final int REQUEST_CODE_OPEN_DOCUMENT_TREE = 1002;

public void openFilePicker() {
    Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
    intent.addCategory(Intent.CATEGORY_OPENABLE);
    intent.setType("*/*");
    startActivityForResult(intent, REQUEST_CODE_OPEN_DOCUMENT);
}

public void openFolderPicker() {
    Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
    startActivityForResult(intent, REQUEST_CODE_OPEN_DOCUMENT_TREE);
}

@Override
protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    
    if (resultCode == RESULT_OK && data != null) {
        Uri uri = data.getData();
        
        if (requestCode == REQUEST_CODE_OPEN_DOCUMENT) {
            String fileName = getFileName(uri);
            String logicalPath = "samp/custom/" + fileName;
            
            SAFStorageManager.getInstance(this).registerUri(logicalPath, uri);
            
            AssetManager.get().registerSAFUri(logicalPath, uri.toString());
        } else if (requestCode == REQUEST_CODE_OPEN_DOCUMENT_TREE) {
            String logicalPath = "samp/custom/";
            
            SAFStorageManager.getInstance(this).registerUri(logicalPath, uri);
            
            AssetManager.get().registerSAFUri(logicalPath, uri.toString());
        }
    }
}

private String getFileName(Uri uri) {
    String result = null;
    if (uri.getScheme().equals("content")) {
        try (Cursor cursor = getContentResolver().query(
            uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int nameIndex = cursor.getColumnIndex(
                    OpenableColumns.DISPLAY_NAME);
                if (nameIndex >= 0) {
                    result = cursor.getString(nameIndex);
                }
            }
        }
    }
    if (result == null) {
        result = uri.getPath();
        int cut = result.lastIndexOf('/');
        if (cut != -1) {
            result = result.substring(cut + 1);
        }
    }
    return result;
}
```

### 8.2 Native Code Usage

```cpp
// Native code uses normally, AssetManager resolves automatically
auto asset = AssetManager::Get()->LoadAsset("samp/custom/main.scm");
if (asset) {
    const uint8_t* data = asset->GetData();
    size_t size = asset->GetSize();
    // Use data normally - code doesn't know it came from SAF
}
```

---

## 9. Search Order

### 9.1 Provider Priorities

The `AssetManager` searches for assets in the following order (highest priority first):

| Provider | Priority | Description |
|----------|----------|-------------|
| **SAFProvider** | 150 | User-selected files via SAF |
| **FilesDirProvider** | 100 | `data/data/com.samp.mobile/files/` |
| **APKAssetProvider** | 50 | APK assets (fallback) |

### 9.2 Search Algorithm

```cpp
// AssetManager::LoadAsset()
std::unique_ptr<Asset> AssetManager::LoadAsset(const std::string& path) {
    // 1. Check cache first
    if (auto cached = m_cache->Get(path)) {
        return cached;
    }
    
    // 2. Search by priority order (highest first)
    for (auto& provider : m_providers) {
        if (provider->Exists(path)) {
            auto asset = provider->Load(path);
            if (asset) {
                m_cache->Put(path, asset);
                return asset;
            }
        }
    }
    
    return nullptr;
}
```

---

## 10. URI Management

### 10.1 URI Persistence

SAF URIs must be persisted to maintain access after app restart:

```java
// Persist URI
private void persistUri(String logicalPath, Uri uri) {
    SharedPreferences prefs = context.getSharedPreferences(
        "saf_uris", Context.MODE_PRIVATE);
    prefs.edit()
        .putString(logicalPath, uri.toString())
        .apply();
}

// Load persisted URIs
private void loadPersistedUris() {
    SharedPreferences prefs = context.getSharedPreferences(
        "saf_uris", Context.MODE_PRIVATE);
    Map<String, ?> all = prefs.getAll();
    
    for (Map.Entry<String, ?> entry : all.entrySet()) {
        String logicalPath = entry.getKey();
        String uriString = entry.getValue().toString();
        Uri uri = Uri.parse(uriString);
        
        uriMappings.put(logicalPath, uri);
        AssetManager.get().registerSAFUri(logicalPath, uriString);
    }
}
```

### 10.2 URI Validation

```java
// Validate URI before use
public boolean validateUri(String uriString) {
    try {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
        return docFile != null && docFile.exists();
    } catch (Exception e) {
        return false;
    }
}
```

---

## 11. Android 16 Compatibility

### 11.1 Privacy Policies

- **No direct access to `/data/`**: SAF uses URIs, not physical paths
- **Storage Scoped**: Respects Android 14+ storage scope
- **Permissions**: Does not require explicit storage permissions
- **Persistence**: Persistent permissions via `takePersistableUriPermission()`

### 11.2 Safe Paths

| Type | Path | Access | SAF |
|------|------|--------|-----|
| **SAF** | URI `content://...` | ✅ Via `DocumentFile` | ✅ |
| **App Private** | `data/data/com.samp.mobile/files/` | ✅ Via `Context.getFilesDir()` | ❌ |
| **APK Assets** | `assets/` | ✅ Via `AssetManager` | ❌ |

---

## 12. Performance Considerations

### 12.1 SAF Overhead

| Operation | Overhead | Solution |
|-----------|----------|----------|
| **File read** | ~2-3x slower than local | In-memory cache for frequent files |
| **Directory listing** | ~5-10x slower | Cache listings |
| **Existence check** | ~2x slower | Cache metadata |

### 12.2 Optimizations

```cpp
// Cache for frequent SAF files
class SAFCache {
    void Put(const std::string& uri, std::shared_ptr<Asset> asset);
    std::shared_ptr<Asset> Get(const std::string& uri);
    void Invalidate(const std::string& uri);
    
private:
    std::unordered_map<std::string, std::weak_ptr<Asset>> m_cache;
    std::mutex m_mutex;
    size_t m_maxSize;
};
```

---

## 13. Implementation Guide

### 13.1 Step 1: Update AssetProviderType

```cpp
// Asset.h
enum class AssetProviderType {
    SAF,        // Add
    FilesDir,
    APK,
    Network,
    External
};
```

### 13.2 Step 2: Create SAFProvider

1. Create `providers/SAFProvider.h` and `SAFProvider.cpp`
2. Implement `IAssetProvider` interface
3. Add SAF-specific methods

### 13.3 Step 3: Extend AssetJNIBridge

1. Add SAF methods to `AssetJNIBridge.h`
2. Implement methods in `AssetJNIBridge.cpp`
3. Initialize `jmethodID` in `Initialize()`

### 13.4 Step 4: Create SAFStorageManager (Java)

1. Create `SAFStorageManager.java`
2. Implement SAF access methods
3. Implement URI persistence

### 13.5 Step 5: Register SAFProvider in AssetManager

```cpp
// AssetManager.cpp
void AssetManager::RegisterProviders(JavaVM* jvm, jobject context) {
    // Register SAFProvider first (highest priority)
    m_providers.push_back(std::make_unique<SAFProvider>(jvm, context));
    
    // Register other providers
    m_providers.push_back(std::make_unique<FilesDirProvider>(jvm, context));
    m_providers.push_back(std::make_unique<APKAssetProvider>(jvm, context));
    
    // Sort by priority (highest first)
    std::sort(m_providers.begin(), m_providers.end(),
        [](const auto& a, const auto& b) {
            return a->GetPriority() > b->GetPriority();
        });
}
```

### 13.6 Step 6: Add UI for File Selection

1. Add buttons in UI to open file picker
2. Implement `onActivityResult()` to process URIs
3. Register URIs in `SAFProvider`

---

## 14. Usage Examples

### 14.1 Example 1: Load User-Selected File

```cpp
// Native code
auto asset = AssetManager::Get()->LoadAsset("samp/custom/main.scm");
if (asset && asset->GetProviderType() == AssetProviderType::SAF) {
    FLog("File loaded from SAF: %s", asset->GetPath().c_str());
    const uint8_t* data = asset->GetData();
    size_t size = asset->GetSize();
    // Process data
}
```

### 14.2 Example 2: List Files in SAF Folder

```cpp
// Native code
auto files = AssetManager::Get()->ListAssets("samp/custom/");
for (const auto& file : files) {
    FLog("File found: %s", file.c_str());
}
```

### 14.3 Example 3: Check if File Exists in SAF

```cpp
// Native code
if (AssetManager::Get()->AssetExists("samp/custom/main.scm")) {
    FLog("File exists in SAF");
}
```

---

## 15. Troubleshooting

### 15.1 Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| **Invalid URI after restart** | Permission not persisted | Check `takePersistableUriPermission()` |
| **File not found** | URI not registered | Check registration in `SAFProvider` |
| **Slow performance** | Too many SAF calls | Implement cache |
| **Crash when accessing URI** | Thread safety | Use mutex and `GetJNIEnvSafe()` |

### 15.2 Debug

```cpp
// Enable detailed logs
#define SAF_DEBUG 1

#if SAF_DEBUG
    FLog("[SAFProvider] Loading: %s", path.c_str());
    FLog("[SAFProvider] URI: %s", uriString.c_str());
    FLog("[SAFProvider] Size: %zu", size);
#endif
```

### 15.3 Validation

```java
// Validate URI before use
public boolean validateUri(String uriString) {
    try {
        Uri uri = Uri.parse(uriString);
        DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
        return docFile != null && docFile.exists();
    } catch (Exception e) {
        return false;
    }
}
```

---

**Last update:** January 2026  
**Document version:** 1.0
