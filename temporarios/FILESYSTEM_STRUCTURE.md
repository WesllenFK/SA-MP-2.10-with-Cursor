# Estrutura Detalhada: Sistema de Arquivos Modular

## 📁 Estrutura de Arquivos

```
core/filesystem/
├── FileSource.h                    # 50 linhas - Enums e tipos básicos
├── IFileSystemService.h            # 80 linhas - Interface abstrata
├── FileSystemService.h             # 120 linhas - Header da implementação
├── FileSystemService.cpp           # 450 linhas - Implementação principal
├── FileLoader.h                    # 40 linhas - Wrapper header
└── FileLoader.cpp                  # 150 linhas - Wrapper implementação

platform/android/
├── AssetManagerBridge.h             # 30 linhas - Ponte JNI header
└── AssetManagerBridge.cpp           # 200 linhas - Ponte JNI implementação

game/hooks/
└── FileHooks.cpp                    # 50 linhas - Hook NvFOpen atualizado
```

**Total**: ~1170 linhas de código novo

---

## 📄 Detalhamento por Arquivo

### 1. `core/filesystem/FileSource.h`

**Propósito**: Define enums e estruturas básicas

**Conteúdo**:
```cpp
namespace Core::FileSystem {

enum class FileSource {
    APK_ASSETS,          // Arquivos dentro do APK (somente leitura)
    EXTERNAL_STORAGE,    // Android/data/com.samp.mobile/files/ (leitura/escrita)
    CACHE,               // Cache interno (leitura/escrita temporária)
    AUTO                 // Tenta todas as fontes em ordem
};

enum class FileOpenMode {
    READ,                // "r"
    WRITE,               // "w"
    APPEND,              // "a"
    READ_BINARY,         // "rb"
    WRITE_BINARY         // "wb"
};

struct FileHandle {
    FILE* file = nullptr;
    FileSource source = FileSource::AUTO;
    std::string path;
    bool isValid() const { return file != nullptr; }
};

} // namespace
```

**Linhas**: ~50
**Dependências**: `<cstdio>`, `<string>`

---

### 2. `core/filesystem/IFileSystemService.h`

**Propósito**: Interface abstrata do sistema de arquivos

**Funções Públicas** (10 funções):

```cpp
class IFileSystemService {
public:
    virtual ~IFileSystemService() = default;
    
    // 1. Abre arquivo com fallback automático
    virtual FileHandle OpenFile(
        const std::string& path, 
        FileOpenMode mode = FileOpenMode::READ_BINARY
    ) = 0;
    
    // 2. Abre arquivo de fonte específica
    virtual FileHandle OpenFileFromSource(
        const std::string& path, 
        FileSource source, 
        FileOpenMode mode = FileOpenMode::READ_BINARY
    ) = 0;
    
    // 3. Verifica se arquivo existe em alguma fonte
    virtual bool FileExists(const std::string& path) = 0;
    
    // 4. Verifica se arquivo existe em fonte específica
    virtual bool FileExistsInSource(
        const std::string& path, 
        FileSource source
    ) = 0;
    
    // 5. Lê arquivo completo para buffer
    virtual std::vector<uint8_t> ReadFile(const std::string& path) = 0;
    
    // 6. Escreve buffer em arquivo
    virtual bool WriteFile(
        const std::string& path, 
        const std::vector<uint8_t>& data
    ) = 0;
    
    // 7. Obtém caminho completo de uma fonte
    virtual std::string GetSourcePath(FileSource source) const = 0;
    
    // 8. Extrai assets do APK para storage
    virtual bool ExtractAssetsIfNeeded(
        const std::vector<std::string>& assetPaths
    ) = 0;
    
    // 9. Registra redirecionamento de arquivo
    virtual void RegisterFileRedirect(
        const std::string& originalPath, 
        const std::string& newPath
    ) = 0;
    
    // 10. Resolve caminho considerando redirecionamentos
    virtual std::string ResolvePath(const std::string& path) const = 0;
};
```

