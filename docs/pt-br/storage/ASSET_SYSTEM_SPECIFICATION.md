# Sistema de Carregamento de Assets - Especificação Técnica Completa

**Versão**: 1.0  
**Última atualização**: Janeiro 2026  
**Compatibilidade**: Android 16 (API 36), NDK 26.2, Monolito Modular

---

## Índice

1. [Visão Geral](#1-visão-geral)
2. [Arquitetura do Sistema](#2-arquitetura-do-sistema)
3. [Estruturas de Dados](#3-estruturas-de-dados)
4. [Providers de Assets](#4-providers-de-assets)
5. [Sistema de Carregamento](#5-sistema-de-carregamento)
6. [Inicialização de Assets](#6-inicialização-de-assets)
7. [Integração JNI](#7-integração-jni)
8. [Compatibilidade Android 16](#8-compatibilidade-android-16)
9. [Substituição Completa do Sistema Antigo](#9-substituição-completa-do-sistema-antigo)
10. [Guia de Integração](#10-guia-de-integração)
11. [Extensibilidade Futura](#11-extensibilidade-futura)

---

## 1. Visão Geral

### 1.1 Objetivo

Sistema modular unificado de carregamento de assets que:

- Centraliza todo o acesso a arquivos do projeto
- Copia assets de `assets/` (APK) para `data/data/` na primeira execução (inicialização)
- Fornece API única para Java e C++ via JNI
- Substitui completamente todos os sistemas antigos de carregamento
- Estrutura modular preparada para extensões futuras (download, import, etc.)
- Segue estritamente a arquitetura do monolito modular

### 1.2 Princípios de Design

| Princípio | Descrição |
|-----------|-----------|
| **Modularidade** | Segue padrão do monolito modular, integrado via `SAMPCore` |
| **Segurança** | Usa apenas `data/data/com.samp.mobile/` (protegido por UID) |
| **Unificação** | Um único ponto de entrada para todos os assets (Java e C++) |
| **Substituição Completa** | Nenhum sistema antigo será mantido - substituição total e imediata |
| **Extensibilidade** | Estrutura preparada para extensões futuras (download, import, etc.) |
| **Compatibilidade** | Compatível com Android 16+ e políticas de privacidade |

### 1.3 Requisitos

**Client-Side:**

- Android NDK 26.2+
- C++17
- Java/Kotlin
- Android 16 (API 36)
- Arquitetura Monolito Modular

**Inicialização:**

- Assets de `app/src/main/assets/` (APK)
- Destino: `data/data/com.samp.mobile/files/`
- Cópia única na primeira execução

---

## 2. Arquitetura do Sistema

### 2.1 Diagrama Geral

```
┌─────────────────────────────────────────────────────────────────┐
│                    SISTEMA DE ASSETS                              │
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
│  │✅ IMPLEMENTAR│   │✅ IMPLEMENTAR│   │✅ IMPLEMENTAR│         │
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

### 2.2 Estrutura de Diretórios

```
platform/
└── storage/
    ├── AssetManager.h/cpp           # Sistema principal unificado
    ├── AssetLoader.h/cpp            # Carregadores especializados
    ├── AssetPathResolver.h/cpp      # Resolução de caminhos
    ├── AssetCache.h/cpp             # Cache e validação
    ├── AssetInitialization.h/cpp    # Inicialização (cópia do APK)
    │
    ├── loaders/
    │   ├── FileLoader.h/cpp         # Arquivos genéricos
    │   ├── TextureLoader.h/cpp      # Texturas
    │   ├── AudioLoader.h/cpp        # Áudio
    │   ├── ScriptLoader.h/cpp       # Scripts SCM/IMG
    │   └── DataLoader.h/cpp         # Dados do jogo
    │
    ├── providers/
    │   ├── IAssetProvider.h         # Interface base
    │   ├── SAFProvider.h/cpp        # Storage Access Framework ✅
    │   ├── APKAssetProvider.h/cpp   # Assets do APK (fallback) ✅
    │   ├── FilesDirProvider.h/cpp   # data/data/ ✅
    │   ├── NetworkProvider.h        # Futuro: interface apenas ⏳
    │   └── ExternalProvider.h       # Futuro: interface apenas ⏳
    │
    └── jni/
        └── AssetJNIBridge.h/cpp     # Ponte JNI
```

---

## 3. Estruturas de Dados

### 3.1 Asset (Base Abstrata)

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

### 3.2 Asset Especializados

```cpp
// Texturas
class TextureAsset : public Asset {
public:
    RwTexture* GetTexture() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
private:
    RwTexture* m_texture;
    int m_width, m_height;
};

// Áudio
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
    SAF,        // Storage Access Framework (prioridade 150) - Ver SAF_INTEGRATION_SPECIFICATION.md
    FilesDir,   // data/data/com.samp.mobile/files/ (prioridade 100)
    APK,        // assets/ (prioridade 50)
    External,   // Futuro: storage externo
    Network     // Futuro: download
};
```

---

## 4. Providers de Assets

### 4.1 Interface Base

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

### 4.2 Ordem de Busca

A ordem de busca dos providers segue a prioridade:

1. **SAFProvider** (prioridade 150)
   - Caminho: URIs do Storage Access Framework (`content://...`)
   - Uso: Arquivos selecionados pelo usuário via SAF
   - Acesso: Via JNI (`SAFStorageManager`) e `DocumentFile`
   - **Documentação completa:** Ver `SAF_INTEGRATION_SPECIFICATION.md`

2. **FilesDirProvider** (prioridade 100)
   - Caminho: `data/data/com.samp.mobile/files/`
   - Uso: Assets migrados e modificados pelo usuário
   - Acesso: Via JNI (`AssetStorageManager`)

3. **APKAssetProvider** (prioridade 50)
   - Caminho: `assets/` dentro do APK
   - Uso: Fallback para assets padrão do jogo
   - Acesso: Via `AssetManager` do Android

**Nota:** NetworkProvider e ExternalProvider serão adicionados futuramente através da interface `IAssetProvider`. O sistema atual suporta SAF, FilesDir e APK.

### 4.3 FilesDirProvider

```cpp
class FilesDirProvider : public IAssetProvider {
public:
    FilesDirProvider(JavaVM* jvm, jobject context);
    AssetProviderType GetType() const override { return AssetProviderType::FilesDir; }
    int GetPriority() const override { return 100; }
    
    std::unique_ptr<Asset> Load(const std::string& path) override;
    bool Exists(const std::string& path) const override;
    std::vector<std::string> List(const std::string& directory) const override;
    
private:
    JavaVM* m_jvm;
    jobject m_context;
    
    std::string ResolveFullPath(const std::string& logicalPath) const;
};
```

### 4.4 APKAssetProvider

```cpp
class APKAssetProvider : public IAssetProvider {
public:
    APKAssetProvider(JavaVM* jvm, jobject context);
    AssetProviderType GetType() const override { return AssetProviderType::APK; }
    int GetPriority() const override { return 50; }
    
    std::unique_ptr<Asset> Load(const std::string& path) override;
    bool Exists(const std::string& path) const override;
    std::vector<std::string> List(const std::string& directory) const override;
    
private:
    JavaVM* m_jvm;
    jobject m_context;
};
```

---

## 5. Sistema de Carregamento

### 5.1 AssetManager

```cpp
namespace platform::storage {

class AssetManager {
public:
    // Singleton via SAMPCore
    static AssetManager* Get();
    
    // Inicialização (chamado no Bootstrap)
    bool Initialize(JavaVM* jvm, jobject context);
    void Shutdown();
    
    // API principal de carregamento
    std::unique_ptr<Asset> LoadAsset(const std::string& path);
    std::shared_ptr<Asset> LoadAssetShared(const std::string& path);
    
    // Carregamento especializado
    std::unique_ptr<TextureAsset> LoadTexture(const std::string& path);
    std::unique_ptr<AudioAsset> LoadAudio(const std::string& path);
    std::unique_ptr<ScriptAsset> LoadScript(const std::string& path);
    
    // Resolução de caminhos
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

### 5.2 AssetPathResolver

```cpp
class AssetPathResolver {
public:
    struct ResolvedPath {
        std::string physicalPath;
        AssetProviderType provider;
        bool exists;
    };
    
    ResolvedPath Resolve(const std::string& logicalPath);
    
    // Mapeamentos de caminhos do GTASA
    void RegisterMapping(const std::string& gamePath, const std::string& assetPath);
    
    // Caminhos conhecidos
    void RegisterKnownPaths();
    
private:
    // Mapeamentos
    // "DATA/GTA.DAT" -> "SAMP/gta.dat"
    // "mainV1.scm" -> "SAMP/main.scm"
    // "DATA/PEDS.IDE" -> "SAMP/peds.ide"
    std::unordered_map<std::string, std::string> m_pathMappings;
};
```

**Mapeamentos Principais:**

| Caminho GTASA | Caminho Asset |
|---------------|---------------|
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

### 5.3 AssetCache

```cpp
class AssetCache {
public:
    void Put(const std::string& path, std::shared_ptr<Asset> asset);
    std::shared_ptr<Asset> Get(const std::string& path);
    void Remove(const std::string& path);
    void Clear();
    size_t GetSize() const;
    
private:
    std::unordered_map<std::string, std::weak_ptr<Asset>> m_cache;
    std::mutex m_mutex;
    size_t m_totalSize;
};
```

---

## 6. Inicialização de Assets

### 6.1 Processo de Inicialização

O `AssetInitialization` service realiza a cópia inicial dos assets do APK:

1. **Verificação**
   - Verifica se os assets já foram inicializados
   - Se sim, pula a cópia

2. **Escaneamento**
   - Escaneia `assets/` no APK
   - Lista todos os arquivos a copiar

3. **Validação**
   - Verifica espaço em disco disponível
   - Valida integridade dos arquivos do APK

4. **Cópia**
   - Copia arquivos de `assets/` (APK) para `data/data/com.samp.mobile/files/`
   - Mantém estrutura de diretórios
   - Preserva permissões

5. **Verificação**
   - Confirma arquivos copiados
   - Marca inicialização como completa

**Nota Importante:** Não há migração de `data/android/` porque o sistema antigo será completamente removido. Apenas assets do APK são copiados na primeira execução.

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
    
    // Copia apenas assets do APK (não copia de sistema antigo)
    InitResult InitializeFromAPK(JavaVM* jvm, jobject context);
    
private:
    bool ShouldInitialize(); // Verifica se já foi inicializado
    bool CopyAssetFromAPK(const std::string& assetPath, const std::string& destPath);
    bool ValidateFile(const std::string& path);
};
```

---

## 7. Integração JNI

### 7.1 AssetJNIBridge

```cpp
namespace platform::storage::jni {

class AssetJNIBridge {
public:
    static void Initialize(JNIEnv* env, jobject context);
    static void Shutdown();
    
    // Chamadas de C++ para Java
    static bool FileExists(const std::string& path);
    static std::vector<uint8_t> ReadFile(const std::string& path);
    static bool WriteFile(const std::string& path, const void* data, size_t size);
    static bool DeleteFile(const std::string& path);
    static std::vector<std::string> ListFiles(const std::string& directory);
    static std::string GetBasePath();
    
    // Nota: Callbacks para download/import serão adicionados futuramente
    
private:
    static JavaVM* s_jvm;
    static jobject s_context;
    static jclass s_assetManagerClass;
    static jmethodID s_fileExistsMethod;
    static jmethodID s_readFileMethod;
    static jmethodID s_writeFileMethod;
    // ...
    
    static JNIEnv* GetEnv();
};

}
```

### 7.2 AssetStorageManager (Java)

```java
package com.samp.mobile.platform.storage;

public class AssetStorageManager {
    private static AssetStorageManager instance;
    private Context context;
    private String basePath;
    
    public static AssetStorageManager getInstance(Context context) {
        if (instance == null) {
            instance = new AssetStorageManager(context);
        }
        return instance;
    }
    
    public AssetStorageManager(Context context) {
        this.context = context;
        this.basePath = context.getFilesDir().getAbsolutePath() + "/";
    }
    
    public String getBasePath() {
        return basePath;
    }
    
    public boolean fileExists(String path) {
        File file = new File(basePath + path);
        return file.exists() && file.isFile();
    }
    
    public byte[] readFile(String path) throws IOException {
        File file = new File(basePath + path);
        if (!file.exists()) {
            return null;
        }
        
        FileInputStream fis = new FileInputStream(file);
        byte[] data = new byte[(int) file.length()];
        fis.read(data);
        fis.close();
        return data;
    }
    
    public boolean writeFile(String path, byte[] data) {
        File file = new File(basePath + path);
        file.getParentFile().mkdirs();
        
        try {
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(data);
            fos.close();
            return true;
        } catch (IOException e) {
            return false;
        }
    }
    
    public InitResult initializeAssets() {
        // Implementação da inicialização (cópia do APK)
    }
}
```

---

## 8. Compatibilidade Android 16

### 8.1 Políticas de Privacidade

- **Sem acesso a `/data/` diretamente**: Usar apenas `data/data/com.samp.mobile/`
- **Storage Scoped**: Respeitar escopo de armazenamento do Android 14+
- **Permissões**: Não requer permissões de armazenamento externo

### 8.2 Caminhos Seguros

| Tipo | Caminho | Acesso |
|------|---------|--------|
| **App Private** | `data/data/com.samp.mobile/files/` | ✅ Via `Context.getFilesDir()` |
| **APK Assets** | `assets/` | ✅ Via `AssetManager` |
| **External (Antigo)** | `data/android/com.samp.mobile/files/` | ❌ Deprecado |
| **System Root** | `/data/` | ❌ Proibido no Android 16+ |

### 8.3 Validações

```cpp
// Validação de caminho seguro
bool AssetManager::IsPathSafe(const std::string& path) {
    // Não permitir caminhos absolutos fora do app
    if (path.find("/data/") == 0 || path.find("../") != std::string::npos) {
        return false;
    }
    
    // Permitir apenas caminhos relativos dentro do app
    return true;
}
```

---

## 9. Substituição Completa do Sistema Antigo

### 9.1 Princípio: Nenhum Sistema Antigo Será Mantido

**Importante:** Não haverá período de transição ou compatibilidade. O sistema antigo será completamente removido e todos os pontos de uso serão atualizados imediatamente para o novo sistema.

### 9.2 Tabela de Substituição Completa

| Sistema Antigo | Status | Novo Sistema | Ação |
|----------------|--------|--------------|------|
| `g_pszStorage` | ❌ **REMOVER** | `AssetManager::Get()->GetBasePath()` | Eliminar variável global completamente |
| `NvFOpen` hook | ❌ **REMOVER** | `AssetManager::LoadAsset()` | Remover hook, usar AssetManager diretamente |
| `NvAPKFileHelper` | ❌ **REMOVER** | `APKAssetProvider` | Remover classe Java, refatorar para provider |
| `NvEventQueueActivity.loadFile()` | ❌ **REMOVER** | `AssetStorageManager.loadAsset()` | Remover método, usar AssetStorageManager |
| `NvEventQueueActivity.loadTexture()` | ❌ **REMOVER** | `AssetStorageManager.loadTexture()` | Remover método, usar AssetStorageManager |
| `TextureDatabaseRuntime::Load()` | ❌ **REMOVER** | `AssetManager::LoadTexture()` | Remover sistema antigo, usar TextureLoader |
| `CFileMgr::OpenFile()` | ❌ **REMOVER** | `AssetManager::LoadAsset()` | Refatorar para usar AssetManager |
| Todos os usos de `g_pszStorage` | ❌ **ATUALIZAR** | `AssetManager::GetBasePath()` | Substituir todas as referências |

### 9.3 Exemplo de Substituição Direta

**Antes (sistema antigo):**

```cpp
// hooks.cpp - NvFOpen hook (REMOVER COMPLETAMENTE)
stFile* NvFOpen(const char* r0, const char* r1, int r2, int r3) {
    static char path[255];
    sprintf(path, "%s%s", g_pszStorage, r1); // ❌ g_pszStorage não existe mais
    FILE* f = fopen(path, "rb");
    // ...
}

// main.cpp - g_pszStorage (REMOVER COMPLETAMENTE)
char* g_pszStorage = nullptr; // ❌ REMOVER
```

**Depois (novo sistema):**

```cpp
// Onde antes usava NvFOpen, agora usa AssetManager diretamente
auto asset = AssetManager::Get()->LoadAsset(logicalPath);
if (asset) {
    const void* data = asset->GetData();
    size_t size = asset->GetSize();
    // Usar dados diretamente - não precisa mais de FILE*
}
```

### 9.4 Arquivos Que Serão Removidos Completamente

- ❌ `app/src/main/cpp/samp/game/hooks.cpp` - Função `NvFOpen` (remover completamente)
- ❌ `app/src/main/java/com/nvidia/devtech/NvAPKFileHelper.java` (remover completamente)
- ❌ `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java` - Métodos `loadFile()` e `loadTexture()` (remover)
- ❌ `app/src/main/cpp/samp/main.cpp` - Variável `g_pszStorage` e `setStoragePath` (remover)
- ❌ Sistema `TextureDatabaseRuntime` (será substituído por `TextureLoader`)

### 9.5 Todos os Sistemas Serão Atualizados Imediatamente

Não haverá suporte ao sistema antigo. Todos os sistemas que dependem de carregamento de arquivos serão atualizados na mesma implementação para usar `AssetManager`.

---

## 10. Guia de Integração

### 10.1 Inicialização no Bootstrap

```cpp
// core/bootstrap/Bootstrap.cpp
#include "platform/storage/AssetManager.h"

bool Bootstrap::Initialize() {
    // ... outras inicializações
    
    // Inicializar AssetManager
    JavaVM* jvm = GetJVM();
    jobject context = GetContext();
    
    if (!platform::storage::AssetManager::Get()->Initialize(jvm, context)) {
        FLog("Failed to initialize AssetManager");
        return false;
    }
    
    // Inicializar assets (cópia do APK) na primeira execução
    auto initialization = AssetInitialization();
    auto result = initialization.InitializeFromAPK(jvm, context);
    if (!result.success) {
        FLog("Asset initialization failed: %zu files copied", result.filesCopied);
    }
    
    return true;
}
```

### 10.2 Uso em Módulos

```cpp
// Em qualquer módulo do projeto
#include "core/services/ServiceLocator.h"
#include "platform/storage/AssetManager.h"

// Carregar textura
auto texture = ServiceLocator::Assets()->LoadTexture("samp/icon");
if (texture) {
    RwTexture* tex = texture->GetTexture();
    // Usar textura
}

// Carregar script
auto script = ServiceLocator::Assets()->LoadScript("SAMP/main.scm");
if (script) {
    const void* data = script->GetScriptData();
    size_t size = script->GetScriptSize();
    // Processar script
}
```

### 10.3 Integração com SAMPCore

```cpp
// core/services/ServiceLocator.h
namespace core::services {

class ServiceLocator {
public:
    static platform::storage::AssetManager* Assets() {
        return s_assetManager;
    }
    
private:
    static platform::storage::AssetManager* s_assetManager;
};

}
```

---

## 11. Extensibilidade Futura

O sistema foi projetado para ser modular e extensível. Embora não implementemos recursos de download, importação, modificação e verificação agora, a estrutura permite adicionar essas funcionalidades futuramente sem alterar a API pública principal.

### 11.1 Como Extender o Sistema

**Adicionar um novo Provider:**

1. Criar classe que implementa `IAssetProvider`
2. Registrar no `AssetManager::RegisterProviders()`
3. O sistema automaticamente usará quando necessário (baseado em prioridade)

**Exemplo futuro (NetworkProvider):**

```cpp
// providers/NetworkProvider.h - Apenas interface por enquanto
class NetworkProvider : public IAssetProvider {
public:
    NetworkProvider() = default;
    AssetProviderType GetType() const override { return AssetProviderType::Network; }
    int GetPriority() const override { return 25; }
    
    // Métodos retornam nullptr/empty por enquanto
    std::unique_ptr<Asset> Load(const std::string& path) override {
        // TODO: Implementar quando necessário
        return nullptr;
    }
    
    bool Exists(const std::string& path) const override {
        // TODO: Implementar quando necessário
        return false;
    }
    
    std::vector<std::string> List(const std::string& directory) const override {
        // TODO: Implementar quando necessário
        return {};
    }
};
```

**Extensões Implementadas:**

- ✅ **Importação de assets (via SAF - Storage Access Framework)** - Ver `SAF_INTEGRATION_SPECIFICATION.md`

**Futuras Extensões (não implementadas agora):**

- Download de assets (via NetworkProvider)
- Modificação de assets (escrita em FilesDirProvider)
- Verificação de integridade (checksums)
- Deleção de assets

A estrutura modular permite adicionar essas funcionalidades sem modificar o core do `AssetManager`.

---

**Última atualização:** Janeiro 2026  
**Versão do documento:** 1.0
