# Plano de Implementação: Sistema de Arquivos Modular

## Índice

1. [Visão Geral](#visão-geral)
2. [Regras de Programação Segura](#regras-de programação-segura)
3. [Fase 0: Preparação e Infraestrutura](#fase-0-preparação-e-infraestrutura)
4. [Fase 1: Interfaces e Tipos Básicos](#fase-1-interfaces-e-tipos-básicos)
5. [Fase 2: Implementação Core](#fase-2-implementação-core)
6. [Fase 3: Ponte JNI Android](#fase-3-ponte-jni-android)
7. [Fase 4: Wrapper de Compatibilidade](#fase-4-wrapper-de-compatibilidade)
8. [Fase 5: Integração com Bootstrap](#fase-5-integração-com-bootstrap)
9. [Fase 6: Migração de Hooks](#fase-6-migração-de-hooks)
10. [Fase 7: Integração Java](#fase-7-integração-java)
11. [Fase 8: Testes e Validação](#fase-8-testes-e-validação)
12. [Checklists de Progresso](#checklists-de-progresso)

---

## Visão Geral

### Objetivo

Implementar sistema de arquivos modular que:
- Centraliza acesso a arquivos via `IFileSystemService`
- Suporta múltiplas fontes (APK, Storage, Cache)
- Mantém 100% de compatibilidade com código legado
- Previne problemas de thread safety, memória e race conditions

### Princípios

1. **Segurança Primeiro**: Todas as operações verificam precondições
2. **Thread Safety**: Mutexes onde necessário, documentação clara
3. **RAII**: Smart pointers e gerenciamento automático de recursos
4. **Validação**: Verificações em cada camada
5. **Compatibilidade**: Wrappers garantem que código legado continua funcionando

---

## Regras de Programação Segura

### R1: Thread Safety

**Regra**: Operações em estruturas compartilhadas DEVEM usar mutexes.

**Aplicação**:
- `std::map<std::string, std::string> m_redirects` → `std::mutex m_redirectsMutex`
- `void* m_assetManager` → `std::mutex m_assetManagerMutex`
- `std::set<std::string> m_extracting` → `std::mutex m_extractionMutex`

**Padrão**:
```cpp
// SEMPRE usar lock_guard para RAII
std::lock_guard<std::mutex> lock(m_mutex);
// Operação
```

**CHECKLIST**:
- [ ] Todos os membros mutáveis compartilhados têm mutex correspondente
- [ ] Todas as leituras/escritas usam lock_guard
- [ ] Sem locks aninhados (ou usar recursive_mutex se necessário)
- [ ] Documentado quais funções são thread-safe

### R2: Validação de Parâmetros

**Regra**: TODAS as funções públicas validam parâmetros de entrada.

**Aplicação**:
- `path.empty()` → retornar erro
- `path.find("..") != npos` → rejeitar (path injection)
- `m_storagePath.empty()` → retornar handle inválido
- `m_assetManager == nullptr` → retornar erro ou usar fallback

**Padrão**:
```cpp
FileHandle FileSystemService::OpenFile(const std::string& path, ...) {
    // VALIDAÇÃO 1: Path não vazio
    if (path.empty()) {
        LOGE("OpenFile: path is empty");
        return FileHandle{};
    }
    
    // VALIDAÇÃO 2: Path não contém ".."
    if (path.find("..") != std::string::npos) {
        LOGE("OpenFile: path contains '..', rejecting: %s", path.c_str());
        return FileHandle{};
    }
    
    // VALIDAÇÃO 3: Storage inicializado
    if (m_storagePath.empty()) {
        LOGW("OpenFile: FileSystemService not initialized");
        return FileHandle{};
    }
    
    // ... resto do código
}
```

**CHECKLIST**:
- [ ] Todas as funções públicas validam path.empty()
- [ ] Paths com ".." são rejeitados
- [ ] Verifica inicialização antes de usar membros
- [ ] Logging apropriado para cada tipo de erro

### R3: Gerenciamento de Memória

**Regra**: SEMPRE usar RAII. Evitar new/delete explícitos quando possível.

**Aplicação**:
- `FILE*` → documentar responsabilidade de `fclose()`
- Ponteiros nativos → considerar smart pointers
- Buffers → `std::vector` ou `std::string`

**Padrão**:
```cpp
// BOM: RAII com vector
std::vector<uint8_t> ReadFile(const std::string& path) {
    // ... aloca automaticamente, desaloca automaticamente
}

// BOM: Documentar responsabilidade
// "Código que chama OpenFile() é responsável por fclose()"
FILE* OpenFile(...) { return file; }

// RUIM: new sem delete correspondente visível
char* buffer = new char[size]; // ❌
```

**CHECKLIST**:
- [ ] Sem new/delete explícitos (exceto quando necessário para APIs C)
- [ ] Documentado responsabilidade de recursos (FILE*, etc)
- [ ] Uso de std::vector/std::string para buffers
- [ ] Verificação de nullptr antes de usar ponteiros

### R4: Buffer Overflow Prevention

**Regra**: NUNCA usar sprintf() sem verificação de tamanho. Preferir std::string.

**Aplicação**:
- Paths → `std::string` sempre
- Se precisar de buffer fixo → `snprintf()` com verificação
- Concatenar paths → `std::string::operator+`

**Padrão**:
```cpp
// BOM: std::string (sem limite)
std::string GetStoragePath(const std::string& relativePath) const {
    return m_storagePath + NormalizePath(relativePath);
}

// BOM: snprintf com verificação
char buffer[256];
int written = snprintf(buffer, sizeof(buffer), "%s%s", prefix, path);
if (written < 0 || written >= sizeof(buffer)) {
    LOGE("Path too long");
    return "";
}

// RUIM: sprintf sem verificação
char buffer[256];
sprintf(buffer, "%s%s", prefix, path); // ❌ Pode overflow
```

**CHECKLIST**:
- [ ] Sem sprintf() sem verificação
- [ ] Paths sempre em std::string
- [ ] snprintf() com verificação de tamanho se necessário
- [ ] Validação de PATH_MAX (4096) para paths muito longos

### R5: Tratamento de Erros

**Regra**: NUNCA fazer std::terminate() ou abort(). Sempre retornar erro ou usar fallback.

**Aplicação**:
- Arquivo não encontrado → retornar handle inválido + log
- Inicialização falhou → retornar false + log
- Erro crítico → log + retornar estado de erro

**Padrão**:
```cpp
// BOM: Retornar erro
bool Initialize(const std::string& path) {
    if (path.empty()) {
        LOGE("Initialize: path is empty");
        return false;
    }
    // ...
    return true;
}

// BOM: Retornar handle inválido
FileHandle OpenFile(...) {
    if (!file) {
        LOGE("Failed to open file: %s", path.c_str());
        return FileHandle{}; // Handle inválido
    }
    return handle;
}

// RUIM: std::terminate
if (!file) {
    std::terminate(); // ❌ Mata o app
}
```

**CHECKLIST**:
- [ ] Sem std::terminate() ou abort()
- [ ] Erros retornam valores apropriados (false, nullptr, handle inválido)
- [ ] Logging de todos os erros
- [ ] Fallback quando possível

### R6: Inicialização e Ordem

**Regra**: Verificar estado de inicialização em TODAS as operações.

**Aplicação**:
- `m_storagePath.empty()` → não usar
- `m_assetManager == nullptr` → usar fallback
- ServiceLocator → verificar se serviço está registrado

**Padrão**:
```cpp
FileHandle FileSystemService::OpenFile(...) {
    // SEMPRE verificar inicialização
    if (m_storagePath.empty()) {
        LOGW("FileSystemService not initialized");
        return FileHandle{};
    }
    
    // Verificar AssetManager se necessário
    if (needsAssetManager && !m_assetManager) {
        LOGW("AssetManager not set, using storage only");
        // Tenta apenas storage
    }
    
    // ... resto
}
```

**CHECKLIST**:
- [ ] Todas as funções verificam m_storagePath.empty()
- [ ] Verificação de m_assetManager quando necessário
- [ ] Logging quando não inicializado
- [ ] Documentar ordem de inicialização obrigatória

### R7: Path Sanitization

**Regra**: TODOS os paths passam por sanitização antes de uso.

**Aplicação**:
- Remover ".."
- Normalizar barras
- Validar caracteres
- Verificar que está dentro do diretório base

**Padrão**:
```cpp
std::string NormalizePath(const std::string& path) {
    std::string normalized = path;
    
    // Rejeitar ".."
    if (normalized.find("..") != std::string::npos) {
        return ""; // Rejeita
    }
    
    // Remover caracteres de controle
    normalized.erase(
        std::remove_if(normalized.begin(), normalized.end(),
            [](char c) { return c == '\0' || c == '\n' || c == '\r'; }),
        normalized.end()
    );
    
    // Normalizar barras (opcional)
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    return normalized;
}

std::string GetStoragePath(const std::string& relativePath) const {
    std::string normalized = NormalizePath(relativePath);
    if (normalized.empty()) return "";
    
    std::string fullPath = m_storagePath + normalized;
    
    // Verificar que está dentro de m_storagePath
    if (fullPath.find(m_storagePath) != 0) {
        LOGE("Path outside storage directory");
        return "";
    }
    
    return fullPath;
}
```

**CHECKLIST**:
- [ ] Todos os paths passam por NormalizePath()
- [ ] Paths com ".." são rejeitados
- [ ] Verificação de que path está dentro do diretório base
- [ ] Remoção de caracteres de controle

### R8: Extração Atômica

**Regra**: Extração de assets DEVE ser atômica (temp file + rename).

**Aplicação**:
- Escrever para arquivo.tmp
- Verificar sucesso
- Renomear para arquivo final
- Limpar temp em caso de erro

**Padrão**:
```cpp
bool ExtractAssetToStorage(...) {
    std::string tempPath = destPath + ".tmp";
    
    // 1. Lê do APK
    std::vector<uint8_t> data = ReadAsset(...);
    if (data.empty()) return false;
    
    // 2. Escreve para temp
    if (!WriteFile(tempPath, data)) {
        return false;
    }
    
    // 3. Renomeia (atômico)
    if (rename(tempPath.c_str(), destPath.c_str()) != 0) {
        unlink(tempPath.c_str()); // Limpa temp
        return false;
    }
    
    return true;
}
```

**CHECKLIST**:
- [ ] Extração usa arquivo temporário
- [ ] Rename atômico após escrita bem-sucedida
- [ ] Limpeza de temp em caso de erro
- [ ] Verificação de integridade opcional (tamanho)

---

## Fase 0: Preparação e Infraestrutura

### Objetivo

Preparar ambiente e verificar dependências antes de iniciar implementação.

### Tarefas

1. **Verificar Estrutura de Diretórios**
   - [ ] Criar `app/src/main/cpp/samp/core/filesystem/`
   - [ ] Verificar `app/src/main/cpp/samp/platform/android/` existe
   - [ ] Criar `app/src/main/cpp/samp/game/hooks/` se não existir

2. **Verificar Dependências**
   - [ ] ServiceLocator existe e funciona
   - [ ] Bootstrap existe e pode ser estendido
   - [ ] Sistema de logging (LOGI, LOGW, LOGE) disponível
   - [ ] Android NDK configurado (AAssetManager disponível)

3. **Configurar CMakeLists.txt**
   - [ ] Adicionar novos diretórios ao CMakeLists.txt (se necessário)
   - [ ] Verificar que GLOB_RECURSE inclui novos arquivos

4. **Preparar Ambiente de Teste**
   - [ ] APK de teste preparado
   - [ ] Assets listados em assetfile.txt
   - [ ] Dispositivo/emulador Android configurado

### Validação

- [ ] Diretórios criados
- [ ] Dependências verificadas
- [ ] CMake compila sem erros
- [ ] Ambiente de teste pronto

### Checklist de Progresso

```
FASE 0: PREPARAÇÃO
[ ] Estrutura de diretórios criada
[ ] Dependências verificadas
[ ] CMakeLists.txt configurado
[ ] Ambiente de teste pronto
```

---

## Fase 1: Interfaces e Tipos Básicos

### Objetivo

Criar fundamentos: enums, estruturas e interface abstrata.

### Arquivos a Criar

1. **`core/filesystem/FileSource.h`** (~50 linhas)

**Conteúdo**:
```cpp
#pragma once
#include <cstdio>
#include <string>

namespace Core {
namespace FileSystem {

enum class FileSource {
    APK_ASSETS,        // Arquivos dentro do APK (somente leitura)
    EXTERNAL_STORAGE,  // Android/data/.../files/ (leitura/escrita)
    CACHE,             // Cache interno (leitura/escrita temporária)
    AUTO               // Tenta todas as fontes em ordem
};

enum class FileOpenMode {
    READ,          // "r"
    WRITE,         // "w"
    APPEND,        // "a"
    READ_BINARY,   // "rb"
    WRITE_BINARY   // "wb"
};

struct FileHandle {
    FILE* file = nullptr;
    FileSource source = FileSource::AUTO;
    std::string path;
    
    bool isValid() const { return file != nullptr; }
    
    FileHandle() = default;
    FileHandle(FILE* f, FileSource s, const std::string& p)
        : file(f), source(s), path(p) {}
};

} // namespace FileSystem
} // namespace Core
```

**CHECKLIST**:
- [ ] File criado
- [ ] Enums definidos
- [ ] FileHandle com isValid()
- [ ] Namespace correto
- [ ] Compila sem erros

2. **`core/filesystem/IFileSystemService.h`** (~80 linhas)

**Conteúdo**:
```cpp
#pragma once
#include "FileSource.h"
#include <vector>
#include <cstdint>
#include <string>

namespace Core {
namespace FileSystem {

class IFileSystemService {
public:
    virtual ~IFileSystemService() = default;
    
    // Abre arquivo com fallback automático
    virtual FileHandle OpenFile(
        const std::string& path,
        FileOpenMode mode = FileOpenMode::READ_BINARY
    ) = 0;
    
    // Abre arquivo de fonte específica
    virtual FileHandle OpenFileFromSource(
        const std::string& path,
        FileSource source,
        FileOpenMode mode = FileOpenMode::READ_BINARY
    ) = 0;
    
    // Verifica se arquivo existe em alguma fonte
    virtual bool FileExists(const std::string& path) = 0;
    
    // Verifica se arquivo existe em fonte específica
    virtual bool FileExistsInSource(
        const std::string& path,
        FileSource source
    ) = 0;
    
    // Lê arquivo completo
    virtual std::vector<uint8_t> ReadFile(const std::string& path) = 0;
    
    // Escreve arquivo completo
    virtual bool WriteFile(
        const std::string& path,
        const std::vector<uint8_t>& data
    ) = 0;
    
    // Obtém path de uma fonte
    virtual std::string GetSourcePath(FileSource source) const = 0;
    
    // Extrai assets do APK se necessário
    virtual bool ExtractAssetsIfNeeded(
        const std::vector<std::string>& assetPaths
    ) = 0;
    
    // Registra redirecionamento de path
    virtual void RegisterFileRedirect(
        const std::string& originalPath,
        const std::string& newPath
    ) = 0;
    
    // Resolve path usando redirecionamentos
    virtual std::string ResolvePath(const std::string& path) const = 0;
};

} // namespace FileSystem
} // namespace Core
```

**CHECKLIST**:
- [ ] Interface criada
- [ ] Todas as funções virtuais puras
- [ ] Destrutor virtual
- [ ] Parâmetros documentados
- [ ] Compila sem erros

### Validação

- [ ] Arquivos criados e compilam
- [ ] Sem erros de sintaxe
- [ ] Namespaces corretos
- [ ] Estruturas alinhadas com documentação

### Checklist de Progresso

```
FASE 1: INTERFACES E TIPOS BÁSICOS
[ ] FileSource.h criado e compila
[ ] IFileSystemService.h criado e compila
[ ] Enums definidos corretamente
[ ] FileHandle implementado
[ ] Interface completa
```

---

## Fase 2: Implementação Core

### Objetivo

Implementar `FileSystemService` com todas as funcionalidades principais.

### Arquivos a Criar

1. **`core/filesystem/FileSystemService.h`** (~120 linhas)

**CHECKLIST**:
- [ ] Header criado
- [ ] Herda de IFileSystemService
- [ ] Membros privados definidos (mutexes, paths, etc)
- [ ] Funções públicas e privadas declaradas
- [ ] Compila sem erros

2. **`core/filesystem/FileSystemService.cpp`** (~450 linhas)

**Implementação por Função**:

#### 2.1 Construtor e Destrutor

```cpp
FileSystemService::FileSystemService() {
    m_fallbackOrder = {
        FileSource::EXTERNAL_STORAGE,
        FileSource::APK_ASSETS,
        FileSource::CACHE
    };
}

FileSystemService::~FileSystemService() {
    // Limpeza se necessário
}
```

**CHECKLIST**:
- [ ] Construtor inicializa m_fallbackOrder
- [ ] Destrutor limpa recursos
- [ ] Mutexes inicializados corretamente

#### 2.2 Initialize()

```cpp
void FileSystemService::Initialize(const std::string& storagePath) {
    // R6: Validação
    if (storagePath.empty()) {
        LOGE("FileSystemService::Initialize: storagePath is empty");
        return;
    }
    
    // R7: Sanitização
    std::string normalized = NormalizePath(storagePath);
    if (normalized.empty()) {
        LOGE("FileSystemService::Initialize: invalid storagePath");
        return;
    }
    
    // Garantir que termina com /
    if (normalized.back() != '/') {
        normalized += '/';
    }
    
    m_storagePath = normalized;
    
    // Criar diretórios se não existirem
    CreateDirectoryRecursive(m_storagePath + "SAMP");
    CreateDirectoryRecursive(m_storagePath + "data");
    CreateDirectoryRecursive(m_storagePath + "cache");
    
    LOGI("FileSystemService initialized: %s", m_storagePath.c_str());
}
```

**CHECKLIST**:
- [ ] Valida storagePath.empty()
- [ ] Normaliza path
- [ ] Cria diretórios necessários
- [ ] Logging de sucesso

#### 2.3 NormalizePath() (static)

```cpp
std::string FileSystemService::NormalizePath(const std::string& path) {
    std::string normalized = path;
    
    // R7: Rejeitar ".."
    if (normalized.find("..") != std::string::npos) {
        LOGE("NormalizePath: path contains '..', rejecting: %s", path.c_str());
        return "";
    }
    
    // Remover caracteres de controle
    normalized.erase(
        std::remove_if(normalized.begin(), normalized.end(),
            [](char c) {
                return c == '\0' || c == '\n' || c == '\r' || c == '\t';
            }),
        normalized.end()
    );
    
    // Normalizar barras
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Remover barras duplicadas
    // ...
    
    return normalized;
}
```

**CHECKLIST**:
- [ ] Rejeita ".."
- [ ] Remove caracteres de controle
- [ ] Normaliza barras
- [ ] Remove barras duplicadas

#### 2.4 OpenFile()

```cpp
FileHandle FileSystemService::OpenFile(
    const std::string& path,
    FileOpenMode mode
) {
    // R2: Validação
    if (path.empty()) {
        LOGE("OpenFile: path is empty");
        return FileHandle{};
    }
    
    // R6: Verificar inicialização
    if (m_storagePath.empty()) {
        LOGW("OpenFile: FileSystemService not initialized");
        return FileHandle{};
    }
    
    // R7: Sanitizar
    std::string normalized = NormalizePath(path);
    if (normalized.empty()) {
        return FileHandle{};
    }
    
    // Resolver redirecionamentos
    std::string resolved = ResolvePath(normalized);
    
    // Tentar fontes em ordem
    for (FileSource source : m_fallbackOrder) {
        FileHandle handle = OpenFileFromSource(resolved, source, mode);
        if (handle.isValid()) {
            return handle;
        }
    }
    
    LOGE("OpenFile: file not found in any source: %s", resolved.c_str());
    return FileHandle{};
}
```

**CHECKLIST**:
- [ ] Valida path.empty()
- [ ] Verifica inicialização
- [ ] Normaliza path
- [ ] Resolve redirecionamentos
- [ ] Tenta fontes em ordem
- [ ] Retorna handle inválido se não encontrar

#### 2.5 OpenFromStorage()

```cpp
FileHandle FileSystemService::OpenFromStorage(
    const std::string& path,
    FileOpenMode mode
) {
    std::string fullPath = GetStoragePath(path);
    if (fullPath.empty()) {
        return FileHandle{};
    }
    
    FILE* file = OpenNativeFile(fullPath, mode);
    if (!file) {
        return FileHandle{};
    }
    
    return FileHandle(file, FileSource::EXTERNAL_STORAGE, fullPath);
}
```

**CHECKLIST**:
- [ ] Obtém fullPath via GetStoragePath()
- [ ] Usa OpenNativeFile()
- [ ] Retorna handle com source correto

#### 2.6 OpenFromAPK()

```cpp
FileHandle FileSystemService::OpenFromAPK(const std::string& path) {
    // R6: Verificar AssetManager
    void* assetMgr = GetAssetManager(); // Thread-safe
    if (!assetMgr) {
        return FileHandle{};
    }
    
    // Verificar se existe no cache primeiro
    std::string cachePath = GetCachePath(path);
    if (FileExistsInSource(cachePath, FileSource::EXTERNAL_STORAGE)) {
        return OpenFromStorage(cachePath, FileOpenMode::READ_BINARY);
    }
    
    // R3: Extração thread-safe
    {
        std::lock_guard<std::mutex> lock(m_extractionMutex);
        
        // Double-check
        if (FileExistsInSource(cachePath, FileSource::EXTERNAL_STORAGE)) {
            return OpenFromStorage(cachePath, FileOpenMode::READ_BINARY);
        }
        
        // Verifica se outra thread está extraindo
        if (m_extracting.find(path) != m_extracting.end()) {
            LOGW("OpenFromAPK: another thread is extracting: %s", path.c_str());
            return FileHandle{}; // Retry depois
        }
        
        m_extracting.insert(path);
    }
    
    // Extrair (fora do lock)
    bool success = ExtractAssetToStorage(path, cachePath);
    
    {
        std::lock_guard<std::mutex> lock(m_extractionMutex);
        m_extracting.erase(path);
    }
    
    if (!success) {
        return FileHandle{};
    }
    
    return OpenFromStorage(cachePath, FileOpenMode::READ_BINARY);
}
```

**CHECKLIST**:
- [ ] Verifica AssetManager (thread-safe)
- [ ] Verifica cache primeiro
- [ ] Double-check locking
- [ ] Thread-safe extraction
- [ ] Limpa m_extracting após extração

#### 2.7 ExtractAssetToStorage()

```cpp
bool FileSystemService::ExtractAssetToStorage(
    const std::string& assetPath,
    const std::string& destPath
) {
    // R8: Extração atômica
    void* assetMgr = GetAssetManager();
    if (!assetMgr) {
        return false;
    }
    
    // Lê do APK
    std::vector<uint8_t> data = AssetManagerBridge::ReadAsset(
        assetMgr, assetPath
    );
    if (data.empty()) {
        LOGE("ExtractAssetToStorage: failed to read asset: %s", assetPath.c_str());
        return false;
    }
    
    // Cria diretório
    std::string dir = destPath.substr(0, destPath.find_last_of('/'));
    if (!CreateDirectoryRecursive(dir)) {
        LOGE("ExtractAssetToStorage: failed to create directory: %s", dir.c_str());
        return false;
    }
    
    // R8: Escreve para temp
    std::string tempPath = destPath + ".tmp";
    if (!WriteFile(tempPath, data)) {
        return false;
    }
    
    // R8: Renomeia (atômico)
    if (rename(tempPath.c_str(), destPath.c_str()) != 0) {
        unlink(tempPath.c_str()); // Limpa temp
        LOGE("ExtractAssetToStorage: failed to rename: %s", destPath.c_str());
        return false;
    }
    
    LOGI("ExtractAssetToStorage: extracted: %s", destPath.c_str());
    return true;
}
```

**CHECKLIST**:
- [ ] Verifica AssetManager
- [ ] Lê do APK
- [ ] Cria diretório
- [ ] Escreve para temp
- [ ] Renomeia atômico
- [ ] Limpa temp em erro

#### 2.8 RegisterFileRedirect() / ResolvePath()

```cpp
void FileSystemService::RegisterFileRedirect(
    const std::string& originalPath,
    const std::string& newPath
) {
    // R1: Thread-safe
    std::lock_guard<std::mutex> lock(m_redirectsMutex);
    m_redirects[originalPath] = newPath;
}

std::string FileSystemService::ResolvePath(const std::string& path) const {
    // R1: Thread-safe
    std::lock_guard<std::mutex> lock(m_redirectsMutex);
    auto it = m_redirects.find(path);
    if (it != m_redirects.end()) {
        return it->second;
    }
    return path;
}
```

**CHECKLIST**:
- [ ] Thread-safe com mutex
- [ ] ResolvePath consulta mapa
- [ ] Retorna path original se não houver redirecionamento

### Validação

- [ ] Todas as funções implementadas
- [ ] Regras de programação segura aplicadas
- [ ] Compila sem erros
- [ ] Sem warnings críticos

### Checklist de Progresso

```
FASE 2: IMPLEMENTAÇÃO CORE
[ ] FileSystemService.h criado
[ ] FileSystemService.cpp implementado
[ ] Initialize() com validação
[ ] NormalizePath() implementado
[ ] OpenFile() com fallback
[ ] OpenFromStorage() implementado
[ ] OpenFromAPK() thread-safe
[ ] ExtractAssetToStorage() atômico
[ ] RegisterFileRedirect() thread-safe
[ ] ResolvePath() thread-safe
[ ] Todas as regras de programação aplicadas
```

---

## Fase 3: Ponte JNI Android

### Objetivo

Criar ponte JNI para acessar AssetManager do Java.

### Arquivos a Criar

1. **`platform/android/AssetManagerBridge.h`** (~30 linhas)

2. **`platform/android/AssetManagerBridge.cpp`** (~200 linhas)

**Implementação Principal**:

```cpp
void* AssetManagerBridge::GetAssetManagerFromJNI(void* jniEnv, jobject assetManager) {
    JNIEnv* env = (JNIEnv*)jniEnv;
    
    // Verifica se thread está anexada à JVM
    JavaVM* vm = nullptr;
    env->GetJavaVM(&vm);
    
    JNIEnv* currentEnv = nullptr;
    jint result = vm->GetEnv((void**)&currentEnv, JNI_VERSION_1_6);
    
    if (result == JNI_EDETACHED) {
        // Thread não está anexada, anexar
        vm->AttachCurrentThread(&currentEnv, nullptr);
        env = currentEnv;
    }
    
    // Obtém AAssetManager
    jclass assetManagerClass = env->GetObjectClass(assetManager);
    jmethodID getNativeAssetManager = env->GetMethodID(
        assetManagerClass, "getNativeAssetManager", "()J"
    );
    
    if (!getNativeAssetManager) {
        // Fallback: usar AAssetManager_fromJava
        AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
        return static_cast<void*>(mgr);
    }
    
    jlong nativePtr = env->CallLongMethod(assetManager, getNativeAssetManager);
    return reinterpret_cast<void*>(nativePtr);
}

std::vector<uint8_t> AssetManagerBridge::ReadAsset(
    void* assetManager,
    const std::string& path
) {
    if (!assetManager) {
        return {};
    }
    
    AAssetManager* mgr = static_cast<AAssetManager*>(assetManager);
    AAsset* asset = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_BUFFER);
    
    if (!asset) {
        return {};
    }
    
    off_t length = AAsset_getLength(asset);
    std::vector<uint8_t> data(length);
    
    int bytesRead = AAsset_read(asset, data.data(), length);
    AAsset_close(asset);
    
    if (bytesRead != length) {
        return {};
    }
    
    return data;
}
```

**CHECKLIST**:
- [ ] GetAssetManagerFromJNI verifica thread anexada
- [ ] ReadAsset lê asset completo
- [ ] Fecha AAsset após uso
- [ ] Tratamento de erros

### Validação

- [ ] Compila sem erros
- [ ] JNI funciona corretamente
- [ ] AssetManager obtido do Java

### Checklist de Progresso

```
FASE 3: PONTE JNI ANDROID
[ ] AssetManagerBridge.h criado
[ ] AssetManagerBridge.cpp implementado
[ ] GetAssetManagerFromJNI thread-safe
[ ] ReadAsset implementado
[ ] AssetExists implementado
[ ] JNI testado
```

---

## Fase 4: Wrapper de Compatibilidade

### Objetivo

Criar wrapper que mantém compatibilidade com código legado.

### Arquivos a Criar

1. **`core/filesystem/FileLoader.h`** (~40 linhas)

2. **`core/filesystem/FileLoader.cpp`** (~150 linhas)

**Implementação Principal**:

```cpp
FILE* FileLoader::Open(const char* path, const char* mode) {
    auto* fs = ServiceLocator::Get<IFileSystemService>();
    if (!fs) {
        LOGE("FileLoader::Open: FileSystemService not registered");
        return nullptr;
    }
    
    FileOpenMode openMode = ParseMode(mode);
    FileHandle handle = fs->OpenFile(path, openMode);
    return handle.file;
}

FILE* FileLoader::OpenForGame(const char* gamePath) {
    auto* fs = ServiceLocator::Get<IFileSystemService>();
    if (!fs) {
        LOGE("FileLoader::OpenForGame: FileSystemService not registered");
        return nullptr;
    }
    
    // Resolve path automaticamente
    std::string resolved = fs->ResolvePath(gamePath);
    
    FileHandle handle = fs->OpenFile(resolved, FileOpenMode::READ_BINARY);
    return handle.file;
}

void FileLoader::RegisterGameRedirects(IFileSystemService* fs) {
    if (!fs) return;
    
    // Registra todos os redirecionamentos do jogo
    fs->RegisterFileRedirect("DATA/GTA.DAT", "SAMP/gta.dat");
    fs->RegisterFileRedirect("DATA/PEDS.IDE", "SAMP/peds.ide");
    fs->RegisterFileRedirect("DATA/VEHICLES.IDE", "SAMP/vehicles.ide");
    // ... todos os outros
}
```

**CHECKLIST**:
- [ ] Open() delega para FileSystemService
- [ ] OpenForGame() resolve paths
- [ ] RegisterGameRedirects() registra todos os redirecionamentos
- [ ] Tratamento de ServiceLocator não registrado

### Validação

- [ ] Wrapper compila
- [ ] Compatível com código legado
- [ ] Redirecionamentos registrados

### Checklist de Progresso

```
FASE 4: WRAPPER DE COMPATIBILIDADE
[ ] FileLoader.h criado
[ ] FileLoader.cpp implementado
[ ] Open() implementado
[ ] OpenForGame() implementado
[ ] RegisterGameRedirects() completo
[ ] ParseMode() implementado
```

---

## Fase 5: Integração com Bootstrap

### Objetivo

Integrar FileSystemService no Bootstrap e ServiceLocator.

### Arquivos a Modificar

1. **`core/bootstrap/ServiceLocator.h`**

Adicionar:
```cpp
class IFileSystemService;

class Services {
public:
    // ... existentes
    static void Register(Core::FileSystem::IFileSystemService* filesystem);
    static Core::FileSystem::IFileSystemService* FileSystem();
private:
    static inline Core::FileSystem::IFileSystemService* s_filesystem = nullptr;
};
```

2. **`core/bootstrap/ServiceLocator.cpp`**

Adicionar:
```cpp
#include "../filesystem/IFileSystemService.h"

void Services::Register(Core::FileSystem::IFileSystemService* filesystem) {
    s_filesystem = filesystem;
}

Core::FileSystem::IFileSystemService* Services::FileSystem() {
    return s_filesystem;
}

void Services::Clear() {
    // ... existentes
    s_filesystem = nullptr;
}
```

3. **`core/bootstrap/Bootstrap.cpp`**

Modificar:
```cpp
#include "../filesystem/FileSystemService.h"
#include "../filesystem/FileLoader.h"

namespace Bootstrap {
    void Initialize() {
        if (s_initialized) {
            return;
        }
        
        // Fase Platform: Inicializa filesystem
        auto* fs = new Core::FileSystem::FileSystemService();
        Services::Register(fs);
        // Nota: Initialize() será chamado depois via JNI
        
        // Fase Game: Configura redirecionamentos
        // Será chamado depois de setStoragePath()
        
        s_initialized = true;
    }
}
```

**CHECKLIST**:
- [ ] ServiceLocator.h atualizado
- [ ] ServiceLocator.cpp implementado
- [ ] Bootstrap.cpp integrado
- [ ] FileSystemService registrado
- [ ] Compila sem erros

### Validação

- [ ] ServiceLocator registra FileSystemService
- [ ] Bootstrap inicializa corretamente
- [ ] Sem erros de compilação

### Checklist de Progresso

```
FASE 5: INTEGRAÇÃO COM BOOTSTRAP
[ ] ServiceLocator.h atualizado
[ ] ServiceLocator.cpp atualizado
[ ] Bootstrap.cpp integrado
[ ] FileSystemService registrado
[ ] Compila sem erros
```

---

## Fase 6: Migração de Hooks

### Objetivo

Migrar hooks de arquivo para usar novo sistema.

### Arquivos a Modificar

1. **`game/hooks.cpp`** (ou criar `game/hooks/FileHooks.cpp`)

Substituir `NvFOpen()`:

```cpp
#include "core/filesystem/FileLoader.h"

stFile* NvFOpen(const char* r0, const char* r1, int r2, int r3) {
    // Usa FileLoader que faz todo o trabalho
    FILE* file = FileLoader::OpenForGame(r1);
    
    if (!file) {
        FLog("NvFOpen: file not found (%s)", r1);
        return nullptr;
    }
    
    // Aloca stFile
    stFile* st = (stFile*)malloc(sizeof(stFile));
    if (!st) {
        fclose(file);
        return nullptr;
    }
    
    st->isFileExist = true;
    st->f = file;
    
    return st;
}
```

**CHECKLIST**:
- [ ] NvFOpen() usa FileLoader
- [ ] Alocação de stFile verificada
- [ ] fclose() se alocação falhar
- [ ] Logging de erros

2. **`game/CFileMgr.cpp`**

Modificar `OpenFile()`:

```cpp
FILE* CFileMgr::OpenFile(const char* path, const char* mode) {
    // Delega para FileLoader
    return FileLoader::Open(path, mode);
}
```

**CHECKLIST**:
- [ ] CFileMgr::OpenFile() delega para FileLoader
- [ ] Código legado continua funcionando
- [ ] Compila sem erros

### Validação

- [ ] Hooks migrados
- [ ] Código legado funciona
- [ ] Arquivos carregam corretamente

### Checklist de Progresso

```
FASE 6: MIGRAÇÃO DE HOOKS
[ ] NvFOpen() migrado
[ ] CFileMgr::OpenFile() migrado
[ ] Hooks funcionam
[ ] Arquivos carregam
[ ] Sem regressões
```

---

## Fase 7: Integração Java

### Objetivo

Adicionar suporte JNI no Java para AssetManager.

### Arquivos a Modificar

1. **`app/src/main/java/com/samp/mobile/game/SAMP.java`**

Adicionar:
```java
public native void setAssetManager(Object assetManager);

@Override
public void onCreate(Bundle savedInstanceState) {
    // ... código existente
    
    // Após setStoragePath()
    String storagePath = getExternalFilesDir(null).getAbsolutePath() + "/";
    setStoragePath(storagePath);
    
    // NOVO: Passa AssetManager
    setAssetManager(getAssets());
    
    // ... resto
}
```

2. **`app/src/main/cpp/samp/main.cpp`**

Adicionar JNI:
```cpp
#include "core/filesystem/FileSystemService.h"
#include "platform/android/AssetManagerBridge.h"

JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_setAssetManager(
    JNIEnv *pEnv, jobject thiz, jobject assetManager)
{
    void* aassetMgr = AssetManagerBridge::GetAssetManagerFromJNI(pEnv, assetManager);
    
    auto* fs = Services::FileSystem();
    if (fs) {
        static_cast<Core::FileSystem::FileSystemService*>(fs)->SetAssetManager(aassetMgr);
    }
}

// Modificar setStoragePath existente:
JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_setStoragePath(
    JNIEnv *pEnv, jobject thiz, jstring path)
{
    // ... código existente ...
    
    // NOVO: Inicializa FileSystemService
    auto* fs = Services::FileSystem();
    if (fs) {
        fs->Initialize(storagePath);
        
        // Registra redirecionamentos do jogo
        FileLoader::RegisterGameRedirects(fs);
        
        // Extrai assets se necessário (opcional, pode ser lazy)
        // std::vector<std::string> assets = {...};
        // fs->ExtractAssetsIfNeeded(assets);
    }
}
```

**CHECKLIST**:
- [ ] SAMP.java tem setAssetManager()
- [ ] main.cpp tem JNI setAssetManager
- [ ] setStoragePath inicializa FileSystemService
- [ ] Redirecionamentos registrados
- [ ] Compila sem erros

### Validação

- [ ] JNI funciona
- [ ] AssetManager passa do Java
- [ ] FileSystemService inicializado

### Checklist de Progresso

```
FASE 7: INTEGRAÇÃO JAVA
[ ] SAMP.java atualizado
[ ] JNI setAssetManager implementado
[ ] setStoragePath inicializa FileSystemService
[ ] Redirecionamentos registrados
[ ] Compila sem erros
```

---

## Fase 8: Testes e Validação

### Objetivo

Testar todas as funcionalidades e validar que tudo funciona.

### Testes por Funcionalidade

1. **Teste de Inicialização**
   - [ ] FileSystemService inicializa corretamente
   - [ ] Storage path válido
   - [ ] Diretórios criados
   - [ ] ServiceLocator registrado

2. **Teste de Abertura de Arquivos**
   - [ ] Arquivos do storage abrem
   - [ ] Arquivos do APK extraem e abrem
   - [ ] Fallback funciona
   - [ ] Redirecionamentos funcionam

3. **Teste de Thread Safety**
   - [ ] Múltiplas threads abrindo arquivos
   - [ ] Extração concorrente não corrompe
   - [ ] Redirecionamentos thread-safe

4. **Teste de Erros**
   - [ ] Path vazio retorna erro
   - [ ] Path com ".." rejeitado
   - [ ] Arquivo não encontrado retorna handle inválido
   - [ ] Sem std::terminate()

5. **Teste de Compatibilidade**
   - [ ] NvFOpen() funciona
   - [ ] CFileMgr::OpenFile() funciona
   - [ ] Código legado carrega arquivos
   - [ ] Sem regressões

6. **Teste de Performance**
   - [ ] Overhead aceitável (< 1ms por abertura)
   - [ ] Extração não bloqueia thread principal
   - [ ] Cache funciona

### Checklist Final

```
FASE 8: TESTES E VALIDAÇÃO
[ ] Teste de inicialização passou
[ ] Teste de abertura de arquivos passou
[ ] Teste de thread safety passou
[ ] Teste de erros passou
[ ] Teste de compatibilidade passou
[ ] Teste de performance aceitável
[ ] Todos os arquivos do jogo carregam
[ ] App não crasha
[ ] Sem regressões
```

---

## Checklists de Progresso Global

### Progresso Geral

```
IMPLEMENTAÇÃO: SISTEMA DE ARQUIVOS MODULAR

FASE 0: PREPARAÇÃO
[ ] Estrutura de diretórios
[ ] Dependências verificadas
[ ] CMakeLists.txt
[ ] Ambiente de teste

FASE 1: INTERFACES E TIPOS
[ ] FileSource.h
[ ] IFileSystemService.h

FASE 2: IMPLEMENTAÇÃO CORE
[ ] FileSystemService.h
[ ] FileSystemService.cpp
[ ] Todas as funções implementadas

FASE 3: PONTE JNI
[ ] AssetManagerBridge.h
[ ] AssetManagerBridge.cpp

FASE 4: WRAPPER
[ ] FileLoader.h
[ ] FileLoader.cpp

FASE 5: BOOTSTRAP
[ ] ServiceLocator atualizado
[ ] Bootstrap integrado

FASE 6: HOOKS
[ ] NvFOpen migrado
[ ] CFileMgr migrado

FASE 7: JAVA
[ ] SAMP.java atualizado
[ ] JNI implementado

FASE 8: TESTES
[ ] Todos os testes passaram
[ ] Sem regressões
```

### Verificação de Regras

```
REGRAS DE PROGRAMAÇÃO SEGURA

R1: THREAD SAFETY
[ ] Mutexes em estruturas compartilhadas
[ ] lock_guard usado sempre
[ ] Sem locks aninhados
[ ] Documentado thread-safety

R2: VALIDAÇÃO
[ ] Parâmetros validados
[ ] Paths sanitizados
[ ] Inicialização verificada

R3: MEMÓRIA
[ ] RAII usado
[ ] Sem new/delete explícitos
[ ] Documentado responsabilidade

R4: BUFFER OVERFLOW
[ ] Sem sprintf() sem verificação
[ ] std::string sempre
[ ] snprintf() com verificação

R5: TRATAMENTO DE ERROS
[ ] Sem std::terminate()
[ ] Erros retornam valores apropriados
[ ] Logging de erros

R6: INICIALIZAÇÃO
[ ] Verificação em todas as operações
[ ] Logging quando não inicializado

R7: PATH SANITIZATION
[ ] NormalizePath() sempre
[ ] ".." rejeitado
[ ] Dentro do diretório base

R8: EXTRAÇÃO ATÔMICA
[ ] Temp file + rename
[ ] Limpeza de temp
```

---

**Documento criado em**: 2024
**Versão**: 1.0