**Linhas**: ~80
**Dependências**: `FileSource.h`, `<vector>`, `<string>`, `<cstdint>`

---

### 3. `core/filesystem/FileSystemService.h`

**Propósito**: Header da implementação principal

**Membros Privados**:
```cpp
class FileSystemService : public IFileSystemService {
private:
    std::string m_storagePath;              // Caminho base do storage
    void* m_assetManager = nullptr;         // AAssetManager* (JNI)
    bool m_assetsExtracted = false;         // Flag de extração
    std::map<std::string, std::string> m_redirects;  // Redirecionamentos
    mutable std::mutex m_redirectsMutex;    // Thread-safety
    std::vector<FileSource> m_fallbackOrder; // Ordem de fallback
```

**Funções Públicas** (12 funções):
```cpp
public:
    FileSystemService();                    // Construtor
    ~FileSystemService() override;          // Destrutor
    
    // Inicialização
    void Initialize(const std::string& storagePath);
    void SetAssetManager(void* assetManager);
    
    // Todas as funções da interface IFileSystemService (10 funções)
    FileHandle OpenFile(...) override;
    FileHandle OpenFileFromSource(...) override;
    bool FileExists(...) override;
    bool FileExistsInSource(...) override;
    std::vector<uint8_t> ReadFile(...) override;
    bool WriteFile(...) override;
    std::string GetSourcePath(...) const override;
    bool ExtractAssetsIfNeeded(...) override;
    void RegisterFileRedirect(...) override;
    std::string ResolvePath(...) const override;
```

**Funções Privadas** (8 funções):
```cpp
private:
    FileHandle OpenFromStorage(const std::string& path, FileOpenMode mode);
    FileHandle OpenFromAPK(const std::string& path);
    bool ExtractAssetToStorage(const std::string& assetPath, const std::string& destPath);
    std::string GetStoragePath(const std::string& relativePath) const;
    std::string GetCachePath(const std::string& relativePath) const;
    FILE* OpenNativeFile(const std::string& fullPath, FileOpenMode mode);
    static std::string NormalizePath(const std::string& path);
    static bool CreateDirectoryRecursive(const std::string& path);
```

**Linhas**: ~120
**Dependências**: `IFileSystemService.h`, `<map>`, `<mutex>`, `<vector>`

---

### 4. `core/filesystem/FileSystemService.cpp`

**Propósito**: Implementação completa do serviço

**Funções Principais** (20 funções):

#### 4.1 Construtor/Destrutor
```cpp
FileSystemService::FileSystemService() {
    // Inicializa ordem de fallback: Storage → APK → Cache
    m_fallbackOrder = {
        FileSource::EXTERNAL_STORAGE,
        FileSource::APK_ASSETS,
        FileSource::CACHE
    };
}

FileSystemService::~FileSystemService() = default;
```

#### 4.2 Inicialização
```cpp
void FileSystemService::Initialize(const std::string& storagePath) {
    // Normaliza e armazena path
    // Log de inicialização
}

void FileSystemService::SetAssetManager(void* assetManager) {
    // Armazena ponteiro para AAssetManager
}
```

#### 4.3 Abertura de Arquivos
```cpp
FileHandle FileSystemService::OpenFile(const std::string& path, FileOpenMode mode) {
    // 1. Resolve redirecionamentos
    // 2. Se escrita: apenas storage/cache
    // 3. Se leitura: tenta todas as fontes em ordem
    // 4. Retorna handle válido ou vazio
}

FileHandle FileSystemService::OpenFileFromSource(...) {
    // Abre de fonte específica
    // Delega para OpenFromStorage() ou OpenFromAPK()
}

FileHandle FileSystemService::OpenFromStorage(...) {
    // Constrói path completo
    // Chama OpenNativeFile()
    // Retorna FileHandle
}

FileHandle FileSystemService::OpenFromAPK(...) {
    // 1. Verifica se já foi extraído para cache
    // 2. Se não, extrai via ExtractAssetToStorage()
    // 3. Abre do cache via OpenFromStorage()
}
```

