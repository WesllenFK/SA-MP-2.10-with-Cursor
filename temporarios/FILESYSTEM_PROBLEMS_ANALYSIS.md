# Análise Detalhada de Problemas: Modificação do Sistema de Arquivos

## 📋 Índice

1. [Problemas de Inicialização e Ordem](#problemas-de-inicialização-e-ordem)
2. [Problemas de Thread Safety](#problemas-de-thread-safety)
3. [Problemas de Memória e Recursos](#problemas-de-memória-e-recursos)
4. [Problemas de Buffer Overflow e Paths](#problemas-de-buffer-overflow-e-paths)
5. [Problemas de Compatibilidade com libGTASA](#problemas-de-compatibilidade-com-libgtasa)
6. [Problemas de Performance](#problemas-de-performance)
7. [Problemas de Sincronização JNI](#problemas-de-sincronização-jni)
8. [Problemas de Extração de Assets](#problemas-de-extração-de-assets)
9. [Problemas de Fallback e Erros](#problemas-de-fallback-e-erros)
10. [Problemas Específicos do Android](#problemas-específicos-do-android)
11. [Problemas de Migração Gradual](#problemas-de-migração-gradual)
12. [Cenários de Falha Crítica](#cenários-de-falha-crítica)

---

## 1. Problemas de Inicialização e Ordem

### 1.1 Race Condition na Inicialização

**Problema**: `FileSystemService` pode ser usado antes de ser inicializado

**Cenário**:
```cpp
// Thread 1: Bootstrap (Fase Platform)
ServiceLocator::Register<IFileSystemService>(new FileSystemService());
// FileSystemService criado mas NÃO inicializado (sem storage path)

// Thread 2: Hooks instalados (Fase Game)
NvFOpen("DATA/GTA.DAT", ...)  // Chamado ANTES de setStoragePath()
→ FileSystemService::OpenFile() 
→ m_storagePath está vazio!
→ Falha silenciosa ou crash
```

**Causa Raiz**:
- `FileSystemService` criado no Bootstrap (Fase Platform)
- `setStoragePath()` chamado depois no `SAMP.onCreate()` (Java)
- Hooks podem ser instalados antes de `setStoragePath()`

**Solução**:
```cpp
// FileSystemService.cpp
FileHandle FileSystemService::OpenFile(...) {
    if (m_storagePath.empty()) {
        LOGW("FileSystemService not initialized, waiting...");
        // Opção 1: Retornar nullptr e esperar inicialização
        // Opção 2: Bloquear até inicialização (não recomendado - pode deadlock)
        // Opção 3: Usar fallback temporário
        return FileHandle{};
    }
    // ... resto do código
}
```

**Mitigação**:
- ✅ Verificar `m_storagePath.empty()` em todas as operações
- ✅ Logging claro quando não inicializado
- ✅ Documentar ordem de inicialização obrigatória

---

### 1.2 AssetManager Não Definido

**Problema**: `ExtractAssetsIfNeeded()` chamado antes de `SetAssetManager()`

**Cenário**:
```cpp
// Bootstrap Fase Game
fs->ExtractAssetsIfNeeded(assets);  // Chamado
// Mas SetAssetManager() ainda não foi chamado (SAMP.onCreate() não executou)

// ExtractAssetsIfNeeded() tenta usar m_assetManager
→ m_assetManager == nullptr
→ ExtractAssetToStorage() falha silenciosamente
→ Assets não extraídos
→ Jogo falha ao carregar arquivos
```

**Causa Raiz**:
- `ExtractAssetsIfNeeded()` chamado no Bootstrap (Fase Game)
- `SetAssetManager()` chamado depois em `SAMP.onCreate()`
- Ordem de execução não garantida

**Solução**:
```cpp
bool FileSystemService::ExtractAssetsIfNeeded(...) {
    // Verifica se AssetManager está disponível
    if (!m_assetManager) {
        LOGW("AssetManager not set, skipping extraction. Will retry on next file access.");
        return false;  // Não é erro fatal, apenas adia extração
    }
    
    // ... resto do código
}

// E em OpenFromAPK():
FileHandle FileSystemService::OpenFromAPK(...) {
    if (!m_assetManager) {
        // Tenta obter AssetManager do ServiceLocator ou JNI
        // Se não disponível, retorna handle vazio
        return FileHandle{};
    }
    // ... extração sob demanda
}
```

**Mitigação**:
- ✅ Extração sob demanda (lazy) em vez de upfront
- ✅ Verificar `m_assetManager` antes de usar
- ✅ Retry automático quando AssetManager ficar disponível

---

### 1.3 Dependência Circular de Inicialização

**Problema**: `FileSystemService` precisa de `ServiceLocator`, mas `ServiceLocator` pode precisar de arquivos

**Cenário**:
```cpp
// ServiceLocator precisa carregar configuração de arquivo
ServiceLocator::Initialize() {
    // Tenta carregar config de arquivo
    FileSystemService::ReadFile("config.json");  // ❌ FileSystemService não registrado ainda!
}
```

**Causa Raiz**: Dependência circular não identificada

**Solução**:
- ✅ `FileSystemService` não deve depender de outros serviços na inicialização
- ✅ Configurações devem ser carregadas depois de todos os serviços registrados
- ✅ Usar inicialização em fases (Bootstrap já faz isso)

---

## 2. Problemas de Thread Safety

### 2.1 Acesso Concorrente a `m_redirects`

**Problema**: Múltiplas threads acessando mapa de redirecionamentos simultaneamente

**Cenário**:
```cpp
// Thread 1 (Main): Carregando arquivo
FileSystemService::ResolvePath("DATA/GTA.DAT")
→ lock_guard lock(m_redirectsMutex)  // Lock adquirido
→ Consulta m_redirects

// Thread 2 (CdStreamThread): Registrando novo redirecionamento
FileSystemService::RegisterFileRedirect("DATA/NEW.DAT", "SAMP/new.dat")
→ lock_guard lock(m_redirectsMutex)  // Bloqueia esperando Thread 1
→ Deadlock potencial se houver outro lock
```

**Causa Raiz**:
- `m_redirects` é compartilhado entre threads
- `CdStreamThread` (thread secundária) pode carregar arquivos
- Main thread também carrega arquivos

**Solução**:
```cpp
// FileSystemService.cpp
std::string FileSystemService::ResolvePath(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_redirectsMutex);  // ✅ Já implementado
    auto it = m_redirects.find(path);
    if (it != m_redirects.end()) {
        return it->second;
    }
    return path;
}

void FileSystemService::RegisterFileRedirect(...) {
    std::lock_guard<std::mutex> lock(m_redirectsMutex);  // ✅ Já implementado
    m_redirects[originalPath] = newPath;
}
```

**Mitigação**:
- ✅ Mutex já implementado (bom!)
- ✅ Verificar que TODAS as operações no mapa usam mutex
- ✅ Evitar locks aninhados
- ✅ Documentar thread-safety de cada função

---

### 2.2 FILE* Compartilhado Entre Threads

**Problema**: `FILE*` retornado pode ser usado em thread diferente da que abriu

**Cenário**:
```cpp
// Thread 1 (Main)
FILE* file = FileLoader::OpenForGame("DATA/GTA.DAT");

// Thread 2 (CdStreamThread) - usa o mesmo FILE*
fread(buffer, 1, size, file);  // ❌ FILE* não é thread-safe!
→ Race condition
→ Dados corrompidos
→ Crash
```

**Causa Raiz**:
- `FILE*` padrão do C não é thread-safe
- `fread()`, `fseek()`, etc. não são thread-safe
- Código legado pode compartilhar `FILE*` entre threads

**Solução**:
```cpp
// FileSystemService NÃO deve compartilhar FILE* entre threads
// Cada thread deve abrir seu próprio handle

// Documentação clara:
// "FileHandle retornado por OpenFile() NÃO é thread-safe.
//  Cada thread deve abrir seu próprio handle.
//  Não compartilhe FILE* entre threads."
```

**Mitigação**:
- ✅ Documentar claramente que `FILE*` não é thread-safe
- ✅ Cada thread abre seu próprio handle
- ✅ Não armazenar `FILE*` em variáveis globais compartilhadas
- ✅ Considerar wrapper thread-safe no futuro (opcional)

---

### 2.3 Race Condition na Extração de Assets

**Problema**: Múltiplas threads tentando extrair o mesmo asset simultaneamente

**Cenário**:
```cpp
// Thread 1 (Main)
OpenFromAPK("SAMP/main.scm")
→ Verifica se existe no cache: NÃO
→ Inicia extração...

// Thread 2 (CdStreamThread) - simultaneamente
OpenFromAPK("SAMP/main.scm")
→ Verifica se existe no cache: AINDA NÃO (Thread 1 ainda extraindo)
→ Inicia extração também!
→ Duas threads escrevendo o mesmo arquivo
→ Corrupção de dados
```

**Causa Raiz**:
- Verificação de existência e escrita não são atômicas
- Sem lock durante extração

**Solução**:
```cpp
// FileSystemService.cpp
std::mutex m_extractionMutex;  // NOVO: Mutex para extrações
std::set<std::string> m_extracting;  // NOVO: Assets sendo extraídos

FileHandle FileSystemService::OpenFromAPK(const std::string& path) {
    std::string cachePath = GetCachePath(path);
    
    // Verifica se já existe
    if (FileExistsInSource(cachePath, FileSource::EXTERNAL_STORAGE)) {
        return OpenFromStorage(cachePath, FileOpenMode::READ_BINARY);
    }
    
    // Lock para extração
    {
        std::lock_guard<std::mutex> lock(m_extractionMutex);
        
        // Verifica novamente (double-check)
        if (FileExistsInSource(cachePath, FileSource::EXTERNAL_STORAGE)) {
            return OpenFromStorage(cachePath, FileOpenMode::READ_BINARY);
        }
        
        // Verifica se outra thread já está extraindo
        if (m_extracting.find(path) != m_extracting.end()) {
            // Aguarda extração completar (ou retorna e tenta novamente)
            return FileHandle{};  // Retry depois
        }
        
        // Marca como extraindo
        m_extracting.insert(path);
    }
    
    // Extrai (fora do lock para não bloquear outras extrações)
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

**Mitigação**:
- ✅ Mutex para extrações
- ✅ Double-check locking pattern
- ✅ Set de assets sendo extraídos
- ✅ Timeout para evitar deadlock

---

### 2.4 Race Condition no `g_pszStorage`

**Problema**: `g_pszStorage` acessado sem lock enquanto pode ser modificado

**Cenário**:
```cpp
// Thread 1 (Main)
g_pszStorage = newPath;  // Modificando

// Thread 2 (CdStreamThread) - simultaneamente
if (g_pszStorage == nullptr) {  // ❌ Race condition!
    // Pode ler valor parcial ou nullptr
}
```

**Causa Raiz**:
- `g_pszStorage` é global e acessado sem sincronização
- Modificação e leitura não são atômicas

**Solução**:
```cpp
// Opção 1: Manter g_pszStorage apenas para compatibilidade
// FileSystemService gerencia seu próprio storage path
// g_pszStorage é preenchido uma vez e nunca mais modificado

// Opção 2: Usar std::atomic ou mutex
std::atomic<char*> g_pszStorage;  // Thread-safe

// Opção 3: Remover dependência de g_pszStorage completamente
// Migrar todo código para usar FileSystemService
```

**Mitigação**:
- ✅ `g_pszStorage` definido UMA vez no início (JNI)
- ✅ Depois disso, apenas leitura (não modificar)
- ✅ Documentar que é read-only após inicialização
- ✅ Migrar código para não usar `g_pszStorage` diretamente

---

## 3. Problemas de Memória e Recursos

### 3.1 FILE* Não Fechado (Memory Leak)

**Problema**: `FILE*` retornado pode não ser fechado pelo código legado

**Cenário**:
```cpp
// Código legado
FILE* file = CFileMgr::OpenFile("data.txt", "r");
// ... usa arquivo
// ❌ Esquece de chamar fclose(file)
→ Memory leak
→ File descriptor leak
→ Limite de arquivos abertos atingido
```

**Causa Raiz**:
- Código legado pode não fechar arquivos
- `FileSystemService` retorna `FILE*` mas não gerencia ciclo de vida
- Sem RAII para `FILE*`

**Solução**:
```cpp
// Opção 1: Wrapper RAII (recomendado para código novo)
class FileHandleRAII {
    FILE* m_file;
public:
    FileHandleRAII(FILE* file) : m_file(file) {}
    ~FileHandleRAII() { if (m_file) fclose(m_file); }
    FILE* get() { return m_file; }
};

// Opção 2: Documentar claramente responsabilidade
// "Código que chama OpenFile() é responsável por fclose()"

// Opção 3: Tracking de arquivos abertos (debug apenas)
#ifdef DEBUG
std::set<FILE*> s_openFiles;
void TrackFileOpen(FILE* f) { s_openFiles.insert(f); }
void TrackFileClose(FILE* f) { s_openFiles.erase(f); }
// No shutdown, verificar se há arquivos não fechados
#endif
```

**Mitigação**:
- ✅ Documentar claramente responsabilidade de `fclose()`
- ✅ Wrapper RAII para código novo
- ✅ Tracking em debug para detectar leaks
- ✅ Valgrind/AddressSanitizer para detectar leaks

---

### 3.2 Buffer Overflow em Paths

**Problema**: `sprintf()` pode causar buffer overflow

**Cenário**:
```cpp
// hooks.cpp (código atual)
static char path[255]{};
sprintf(path, "%s%s", g_pszStorage, r1);  // ❌ Se path > 255, overflow!

// Se g_pszStorage = "/storage/emulated/0/Android/data/com.samp.mobile/files/"
// E r1 = "DATA/VERY/LONG/PATH/TO/FILE.DAT"
// Total pode exceder 255 caracteres
→ Buffer overflow
→ Corrupção de memória
→ Crash ou comportamento indefinido
```

**Causa Raiz**:
- Buffers fixos (`char[255]`, `char[512]`)
- `sprintf()` não verifica limites
- Paths podem ser longos no Android

**Solução**:
```cpp
// FileSystemService.cpp
std::string FileSystemService::GetStoragePath(const std::string& relativePath) const {
    // Usa std::string (sem limite fixo)
    return m_storagePath + NormalizePath(relativePath);
}

// FileLoader.cpp
FILE* FileLoader::OpenForGame(const char* gamePath) {
    auto* fs = ServiceLocator::Get<IFileSystemService>();
    if (!fs) return nullptr;
    
    // Resolve path (retorna std::string, sem buffer fixo)
    std::string resolved = fs->ResolvePath(gamePath);
    
    // Abre via FileSystemService (usa std::string internamente)
    FileHandle handle = fs->OpenFile(resolved, FileOpenMode::READ_BINARY);
    return handle.file;
}
```

**Mitigação**:
- ✅ Usar `std::string` em vez de buffers fixos
- ✅ `snprintf()` se precisar de buffer fixo (com verificação)
- ✅ Validar tamanho máximo de paths
- ✅ Testar com paths muito longos

---

### 3.3 Memory Leak em `stFile` Não Liberado

**Problema**: `NvFOpen()` aloca `stFile` mas código legado pode não liberar

**Cenário**:
```cpp
// hooks.cpp (código atual)
stFile* NvFOpen(...) {
    stFile* st = (stFile*)malloc(sizeof(stFile));  // Aloca
    // ...
    return st;
}

// Código legado (libGTASA)
stFile* file = NvFOpen(...);
// ... usa arquivo
// ❌ Esquece de free(file)
→ Memory leak
```

**Causa Raiz**:
- `malloc()` sem `free()` correspondente
- Código legado pode não gerenciar memória corretamente

**Solução**:
```cpp
// Verificar se libGTASA tem função de fechamento correspondente
// Se não, implementar hook para NvFClose() se existir

// Ou usar smart pointer (se possível)
// Mas cuidado: libGTASA espera ponteiro bruto
```

**Mitigação**:
- ✅ Verificar se há `NvFClose()` correspondente
- ✅ Implementar hook de fechamento se necessário
- ✅ Tracking de `stFile` alocados (debug)
- ✅ Documentar responsabilidade de liberação

---

## 4. Problemas de Buffer Overflow e Paths

### 4.1 Path Muito Longo

**Problema**: Paths do Android podem exceder limites

**Cenário**:
```cpp
// Android path típico:
/storage/emulated/0/Android/data/com.samp.mobile/files/SAMP/main.scm
// = 75 caracteres (OK)

// Mas se houver redirecionamentos aninhados ou paths muito profundos:
/storage/emulated/0/Android/data/com.samp.mobile/files/cache/SAMP/very/deep/path/to/file.dat
// = 100+ caracteres

// Se usar buffer fixo:
char path[255];  // Pode ser suficiente, mas...
sprintf(path, "%s%s", storage, relative);  // ❌ Se total > 255, overflow
```

**Causa Raiz**:
- Paths do Android são longos
- Redirecionamentos podem adicionar profundidade
- Buffers fixos não são seguros

**Solução**:
```cpp
// SEMPRE usar std::string
std::string FileSystemService::GetStoragePath(const std::string& relativePath) const {
    return m_storagePath + NormalizePath(relativePath);
}

// Se precisar converter para char* (para APIs legadas):
std::string fullPath = GetStoragePath(relativePath);
if (fullPath.length() >= PATH_MAX) {
    LOGE("Path too long: %s", fullPath.c_str());
    return nullptr;
}
// Usar fullPath.c_str() temporariamente
```

**Mitigação**:
- ✅ Usar `std::string` sempre
- ✅ Validar `PATH_MAX` (4096 no Linux/Android)
- ✅ Logging quando path muito longo
- ✅ Testar com paths extremos

---

### 4.2 Path Injection (Segurança)

**Problema**: Paths maliciosos podem acessar arquivos fora do diretório do app

**Cenário**:
```cpp
// Código malicioso ou bug
std::string maliciousPath = "../../../system/etc/passwd";

FileSystemService::OpenFile(maliciousPath, ...)
→ GetStoragePath() = "/storage/.../files/../../../system/etc/passwd"
→ fopen() pode acessar arquivo do sistema!  // ❌ SEGURANÇA!
```

**Causa Raiz**:
- Paths não são sanitizados
- `..` permite sair do diretório
- Sem validação de paths

**Solução**:
```cpp
std::string FileSystemService::NormalizePath(const std::string& path) {
    std::string normalized = path;
    
    // Remove ".." e "." perigosos
    // Remove barras duplicadas
    // Garante que path não sai do diretório base
    
    // Verifica se path contém ".."
    if (normalized.find("..") != std::string::npos) {
        LOGE("Path contains '..', rejecting: %s", path.c_str());
        return "";  // Rejeita
    }
    
    // Remove barras duplicadas
    // ... implementação
    
    return normalized;
}

// E em GetStoragePath():
std::string FileSystemService::GetStoragePath(const std::string& relativePath) const {
    std::string normalized = NormalizePath(relativePath);
    if (normalized.empty()) {
        return "";  // Path inválido
    }
    
    std::string fullPath = m_storagePath + normalized;
    
    // Verifica se fullPath ainda está dentro de m_storagePath
    if (fullPath.find(m_storagePath) != 0) {
        LOGE("Path outside storage directory, rejecting: %s", fullPath.c_str());
        return "";
    }
    
    return fullPath;
}
```

**Mitigação**:
- ✅ Sanitizar todos os paths
- ✅ Rejeitar paths com `..`
- ✅ Validar que path está dentro do diretório base
- ✅ Testar com paths maliciosos

---

### 4.3 Caracteres Especiais em Paths

**Problema**: Paths com caracteres especiais podem causar problemas

**Cenário**:
```cpp
// Path com caracteres especiais
std::string path = "SAMP/file with spaces.dat";
// Ou
std::string path = "SAMP/file\nwith\nnewlines.dat";

// fopen() pode falhar ou comportamento indefinido
```

**Causa Raiz**:
- Paths podem conter caracteres inválidos
- `fopen()` não aceita alguns caracteres
- Sem validação de caracteres

**Solução**:
```cpp
std::string FileSystemService::NormalizePath(const std::string& path) {
    std::string normalized = path;
    
    // Remove caracteres inválidos
    normalized.erase(
        std::remove_if(normalized.begin(), normalized.end(),
            [](char c) {
                // Caracteres inválidos em paths Unix/Android
                return c == '\0' || c == '\n' || c == '\r' || 
                       c == '\t' || c == '\b';
            }),
        normalized.end()
    );
    
    // Substitui espaços por underscores (opcional)
    // std::replace(normalized.begin(), normalized.end(), ' ', '_');
    
    return normalized;
}
```

**Mitigação**:
- ✅ Validar e sanitizar caracteres
- ✅ Remover caracteres de controle
- ✅ Documentar caracteres permitidos
- ✅ Testar com paths com caracteres especiais

---

## 5. Problemas de Compatibilidade com libGTASA

### 5.1 libGTASA Espera Paths Específicos

**Problema**: libGTASA pode esperar paths em formato específico

**Cenário**:
```cpp
// libGTASA chama NvFOpen("DATA/GTA.DAT", ...)
// Espera que arquivo esteja em formato específico ou localização específica

// Novo sistema redireciona para "SAMP/gta.dat"
// Mas libGTASA pode ter lógica interna que depende do path original
→ Comportamento inesperado
→ Crash
```

**Causa Raiz**:
- libGTASA é binário fechado
- Lógica interna desconhecida
- Pode depender de paths específicos

**Solução**:
```cpp
// Manter compatibilidade máxima
// Redirecionar apenas quando necessário
// Testar extensivamente

// Opção: Manter arquivo no path original E no novo path
// (duplicação, mas garante compatibilidade)
```

**Mitigação**:
- ✅ Testar todos os arquivos críticos
- ✅ Manter fallback para comportamento antigo
- ✅ Logging detalhado para debug
- ✅ Opção de desabilitar redirecionamentos (flag de debug)

---

### 5.2 libGTASA Pode Fazer Seek em Arquivos

**Problema**: libGTASA pode fazer `fseek()` em arquivos abertos

**Cenário**:
```cpp
// libGTASA
FILE* file = NvFOpen("DATA/GTA.DAT", ...);
fseek(file, 100, SEEK_SET);  // Seek para posição 100
fread(buffer, 1, 50, file);  // Lê 50 bytes

// Se arquivo foi extraído do APK para cache:
// ✅ Deve funcionar normalmente (FILE* é válido)

// Mas se arquivo está sendo lido do APK via AssetManager:
// ❌ AAsset não suporta seek direto!
// Precisa ser extraído primeiro
```

**Causa Raiz**:
- `AAsset` (APK) não suporta `fseek()` direto
- Precisa ser extraído para filesystem primeiro
- `FILE*` de cache funciona normalmente

**Solução**:
```cpp
// Já implementado: OpenFromAPK() extrai para cache primeiro
// Depois abre do cache como FILE* normal
// ✅ Suporta fseek() normalmente
```

**Mitigação**:
- ✅ Sempre extrair para cache antes de abrir
- ✅ Nunca retornar `FILE*` diretamente do APK
- ✅ Testar `fseek()` em todos os arquivos
- ✅ Verificar que `ftell()` funciona

---

### 5.3 libGTASA Pode Ler Arquivos em Chunks

**Problema**: libGTASA pode ler arquivo em múltiplas chamadas `fread()`

**Cenário**:
```cpp
// libGTASA
FILE* file = NvFOpen("SAMP/script.img", ...);
fread(buffer1, 1, 1024, file);  // Lê primeiros 1024 bytes
fread(buffer2, 1, 1024, file);  // Lê próximos 1024 bytes
// ... continua lendo

// Se arquivo foi extraído para cache:
// ✅ Funciona normalmente (FILE* mantém posição)

// Mas se extração falhou parcialmente:
// ❌ Arquivo corrompido
// ❌ fread() retorna dados incorretos
```

**Causa Raiz**:
- Leitura sequencial depende de arquivo válido
- Extração parcial pode corromper arquivo
- Sem verificação de integridade

**Solução**:
```cpp
bool FileSystemService::ExtractAssetToStorage(...) {
    // 1. Lê asset completo do APK
    std::vector<uint8_t> data = AssetManagerBridge::ReadAsset(...);
    if (data.empty()) {
        return false;
    }
    
    // 2. Cria diretório
    CreateDirectoryRecursive(dir);
    
    // 3. Escreve arquivo completo de uma vez (atômico)
    // Opção A: Escreve para arquivo temporário primeiro
    std::string tempPath = destPath + ".tmp";
    if (!WriteFile(tempPath, data)) {
        return false;
    }
    
    // Opção B: Renomeia (atômico no Linux/Android)
    if (rename(tempPath.c_str(), destPath.c_str()) != 0) {
        unlink(tempPath.c_str());  // Limpa temp
        return false;
    }
    
    return true;
}
```

**Mitigação**:
- ✅ Extração atômica (temp file + rename)
- ✅ Verificar integridade após extração (checksum opcional)
- ✅ Retry em caso de falha
- ✅ Logging de erros de extração

---

## 6. Problemas de Performance

### 6.1 Extração de Assets Lenta na Primeira Execução

**Problema**: Extrair 15-60MB de assets pode levar vários segundos

**Cenário**:
```cpp
// Primeira execução
ExtractAssetsIfNeeded(assets);  // 15-60MB
→ Leitura do APK (lenta - APK é ZIP)
→ Escrita no storage (pode ser lenta em SD cards)
→ 5-30 segundos de delay
→ Usuário vê tela preta
→ Pode pensar que app travou
```

**Causa Raiz**:
- APK é arquivo ZIP (descompactação lenta)
- Storage pode ser lento (SD card)
- Extração bloqueante na thread principal

**Solução**:
```cpp
// Opção 1: Extração assíncrona (recomendado)
void FileSystemService::ExtractAssetsIfNeededAsync(...) {
    std::thread([this, assetPaths]() {
        ExtractAssetsIfNeeded(assetPaths);
        // Notifica UI quando completo
    }).detach();
}

// Opção 2: Extração sob demanda (lazy)
// Não extrair tudo upfront
// Extrair apenas quando arquivo é acessado
// Primeiro acesso pode ser lento, mas não bloqueia inicialização

// Opção 3: Mostrar progresso
// UI mostra "Extracting assets... 5/12"
// Usuário vê que algo está acontecendo
```

**Mitigação**:
- ✅ Extração em background thread
- ✅ Mostrar progresso ao usuário
- ✅ Extração lazy (sob demanda)
- ✅ Cache de flag para evitar reextração

---

### 6.2 Overhead do Novo Sistema

**Problema**: Novo sistema adiciona overhead em cada abertura de arquivo

**Cenário**:
```cpp
// ANTES (direto)
fopen(path, "rb");  // ~0.1ms

// DEPOIS (via novo sistema)
FileLoader::OpenForGame(path)
→ ServiceLocator::Get()  // Lookup
→ ResolvePath()  // Consulta mapa
→ OpenFile()  // Tenta múltiplas fontes
→ fopen()  // Finalmente abre
// Total: ~0.5-1ms (5-10x mais lento)
```

**Causa Raiz**:
- Múltiplas camadas de abstração
- Lookups e verificações adicionais
- Overhead de fallback

**Solução**:
```cpp
// Otimizações:

// 1. Cache de ServiceLocator lookup
static IFileSystemService* s_cachedFS = nullptr;
if (!s_cachedFS) {
    s_cachedFS = ServiceLocator::Get<IFileSystemService>();
}

// 2. Cache de redirecionamentos resolvidos
std::map<std::string, std::string> s_resolvedCache;

// 3. Fast path para arquivos no storage
// Se arquivo existe no storage, não tenta outras fontes

// 4. Profiling para identificar gargalos
```

**Mitigação**:
- ✅ Cache de lookups frequentes
- ✅ Fast path para casos comuns
- ✅ Profiling para medir overhead real
- ✅ Otimizar apenas se necessário (premature optimization é ruim)

---

### 6.3 Múltiplas Tentativas de Fallback

**Problema**: Fallback tenta múltiplas fontes mesmo quando não necessário

**Cenário**:
```cpp
// Arquivo existe no storage
OpenFile("SAMP/settings.ini")
→ Tenta Storage: ✅ Encontrado
→ Mas ainda tenta APK (desnecessário)
→ E ainda tenta Cache (desnecessário)
→ Overhead desnecessário
```

**Causa Raiz**:
- Fallback sempre tenta todas as fontes
- Não para na primeira fonte que funciona

**Solução**:
```cpp
// Já implementado corretamente:
FileHandle FileSystemService::OpenFile(...) {
    // Tenta fontes em ordem
    for (FileSource source : m_fallbackOrder) {
        FileHandle handle = OpenFileFromSource(resolvedPath, source, mode);
        if (handle.isValid()) {
            return handle;  // ✅ Para na primeira que funciona
        }
    }
    return FileHandle{};
}
```

**Mitigação**:
- ✅ Já implementado corretamente (para na primeira fonte)
- ✅ Verificar que implementação está correta
- ✅ Profiling para confirmar

---

## 7. Problemas de Sincronização JNI

### 7.1 JNIEnv* Não é Thread-Safe

**Problema**: `JNIEnv*` não pode ser usado de threads nativas

**Cenário**:
```cpp
// Thread nativa (CdStreamThread)
AssetManagerBridge::GetAssetManagerFromJNI(env, context)
→ Usa env->GetObjectClass()  // ❌ JNIEnv* não é válido nesta thread!
→ Crash ou comportamento indefinido
```

**Causa Raiz**:
- `JNIEnv*` é válido apenas na thread que o recebeu
- Threads nativas precisam anexar à JVM primeiro
- `AssetManagerBridge` pode ser chamado de thread nativa

**Solução**:
```cpp
// AssetManagerBridge.cpp
void* AssetManagerBridge::GetAssetManagerFromJNI(void* jniEnv, void* context) {
    JNIEnv* env = (JNIEnv*)jniEnv;
    
    // Verifica se thread está anexada à JVM
    JavaVM* vm = nullptr;
    env->GetJavaVM(&vm);
    
    JNIEnv* currentEnv = nullptr;
    jint result = vm->GetEnv((void**)&currentEnv, JNI_VERSION_1_6);
    
    if (result == JNI_EDETACHED) {
        // Thread não está anexada, anexar
        vm->AttachCurrentThread(&currentEnv, nullptr);
        // Usar currentEnv em vez de env
        env = currentEnv;
    }
    
    // ... resto do código
    
    // Se anexou, desanexar depois (opcional)
    // vm->DetachCurrentThread();
}
```

**Mitigação**:
- ✅ Verificar se thread está anexada à JVM
- ✅ Anexar thread se necessário
- ✅ Usar `JavaVM*` para obter `JNIEnv*` em threads nativas
- ✅ Documentar thread-safety de funções JNI

---

### 7.2 AssetManager Pode Ser Invalidado

**Problema**: `AssetManager` pode ser invalidado se Context for destruído

**Cenário**:
```cpp
// onCreate()
setAssetManager(getAssets());  // AssetManager válido

// onDestroy() (app sendo fechado)
// Context destruído
// AssetManager invalidado

// Mas thread nativa ainda tentando usar
AssetManagerBridge::ReadAsset(assetManager, ...)
→ AAssetManager inválido
→ Crash
```

**Causa Raiz**:
- `AssetManager` é válido apenas enquanto `Context` existe
- Threads nativas podem sobreviver à destruição do Context
- Sem verificação de validade

**Solução**:
```cpp
// Opção 1: Não usar AssetManager após onDestroy()
// Garantir que todas as threads param antes de onDestroy()

// Opção 2: Verificar validade antes de usar
bool AssetManagerBridge::IsAssetManagerValid(void* assetManager) {
    if (!assetManager) return false;
    // Tentar abrir asset de teste
    AAsset* test = AAssetManager_open((AAssetManager*)assetManager, "test", AASSET_MODE_UNKNOWN);
    if (test) {
        AAsset_close(test);
        return true;
    }
    return false;
}

// Opção 3: Extrair todos os assets necessários no início
// Depois disso, não precisa mais do AssetManager
```

**Mitigação**:
- ✅ Extrair assets críticos no início
- ✅ Verificar validade antes de usar
- ✅ Garantir que threads param antes de onDestroy()
- ✅ Logging quando AssetManager inválido

---

### 7.3 Race Condition JNI → Native

**Problema**: `setAssetManager()` pode ser chamado enquanto `OpenFile()` está executando

**Cenário**:
```cpp
// Thread 1 (Main - JNI)
setAssetManager(getAssets());
→ m_assetManager = newValue;  // Modificando

// Thread 2 (CdStreamThread) - simultaneamente
OpenFromAPK("SAMP/main.scm")
→ if (m_assetManager) {  // ❌ Lê valor parcial ou antigo
    ReadAsset(m_assetManager, ...);
}
```

**Causa Raiz**:
- `m_assetManager` modificado sem lock
- Leitura e escrita não são atômicas
- Race condition

**Solução**:
```cpp
// FileSystemService.h
std::mutex m_assetManagerMutex;  // NOVO
void* m_assetManager = nullptr;

// FileSystemService.cpp
void FileSystemService::SetAssetManager(void* assetManager) {
    std::lock_guard<std::mutex> lock(m_assetManagerMutex);
    m_assetManager = assetManager;
}

void* FileSystemService::GetAssetManager() const {
    std::lock_guard<std::mutex> lock(m_assetManagerMutex);
    return m_assetManager;
}

// E em OpenFromAPK():
void* assetMgr = GetAssetManager();  // Thread-safe
if (!assetMgr) {
    return FileHandle{};
}
```

**Mitigação**:
- ✅ Mutex para `m_assetManager`
- ✅ Getter thread-safe
- ✅ Documentar thread-safety
- ✅ Testar com acesso concorrente

---

## 8. Problemas de Extração de Assets

### 8.1 Espaço em Disco Insuficiente

**Problema**: Extração pode falhar se não houver espaço suficiente

**Cenário**:
```cpp
// Disco quase cheio (apenas 10MB livres)
ExtractAssetsIfNeeded(assets);  // Precisa de 15-60MB
→ Tenta extrair primeiro arquivo (5MB) ✅
→ Tenta extrair segundo arquivo (10MB) ✅
→ Tenta extrair terceiro arquivo (5MB) ❌ Sem espaço!
→ Extração parcial
→ Flag .assets_extracted criada mesmo assim
→ Próxima execução não tenta extrair novamente
→ Arquivos faltando
→ Jogo não funciona
```

**Causa Raiz**:
- Sem verificação de espaço antes de extrair
- Extração parcial não é detectada
- Flag criada mesmo com falha

**Solução**:
```cpp
bool FileSystemService::ExtractAssetsIfNeeded(...) {
    // 1. Verifica espaço disponível
    struct statvfs vfs;
    if (statvfs(m_storagePath.c_str(), &vfs) == 0) {
        uint64_t freeSpace = (uint64_t)vfs.f_bavail * vfs.f_frsize;
        uint64_t neededSpace = CalculateNeededSpace(assetPaths);
        
        if (freeSpace < neededSpace) {
            LOGE("Insufficient disk space: %llu bytes free, %llu bytes needed",
                 freeSpace, neededSpace);
            return false;
        }
    }
    
    // 2. Extrai arquivos
    bool allSuccess = true;
    for (const auto& assetPath : assetPaths) {
        if (!ExtractAssetToStorage(assetPath, destPath)) {
            allSuccess = false;
            // Remove arquivos já extraídos em caso de falha?
            // Ou deixa parcial e tenta completar depois?
        }
    }
    
    // 3. Cria flag APENAS se todos extraídos com sucesso
    if (allSuccess) {
        std::ofstream flag(flagFile);
        flag.close();
        m_assetsExtracted = true;
    }
    
    return allSuccess;
}
```

**Mitigação**:
- ✅ Verificar espaço antes de extrair
- ✅ Calcular espaço necessário
- ✅ Criar flag apenas se extração completa
- ✅ Retry em próxima execução se falhou

---

### 8.2 Extração Parcial (Corrupção)

**Problema**: Extração pode ser interrompida (app fechado, crash, etc.)

**Cenário**:
```cpp
// Extração em progresso
ExtractAssetToStorage("SAMP/main.scm", destPath)
→ Escrevendo arquivo...
→ App crasha ou é fechado
→ Arquivo parcialmente escrito
→ Arquivo corrompido

// Próxima execução
→ Verifica flag: existe
→ Assume que assets estão extraídos
→ Tenta abrir arquivo corrompido
→ Crash ou comportamento indefinido
```

**Causa Raiz**:
- Extração não é atômica
- Arquivo pode ser parcialmente escrito
- Flag criada antes de verificar integridade

**Solução**:
```cpp
bool FileSystemService::ExtractAssetToStorage(...) {
    // 1. Escreve para arquivo temporário
    std::string tempPath = destPath + ".tmp";
    if (!WriteFile(tempPath, data)) {
        return false;
    }
    
    // 2. Verifica integridade (opcional - checksum)
    // ...
    
    // 3. Renomeia (atômico)
    if (rename(tempPath.c_str(), destPath.c_str()) != 0) {
        unlink(tempPath.c_str());
        return false;
    }
    
    return true;
}

// E verificar integridade na próxima execução:
bool FileSystemService::VerifyExtractedAssets(...) {
    for (const auto& assetPath : assetPaths) {
        std::string destPath = GetStoragePath(assetPath);
        
        // Verifica se arquivo existe E tem tamanho > 0
        struct stat st;
        if (stat(destPath.c_str(), &st) != 0 || st.st_size == 0) {
            LOGW("Asset appears corrupted or missing: %s", assetPath.c_str());
            return false;
        }
    }
    return true;
}
```

**Mitigação**:
- ✅ Extração atômica (temp + rename)
- ✅ Verificar integridade após extração
- ✅ Verificar na próxima execução
- ✅ Reextrair se corrompido

---

### 8.3 Assets Modificados no APK (Update)

**Problema**: APK atualizado mas assets antigos ainda no storage

**Cenário**:
```cpp
// Versão 1.0 do app
ExtractAssetsIfNeeded(assets);  // Extrai assets v1.0
→ Flag criada

// App atualizado para versão 2.0
→ APK tem assets v2.0 (diferentes)
→ Mas flag ainda existe
→ Não extrai novamente
→ Usa assets v1.0 antigos
→ Incompatibilidade
→ Crash ou bugs
```

**Causa Raiz**:
- Flag não verifica versão do APK
- Não detecta mudanças nos assets
- Sem mecanismo de invalidação

**Solução**:
```cpp
// Opção 1: Incluir versão do APK na flag
std::string flagFile = m_storagePath + ".assets_extracted_v" + APK_VERSION;

// Opção 2: Checksum dos assets
std::string CalculateAssetsChecksum(const std::vector<std::string>& assets) {
    // Calcula checksum de todos os assets
    // Inclui no nome da flag
}

// Opção 3: Timestamp do APK
long GetAPKTimestamp() {
    // Obtém timestamp de modificação do APK
    // Compara com flag
}

// Verificação:
bool FileSystemService::ShouldExtractAssets(...) {
    std::string flagFile = m_storagePath + ".assets_extracted";
    
    if (!FileExistsInSource(flagFile, FileSource::EXTERNAL_STORAGE)) {
        return true;  // Flag não existe
    }
    
    // Lê versão/checksum da flag
    // Compara com versão/checksum atual
    // Retorna true se diferente
}
```

**Mitigação**:
- ✅ Incluir versão/checksum na flag
- ✅ Verificar na inicialização
- ✅ Reextrair se versão diferente
- ✅ Documentar comportamento de atualização

---

## 9. Problemas de Fallback e Erros

### 9.1 Fallback Silencioso Pode Mascarar Problemas

**Problema**: Fallback automático pode esconder problemas de configuração

**Cenário**:
```cpp
// Arquivo deveria estar no storage mas não está
OpenFile("SAMP/settings.ini")
→ Tenta Storage: ❌ Não encontrado
→ Tenta APK: ✅ Encontrado (fallback)
→ Extrai para cache
→ Abre do cache
→ ✅ Funciona

// Mas problema real: Por que arquivo não está no storage?
// → Mod foi removido?
// → Permissões?
// → Bug?
// → Problema mascarado pelo fallback
```

**Causa Raiz**:
- Fallback funciona "demais"
- Problemas reais não são detectados
- Sem logging adequado

**Solução**:
```cpp
FileHandle FileSystemService::OpenFile(...) {
    bool triedStorage = false;
    
    for (FileSource source : m_fallbackOrder) {
        if (source == FileSource::EXTERNAL_STORAGE) {
            triedStorage = true;
        }
        
        FileHandle handle = OpenFileFromSource(resolvedPath, source, mode);
        if (handle.isValid()) {
            if (triedStorage && handle.source != FileSource::EXTERNAL_STORAGE) {
                // Arquivo não estava no storage esperado, mas encontrado em fallback
                LOGW("File not in expected location (storage), using fallback: %s (source: %d)",
                     resolvedPath.c_str(), (int)handle.source);
            }
            return handle;
        }
    }
    
    LOGE("File not found in any source: %s", resolvedPath.c_str());
    return FileHandle{};
}
```

**Mitigação**:
- ✅ Logging quando fallback é usado
- ✅ Warnings quando arquivo não está no local esperado
- ✅ Diferentes níveis de log (debug vs production)
- ✅ Estatísticas de uso de fallback

---

### 9.2 Erro Fatal em `CSettings`

**Problema**: `CSettings` faz `std::terminate()` se arquivo não existir

**Cenário**:
```cpp
// settings.cpp
CSettings::CSettings() {
    sprintf(buff, "%sSAMP/settings.ini", g_pszStorage);
    INIReader reader(buff);
    
    if(reader.ParseError() < 0) {
        FLog("Error: can't load %s", buff);
        std::terminate();  // ❌ MATA O APP!
        return;
    }
}

// Se settings.ini não existe:
→ std::terminate()
→ App fecha imediatamente
→ Sem chance de criar arquivo padrão
→ Sem fallback
```

**Causa Raiz**:
- Tratamento de erro muito agressivo
- Sem fallback ou valores padrão
- Arquivo pode não existir na primeira execução

**Solução**:
```cpp
// Opção 1: Criar settings.ini padrão se não existir
CSettings::CSettings() {
    char buff[0x7F];
    sprintf(buff, "%sSAMP/settings.ini", g_pszStorage);
    
    // Verifica se existe
    if (!FileExists(buff)) {
        // Cria arquivo padrão
        CreateDefaultSettings(buff);
    }
    
    INIReader reader(buff);
    if(reader.ParseError() < 0) {
        FLog("Error: can't load %s, using defaults", buff);
        // Usa valores padrão em vez de terminate
        LoadDefaults();
        return;
    }
    // ... resto
}

// Opção 2: Usar FileLoader que tem fallback
FILE* file = FileLoader::Open("SAMP/settings.ini", "r");
if (!file) {
    // Cria padrão
}
```

**Mitigação**:
- ✅ Remover `std::terminate()`
- ✅ Criar arquivo padrão se não existir
- ✅ Usar valores padrão em caso de erro
- ✅ Logging de erro sem matar app

---

### 9.3 Arquivo Não Encontrado vs Permissão Negada

**Problema**: Diferentes erros podem ter a mesma aparência

**Cenário**:
```cpp
// Caso 1: Arquivo não existe
fopen("SAMP/settings.ini", "r") → nullptr
errno = ENOENT

// Caso 2: Sem permissão
fopen("SAMP/settings.ini", "r") → nullptr
errno = EACCES

// Código atual trata ambos da mesma forma
if (!file) {
    FLog("Fail open file");  // ❌ Não diferencia
}
```

**Causa Raiz**:
- Sem verificação de `errno`
- Diferentes problemas têm mesmo sintoma
- Debugging difícil

**Solução**:
```cpp
FILE* FileSystemService::OpenNativeFile(...) {
    FILE* file = fopen(fullPath.c_str(), modeStr);
    if (!file) {
        int err = errno;
        switch (err) {
            case ENOENT:
                LOGW("File not found: %s", fullPath.c_str());
                break;
            case EACCES:
                LOGE("Permission denied: %s", fullPath.c_str());
                break;
            case ENOSPC:
                LOGE("No space left on device: %s", fullPath.c_str());
                break;
            default:
                LOGE("Failed to open file: %s (errno: %d)", fullPath.c_str(), err);
                break;
        }
    }
    return file;
}
```

**Mitigação**:
- ✅ Verificar `errno` após `fopen()`
- ✅ Logging específico por tipo de erro
- ✅ Ações diferentes para diferentes erros
- ✅ Documentar códigos de erro comuns

---

## 10. Problemas Específicos do Android

### 10.1 Storage Desmontado Durante Execução

**Problema**: Storage pode ser desmontado (SD card removido)

**Cenário**:
```cpp
// App rodando normalmente
OpenFile("SAMP/settings.ini")  // ✅ Funciona

// Usuário remove SD card
// Storage desmontado

// App tenta abrir arquivo novamente
OpenFile("SAMP/settings.ini")
→ fopen() no storage desmontado
→ ❌ Falha
→ Sem fallback (APK também pode estar no SD card)
→ App quebra
```

**Causa Raiz**:
- Storage pode ser removido dinamicamente
- Sem verificação de disponibilidade
- Sem tratamento de remontagem

**Solução**:
```cpp
bool FileSystemService::IsStorageAvailable() const {
    struct statvfs vfs;
    return statvfs(m_storagePath.c_str(), &vfs) == 0;
}

FileHandle FileSystemService::OpenFromStorage(...) {
    if (!IsStorageAvailable()) {
        LOGW("Storage not available, trying APK fallback");
        return FileHandle{};  // Tenta APK
    }
    
    // ... resto do código
}

// E verificar periodicamente:
void FileSystemService::CheckStorageAvailability() {
    static bool lastState = true;
    bool currentState = IsStorageAvailable();
    
    if (lastState != currentState) {
        if (currentState) {
            LOGI("Storage remounted");
        } else {
            LOGW("Storage unmounted");
        }
        lastState = currentState;
    }
}
```

**Mitigação**:
- ✅ Verificar disponibilidade antes de usar
- ✅ Fallback para APK se storage indisponível
- ✅ Detectar remontagem
- ✅ Notificar usuário se necessário

---

### 10.2 Android 11+ Scoped Storage

**Problema**: Android 11+ tem restrições de acesso a storage

**Cenário**:
```cpp
// Android 10
getExternalFilesDir() → /storage/emulated/0/Android/data/.../files/
→ Acesso total ✅

// Android 11+
getExternalFilesDir() → /storage/emulated/0/Android/data/.../files/
→ Acesso apenas ao diretório do app ✅ (ainda funciona)
→ Mas se código tentar acessar diretório pai:
→ ❌ Acesso negado
```

**Causa Raiz**:
- Scoped Storage no Android 11+
- Acesso restrito ao diretório do app
- Código pode tentar acessar fora

**Solução**:
```cpp
// Já usando getExternalFilesDir() que é compatível
// Mas validar que paths não saem do diretório:

std::string FileSystemService::GetStoragePath(...) const {
    std::string fullPath = m_storagePath + NormalizePath(relativePath);
    
    // Verifica que está dentro de m_storagePath
    if (fullPath.find(m_storagePath) != 0) {
        LOGE("Path outside app directory (Scoped Storage violation): %s", fullPath.c_str());
        return "";
    }
    
    return fullPath;
}
```

**Mitigação**:
- ✅ Já usando `getExternalFilesDir()` (correto)
- ✅ Validar que paths não saem do diretório
- ✅ Testar em Android 11+
- ✅ Documentar compatibilidade

---

### 10.3 Permissões de Storage (Android 10 e Anterior)

**Problema**: Android 10 e anterior podem precisar de permissões

**Cenário**:
```cpp
// Android 10 (API 29)
// Precisa de READ_EXTERNAL_STORAGE para alguns casos
// Mas getExternalFilesDir() não precisa

// Código pode tentar acessar fora de getExternalFilesDir()
→ Sem permissão
→ Falha
```

**Causa Raiz**:
- Permissões legadas no Android 10
- Código pode tentar acessar fora do diretório do app
- Sem verificação de permissões

**Solução**:
```cpp
// Já correto: usar apenas getExternalFilesDir()
// Não precisa de permissões

// Mas se código legado tentar acessar fora:
// Validar e rejeitar
```

**Mitigação**:
- ✅ Usar apenas `getExternalFilesDir()` (sem permissões)
- ✅ Validar paths
- ✅ Testar em Android 10
- ✅ Documentar que não precisa permissões

---

## 11. Problemas de Migração Gradual

### 11.1 Código Legado Ainda Usa `g_pszStorage` Diretamente

**Problema**: Algum código pode acessar `g_pszStorage` diretamente

**Cenário**:
```cpp
// Código legado não migrado
void SomeLegacyFunction() {
    char path[255];
    sprintf(path, "%s%s", g_pszStorage, "file.txt");  // ❌ Ainda usa g_pszStorage
    fopen(path, "r");
}

// Se g_pszStorage não foi preenchido corretamente:
→ Path incorreto
→ Arquivo não encontrado
```

**Causa Raiz**:
- Migração gradual deixa código legado
- `g_pszStorage` pode não estar sincronizado
- Dependências ocultas

**Solução**:
```cpp
// Garantir que g_pszStorage sempre está preenchido
void FileSystemService::Initialize(const std::string& storagePath) {
    m_storagePath = storagePath;
    
    // Preenche g_pszStorage para compatibilidade
    strncpy(s_szStoragePath, storagePath.c_str(), sizeof(s_szStoragePath) - 1);
    s_szStoragePath[sizeof(s_szStoragePath) - 1] = '\0';
    g_pszStorage = s_szStoragePath;
    g_bStoragePathSetViaJNI = true;
}
```

**Mitigação**:
- ✅ Manter `g_pszStorage` preenchido
- ✅ Buscar todos os usos de `g_pszStorage` no código
- ✅ Migrar gradualmente
- ✅ Testar código legado

---

### 11.2 Inconsistência Entre Sistemas

**Problema**: Código novo e legado podem ter comportamentos diferentes

**Cenário**:
```cpp
// Código novo (migrado)
FileLoader::Open("SAMP/settings.ini")  // Usa novo sistema
→ Fallback para APK se não encontrar

// Código legado (não migrado)
CFileMgr::OpenFile("SAMP/settings.ini")  // Usa sistema antigo
→ Sem fallback
→ Falha se não encontrar

// Comportamento inconsistente
→ Alguns arquivos carregam, outros não
→ Bugs difíceis de debugar
```

**Causa Raiz**:
- Migração gradual
- Dois sistemas coexistindo
- Comportamentos diferentes

**Solução**:
```cpp
// Fazer CFileMgr usar FileLoader internamente
FILE* CFileMgr::OpenFile(const char* path, const char* mode) {
    // Delega para FileLoader
    return FileLoader::Open(path, mode);
}

// Assim, código legado também usa novo sistema
// Comportamento consistente
```

**Mitigação**:
- ✅ Fazer wrappers legados usarem novo sistema
- ✅ Comportamento consistente
- ✅ Migração transparente
- ✅ Testar ambos os caminhos

---

## 12. Cenários de Falha Crítica

### 12.1 Falha em Cascata

**Problema**: Uma falha pode causar múltiplas falhas

**Cenário**:
```cpp
// 1. Storage path não definido
FileSystemService não inicializado

// 2. NvFOpen() falha
NvFOpen("DATA/GTA.DAT") → nullptr

// 3. libGTASA tenta usar arquivo
fread(buffer, 1, size, nullptr)  // ❌ Crash!

// 4. Ou libGTASA verifica nullptr mas...
// Outros sistemas dependem do arquivo carregado
→ Múltiplas falhas em cascata
→ App completamente quebrado
```

**Causa Raiz**:
- Dependências entre sistemas
- Falha inicial propaga
- Sem recuperação

**Solução**:
```cpp
// Validação rigorosa na inicialização
void FileSystemService::Initialize(const std::string& storagePath) {
    if (storagePath.empty()) {
        LOGE("CRITICAL: Storage path is empty!");
        // Pode usar fallback temporário ou abortar
        return;
    }
    
    // Verifica se diretório existe e é acessível
    if (access(storagePath.c_str(), F_OK | R_OK | W_OK) != 0) {
        LOGE("CRITICAL: Storage path not accessible: %s", storagePath.c_str());
        return;
    }
    
    m_storagePath = storagePath;
    LOGI("FileSystemService initialized successfully");
}

// E verificar antes de usar:
FileHandle FileSystemService::OpenFile(...) {
    if (m_storagePath.empty()) {
        LOGE("CRITICAL: FileSystemService not initialized!");
        // Pode tentar inicializar com fallback ou abortar
        return FileHandle{};
    }
    // ... resto
}
```

**Mitigação**:
- ✅ Validação rigorosa na inicialização
- ✅ Verificações antes de operações críticas
- ✅ Fallbacks para casos de erro
- ✅ Logging de erros críticos

---

### 12.2 Corrupção de Dados do Jogo

**Problema**: Arquivo corrompido pode corromper estado do jogo

**Cenário**:
```cpp
// Arquivo parcialmente extraído ou corrompido
OpenFile("SAMP/gta.dat")
→ Arquivo existe mas está corrompido (tamanho errado, dados inválidos)

// libGTASA lê dados corrompidos
fread(buffer, 1, size, file);
→ Buffer contém dados inválidos

// Jogo usa dados corrompidos
→ Comportamento indefinido
→ Crash
→ Save corrompido
```

**Causa Raiz**:
- Sem verificação de integridade
- Dados corrompidos não detectados
- Propagação de corrupção

**Solução**:
```cpp
// Verificação de integridade (opcional mas recomendado)
bool FileSystemService::VerifyFileIntegrity(const std::string& path, size_t expectedSize) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    
    if (st.st_size != expectedSize) {
        LOGW("File size mismatch: %s (expected: %zu, actual: %zu)",
             path.c_str(), expectedSize, st.st_size);
        return false;
    }
    
    // Checksum opcional (mais lento)
    // ...
    
    return true;
}

// E em OpenFile():
FileHandle handle = OpenFromStorage(...);
if (handle.isValid()) {
    // Verifica integridade (opcional)
    if (!VerifyFileIntegrity(handle.path, expectedSize)) {
        fclose(handle.file);
        // Tenta reextrair ou usar fallback
        return TryFallback(...);
    }
}
```

**Mitigação**:
- ✅ Verificar tamanho de arquivo
- ✅ Checksum opcional (para arquivos críticos)
- ✅ Reextrair se corrompido
- ✅ Logging de corrupção detectada

---

### 12.3 Deadlock em Extração

**Problema**: Deadlock se múltiplas threads tentam extrair com locks aninhados

**Cenário**:
```cpp
// Thread 1
OpenFromAPK("file1.dat")
→ lock(m_extractionMutex)
→ Tenta extrair
→ Precisa abrir outro arquivo para verificar
→ OpenFile("file2.dat")  // Pode tentar lock novamente
→ Deadlock se file2 também está sendo extraído
```

**Causa Raiz**:
- Locks aninhados
- Recursão em operações de arquivo
- Deadlock potencial

**Solução**:
```cpp
// Usar std::recursive_mutex OU evitar locks aninhados
std::recursive_mutex m_extractionMutex;  // Permite locks aninhados

// OU melhor: evitar locks aninhados
// Extração não deve chamar OpenFile() que pode tentar extrair outro arquivo
```

**Mitigação**:
- ✅ `std::recursive_mutex` se necessário
- ✅ Evitar locks aninhados
- ✅ Timeout em locks
- ✅ Detecção de deadlock (debug)

---

## 📊 Resumo de Problemas por Severidade

### 🔴 Críticos (Podem Quebrar o App)

1. **Race Condition na Inicialização** - FileSystemService usado antes de inicializar
2. **FILE* Não Thread-Safe** - Compartilhamento entre threads
3. **Buffer Overflow em Paths** - Corrupção de memória
4. **std::terminate() em CSettings** - App fecha imediatamente
5. **Falha em Cascata** - Uma falha causa múltiplas

### 🟡 Importantes (Podem Causar Bugs)

6. **AssetManager Não Definido** - Assets não extraídos
7. **Race Condition na Extração** - Arquivos corrompidos
8. **Memory Leak em FILE*** - Limite de arquivos atingido
9. **Extração Parcial** - Arquivos corrompidos
10. **JNIEnv* Thread Safety** - Crash em threads nativas

### 🟢 Menores (Performance/UX)

11. **Extração Lenta** - UX ruim na primeira execução
12. **Overhead do Sistema** - Performance ligeiramente pior
13. **Fallback Silencioso** - Problemas mascarados
14. **Storage Desmontado** - Funcionalidade quebrada temporariamente

---

## ✅ Checklist de Mitigações

### Inicialização
- [ ] Verificar `m_storagePath.empty()` em todas as operações
- [ ] Verificar `m_assetManager` antes de usar
- [ ] Documentar ordem de inicialização obrigatória
- [ ] Extração sob demanda (lazy) em vez de upfront

### Thread Safety
- [ ] Mutex em `m_redirects` ✅ (já implementado)
- [ ] Mutex em `m_assetManager` (NOVO)
- [ ] Mutex em extrações (NOVO)
- [ ] Documentar que `FILE*` não é thread-safe
- [ ] Cada thread abre seu próprio handle

### Memória
- [ ] Documentar responsabilidade de `fclose()`
- [ ] Wrapper RAII para código novo
- [ ] Tracking de arquivos abertos (debug)
- [ ] Verificar `NvFClose()` se existir

### Paths
- [ ] Usar `std::string` sempre (sem buffers fixos)
- [ ] Validar `PATH_MAX`
- [ ] Sanitizar paths (remover `..`, caracteres inválidos)
- [ ] Validar que path está dentro do diretório base

### Compatibilidade
- [ ] Testar todos os arquivos críticos
- [ ] Manter fallback para comportamento antigo
- [ ] Opção de desabilitar redirecionamentos (debug)
- [ ] Logging detalhado

### Performance
- [ ] Extração em background thread
- [ ] Mostrar progresso ao usuário
- [ ] Cache de lookups frequentes
- [ ] Fast path para casos comuns

### JNI
- [ ] Verificar se thread está anexada à JVM
- [ ] Anexar thread se necessário
- [ ] Verificar validade do AssetManager
- [ ] Extrair assets críticos no início

### Extração
- [ ] Verificar espaço antes de extrair
- [ ] Extração atômica (temp + rename)
- [ ] Verificar integridade após extração
- [ ] Incluir versão/checksum na flag

### Erros
- [ ] Verificar `errno` após `fopen()`
- [ ] Logging específico por tipo de erro
- [ ] Remover `std::terminate()`
- [ ] Criar arquivo padrão se não existir

### Android
- [ ] Verificar disponibilidade do storage
- [ ] Detectar remontagem
- [ ] Validar paths (Scoped Storage)
- [ ] Testar em Android 11+

---

**Documento criado em**: 2024
**Versão**: 1.0
**Última atualização**: 2024
