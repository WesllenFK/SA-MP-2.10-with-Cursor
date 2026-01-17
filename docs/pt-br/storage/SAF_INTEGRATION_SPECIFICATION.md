# Integração Storage Access Framework (SAF) - Especificação Técnica Completa

**Versão**: 1.0  
**Última atualização**: Janeiro 2026  
**Compatibilidade**: Android 16 (API 36), NDK 26.2, Monolito Modular

---

## Índice

1. [Visão Geral](#1-visão-geral)
2. [Arquitetura do Sistema](#2-arquitetura-do-sistema)
3. [Storage Access Framework](#3-storage-access-framework)
4. [Estruturas de Dados](#4-estruturas-de-dados)
5. [SAFProvider](#5-safprovider)
6. [Integração JNI](#6-integração-jni)
7. [SAFStorageManager (Java)](#7-safstoragemanager-java)
8. [Fluxo de Uso](#8-fluxo-de-uso)
9. [Ordem de Busca](#9-ordem-de-busca)
10. [Gerenciamento de URIs](#10-gerenciamento-de-uris)
11. [Compatibilidade Android 16](#11-compatibilidade-android-16)
12. [Considerações de Performance](#12-considerações-de-performance)
13. [Guia de Implementação](#13-guia-de-implementação)
14. [Exemplos de Uso](#14-exemplos-de-uso)
15. [Troubleshooting](#15-troubleshooting)

---

## 1. Visão Geral

### 1.1 Objetivo

Integração do Storage Access Framework (SAF) do Android no sistema de carregamento de assets, permitindo que:

- Usuários selecionem arquivos/pastas de qualquer local (Google Drive, Dropbox, armazenamento local, etc.)
- Código nativo (C++) acesse arquivos selecionados via SAF de forma transparente
- Sistema mantenha compatibilidade com Android 16+ e políticas de privacidade
- Integração seja modular e extensível, seguindo a arquitetura do monolito modular

### 1.2 Princípios de Design

| Princípio | Descrição |
|-----------|-----------|
| **Modularidade** | SAFProvider como provider separado, seguindo padrão `IAssetProvider` |
| **Transparência** | Código nativo usa mesma API, sem saber origem do arquivo |
| **Compatibilidade** | Funciona com Android 16+ e políticas de privacidade |
| **Segurança** | Usa APIs oficiais do Android, sem hacks ou workarounds |
| **Performance** | Cache e otimizações para minimizar overhead do SAF |
| **Extensibilidade** | Estrutura preparada para futuras extensões |

### 1.3 Requisitos

**Client-Side:**

- Android NDK 26.2+
- C++17
- Java/Kotlin
- Android 16 (API 36)
- Arquitetura Monolito Modular
- DocumentFile API (androidx.documentfile.provider)

**SAF:**

- Permissões persistentes via `takePersistableUriPermission()`
- Gerenciamento de URIs via `SAFStorageManager`
- Acesso via `ContentResolver` e `DocumentFile`

---

## 2. Arquitetura do Sistema

### 2.1 Diagrama Geral

```
┌─────────────────────────────────────────────────────────────────┐
│              SISTEMA DE ASSETS COM SAF                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────────────────────────────────────────┐     │
│  │                   AssetManager (Core)                    │     │
│  │  • LoadAsset() busca em ordem de prioridade             │     │
│  │  • ResolvePath() • RegisterSAFUri()                     │     │
│  └─────────────────────────────────────────────────────────┘     │
│                              │                                     │
│          ┌───────────────────┼───────────────────┐                │
│          │                   │                   │                │
│          ▼                   ▼                   ▼                │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐         │
│  │ SAFProvider  │   │FilesDirProv. │   │APKProvider   │         │
│  │ (Prioridade  │   │ (Prioridade  │   │ (Prioridade  │         │
│  │    150)      │   │    100)      │   │     50)      │         │
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

### 2.2 Estrutura de Diretórios

```
platform/storage/
├── providers/
│   ├── SAFProvider.h/cpp          # Provider SAF ✅
│   ├── FilesDirProvider.h/cpp            # Provider FilesDir ✅
│   ├── APKAssetProvider.h/cpp            # Provider APK ✅
│   └── IAssetProvider.h                  # Interface base ✅
│
├── jni/
│   └── AssetJNIBridge.h/cpp              # Estendido com SAF ✅
│
└── ...

java/com/samp/mobile/platform/storage/
├── SAFStorageManager.java                # Gerenciador SAF ✅
└── AssetStorageManager.java              # Gerenciador existente ✅
```

---

## 3. Storage Access Framework

### 3.1 O que é o SAF?

O Storage Access Framework (SAF) é uma API do Android que permite aos usuários:

- Selecionar arquivos/pastas de qualquer provedor de armazenamento
- Acessar Google Drive, Dropbox, armazenamento local, etc.
- Manter permissões persistentes após reiniciar o app
- Funcionar sem permissões de armazenamento explícitas

### 3.2 Como Funciona

1. **Seleção pelo Usuário:**
   - App inicia `Intent.ACTION_OPEN_DOCUMENT` ou `ACTION_OPEN_DOCUMENT_TREE`
   - Sistema mostra picker de arquivos
   - Usuário seleciona arquivo/pasta
   - App recebe URI do arquivo selecionado

2. **Permissões Persistentes:**
   - App chama `takePersistableUriPermission()` na URI
   - Permissão persiste após reiniciar o app
   - URI pode ser salva e reutilizada

3. **Acesso aos Arquivos:**
   - Usa `ContentResolver` e `DocumentFile` para acessar arquivos
   - Não é possível usar `File` ou `fopen()` diretamente
   - Tudo via URIs e streams

### 3.3 Limitações do SAF

| Limitação | Descrição | Solução |
|-----------|-----------|---------|
| **Sem caminhos de arquivo** | Não há caminho físico do arquivo | Usar URIs e `DocumentFile` |
| **Sem fopen() direto** | Não pode usar `FILE*` padrão | Ler via `ContentResolver` e copiar para buffer |
| **Performance** | Mais lento que arquivos locais | Implementar cache para arquivos frequentes |
| **Thread Safety** | `ContentResolver` não é thread-safe | Usar mutex e sincronização |

---

## 4. Estruturas de Dados

### 4.1 AssetProviderType (Atualizado)

```cpp
// Asset.h
enum class AssetProviderType {
    SAF,        // Storage Access Framework (prioridade 150)
    FilesDir,   // data/data/com.samp.mobile/files/ (prioridade 100)
    APK,        // assets/ (prioridade 50)
    Network,    // Futuro: download
    External    // Futuro: storage externo
};
```

### 4.2 SAFUriMapping

```cpp
// SAFProvider.h
struct SAFUriMapping {
    std::string logicalPath;    // Caminho lógico: "samp/custom/main.scm"
    std::string uriString;      // URI SAF: "content://..."
    std::string displayName;    // Nome do arquivo para exibição
    time_t registeredAt;        // Timestamp de registro
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

### 5.1 Implementação

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

## 6. Integração JNI

### 6.1 AssetJNIBridge (Estendido)

```cpp
// jni/AssetJNIBridge.h - Adicionar métodos SAF
namespace samp {
namespace platform {
namespace storage {

class AssetJNIBridge {
public:
    // ... métodos existentes ...
    
    // SAF methods
    std::unique_ptr<uint8_t[]> ReadSAFFile(const std::string& uriString, size_t& outSize);
    bool SAFFileExists(const std::string& uriString);
    std::vector<std::string> ListSAFDirectory(const std::string& uriString);
    std::string GetSAFDisplayName(const std::string& uriString);
    
private:
    // ... campos existentes ...
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

### 6.2 Implementação JNI

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

### 7.1 Classe Principal

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
            // URI não suporta permissões persistentes
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
        // Salvar em SharedPreferences ou banco de dados
        // Implementação específica do projeto
    }
    
    private void loadPersistedUris() {
        // Carregar URIs persistidas ao inicializar
        // Implementação específica do projeto
    }
}
```

---

## 8. Fluxo de Uso

### 8.1 Seleção de Arquivo pelo Usuário

```java
// Na Activity/Fragment
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

### 8.2 Uso no Código Nativo

```cpp
// O código nativo usa normalmente, AssetManager resolve automaticamente
auto asset = AssetManager::Get()->LoadAsset("samp/custom/main.scm");
if (asset) {
    const uint8_t* data = asset->GetData();
    size_t size = asset->GetSize();
    // Usar dados normalmente - código não sabe que veio do SAF
}
```

---

## 9. Ordem de Busca

### 9.1 Prioridades dos Providers

O `AssetManager` busca assets na seguinte ordem (maior prioridade primeiro):

| Provider | Prioridade | Descrição |
|----------|------------|-----------|
| **SAFProvider** | 150 | Arquivos selecionados pelo usuário via SAF |
| **FilesDirProvider** | 100 | `data/data/com.samp.mobile/files/` |
| **APKAssetProvider** | 50 | Assets do APK (fallback) |

### 9.2 Algoritmo de Busca

```cpp
// AssetManager::LoadAsset()
std::unique_ptr<Asset> AssetManager::LoadAsset(const std::string& path) {
    // 1. Verificar cache primeiro
    if (auto cached = m_cache->Get(path)) {
        return cached;
    }
    
    // 2. Buscar em ordem de prioridade (maior primeiro)
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

## 10. Gerenciamento de URIs

### 10.1 Persistência de URIs

URIs do SAF devem ser persistidas para manter acesso após reiniciar o app:

```java
// Persistir URI
private void persistUri(String logicalPath, Uri uri) {
    SharedPreferences prefs = context.getSharedPreferences(
        "saf_uris", Context.MODE_PRIVATE);
    prefs.edit()
        .putString(logicalPath, uri.toString())
        .apply();
}

// Carregar URIs persistidas
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

### 10.2 Validação de URIs

```java
// Verificar se URI ainda é válida
public boolean isUriValid(String uriString) {
    Uri uri = Uri.parse(uriString);
    DocumentFile docFile = DocumentFile.fromSingleUri(context, uri);
    return docFile != null && docFile.exists();
}
```

---

## 11. Compatibilidade Android 16

### 11.1 Políticas de Privacidade

- **Sem acesso direto a `/data/`**: SAF usa URIs, não caminhos físicos
- **Storage Scoped**: Respeita escopo de armazenamento do Android 14+
- **Permissões**: Não requer permissões de armazenamento explícitas
- **Persistência**: Permissões persistentes via `takePersistableUriPermission()`

### 11.2 Caminhos Seguros

| Tipo | Caminho | Acesso | SAF |
|------|---------|--------|-----|
| **SAF** | URI `content://...` | ✅ Via `DocumentFile` | ✅ |
| **App Private** | `data/data/com.samp.mobile/files/` | ✅ Via `Context.getFilesDir()` | ❌ |
| **APK Assets** | `assets/` | ✅ Via `AssetManager` | ❌ |

---

## 12. Considerações de Performance

### 12.1 Overhead do SAF

| Operação | Overhead | Solução |
|----------|----------|---------|
| **Leitura de arquivo** | ~2-3x mais lento que local | Cache em memória para arquivos frequentes |
| **Listagem de diretório** | ~5-10x mais lento | Cache de listagens |
| **Verificação de existência** | ~2x mais lento | Cache de metadados |

### 12.2 Otimizações

```cpp
// Cache de arquivos SAF frequentes
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

## 13. Guia de Implementação

### 13.1 Passo 1: Atualizar AssetProviderType

```cpp
// Asset.h
enum class AssetProviderType {
    SAF,        // Adicionar
    FilesDir,
    APK,
    Network,
    External
};
```

### 13.2 Passo 2: Criar SAFProvider

1. Criar `providers/SAFProvider.h` e `SAFProvider.cpp`
2. Implementar interface `IAssetProvider`
3. Adicionar métodos específicos do SAF

### 13.3 Passo 3: Estender AssetJNIBridge

1. Adicionar métodos SAF em `AssetJNIBridge.h`
2. Implementar métodos em `AssetJNIBridge.cpp`
3. Inicializar `jmethodID` no `Initialize()`

### 13.4 Passo 4: Criar SAFStorageManager (Java)

1. Criar `SAFStorageManager.java`
2. Implementar métodos de acesso ao SAF
3. Implementar persistência de URIs

### 13.5 Passo 5: Registrar SAFProvider no AssetManager

```cpp
// AssetManager.cpp
void AssetManager::RegisterProviders(JavaVM* jvm, jobject context) {
    // Registrar SAFProvider primeiro (maior prioridade)
    m_providers.push_back(std::make_unique<SAFProvider>(jvm, context));
    
    // Registrar outros providers
    m_providers.push_back(std::make_unique<FilesDirProvider>(jvm, context));
    m_providers.push_back(std::make_unique<APKAssetProvider>(jvm, context));
    
    // Ordenar por prioridade (maior primeiro)
    std::sort(m_providers.begin(), m_providers.end(),
        [](const auto& a, const auto& b) {
            return a->GetPriority() > b->GetPriority();
        });
}
```

### 13.6 Passo 6: Adicionar UI para Seleção de Arquivos

1. Adicionar botões na UI para abrir file picker
2. Implementar `onActivityResult()` para processar URIs
3. Registrar URIs no `SAFProvider`

---

## 14. Exemplos de Uso

### 14.1 Exemplo 1: Carregar Arquivo Selecionado pelo Usuário

```cpp
// Código nativo
auto asset = AssetManager::Get()->LoadAsset("samp/custom/main.scm");
if (asset && asset->GetProviderType() == AssetProviderType::SAF) {
    FLog("Arquivo carregado do SAF: %s", asset->GetPath().c_str());
    const uint8_t* data = asset->GetData();
    size_t size = asset->GetSize();
    // Processar dados
}
```

### 14.2 Exemplo 2: Listar Arquivos em Pasta SAF

```cpp
// Código nativo
auto files = AssetManager::Get()->ListAssets("samp/custom/");
for (const auto& file : files) {
    FLog("Arquivo encontrado: %s", file.c_str());
}
```

### 14.3 Exemplo 3: Verificar se Arquivo Existe no SAF

```cpp
// Código nativo
if (AssetManager::Get()->AssetExists("samp/custom/main.scm")) {
    FLog("Arquivo existe no SAF");
}
```

---

## 15. Troubleshooting

### 15.1 Problemas Comuns

| Problema | Causa | Solução |
|----------|-------|---------|
| **URI inválida após reiniciar** | Permissão não persistida | Verificar `takePersistableUriPermission()` |
| **Arquivo não encontrado** | URI não registrada | Verificar registro no `SAFProvider` |
| **Performance lenta** | Muitas chamadas ao SAF | Implementar cache |
| **Crash ao acessar URI** | Thread safety | Usar mutex e `GetJNIEnvSafe()` |

### 15.2 Debug

```cpp
// Habilitar logs detalhados
#define SAF_DEBUG 1

#if SAF_DEBUG
    FLog("[SAFProvider] Loading: %s", path.c_str());
    FLog("[SAFProvider] URI: %s", uriString.c_str());
    FLog("[SAFProvider] Size: %zu", size);
#endif
```

### 15.3 Validação

```java
// Validar URI antes de usar
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

**Última atualização:** Janeiro 2026  
**Versão do documento:** 1.0