#### 4.4 Verificação de Existência
```cpp
bool FileSystemService::FileExists(const std::string& path) {
    // Resolve redirecionamentos
    // Tenta todas as fontes em ordem
    // Retorna true se encontrado em qualquer fonte
}

bool FileSystemService::FileExistsInSource(...) {
    // Verifica em fonte específica:
    // - Storage: stat()
    // - APK: AssetManagerBridge::AssetExists()
    // - Cache: stat()
}
```

#### 4.5 Leitura/Escrita
```cpp
std::vector<uint8_t> FileSystemService::ReadFile(const std::string& path) {
    // 1. Abre arquivo
    // 2. Obtém tamanho (fseek/ftell)
    // 3. Lê dados (fread)
    // 4. Fecha arquivo
    // 5. Retorna buffer
}

bool FileSystemService::WriteFile(...) {
    // 1. Abre arquivo para escrita
    // 2. Escreve dados (fwrite)
    // 3. Fecha arquivo
    // 4. Retorna sucesso
}
```

#### 4.6 Extração de Assets
```cpp
bool FileSystemService::ExtractAssetsIfNeeded(...) {
    // 1. Verifica flag .assets_extracted
    // 2. Se não existe, extrai todos os assets
    // 3. Cria flag após sucesso
    // 4. Retorna sucesso
}

bool FileSystemService::ExtractAssetToStorage(...) {
    // 1. Lê asset do APK via AssetManagerBridge
    // 2. Cria diretório de destino
    // 3. Escreve arquivo
    // 4. Retorna sucesso
}
```

#### 4.7 Redirecionamentos
```cpp
void FileSystemService::RegisterFileRedirect(...) {
    // Thread-safe: lock mutex
    // Adiciona ao mapa m_redirects
    // Log de registro
}

std::string FileSystemService::ResolvePath(...) const {
    // Thread-safe: lock mutex
    // Consulta mapa m_redirects
    // Retorna path original ou redirecionado
}
```

#### 4.8 Utilitários
```cpp
std::string FileSystemService::GetStoragePath(...) const {
    // Concatena m_storagePath + relativePath
    // Normaliza path
}

std::string FileSystemService::GetCachePath(...) const {
    // Retorna m_storagePath + "cache/" + relativePath
}

FILE* FileSystemService::OpenNativeFile(...) {
    // Converte FileOpenMode para string
    // Chama fopen()
    // Retorna FILE* ou nullptr
}

static std::string FileSystemService::NormalizePath(...) {
    // Substitui backslashes por forward slashes
    // Remove barras duplicadas
}

static bool FileSystemService::CreateDirectoryRecursive(...) {
    // Cria diretórios recursivamente
    // Usa mkdir() com permissões
}
```

**Linhas**: ~450
**Dependências**: `FileSystemService.h`, `AssetManagerBridge.h`, `Log.h`, `<fstream>`, `<sys/stat.h>`

---

### 5. `core/filesystem/FileLoader.h`

**Propósito**: Wrapper para compatibilidade com código legado

**Funções Estáticas** (3 funções):
```cpp
class FileLoader {
public:
    // 1. fopen() replacement
    static FILE* Open(const char* path, const char* mode);
    
    // 2. Para hooks (NvFOpen)
    static FILE* OpenForGame(const char* gamePath);
    
    // 3. Registra redirecionamentos do jogo
    static void RegisterGameRedirects(IFileSystemService* fs);
};
```

**Linhas**: ~40
**Dependências**: `IFileSystemService.h`, `<cstdio>`

---

### 6. `core/filesystem/FileLoader.cpp`

**Propósito**: Implementação do wrapper

**Funções** (3 funções):

