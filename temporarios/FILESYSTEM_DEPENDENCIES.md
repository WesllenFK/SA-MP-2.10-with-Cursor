# Mapeamento de Dependências: Sistema de Arquivos

## 📋 Índice

1. [Dependências de Código](#dependências-de-código)
2. [Dependências de Arquivos](#dependências-de-arquivos)
3. [Dependências de Sistema](#dependências-de-sistema)
4. [Mapeamento de Impacto](#mapeamento-de-impacto)
5. [Ordem de Compilação](#ordem-de-compilação)

---

## 1. Dependências de Código

### 1.1 Dependências C++

#### Headers Necessários

```
core/filesystem/FileSource.h
    └── Dependências: Nenhuma (apenas std)

core/filesystem/IFileSystemService.h
    └── Dependências: FileSource.h, <vector>, <string>, <cstdint>

core/filesystem/FileSystemService.h
    └── Dependências: IFileSystemService.h, <map>, <mutex>, <vector>

core/filesystem/FileSystemService.cpp
    └── Dependências: 
        - FileSystemService.h
        - AssetManagerBridge.h
        - core/logging/Log.h
        - <fstream>
        - <sys/stat.h>
        - <unistd.h>

core/filesystem/FileLoader.h
    └── Dependências: IFileSystemService.h, <cstdio>

core/filesystem/FileLoader.cpp
    └── Dependências:
        - FileLoader.h
        - core/bootstrap/ServiceLocator.h
        - core/logging/Log.h
        - <cstring>

platform/android/AssetManagerBridge.h
    └── Dependências: 
        - <android/asset_manager.h>
        - <vector>
        - <string>

platform/android/AssetManagerBridge.cpp
    └── Dependências:
        - AssetManagerBridge.h
        - <android/asset_manager_jni.h>
        - <jni.h>

game/hooks/FileHooks.cpp
    └── Dependências:
        - FileLoader.h
        - core/logging/Log.h
        - <cstdlib>
```

#### Bibliotecas do Sistema

```
libc (C Standard Library)
    - fopen, fread, fwrite, fclose
    - fseek, ftell
    - sprintf, strncpy, strcmp

libc++ (C++ Standard Library)
    - std::string
    - std::vector
    - std::map
    - std::mutex

libandroid (Android NDK)
    - AAssetManager_fromJava
    - AAssetManager_open
    - AAsset_read
    - AAsset_close
    - AAsset_getLength

liblog (Android)
    - __android_log_print (via Log.h)

libjni (JNI)
    - JNIEnv
    - jobject
    - GetStringUTFChars
    - ReleaseStringUTFChars
```

### 1.2 Dependências Java

#### Classes Android

```java
android.content.res.AssetManager
    └── Uso: Acesso a assets do APK

android.content.Context
    └── Uso: Obter AssetManager via getAssets()

java.io.FileInputStream
    └── Uso: (Legado) Carregamento de arquivos

java.io.InputStream
    └── Uso: (Legado) Leitura de dados
```

#### JNI Methods

```java
// SAMP.java
public native void setStoragePath(String path);
public native void setAssetManager(Object assetManager);  // NOVO
```

### 1.3 Dependências de Vendor Code

```
vendor/armhook/patch.h
    └── Uso: Hooks (não usado diretamente no novo sistema)

vendor/inih/cpp/INIReader.h
    └── Uso: Parser INI (settings.cpp - não muda)

vendor/SimpleIni/SimpleIni.h
    └── Uso: Parser INI alternativo (não usado no novo sistema)
```

---

## 2. Dependências de Arquivos

### 2.1 Arquivos do Jogo (Obrigatórios)

#### Arquivos Críticos (Falha Fatal se Ausentes)

```
SAMP/settings.ini
    └── Carregado por: CSettings::CSettings()
    └── Uso: Configurações do cliente
    └── Status: Fatal se não existir (std::terminate)
    └── Migração: Pode usar FileLoader para melhor tratamento
```

#### Arquivos Essenciais (Jogo não funciona sem)

```
SAMP/main.scm
    └── Carregado por: NvFOpen("mainV1.scm")
    └── Redirecionamento: "mainV1.scm" → "SAMP/main.scm"
    └── Tamanho: ~500KB - 2MB

SAMP/script.img
    └── Carregado por: NvFOpen("SCRIPTV1.IMG")
    └── Redirecionamento: "SCRIPTV1.IMG" → "SAMP/script.img"
    └── Tamanho: ~10MB - 50MB

SAMP/gta.dat
    └── Carregado por: NvFOpen("DATA/GTA.DAT")
    └── Redirecionamento: "DATA/GTA.DAT" → "SAMP/gta.dat"
    └── Tamanho: ~100KB

SAMP/peds.ide
    └── Carregado por: NvFOpen("DATA/PEDS.IDE")
    └── Redirecionamento: "DATA/PEDS.IDE" → "SAMP/peds.ide"
    └── Tamanho: ~50KB

SAMP/vehicles.ide
    └── Carregado por: NvFOpen("DATA/VEHICLES.IDE")
    └── Redirecionamento: "DATA/VEHICLES.IDE" → "SAMP/vehicles.ide"
    └── Tamanho: ~20KB

SAMP/handling.cfg
    └── Carregado por: NvFOpen("DATA/HANDLING.CFG")
    └── Redirecionamento: "DATA/HANDLING.CFG" → "SAMP/handling.cfg"
    └── Tamanho: ~100KB

SAMP/weapon.dat
    └── Carregado por: NvFOpen("DATA/WEAPON.DAT")
    └── Redirecionamento: "DATA/WEAPON.DAT" → "SAMP/weapon.dat"
    └── Tamanho: ~10KB

data/fonts.dat
    └── Carregado por: NvFOpen("DATA/FONTS.DAT")
    └── Redirecionamento: "DATA/FONTS.DAT" → "data/fonts.dat"
    └── Tamanho: ~500KB

data/pedstats.dat
    └── Carregado por: NvFOpen("DATA/PEDSTATS.DAT")
    └── Redirecionamento: "DATA/PEDSTATS.DAT" → "data/pedstats.dat"
    └── Tamanho: ~50KB

data/timecyc.dat
    └── Carregado por: NvFOpen("DATA/TIMECYC.DAT")
    └── Redirecionamento: "DATA/TIMECYC.DAT" → "data/timecyc.dat"
    └── Tamanho: ~100KB

data/popcycle.dat
    └── Carregado por: NvFOpen("DATA/POPCYCLE.DAT")
    └── Redirecionamento: "DATA/POPCYCLE.DAT" → "data/popcycle.dat"
    └── Tamanho: ~10KB
```

#### Arquivos de Fontes

```
SAMP/fonts/arial_bold.ttf
    └── Carregado por: InitGui() em main.cpp
    └── Path: {storage}SAMP/fonts/{FONT_NAME}
    └── FONT_NAME definido em: main.h (FONT_NAME = "arial_bold.ttf")
    └── Tamanho: ~50KB - 200KB
    └── Pode ser sobrescrito por: settings.ini (gui.Font)
```

### 2.2 Arquivos Opcionais

```
SAMP/vehicleAudioSettings.cfg
    └── Carregado por: readVehiclesAudioSettings()
    └── Status: Opcional (retorna silenciosamente se não existir)
    └── Tamanho: ~10KB - 50KB

NickName.ini
    └── Carregado por: ReadSettingFile() (comentado)
    └── Status: Não usado atualmente
    └── Tamanho: ~1KB
```

### 2.3 Assets do APK

#### Lista de Assets (assetfile.txt)

```
assetfile.txt (123 arquivos listados)
    ├── Fonts/*.TTF, *.otf (13 arquivos)
    ├── Textures/fonts/*.png, *.met (3 arquivos)
    ├── images/*.png (12 arquivos)
    ├── audio/*.ogg, *.xml (3 arquivos)
    ├── Text/*.gxt (7 arquivos)
    ├── data/*.cfg, *.dat, *.scm, *.img (12 arquivos)
    ├── xml/*.xml (7 arquivos)
    ├── json/*.json (2 arquivos)
    ├── socialclub/*.png (59 arquivos)
    └── scache*.txt (3 arquivos)
```

#### Assets Críticos para Extração

```
SAMP/main.scm
SAMP/script.img
SAMP/peds.ide
SAMP/vehicles.ide
SAMP/gta.dat
SAMP/handling.cfg
SAMP/weapon.dat
data/fonts.dat
data/pedstats.dat
data/timecyc.dat
data/popcycle.dat
```

**Total estimado**: ~15MB - 60MB (dependendo dos arquivos)

---

## 3. Dependências de Sistema

### 3.1 Android

#### API Level
- **Mínimo**: API 26 (Android 8.0)
- **Recomendado**: API 29+ (Android 10+)
- **Motivo**: `getExternalFilesDir()` funciona sem permissões

#### Permissões
```
NENHUMA PERMISSÃO NECESSÁRIA
    └── getExternalFilesDir() não requer permissões
    └── Android 11+ (API 30+): Scoped Storage automático
```

#### Storage
```
Android/data/com.samp.mobile/files/
    ├── SAMP/              (arquivos do jogo)
    ├── data/              (arquivos de dados)
    ├── cache/             (cache de assets extraídos)
    └── .assets_extracted  (flag de extração)
```

#### Espaço em Disco
- **Mínimo**: 100MB livres
- **Recomendado**: 500MB+ livres
- **Motivo**: Assets extraídos + cache + dados do jogo

### 3.2 Sistema de Arquivos

#### Requisitos
- Storage externo montado
- Permissões de escrita no diretório do app
- Espaço suficiente para extração

#### Estrutura de Diretórios
```
/storage/emulated/0/Android/data/com.samp.mobile/files/
    ├── SAMP/
    │   ├── main.scm
    │   ├── script.img
    │   ├── *.ide, *.dat, *.cfg
    │   └── fonts/
    ├── data/
    │   └── *.dat
    ├── cache/
    │   └── (assets extraídos do APK)
    └── .assets_extracted
```

---

## 4. Mapeamento de Impacto

### 4.1 Arquivos que Usam Sistema de Arquivos

#### Código Nativo

| Arquivo | Função | Impacto | Ação Necessária |
|---------|--------|---------|------------------|
| `game/hooks.cpp` | `NvFOpen()` | 🔴 ALTO | Modificar para usar `FileLoader` |
| `game/CFileMgr.cpp` | `OpenFile()` | 🟡 MÉDIO | Modificar para usar `FileLoader` |
| `settings.cpp` | `CSettings()` | 🟡 MÉDIO | Opcional: usar `FileLoader` |
| `game/patches.cpp` | `readVehiclesAudioSettings()` | 🟢 BAIXO | Opcional: usar `FileLoader` |
| `main.cpp` | `InitGui()` | 🟡 MÉDIO | Modificar para usar novo sistema |
| `main.cpp` | `setStoragePath()` JNI | 🟡 MÉDIO | Integrar com `FileSystemService` |
| `main.cpp` | `ReadSettingFile()` | 🟢 BAIXO | Código comentado, ignorar |

#### Código Java

| Arquivo | Função | Impacto | Ação Necessária |
|---------|--------|---------|------------------|
| `SAMP.java` | `onCreate()` | 🟡 MÉDIO | Adicionar `setAssetManager()` |
| `SAMP.java` | `setStoragePath()` | 🟢 BAIXO | Já existe, manter |
| `NvEventQueueActivity.java` | `loadFile()` | 🟢 BAIXO | Opcional: migrar |
| `NvEventQueueActivity.java` | `loadTexture()` | 🟢 BAIXO | Opcional: migrar |
| `NvAPKFileHelper.java` | `openFileAndroid()` | 🟢 BAIXO | Pode ser mantido para áudio |

### 4.2 Dependências de Variáveis Globais

#### Variáveis que Serão Substituídas

```cpp
// ANTES (global direto)
extern char* g_pszStorage;
sprintf(path, "%s%s", g_pszStorage, filename);

// DEPOIS (via ServiceLocator)
auto* fs = ServiceLocator::Get<IFileSystemService>();
FileHandle handle = fs->OpenFile(filename);
```

#### Variáveis que Serão Mantidas (Compatibilidade)

```cpp
// Mantido para compatibilidade temporária
extern char* g_pszStorage;  // Será preenchido pelo FileSystemService

// Código legado continua funcionando
sprintf(path, "%s%s", g_pszStorage, filename);  // Ainda funciona
```

### 4.3 Pontos de Integração

#### Bootstrap (Inicialização)

```cpp
// core/bootstrap/Bootstrap.cpp

// Fase Platform
RegisterInitializer(Phase::Platform, []() {
    auto* fs = new FileSystem::FileSystemService();
    ServiceLocator::Register<FileSystem::IFileSystemService>(fs);
    
    // Preenche g_pszStorage para compatibilidade
    // (opcional, pode ser removido depois)
});

// Fase Game
RegisterInitializer(Phase::Game, []() {
    auto* fs = ServiceLocator::Get<FileSystem::IFileSystemService>();
    FileLoader::RegisterGameRedirects(fs);
    
    std::vector<std::string> assets = {
        "SAMP/main.scm",
        "SAMP/script.img",
        // ... outros
    };
    fs->ExtractAssetsIfNeeded(assets);
});
```

#### JNI (Java → Native)

```cpp
// main.cpp

JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_setStoragePath(
    JNIEnv *pEnv, jobject thiz, jstring path)
{
    // ... código existente ...
    
    // NOVO: Inicializa FileSystemService
    auto* fs = ServiceLocator::Get<FileSystem::IFileSystemService>();
    if (fs) {
        fs->Initialize(storagePath);
    }
}

JNIEXPORT void JNICALL Java_com_samp_mobile_game_SAMP_setAssetManager(
    JNIEnv *pEnv, jobject thiz, jobject assetManager)  // NOVO
{
    void* aassetMgr = AssetManagerBridge::GetAssetManagerFromJNI(pEnv, assetManager);
    auto* fs = ServiceLocator::Get<FileSystem::IFileSystemService>();
    if (fs) {
        fs->SetAssetManager(aassetMgr);
    }
}
```

---

## 5. Ordem de Compilação

### 5.1 Dependências de Build

```
1. FileSource.h (sem dependências)
   ↓
2. IFileSystemService.h (depende de FileSource.h)
   ↓
3. AssetManagerBridge.h (sem dependências de filesystem)
   ↓
4. AssetManagerBridge.cpp (depende de AssetManagerBridge.h)
   ↓
5. FileSystemService.h (depende de IFileSystemService.h)
   ↓
6. FileSystemService.cpp (depende de FileSystemService.h + AssetManagerBridge.h)
   ↓
7. FileLoader.h (depende de IFileSystemService.h)
   ↓
8. FileLoader.cpp (depende de FileLoader.h + ServiceLocator.h)
   ↓
9. FileHooks.cpp (depende de FileLoader.h)
```

### 5.2 Ordem de Inicialização (Runtime)

```
1. JNI_OnLoad()
   ↓
2. Bootstrap::Initialize() - Fase Platform
   → Cria FileSystemService
   → Registra no ServiceLocator
   ↓
3. SAMP.onCreate() (Java)
   → setStoragePath() (JNI)
   → FileSystemService::Initialize()
   → setAssetManager() (JNI)  // NOVO
   → FileSystemService::SetAssetManager()
   ↓
4. Bootstrap::Initialize() - Fase Game
   → FileLoader::RegisterGameRedirects()
   → FileSystemService::ExtractAssetsIfNeeded()
   ↓
5. Hooks instalados
   → NvFOpen() agora usa FileLoader
   ↓
6. Jogo inicia
   → Arquivos carregam via novo sistema
```

### 5.3 CMakeLists.txt (Adições Necessárias)

```cmake
# Core filesystem
add_library(filesystem STATIC
    core/filesystem/FileSource.h
    core/filesystem/IFileSystemService.h
    core/filesystem/FileSystemService.h
    core/filesystem/FileSystemService.cpp
    core/filesystem/FileLoader.h
    core/filesystem/FileLoader.cpp
)

# Platform Android
add_library(assetbridge STATIC
    platform/android/AssetManagerBridge.h
    platform/android/AssetManagerBridge.cpp
)

# Game hooks
add_library(filehooks STATIC
    game/hooks/FileHooks.cpp
)

# Dependências
target_link_libraries(filesystem 
    PUBLIC 
        assetbridge
        core_logging
        core_bootstrap
)

target_link_libraries(filehooks
    PUBLIC
        filesystem
)
```

---

## 6. Checklist de Dependências

### 6.1 Dependências de Código

- [x] FileSource.h criado
- [x] IFileSystemService.h criado
- [x] FileSystemService implementado
- [x] FileLoader implementado
- [x] AssetManagerBridge implementado
- [x] ServiceLocator disponível
- [x] Sistema de Log disponível
- [x] Bootstrap configurado

### 6.2 Dependências de Arquivos

- [ ] Lista completa de assets identificada
- [ ] assetfile.txt atualizado (se necessário)
- [ ] Assets movidos para `app/src/main/assets/`
- [ ] Estrutura de diretórios criada

### 6.3 Dependências de Sistema

- [ ] Android API 26+ verificado
- [ ] Permissões verificadas (nenhuma necessária)
- [ ] Espaço em disco verificado
- [ ] Storage externo acessível

### 6.4 Integração

- [ ] CMakeLists.txt atualizado
- [ ] JNI methods adicionados
- [ ] Bootstrap configurado
- [ ] Hooks migrados
- [ ] Testes configurados

---

**Documento criado em**: 2024
**Versão**: 1.0
