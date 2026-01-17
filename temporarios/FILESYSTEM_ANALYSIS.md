# Análise Completa: Novo Sistema de Carregamento de Arquivos

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Análise do Sistema Atual](#análise-do-sistema-atual)
3. [Dependências Identificadas](#dependências-identificadas)
4. [Arquitetura do Novo Sistema](#arquitetura-do-novo-sistema)
5. [Impacto nas Mudanças](#impacto-nas-mudanças)
6. [Plano de Migração](#plano-de-migração)
7. [Riscos e Considerações](#riscos-e-considerações)
8. [Testes Necessários](#testes-necessários)
9. [Checklist de Implementação](#checklist-de-implementação)

---

## 1. Visão Geral

### 1.1 Objetivo
Reformular completamente o sistema de carregamento de arquivos para:
- **Modularidade**: Sistema baseado em interfaces e ServiceLocator
- **Extensibilidade**: Fácil adicionar novas fontes de arquivos
- **Compatibilidade**: Mantém compatibilidade com código legado
- **Automação**: Extração automática de assets do APK
- **Fallback**: Sistema inteligente de fallback entre fontes

### 1.2 Benefícios Esperados
- ✅ Código mais organizado e testável
- ✅ Suporte nativo a múltiplas fontes (APK, Storage, Cache)
- ✅ Redução de código duplicado
- ✅ Melhor tratamento de erros
- ✅ Facilita migração de assets para APK

---

## 2. Análise do Sistema Atual

### 2.1 Pontos de Acesso a Arquivos

#### 2.1.1 Código Nativo (C++)

**1. `NvFOpen()` Hook** (`game/hooks.cpp:1423`)
- **Uso**: Hook principal para abertura de arquivos do jogo
- **Localização**: `game/hooks.cpp`
- **Dependências**: `g_pszStorage` (global)
- **Redirecionamentos hardcoded**:
  ```cpp
  "DATA/GTA.DAT" → "SAMP/gta.dat"
  "DATA/PEDS.IDE" → "SAMP/peds.ide"
  "DATA/VEHICLES.IDE" → "SAMP/vehicles.ide"
  "DATA/HANDLING.CFG" → "SAMP/handling.cfg"
  "DATA/WEAPON.DAT" → "SAMP/weapon.dat"
  "DATA/FONTS.DAT" → "data/fonts.dat"
  "DATA/PEDSTATS.DAT" → "data/pedstats.dat"
  "DATA/TIMECYC.DAT" → "data/timecyc.dat"
  "DATA/POPCYCLE.DAT" → "data/popcycle.dat"
  "mainV1.scm" → "SAMP/main.scm"
  "SCRIPTV1.IMG" → "SAMP/script.img"
  ```
- **Problemas**:
  - Redirecionamentos hardcoded
  - Sem fallback para APK
  - Dependência de global `g_pszStorage`

**2. `CFileMgr::OpenFile()`** (`game/CFileMgr.cpp:9`)
- **Uso**: Wrapper para `fopen()` com prefixo de storage
- **Localização**: `game/CFileMgr.cpp`
- **Dependências**: `g_pszStorage`, `ms_path` (static)
- **Problemas**:
  - Apenas storage externo
  - Sem suporte a assets do APK
  - Sem tratamento de erros robusto

**3. `CFileMgr::LoadFile()`** (`game/CFileMgr.h:26`)
- **Uso**: Carrega arquivo completo em buffer
- **Localização**: `game/CFileMgr.h`
- **Status**: Declarado mas implementação não encontrada no código atual

**4. `CSettings::CSettings()`** (`settings.cpp:15`)
- **Uso**: Carrega `SAMP/settings.ini`
- **Localização**: `samp/settings.cpp`
- **Dependências**: `g_pszStorage`, `INIReader`
- **Problemas**:
  - Falha fatal se arquivo não existir (`std::terminate()`)
  - Sem fallback

**5. `readVehiclesAudioSettings()`** (`game/patches.cpp:36`)
- **Uso**: Carrega `SAMP/vehicleAudioSettings.cfg`
- **Localização**: `game/patches.cpp`
- **Dependências**: `g_pszStorage`
- **Status**: Opcional (retorna silenciosamente se não existir)

**6. `ReadSettingFile()`** (`main.cpp:71`)
- **Uso**: Carrega `NickName.ini` (comentado)
- **Localização**: `main.cpp`
- **Status**: Código comentado, não usado atualmente

**7. `InitGui()`** (`main.cpp:462`)
- **Uso**: Carrega fontes de `SAMP/fonts/`
- **Localização**: `main.cpp`
- **Dependências**: `g_pszStorage`
- **Formato**: `{storage}SAMP/fonts/{FONT_NAME}`

#### 2.1.2 Código Java

**1. `NvEventQueueActivity.loadFile()`** (`NvEventQueueActivity.java:255`)
- **Uso**: Carrega arquivos genéricos (texturas, dados)
- **Localização**: `com/nvidia/devtech/NvEventQueueActivity.java`
- **Fallback**: `/data/` → APK assets
- **Problemas**:
  - Path hardcoded `/data/` (legado)
  - Não usa storage externo do app

**2. `NvEventQueueActivity.loadTexture()`** (`NvEventQueueActivity.java:308`)
- **Uso**: Carrega texturas com processamento
- **Localização**: `com/nvidia/devtech/NvEventQueueActivity.java`
- **Fallback**: `/data/` → APK assets
- **Problemas**: Mesmos do `loadFile()`

**3. `NvAPKFileHelper.openFileAndroid()`** (`NvAPKFileHelper.java:115`)
- **Uso**: Abre arquivos do APK via AssetManager
- **Localização**: `com/nvidia/devtech/NvAPKFileHelper.java`
- **Dependências**: `assetfile.txt` no APK
- **Problemas**:
  - Lista de assets hardcoded em `assetfile.txt`
  - Não integrado com sistema nativo
  - Usado apenas para áudio (`.mp3`)

**4. `SAMP.setStoragePath()`** (`SAMP.java:178`)
- **Uso**: Define storage path via JNI
- **Localização**: `com/samp/mobile/game/SAMP.java`
- **JNI**: `Java_com_samp_mobile_game_SAMP_setStoragePath()`
- **Status**: Funcional, mas não passa AssetManager

### 2.2 Variáveis Globais e Dependências

```cpp
// main.h / main.cpp
extern char* g_pszStorage;                    // Path base do storage
static char s_szStoragePath[512];             // Buffer para JNI
bool g_bStoragePathSetViaJNI = false;         // Flag de inicialização

// WarMedia.java
private String baseDirectory;                // Path base (Java)
private String baseDirectoryRoot;            // Root sem /Android

// NvUtil.java
appLocalValues.put("STORAGE_ROOT", ...);     // Armazenamento Java
appLocalValues.put("STORAGE_ROOT_BASE", ...);
```

### 2.3 Estrutura de Diretórios Atual

```
Android/data/com.samp.mobile/files/
├── SAMP/
│   ├── main.scm
│   ├── script.img
│   ├── peds.ide
│   ├── vehicles.ide
│   ├── gta.dat
│   ├── handling.cfg
│   ├── weapon.dat
│   ├── settings.ini
│   ├── vehicleAudioSettings.cfg (opcional)
│   └── fonts/
│       └── arial_bold.ttf
├── data/
│   ├── fonts.dat
│   ├── pedstats.dat
│   ├── timecyc.dat
│   └── popcycle.dat
└── NickName.ini (opcional, não usado)
```

### 2.4 Problemas Identificados

1. **Código Duplicado**
   - Múltiplos lugares fazem `sprintf(path, "%s%s", g_pszStorage, ...)`
   - Lógica de fallback duplicada (Java e C++)

2. **Dependências Globais**
   - `g_pszStorage` acessado diretamente em vários lugares
   - Difícil testar e mockar

3. **Sem Suporte a APK Assets**
   - Código nativo não acessa assets do APK
   - Java tem suporte mas não integrado

4. **Redirecionamentos Hardcoded**
   - Lista de redirecionamentos espalhada no código
   - Difícil manter e estender

5. **Tratamento de Erros Inconsistente**
   - Alguns lugares fazem `std::terminate()`
   - Outros retornam silenciosamente
   - Sem logging consistente

6. **Sem Extração Automática**
   - Assets precisam ser instalados manualmente
   - Primeira execução pode falhar

---

## 3. Dependências Identificadas

### 3.1 Dependências de Código

#### 3.1.1 C++

**Headers Necessários:**
- `main.h` - Declarações globais
- `core/bootstrap/ServiceLocator.h` - Sistema de serviços
- `core/logging/Log.h` - Sistema de logging
- `platform/android/AssetManagerBridge.h` - Ponte JNI

**Bibliotecas:**
- Android NDK (AAssetManager)
- Standard C Library (fopen, fread, etc.)
- POSIX (stat, mkdir, etc.)

**Vendor Code:**
- `vendor/armhook/patch.h` - Para hooks
- `vendor/inih/cpp/INIReader.h` - Parser INI (settings)

#### 3.1.2 Java

**Classes:**
- `android.content.res.AssetManager`
- `android.content.Context`
- `java.io.FileInputStream`
- `java.io.InputStream`

**JNI:**
- `Java_com_samp_mobile_game_SAMP_setStoragePath()`
- `Java_com_samp_mobile_game_SAMP_setAssetManager()` (NOVO)

### 3.2 Dependências de Arquivos

**Arquivos que DEVEM existir:**
- `SAMP/settings.ini` (fatal se não existir)
- `SAMP/main.scm`
- `SAMP/script.img`
- `SAMP/gta.dat`
- `data/fonts.dat`

**Arquivos OPCIONAIS:**
- `SAMP/vehicleAudioSettings.cfg`
- `NickName.ini`
- Outros arquivos do jogo base

**Assets do APK:**
- `assetfile.txt` (lista de assets)
- Todos os arquivos listados em `assetfile.txt`

### 3.3 Dependências de Sistema

**Android:**
- API Level 26+ (Android 8.0+)
- Permissões: Nenhuma (usa `getExternalFilesDir()`)
- Storage: `Android/data/com.samp.mobile/files/`

**Sistema de Arquivos:**
- Storage externo montado
- Espaço suficiente para extrair assets
- Permissões de escrita no diretório do app

---

## 4. Arquitetura do Novo Sistema

### 4.1 Estrutura de Módulos

```
core/filesystem/
├── FileSource.h              # Enums: FileSource, FileOpenMode
├── IFileSystemService.h      # Interface abstrata
├── FileSystemService.h       # Header da implementação
├── FileSystemService.cpp     # Implementação principal
├── FileLoader.h              # Wrapper para código legado
└── FileLoader.cpp            # Implementação do wrapper

platform/android/
├── AssetManagerBridge.h      # Ponte JNI (header)
└── AssetManagerBridge.cpp    # Ponte JNI (implementação)

game/hooks/
└── FileHooks.cpp             # Hook NvFOpen atualizado
```

### 4.2 Hierarquia de Classes

```
IFileSystemService (interface)
    └── FileSystemService (implementação)
            ├── OpenFile() → OpenFromStorage() / OpenFromAPK()
            ├── FileExists() → FileExistsInSource()
            ├── ReadFile() / WriteFile()
            └── ExtractAssetsIfNeeded()

FileLoader (wrapper estático)
    ├── Open() → FileSystemService::OpenFile()
    ├── OpenForGame() → ResolvePath() + OpenFile()
    └── RegisterGameRedirects() → RegisterFileRedirect()
```

### 4.3 Fluxo de Dados

```
[Código Legado]
    ↓
FileLoader::OpenForGame("DATA/GTA.DAT")
    ↓
FileSystemService::ResolvePath() → "SAMP/gta.dat"
    ↓
FileSystemService::OpenFile() → Tenta fontes em ordem:
    1. EXTERNAL_STORAGE: /storage/.../SAMP/gta.dat
    2. APK_ASSETS: Extrai para cache → abre
    3. CACHE: /storage/.../cache/SAMP/gta.dat
    ↓
Retorna FILE* (compatível com código legado)
```

### 4.4 Integração com ServiceLocator

```cpp
// Bootstrap.cpp
ServiceLocator::Register<IFileSystemService>(new FileSystemService());

// Uso em qualquer lugar
auto* fs = ServiceLocator::Get<IFileSystemService>();
FileHandle handle = fs->OpenFile("path", FileOpenMode::READ_BINARY);
```

---

## 5. Impacto nas Mudanças

### 5.1 Arquivos que Precisam ser Modificados

#### 5.1.1 Criação de Novos Arquivos

**Core:**
- ✅ `core/filesystem/FileSource.h` (NOVO)
- ✅ `core/filesystem/IFileSystemService.h` (NOVO)
- ✅ `core/filesystem/FileSystemService.h` (NOVO)
- ✅ `core/filesystem/FileSystemService.cpp` (NOVO)
- ✅ `core/filesystem/FileLoader.h` (NOVO)
- ✅ `core/filesystem/FileLoader.cpp` (NOVO)

**Platform:**
- ✅ `platform/android/AssetManagerBridge.h` (NOVO)
- ✅ `platform/android/AssetManagerBridge.cpp` (NOVO)

**Game:**
- ✅ `game/hooks/FileHooks.cpp` (NOVO ou modificar existente)

#### 5.1.2 Modificações em Arquivos Existentes

**C++:**
- 🔄 `main.cpp` - Adicionar JNI `setAssetManager()`
- 🔄 `main.cpp` - Modificar `setStoragePath()` para usar FileSystemService
- 🔄 `game/hooks.cpp` - Modificar `NvFOpen()` para usar `FileLoader`
- 🔄 `game/CFileMgr.cpp` - Modificar `OpenFile()` para usar `FileLoader`
- 🔄 `settings.cpp` - Modificar para usar `FileLoader` (opcional)
- 🔄 `game/patches.cpp` - Modificar `readVehiclesAudioSettings()` (opcional)
- 🔄 `main.cpp` - Modificar `InitGui()` para usar novo sistema
- 🔄 `core/bootstrap/Bootstrap.cpp` - Adicionar inicialização do FileSystemService

**Java:**
- 🔄 `SAMP.java` - Adicionar `setAssetManager()` nativo
- 🔄 `SAMP.java` - Chamar `setAssetManager(getAssets())` no `onCreate()`
- 🔄 `NvEventQueueActivity.java` - Opcional: migrar para novo sistema

**CMake:**
- 🔄 `CMakeLists.txt` - Adicionar novos arquivos ao build

### 5.2 Compatibilidade com Código Legado

#### 5.2.1 Código que NÃO Precisa Mudar

- ✅ Código que usa `FILE*` diretamente (via `FileLoader`)
- ✅ Código que usa `fopen()` via `CFileMgr::OpenFile()` (será redirecionado)
- ✅ Hooks que chamam `NvFOpen()` (será redirecionado)

#### 5.2.2 Código que PODE ser Migrado (Opcional)

- 🔄 `NvEventQueueActivity.loadFile()` - Pode usar novo sistema
- 🔄 `NvEventQueueActivity.loadTexture()` - Pode usar novo sistema
- 🔄 `CSettings::CSettings()` - Pode usar `FileLoader` para melhor tratamento de erros

### 5.3 Quebra de Compatibilidade

**NENHUMA** - O sistema foi projetado para ser 100% compatível com código existente através do wrapper `FileLoader`.

---

## 6. Plano de Migração

### 6.1 Fase 1: Infraestrutura Base

**Objetivo**: Criar estrutura básica sem quebrar código existente

1. ✅ Criar `FileSource.h` com enums
2. ✅ Criar `IFileSystemService.h` (interface)
3. ✅ Criar `FileSystemService.h/cpp` (implementação básica)
4. ✅ Criar `AssetManagerBridge.h/cpp`
5. ✅ Integrar no `Bootstrap` (registrar serviço)
6. ✅ Testar inicialização básica

**Critério de Sucesso**: Serviço registrado e acessível via ServiceLocator

### 6.2 Fase 2: Wrapper de Compatibilidade

**Objetivo**: Criar wrapper que mantém compatibilidade

1. ✅ Criar `FileLoader.h/cpp`
2. ✅ Implementar `FileLoader::Open()` básico
3. ✅ Implementar `FileLoader::OpenForGame()` com redirecionamentos
4. ✅ Testar com código legado

**Critério de Sucesso**: Código legado funciona sem modificações

### 6.3 Fase 3: Integração JNI

**Objetivo**: Conectar Java com sistema nativo

1. ✅ Adicionar `setAssetManager()` JNI em `main.cpp`
2. ✅ Modificar `SAMP.java` para passar AssetManager
3. ✅ Implementar `AssetManagerBridge` completamente
4. ✅ Testar acesso a assets do APK

**Critério de Sucesso**: Assets do APK podem ser lidos via nativo

### 6.4 Fase 4: Extração Automática

**Objetivo**: Extrair assets na primeira execução

1. ✅ Implementar `ExtractAssetsIfNeeded()`
2. ✅ Criar lista de assets necessários
3. ✅ Integrar no Bootstrap (fase Game)
4. ✅ Testar extração e flag de controle

**Critério de Sucesso**: Assets extraídos automaticamente na primeira execução

### 6.5 Fase 5: Migração de Hooks

**Objetivo**: Migrar hooks para usar novo sistema

1. ✅ Modificar `NvFOpen()` para usar `FileLoader::OpenForGame()`
2. ✅ Modificar `CFileMgr::OpenFile()` para usar `FileLoader`
3. ✅ Testar todos os arquivos do jogo
4. ✅ Verificar fallback funciona

**Critério de Sucesso**: Todos os arquivos do jogo carregam corretamente

### 6.6 Fase 6: Otimizações e Limpeza

**Objetivo**: Remover código legado e otimizar

1. 🔄 Remover dependência direta de `g_pszStorage` (opcional)
2. 🔄 Migrar `CSettings` para usar novo sistema (opcional)
3. 🔄 Migrar `NvEventQueueActivity` (opcional)
4. ✅ Documentação final
5. ✅ Testes de regressão completos

**Critério de Sucesso**: Sistema limpo e otimizado

---

## 7. Riscos e Considerações

### 7.1 Riscos Técnicos

#### 7.1.1 Performance
- **Risco**: Extração de assets pode ser lenta na primeira execução
- **Mitigação**: 
  - Extração assíncrona (background thread)
  - Mostrar progresso ao usuário
  - Cache de flag para evitar reextração

#### 7.1.2 Espaço em Disco
- **Risco**: Assets duplicados (APK + Storage)
- **Mitigação**:
  - Assets grandes podem ficar apenas no APK
  - Extração sob demanda (lazy loading)
  - Limpeza de cache periódica

#### 7.1.3 Thread Safety
- **Risco**: Acesso concorrente a arquivos
- **Mitigação**:
  - Mutex em operações críticas
  - File handles não compartilhados entre threads
  - Documentar thread-safety

#### 7.1.4 Compatibilidade com libGTASA
- **Risco**: libGTASA pode esperar paths específicos
- **Mitigação**:
  - Manter compatibilidade via redirecionamentos
  - Testar extensivamente
  - Fallback para comportamento antigo se necessário

### 7.2 Riscos de Migração

#### 7.2.1 Regressões
- **Risco**: Arquivos não carregam após migração
- **Mitigação**:
  - Testes extensivos antes de merge
  - Manter código antigo como fallback temporário
  - Logging detalhado para debug

#### 7.2.2 Dependências Ocultas
- **Risco**: Código que acessa `g_pszStorage` diretamente
- **Mitigação**:
  - Busca completa por `g_pszStorage` no código
  - Refatorar gradualmente
  - Manter `g_pszStorage` como compatibilidade temporária

### 7.3 Considerações de Design

#### 7.3.1 Extensibilidade Futura
- **Consideração**: Pode precisar adicionar novas fontes (rede, cloud)
- **Solução**: Interface `IFileSystemService` permite extensão fácil

#### 7.3.2 Testabilidade
- **Consideração**: Sistema precisa ser testável
- **Solução**: Interface permite mocks, ServiceLocator permite substituição

#### 7.3.3 Manutenibilidade
- **Consideração**: Código deve ser fácil de entender e modificar
- **Solução**: Documentação completa, código modular, exemplos claros

---

## 8. Testes Necessários

### 8.1 Testes Unitários

**FileSystemService:**
- ✅ `OpenFile()` com arquivo existente no storage
- ✅ `OpenFile()` com arquivo apenas no APK
- ✅ `OpenFile()` com arquivo inexistente
- ✅ `FileExists()` em todas as fontes
- ✅ `ReadFile()` / `WriteFile()` completos
- ✅ `ResolvePath()` com redirecionamentos
- ✅ `ExtractAssetsIfNeeded()` primeira execução
- ✅ `ExtractAssetsIfNeeded()` execuções subsequentes

**FileLoader:**
- ✅ `Open()` com vários modos
- ✅ `OpenForGame()` com redirecionamentos
- ✅ `RegisterGameRedirects()` configura corretamente

**AssetManagerBridge:**
- ✅ `GetAssetManagerFromJNI()` converte corretamente
- ✅ `AssetExists()` retorna correto
- ✅ `ReadAsset()` lê completamente

### 8.2 Testes de Integração

**Inicialização:**
- ✅ Bootstrap registra FileSystemService
- ✅ JNI recebe AssetManager corretamente
- ✅ Storage path é definido antes de uso
- ✅ Assets são extraídos na primeira execução

**Carregamento de Arquivos:**
- ✅ Todos os arquivos do jogo carregam
- ✅ Fallback funciona quando arquivo não está no storage
- ✅ Redirecionamentos funcionam corretamente
- ✅ Mods sobrescrevem arquivos do APK

**Compatibilidade:**
- ✅ `NvFOpen()` funciona como antes
- ✅ `CFileMgr::OpenFile()` funciona como antes
- ✅ `CSettings` carrega `settings.ini`
- ✅ Fontes carregam de `SAMP/fonts/`

### 8.3 Testes de Regressão

**Cenários Críticos:**
- ✅ Jogo inicia normalmente
- ✅ Todos os arquivos de dados carregam
- ✅ Texturas carregam
- ✅ Fontes carregam
- ✅ Settings carregam
- ✅ Mods funcionam (arquivos sobrescritos)

**Cenários de Erro:**
- ✅ Arquivo não existe (fallback funciona)
- ✅ Storage não disponível (usa APK)
- ✅ APK corrompido (erro tratado)
- ✅ Sem espaço em disco (erro tratado)

### 8.4 Testes de Performance

**Métricas:**
- ⏱️ Tempo de extração de assets (primeira execução)
- ⏱️ Tempo de abertura de arquivo (média)
- ⏱️ Overhead do novo sistema vs antigo
- 💾 Uso de memória (comparação)

**Benchmarks:**
- Comparar `fopen()` direto vs `FileLoader::Open()`
- Medir impacto do fallback
- Medir overhead de redirecionamentos

---

## 9. Checklist de Implementação

### 9.1 Preparação

- [ ] Revisar arquitetura proposta
- [ ] Identificar todos os pontos de acesso a arquivos
- [ ] Criar branch de desenvolvimento
- [ ] Configurar ambiente de testes

### 9.2 Implementação Core

- [ ] Criar `FileSource.h`
- [ ] Criar `IFileSystemService.h`
- [ ] Criar `FileSystemService.h`
- [ ] Implementar `FileSystemService.cpp` básico
- [ ] Implementar `OpenFile()` básico
- [ ] Implementar `FileExists()` básico
- [ ] Testar inicialização

### 9.3 Implementação Wrapper

- [ ] Criar `FileLoader.h`
- [ ] Criar `FileLoader.cpp`
- [ ] Implementar `Open()` básico
- [ ] Implementar `OpenForGame()` com redirecionamentos
- [ ] Implementar `RegisterGameRedirects()`
- [ ] Testar compatibilidade

### 9.4 Implementação Android

- [ ] Criar `AssetManagerBridge.h`
- [ ] Criar `AssetManagerBridge.cpp`
- [ ] Implementar `GetAssetManagerFromJNI()`
- [ ] Implementar `AssetExists()`
- [ ] Implementar `ReadAsset()`
- [ ] Testar acesso a assets

### 9.5 Integração JNI

- [ ] Adicionar `setAssetManager()` JNI em `main.cpp`
- [ ] Modificar `SAMP.java` para passar AssetManager
- [ ] Conectar `FileSystemService` com `AssetManagerBridge`
- [ ] Testar integração completa

### 9.6 Extração de Assets

- [ ] Implementar `ExtractAssetsIfNeeded()`
- [ ] Criar lista de assets necessários
- [ ] Implementar flag de controle (`.assets_extracted`)
- [ ] Integrar no Bootstrap
- [ ] Testar extração automática

### 9.7 Migração de Hooks

- [ ] Modificar `NvFOpen()` para usar `FileLoader`
- [ ] Modificar `CFileMgr::OpenFile()` para usar `FileLoader`
- [ ] Testar todos os arquivos do jogo
- [ ] Verificar fallback funciona

### 9.8 Testes

- [ ] Testes unitários (todos passando)
- [ ] Testes de integração (todos passando)
- [ ] Testes de regressão (todos passando)
- [ ] Testes de performance (dentro do esperado)

### 9.9 Documentação

- [ ] Documentar interface `IFileSystemService`
- [ ] Documentar uso do `FileLoader`
- [ ] Documentar redirecionamentos
- [ ] Documentar extração de assets
- [ ] Atualizar este documento com resultados

### 9.10 Cleanup (Opcional)

- [ ] Remover dependências diretas de `g_pszStorage`
- [ ] Migrar `CSettings` para novo sistema
- [ ] Migrar `NvEventQueueActivity` para novo sistema
- [ ] Remover código legado não usado

---

## 10. Referências e Notas

### 10.1 Arquivos de Referência

- `temporarios/ARCHITECTURE_PLAN.md` - Arquitetura geral do projeto
- `temporarios/SUBSYSTEMS_ARCHITECTURE.md` - Arquitetura de subsistemas
- `app/src/main/cpp/samp/core/bootstrap/ServiceLocator.h` - Sistema de serviços

### 10.2 Notas de Implementação

- **Thread Safety**: `FileSystemService` usa mutex apenas em operações de redirecionamento. Operações de arquivo individuais não são thread-safe (como `fopen()` padrão).

- **Memory Management**: `FileHandle` não gerencia memória do `FILE*`. Código legado continua responsável por `fclose()`.

- **Error Handling**: Sistema não lança exceções. Retorna `nullptr` ou valores vazios em caso de erro, mantendo compatibilidade com código C legado.

- **Path Normalization**: Todos os paths são normalizados (backslash → forward slash, barras duplicadas removidas).

### 10.3 Próximos Passos

Após implementação bem-sucedida:
1. Considerar migração completa de `NvEventQueueActivity`
2. Adicionar suporte a streaming de arquivos grandes
3. Implementar cache inteligente de assets
4. Adicionar suporte a arquivos remotos (futuro)

---

**Documento criado em**: 2024
**Última atualização**: 2024
**Versão**: 1.0