```cpp
FILE* FileLoader::Open(const char* path, const char* mode) {
    // 1. Obtém FileSystemService do ServiceLocator
    // 2. Converte mode string para FileOpenMode
    // 3. Chama FileSystemService::OpenFile()
    // 4. Retorna FILE* do handle
}

FILE* FileLoader::OpenForGame(const char* gamePath) {
    // 1. Obtém FileSystemService
    // 2. Resolve path (redirecionamentos automáticos)
    // 3. Abre arquivo
    // 4. Retorna FILE*
}

void FileLoader::RegisterGameRedirects(IFileSystemService* fs) {
    // Registra todos os redirecionamentos do GTA:
    fs->RegisterFileRedirect("DATA/GTA.DAT", "SAMP/gta.dat");
    fs->RegisterFileRedirect("DATA/PEDS.IDE", "SAMP/peds.ide");
    fs->RegisterFileRedirect("DATA/VEHICLES.IDE", "SAMP/vehicles.ide");
    fs->RegisterFileRedirect("DATA/HANDLING.CFG", "SAMP/handling.cfg");
    fs->RegisterFileRedirect("DATA/WEAPON.DAT", "SAMP/weapon.dat");
    fs->RegisterFileRedirect("DATA/FONTS.DAT", "data/fonts.dat");
    fs->RegisterFileRedirect("DATA/PEDSTATS.DAT", "data/pedstats.dat");
    fs->RegisterFileRedirect("DATA/TIMECYC.DAT", "data/timecyc.dat");
    fs->RegisterFileRedirect("DATA/POPCYCLE.DAT", "data/popcycle.dat");
    fs->RegisterFileRedirect("mainV1.scm", "SAMP/main.scm");
    fs->RegisterFileRedirect("SCRIPTV1.IMG", "SAMP/script.img");
}
```

**Linhas**: ~150
**Dependências**: `FileLoader.h`, `ServiceLocator.h`, `Log.h`, `<cstring>`

---

### 7. `platform/android/AssetManagerBridge.h`

**Propósito**: Ponte JNI para AssetManager do Android

**Funções Estáticas** (3 funções):
```cpp
namespace Platform::Android {

class AssetManagerBridge {
public:
    // 1. Converte jobject para AAssetManager*
    static void* GetAssetManagerFromJNI(void* jniEnv, void* context);
    
    // 2. Verifica se asset existe no APK
    static bool AssetExists(void* assetManager, const std::string& path);
    
    // 3. Lê asset completo do APK
    static std::vector<uint8_t> ReadAsset(
        void* assetManager, 
        const std::string& path
    );
};

} // namespace
```

**Linhas**: ~30
**Dependências**: `<android/asset_manager.h>`, `<vector>`, `<string>`

---

### 8. `platform/android/AssetManagerBridge.cpp`

**Propósito**: Implementação da ponte JNI

**Funções** (3 funções):

```cpp
void* AssetManagerBridge::GetAssetManagerFromJNI(...) {
    // 1. Obtém AssetManager do Context via JNI
    // 2. Chama AAssetManager_fromJava()
    // 3. Retorna AAssetManager*
}

bool AssetManagerBridge::AssetExists(...) {
    // 1. AAssetManager_open() com AASSET_MODE_UNKNOWN
    // 2. Se não null, fecha e retorna true
    // 3. Retorna false se null
}

std::vector<uint8_t> AssetManagerBridge::ReadAsset(...) {
    // 1. AAssetManager_open() com AASSET_MODE_BUFFER
    // 2. AAsset_getLength() para tamanho
    // 3. AAsset_read() para ler dados
    // 4. AAsset_close()
    // 5. Retorna buffer
}
```

**Linhas**: ~200
**Dependências**: `AssetManagerBridge.h`, `<android/asset_manager_jni.h>`, `<jni.h>`

---

### 9. `game/hooks/FileHooks.cpp`

**Propósito**: Hook NvFOpen atualizado

**Funções** (1 função):

```cpp
struct stFile {
    int isFileExist;
    FILE *f;
};

stFile* NvFOpen(const char* r0, const char* r1, int r2, int r3) {
    // 1. Usa FileLoader::OpenForGame() em vez de fopen() direto
    FILE* file = FileLoader::OpenForGame(r1);
    
    // 2. Se falhou, retorna nullptr
    if (!file) {
        FLog("NvFOpen: file not found (%s)", r1);
        return nullptr;
    }
    
    // 3. Cria estrutura stFile (compatível com código legado)
    stFile* st = (stFile*)malloc(sizeof(stFile));
    st->isFileExist = true;
    st->f = file;
    
    return st;
}
```

**Linhas**: ~50
**Dependências**: `FileLoader.h`, `Log.h`, `<cstdlib>`

---

## 🔄 Fluxo de Chamadas

### Exemplo: Carregar `DATA/GTA.DAT`

```
1. libGTASA chama NvFOpen("DATA/GTA.DAT", ...)
   ↓
2. FileHooks.cpp::NvFOpen() intercepta
   ↓
3. FileLoader::OpenForGame("DATA/GTA.DAT")
   ↓
4. FileSystemService::ResolvePath("DATA/GTA.DAT")
   → Consulta m_redirects
   → Retorna "SAMP/gta.dat"
   ↓
5. FileSystemService::OpenFile("SAMP/gta.dat", READ_BINARY)
   ↓
6. Tenta fontes em ordem:
   a) OpenFromStorage("SAMP/gta.dat")
      → GetStoragePath() → "/storage/.../SAMP/gta.dat"
      → OpenNativeFile() → fopen()
      → ✅ Sucesso? Retorna handle
      → ❌ Falha? Próxima fonte
   
   b) OpenFromAPK("SAMP/gta.dat")
      → FileExistsInSource(APK_ASSETS)?
      → ExtractAssetToStorage() → cache/
      → OpenFromStorage("cache/SAMP/gta.dat")
      → ✅ Sucesso? Retorna handle
   
   c) OpenFromStorage("cache/SAMP/gta.dat")
      → Última tentativa
   ↓
7. FileHandle retornado
   ↓
8. FILE* extraído do handle
   ↓
9. stFile criado e retornado
   ↓
10. libGTASA usa FILE* normalmente
```

---

## 📊 Resumo de Funções

### Por Categoria

**Interface (IFileSystemService)**: 10 funções
- Abertura: 2 funções
- Verificação: 2 funções
- Leitura/Escrita: 2 funções
- Configuração: 3 funções
- Assets: 1 função

**Implementação (FileSystemService)**: 20 funções
- Públicas: 12 funções (10 interface + 2 inicialização)
- Privadas: 8 funções

**Wrapper (FileLoader)**: 3 funções
- Compatibilidade: 2 funções
- Configuração: 1 função

**Bridge (AssetManagerBridge)**: 3 funções
- JNI: 1 função
- Acesso: 2 funções

**Hook (FileHooks)**: 1 função
- Interceptação: 1 função

**Total**: 37 funções principais

---

## 🔗 Dependências entre Módulos

```
FileLoader
    ↓ usa
FileSystemService (implementa)
    ↓ implementa
IFileSystemService (interface)
    ↑ usa
AssetManagerBridge
    ↑ usa
JNI/AAssetManager

FileHooks
    ↓ usa
FileLoader
```

---

## 📝 Notas de Implementação

### Thread Safety
- `m_redirectsMutex`: Protege operações no mapa de redirecionamentos
- Operações de arquivo individuais: Não thread-safe (como `fopen()` padrão)
- Cada thread deve ter seu próprio `FILE*` handle

### Memory Management
- `FileHandle`: Não gerencia memória do `FILE*`
- Código legado continua responsável por `fclose()`
- `FileSystemService`: Não deleta `FILE*` automaticamente

### Error Handling
- Não lança exceções (compatibilidade C)
- Retorna `nullptr` ou valores vazios em caso de erro
- Logging via `FLog()` / `LOGI()` / `LOGW()` / `LOGE()`

### Path Handling
- Todos os paths são normalizados
- Backslashes convertidos para forward slashes
- Barras duplicadas removidas
- Paths relativos são resolvidos contra `m_storagePath`

---

**Documento criado em**: 2024
**Versão**: 1.0
