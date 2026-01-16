# Análise Detalhada de Migração

Este documento contém a análise de cada arquivo do projeto, suas dependências, e as mudanças necessárias para a reestruturação modular.

---

## Legenda

| Símbolo | Significado |
|---------|-------------|
| 📁 | Diretório |
| 📄 | Arquivo |
| ➡️ | Mover para |
| 🔗 | Dependência |
| ⚠️ | Problema identificado |
| ✏️ | Mudança necessária |

---

## 1. Arquivos Raiz (`samp/`)

### 📄 main.cpp / main.h

**Localização atual:** `samp/main.cpp`, `samp/main.h`
**Mover para:** `samp/core/main.cpp`, `samp/core/main.h`

**Includes atuais (main.cpp):**
```cpp
#include <jni.h>
#include <pthread.h>
#include <syscall.h>
#include "main.h"
#include "game/game.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"
#include "audiostream.h"
#include "java/jniutil.h"
#include <dlfcn.h>
#include "StackTrace.h"
#include "servers.h"
#include "voice_new/Plugin.h"
#include "vendor/armhook/patch.h"
#include "vendor/str_obfuscator/str_obfuscator.hpp"
#include "settings.h"
#include "crashlytics.h"
#include "game/multitouch.h"
#include "armhook/patch.h"
#include "util/CUtil.h"
#include "obfusheader.h"
```

**Globals expostos:**
```cpp
extern char* g_pszStorage;
extern bool g_bStoragePathSetViaJNI;
extern UI* pUI;
extern CGame* pGame;
extern CNetGame* pNetGame;
extern CPlayerTags* pPlayerTags;
extern CSnapShotHelper* pSnapShotHelper;
extern CAudioStream* pAudioStream;
extern CJavaWrapper* pJavaWrapper;
extern CSettings* pSettings;
extern MaterialTextGenerator* pMaterialTextGenerator;
extern uintptr_t g_libGTASA;
extern uintptr_t g_libSAMP;
extern JavaVM* javaVM;
```

**⚠️ Problemas:**
1. Mistura inicialização, JNI handlers, signal handlers, logging
2. Globals expostos sem encapsulamento
3. Funções utilitárias misturadas (FLog, ChatLog, MyLog, etc)

**✏️ Mudanças necessárias:**
1. Separar em arquivos:
   - `core/main.cpp` - Entry point e inicialização
   - `core/globals.cpp/h` - SAMPCore com acesso centralizado
   - `core/logging.cpp/h` - Funções de log
   - `platform/android/signal_handler.cpp` - Signal handlers
2. Atualizar includes para novos caminhos

---

### 📄 settings.cpp / settings.h

**Localização atual:** `samp/settings.cpp`, `samp/settings.h`
**Mover para:** `samp/core/settings.cpp`, `samp/core/settings.h`

**Includes atuais:**
```cpp
#include "main.h"
#include "settings.h"
#include "vendor/inih/cpp/INIReader.h"
#include "vendor/SimpleIni/SimpleIni.h"
#include "game/game.h"
```

**Dependências externas:**
- `g_pszStorage` (global)
- `pGame` (global - não deveria depender)
- INIReader (vendor)

**⚠️ Problemas:**
1. Depende de `pGame` desnecessariamente
2. Usa global `g_pszStorage` diretamente

**✏️ Mudanças necessárias:**
1. Remover dependência de `game.h`
2. Receber storage path como parâmetro no construtor
3. Atualizar includes:
   ```cpp
   #include "main.h"           → #include "core/main.h"
   #include "vendor/inih/..."  → sem mudança (vendor não muda)
   ```

---

### 📄 log.cpp / log.h

**Localização atual:** `samp/log.cpp`, `samp/log.h`
**Mover para:** `samp/core/logging.cpp`, `samp/core/logging.h`

**Includes atuais:**
```cpp
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include "log.h"
#include <android/log.h>
```

**⚠️ Problemas:**
1. Código praticamente comentado/não usado
2. Macros LOGI, LOGE, LOGW deveriam estar aqui

**✏️ Mudanças necessárias:**
1. Mover macros de logging de `log.h` 
2. Mover funções FLog, ChatLog, MyLog de `main.cpp` para cá
3. Criar interface unificada de logging

---

### 📄 audiostream.cpp / audiostream.h

**Localização atual:** `samp/audiostream.cpp`, `samp/audiostream.h`
**Mover para:** `samp/audio/audiostream.cpp`, `samp/audio/audiostream.h`

**Includes atuais:**
```cpp
#include <memory>
#include <unistd.h>
#include "audiostream.h"
#include "log.h"
#include "game/game.h"
#include "../main.h"
#include "../vendor/bass/bass.h"
```

**Dependências:**
- `pGame` (global) - usado para `IsGamePaused()`
- BASS library (vendor)

**⚠️ Problemas:**
1. Globals para estado do stream (g_szAudioStreamUrl, etc)
2. Depende de pGame

**✏️ Mudanças necessárias:**
1. Encapsular globals dentro da classe
2. Receber callback para verificar pause state em vez de acessar pGame diretamente
3. Atualizar includes:
   ```cpp
   #include "log.h"           → #include "core/logging.h"
   #include "game/game.h"     → remover (usar callback)
   #include "../main.h"       → #include "core/main.h"
   ```

---

### 📄 playertags.cpp / playertags.h

**Localização atual:** `samp/playertags.cpp`, `samp/playertags.h`
**Mover para:** `samp/multiplayer/features/playertags.cpp`, `samp/multiplayer/features/playertags.h`

**Includes atuais:**
```cpp
#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"
#include "util/CUtil.h"
#include "game/World.h"
```

**Dependências:**
- `pGame`, `pNetGame` (globals)
- `g_libGTASA` (para offset de CCamera)
- ImGui/ImGuiRenderer
- RenderWare
- UISettings

**⚠️ Problemas:**
1. Acesso direto a offsets do jogo (hardcoded)
2. Depende de múltiplos módulos

**✏️ Mudanças necessárias:**
1. Atualizar includes para novos caminhos
2. Acessar dependências via SAMPCore em vez de globals

---

### 📄 nv_event.cpp / nv_event.h

**Localização atual:** `samp/nv_event.cpp`, `samp/nv_event.h`
**Mover para:** `samp/platform/android/nv_event.cpp`, `samp/platform/android/nv_event.h`

**Includes atuais:**
```cpp
#include "nv_event.h"
#include "main.h"
#include "game/RW/RenderWare.h"
#include "game/game.h"
#include "net/netgame.h"
#include <list>
#include <queue>
```

**Funcionalidade:** Handler de eventos de toque vindos do Java (JNI)

**⚠️ Problemas:**
1. Inclui game.h e netgame.h mas não usa (remover)

**✏️ Mudanças necessárias:**
1. Remover includes desnecessários
2. Mover para platform/android/

---

### 📄 CFPSFix.cpp / CFPSFix.h

**Localização atual:** `samp/CFPSFix.cpp`, `samp/CFPSFix.h`
**Mover para:** `samp/platform/android/fps_fix.cpp`, `samp/platform/android/fps_fix.h`

**Includes atuais:**
```cpp
#include "CFPSFix.h"
#include "main.h"
#include <sys/syscall.h>
```

**Funcionalidade:** Thread affinity fix para FPS

**✏️ Mudanças necessárias:**
1. Mover para platform/android/
2. Renomear para snake_case (fps_fix)

---

### 📄 crashlytics.h

**Localização atual:** `samp/crashlytics.h`
**Mover para:** `samp/platform/crashlytics.h`

**Funcionalidade:** Firebase Crashlytics NDK API

**✏️ Mudanças:** Apenas mover, é self-contained

---

### 📄 StackTrace.h

**Localização atual:** `samp/StackTrace.h`
**Mover para:** `samp/platform/android/stacktrace.h`

**Dependências:**
- `g_libGTASA`, `g_libSAMP` (globals para offsets)
- FLog function

**✏️ Mudanças necessárias:**
1. Mover para platform/android/
2. Atualizar include de main.h

---

### 📄 servers.h

**Localização atual:** `samp/servers.h`
**Mover para:** `samp/core/servers.h`

**Conteúdo:** Definições de servidores (IP/porta)

**✏️ Mudanças:** Apenas mover

---

### 📄 obfusheader.h

**Localização atual:** `samp/obfusheader.h`
**Mover para:** `samp/vendor/obfusheader/obfusheader.h` ou manter em core

**Funcionalidade:** Ofuscação de strings

**✏️ Mudanças:** Mover para vendor ou core/

---

## 2. Pasta `game/`

### 📁 Estrutura Atual
```
game/
├── [arquivos raiz - 145 arquivos .cpp/.h]
├── Animation/     (23 arquivos)
├── Collision/     (31 arquivos)
├── Core/          (37 arquivos)
├── Entity/        (22 arquivos)
├── Enums/         (28 arquivos)
├── Events/        (8 arquivos)
├── Models/        (12 arquivos)
├── Pipelines/     (8 arquivos)
├── Plugins/       (6 arquivos)
├── RW/            (34 arquivos)
├── Tasks/         (19 arquivos)
├── Textures/      (10 arquivos)
└── Widgets/       (16 arquivos)
```

### 📁 Nova Estrutura Proposta
```
game/
├── hooks/
│   ├── hooks.cpp/h       ← de game/hooks.cpp
│   └── patches.cpp/h     ← de game/patches.cpp
├── engine/
│   ├── game.cpp/h        ← de game/game.cpp
│   ├── world.cpp/h       ← de game/World.cpp
│   ├── camera.cpp/h      ← de game/Camera.cpp
│   ├── streaming.cpp/h   ← de game/Streaming.cpp
│   ├── pools.cpp/h       ← de game/Pools.cpp
│   └── timer.cpp/h       ← de game/Timer.cpp
├── entities/
│   ├── playerped.cpp/h   ← de game/playerped.cpp
│   ├── vehicle.cpp/h     ← de game/vehicle.cpp
│   ├── object.cpp/h      ← de game/object.cpp
│   ├── actor.cpp/h       ← de game/actor.cpp
│   └── Entity/           ← pasta Entity/ inteira
├── rendering/
│   ├── RW/               ← pasta RW/ inteira
│   ├── Textures/         ← pasta Textures/ inteira
│   ├── font.cpp/h
│   ├── sprite2d.cpp/h
│   ├── textdraw.cpp/h
│   ├── Coronas.cpp/h
│   └── Shadows.cpp/h
├── physics/
│   ├── Collision/        ← pasta Collision/ inteira
│   └── aimstuff.cpp/h
├── animation/
│   └── Animation/        ← pasta Animation/ inteira
├── input/
│   ├── pad.cpp/h
│   └── multitouch.cpp/h
├── core/                 ← pasta Core/ (types básicos)
│   └── Core/             ← pasta Core/ inteira
└── [outros]
    ├── Enums/
    ├── Events/
    ├── Models/
    ├── Pipelines/
    ├── Plugins/
    ├── Tasks/
    └── Widgets/
```

---

### 📄 game/hooks.cpp

**Mover para:** `samp/game/hooks/hooks.cpp`

**Includes atuais (47 includes!):**
```cpp
#include <GLES2/gl2.h>
#include "../main.h"
#include "../vendor/armhook/patch.h"
#include "game.h"
#include "../net/netgame.h"
#include "../gui/gui.h"
#include "Textures/TextureDatabase.h"
#include "Textures/TextureDatabaseEntry.h"
#include "Textures/TextureDatabaseRuntime.h"
#include "Scene.h"
#include "sprite2d.h"
#include "Entity/PlayerPedGta.h"
#include "Pools.h"
#include "java/jniutil.h"
#include "game/Models/ModelInfo.h"
#include "MatrixLink.h"
#include "MatrixLinkList.h"
#include "game/Collision/Collision.h"
#include "TxdStore.h"
#include "util/CUtil.h"
#include "Coronas.h"
#include "multitouch.h"
#include "Streaming.h"
#include "References.h"
#include "VisibilityPlugins.h"
#include "game/Animation/AnimManager.h"
#include "FileLoader.h"
#include "Renderer.h"
#include "CrossHair.h"
#include "World.h"
#include "Core/Matrix.h"
```

**Globals usados:**
```cpp
extern UI* pUI;
extern CGame* pGame;
extern CNetGame *pNetGame;
extern MaterialTextGenerator* pMaterialTextGenerator;
extern CJavaWrapper* pJavaWrapper;
```

**Funções principais:**
- `FindPlayerIDFromGtaPtr()` - Encontra PlayerID de um ponteiro GTA
- `FindActorIDFromGtaPtr()` - Encontra ActorID de um ponteiro GTA
- `RenderEffects()` - Renderiza efeitos
- `ShowHud()` - Mostra HUD
- `MainLoop()` hook - Hook do loop principal
- Diversos hooks de renderização, física, etc.

**⚠️ Problemas:**
1. Arquivo gigante (2000+ linhas)
2. Muitos includes
3. Mistura hooks de diferentes sistemas

**✏️ Mudanças necessárias:**
1. Dividir em múltiplos arquivos:
   - `hooks/render_hooks.cpp` - Hooks de renderização
   - `hooks/entity_hooks.cpp` - Hooks de entidades
   - `hooks/physics_hooks.cpp` - Hooks de física
   - `hooks/hooks_main.cpp` - Instalação de hooks
2. Atualizar todos os includes para novos caminhos

---

### 📄 game/patches.cpp

**Mover para:** `samp/game/hooks/patches.cpp`

**Includes atuais:**
```cpp
#include "../main.h"
#include "../game/game.h"
#include "../vendor/armhook/patch.h"
#include "vehicleColoursTable.h"
#include "../settings.h"
#include "game.h"
#include "World.h"
#include "net/netgame.h"
```

**Funções principais:**
- `readVehiclesAudioSettings()` - Lê configurações de áudio de veículos
- `ApplyFPSPatch()` - Aplica patch de FPS
- `DisableAutoAim()` - Desabilita auto-aim
- `ApplySAMPPatchesInGame()` - Aplica patches SA-MP

**✏️ Mudanças:**
1. Mover para `game/hooks/`
2. Atualizar includes

---

### 📄 game/game.cpp / game.h

**Mover para:** `samp/game/engine/game.cpp`, `samp/game/engine/game.h`

**Includes de game.h:**
```cpp
#include "common.h"
#include "RW/RenderWare.h"
#include "aimstuff.h"
#include "Camera.h"
#include "playerped.h"
#include "actor.h"
#include "vehicle.h"
#include "object.h"
#include "font.h"
#include "textdraw.h"
#include "scripting.h"
#include "util.h"
#include "radarcolors.h"
#include "pad.h"
#include "snapshothelper.h"
#include "materialtextgenerator.h"
#include <queue>
#include <mutex>
#include "../game/Core/Quaternion.h"
```

**Classe CGame - Métodos principais:**
- `Initialize()`, `StartGame()` - Inicialização
- `SetWorldTime()`, `SetWorldWeather()` - Controle de mundo
- `NewPlayer()`, `NewVehicle()`, `NewObject()` - Criação de entidades
- `RequestModel()`, `LoadRequestedModels()` - Streaming
- `FindPlayerPed()` - Acesso ao jogador local
- `PostToMainThread()`, `ProcessMainThreadTasks()` - Thread-safe operations

**✏️ Mudanças:**
1. Mover para `game/engine/`
2. Atualizar includes relativos

---

### 📄 game/playerped.cpp / playerped.h

**Mover para:** `samp/game/entities/playerped.cpp`, `samp/game/entities/playerped.h`

**Includes de playerped.h:**
```cpp
#include "vehicle.h"
#include "object.h"
#include "game/Entity/CPedGTA.h"
#include "aimstuff.h"
```

**⚠️ Dependências circulares potenciais:**
- playerped.h inclui vehicle.h
- vehicle.h pode incluir playerped.h para passageiros

**✏️ Mudanças:**
1. Mover para `game/entities/`
2. Usar forward declarations para quebrar ciclos
3. Atualizar includes

---

### 📄 game/multitouch.cpp / multitouch.h

**Mover para:** `samp/game/input/multitouch.cpp`, `samp/game/input/multitouch.h`

**Funcionalidade:** Sistema de multitouch customizado

**✏️ Mudanças:**
1. Mover para `game/input/`
2. Atualizar includes

---

## 3. Pasta `net/`

### 📁 Estrutura Atual
```
net/
├── actorpool.cpp/h
├── gangzonepool.cpp/h
├── localplayer.cpp/h
├── menupool.cpp/h
├── netgame.cpp/h
├── netrpc.cpp
├── objectpool.cpp/h
├── pickuppool.cpp/h
├── playerbubblepool.cpp/h
├── playerpool.cpp/h
├── remoteplayer.cpp/h
├── scriptrpc.cpp
├── textdrawpool.cpp/h
├── textlabelpool.cpp/h
└── vehiclepool.cpp/h
```

### 📁 Nova Estrutura Proposta
```
multiplayer/
├── netgame.cpp/h
├── local_player.cpp/h       ← renomear de localplayer
├── remote_player.cpp/h      ← renomear de remoteplayer
├── pools/
│   ├── player_pool.cpp/h
│   ├── vehicle_pool.cpp/h
│   ├── object_pool.cpp/h
│   ├── actor_pool.cpp/h
│   ├── pickup_pool.cpp/h
│   ├── textdraw_pool.cpp/h
│   ├── textlabel_pool.cpp/h
│   ├── gangzone_pool.cpp/h
│   ├── menu_pool.cpp/h
│   └── playerbubble_pool.cpp/h
├── rpc/
│   ├── rpc_handlers.cpp/h   ← de netrpc.cpp
│   └── script_rpc.cpp/h     ← de scriptrpc.cpp
└── sync/
    └── sync_data.h          ← structs de sync de localplayer.h
```

---

### 📄 net/netgame.cpp

**Mover para:** `samp/multiplayer/netgame.cpp`

**Includes atuais:**
```cpp
#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../gui/gui.h"
#include "../audiostream.h"
#include "../voice_new/MicroIcon.h"
#include "../voice_new/SpeakerList.h"
#include "../voice_new/Network.h"
#include "java/jniutil.h"
```

**Globals usados:**
```cpp
extern UI* pUI;
extern CGame* pGame;
extern CAudioStream* pAudioStream;
extern CJavaWrapper* pJavaWrapper;
```

**⚠️ Problemas:**
1. Depende diretamente de GUI, Voice, Audio
2. Muitos globals

**✏️ Mudanças:**
1. Mover para `multiplayer/`
2. Usar callbacks/eventos em vez de acessar GUI diretamente
3. Atualizar todos os includes

---

### 📄 net/netrpc.cpp

**Mover para:** `samp/multiplayer/rpc/rpc_handlers.cpp`

**Includes atuais:**
```cpp
#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../gui/gui.h"
#include "../vendor/encoding/encoding.h"
#include "../settings.h"
```

**Globals usados:**
```cpp
extern UI* pUI;
extern CGame *pGame;
extern CNetGame *pNetGame;
extern CSettings *pSettings;
```

**Funções principais (RPCs):**
- `InitGame()` - Inicialização do jogo
- `ServerJoin()` - Jogador entrou
- `ServerQuit()` - Jogador saiu
- `ClientMessage()` - Mensagem do servidor
- `WorldTime()` - Tempo do mundo
- `SetSpawnInfo()` - Info de spawn
- E muitas outras...

**✏️ Mudanças:**
1. Mover para `multiplayer/rpc/`
2. Considerar dividir em múltiplos arquivos por categoria

---

### 📄 net/localplayer.h

**Mover para:** `samp/multiplayer/local_player.h`

**Conteúdo importante - Structs de sync:**
```cpp
typedef struct _ONFOOT_SYNC_DATA { ... } ONFOOT_SYNC_DATA;
typedef struct _INCAR_SYNC_DATA { ... } INCAR_SYNC_DATA;
typedef struct _PASSENGER_SYNC_DATA { ... } PASSENGER_SYNC_DATA;
typedef struct _AIM_SYNC_DATA { ... } AIM_SYNC_DATA;
typedef struct _BULLET_SYNC_DATA { ... } BULLET_SYNC_DATA;
typedef struct _TRAILER_SYNC_DATA { ... } TRAILER_SYNC_DATA;
typedef struct _UNOCCUPIED_SYNC_DATA { ... } UNOCCUPIED_SYNC_DATA;
```

**✏️ Mudanças:**
1. Extrair structs de sync para `multiplayer/sync/sync_data.h`
2. Mover classe para `multiplayer/local_player.h`

---

## 4. Pasta `gui/`

### 📁 Estrutura Atual
```
gui/
├── gui.cpp/h
├── imguirenderer.cpp/h
├── imguiwrapper.cpp/h
├── uisettings.cpp/h
├── widget.cpp/h
├── widgets/
│   ├── button.cpp/h
│   ├── editbox.cpp/h
│   ├── image.cpp/h
│   ├── label.cpp/h
│   ├── layout.cpp/h
│   ├── listbox.cpp/h
│   ├── progressbar.cpp/h
│   └── scrollpanel.cpp/h
└── samp_widgets/
    ├── buttonpanel.cpp/h
    ├── chat.cpp/h
    ├── keyboard.cpp/h
    ├── playertablist.cpp/h
    ├── spawn.cpp/h
    ├── splashscreen.cpp/h
    ├── voicebutton.h
    └── dialogs/
        ├── content.cpp/h
        ├── dialog.cpp/h
        ├── inputwidget.cpp/h
        ├── listwidget.cpp/h
        ├── msgbox.cpp/h
        └── tablistwidget.cpp/h
```

### 📁 Nova Estrutura (mantém organização, apenas move)
```
ui/
├── ui_manager.cpp/h        ← renomear de gui.cpp/h
├── imgui_renderer.cpp/h    ← de imguirenderer.cpp/h
├── imgui_wrapper.cpp/h     ← de imguiwrapper.cpp/h
├── ui_settings.cpp/h       ← de uisettings.cpp/h
├── widgets/                ← mantém
│   └── [todos os widgets base]
└── screens/                ← renomear de samp_widgets
    ├── chat.cpp/h
    ├── dialog.cpp/h
    ├── keyboard.cpp/h
    ├── spawn.cpp/h
    ├── scoreboard.cpp/h    ← de playertablist.cpp/h
    └── [outros]
```

---

### 📄 gui/gui.cpp

**Mover para:** `samp/ui/ui_manager.cpp`

**Includes atuais:**
```cpp
#include "../main.h"
#include "../game/game.h"
#include "../net/netgame.h"
#include "gui.h"
#include "../playertags.h"
#include "../net/playerbubblepool.h"
#include "vendor/str_obfuscator/str_obfuscator.hpp"
#include "../voice_new/Plugin.h"
#include "../voice_new/MicroIcon.h"
#include "../voice_new/SpeakerList.h"
#include "../voice_new/Network.h"
#include "../gui/samp_widgets/voicebutton.h"
#include "game/Textures/TextureDatabaseRuntime.h"
#include "game/Streaming.h"
#include "game/Pools.h"
```

**⚠️ Problemas:**
1. Depende de voice, game, net diretamente
2. Muitos includes

**✏️ Mudanças:**
1. Renomear para ui_manager
2. Usar eventos/callbacks para comunicação com outros módulos
3. Atualizar includes

---

## 5. Pasta `voice_new/`

### 📁 Estrutura Atual
```
voice_new/
├── Channel.cpp/h
├── ControlPacket.cpp/h
├── Effect.cpp/h
├── GlobalStream.cpp/h
├── Header.h
├── LocalStream.cpp/h
├── MicroIcon.cpp/h
├── Network.cpp/h
├── Parameter.cpp/h
├── Playback.cpp/h
├── Plugin.cpp/h
├── PluginConfig.cpp/h
├── Record.cpp/h
├── SetController.cpp/h
├── SlideController.cpp/h
├── SpeakerList.cpp/h
├── Stream.cpp/h
├── StreamAtObject.cpp/h
├── StreamAtPlayer.cpp/h
├── StreamAtPoint.cpp/h
├── StreamAtVehicle.cpp/h
├── StreamInfo.cpp/h
├── VoicePacket.cpp/h
└── include/
    ├── SPSCQueue.h
    └── util/
        ├── Memory.hpp
        ├── Render.cpp/h
        ├── Samp.cpp/h
        └── Timer.cpp/h
```

### 📁 Nova Estrutura
```
audio/
├── audio_manager.cpp/h     ← novo, agrupa audiostream + voice
├── audio_stream.cpp/h      ← de samp/audiostream.cpp
└── voice/
    ├── voice_manager.cpp/h ← de Plugin.cpp
    ├── network.cpp/h
    ├── playback.cpp/h
    ├── record.cpp/h
    ├── streams/
    │   ├── stream.cpp/h
    │   ├── local_stream.cpp/h
    │   ├── global_stream.cpp/h
    │   └── [stream_at_*.cpp/h]
    ├── effects/
    │   └── effect.cpp/h
    └── ui/
        ├── micro_icon.cpp/h
        └── speaker_list.cpp/h
```

---

## 6. Pasta `java/`

### 📁 Estrutura Atual
```
java/
├── editobject.cpp/h
└── jniutil.cpp/h
```

### 📁 Nova Estrutura
```
platform/
└── android/
    ├── jni_bridge.cpp/h    ← de jniutil.cpp/h
    ├── edit_object.cpp/h   ← de editobject.cpp/h
    ├── nv_event.cpp/h      ← de samp/nv_event.cpp
    ├── fps_fix.cpp/h       ← de samp/CFPSFix.cpp
    ├── stacktrace.h        ← de samp/StackTrace.h
    └── storage.cpp/h       ← novo, gerencia paths
```

---

## 7. Tabela Resumo de Movimentação

| Arquivo Original | Novo Local | Mudanças de Include |
|-----------------|------------|---------------------|
| `samp/main.cpp` | `samp/core/main.cpp` | Atualizar todos |
| `samp/main.h` | `samp/core/main.h` | Atualizar todos |
| `samp/settings.cpp` | `samp/core/settings.cpp` | Remover game.h |
| `samp/log.cpp` | `samp/core/logging.cpp` | Adicionar funções de main.cpp |
| `samp/audiostream.cpp` | `samp/audio/audiostream.cpp` | Atualizar |
| `samp/playertags.cpp` | `samp/multiplayer/features/playertags.cpp` | Atualizar |
| `samp/nv_event.cpp` | `samp/platform/android/nv_event.cpp` | Remover includes não usados |
| `samp/CFPSFix.cpp` | `samp/platform/android/fps_fix.cpp` | Atualizar |
| `game/hooks.cpp` | `samp/game/hooks/hooks.cpp` | Dividir e atualizar |
| `game/patches.cpp` | `samp/game/hooks/patches.cpp` | Atualizar |
| `game/game.cpp` | `samp/game/engine/game.cpp` | Atualizar |
| `game/playerped.cpp` | `samp/game/entities/playerped.cpp` | Atualizar |
| `net/netgame.cpp` | `samp/multiplayer/netgame.cpp` | Atualizar |
| `net/netrpc.cpp` | `samp/multiplayer/rpc/rpc_handlers.cpp` | Atualizar |
| `gui/gui.cpp` | `samp/ui/ui_manager.cpp` | Renomear e atualizar |
| `voice_new/Plugin.cpp` | `samp/audio/voice/voice_manager.cpp` | Atualizar |
| `java/jniutil.cpp` | `samp/platform/android/jni_bridge.cpp` | Atualizar |

---

## 8. Ordem de Migração Recomendada

### Fase 1: Core (sem quebrar nada)
1. Criar pastas da nova estrutura
2. Mover `settings.cpp/h` → `core/`
3. Mover `log.cpp/h` → `core/logging.cpp/h`
4. Criar `core/globals.h` com SAMPCore
5. Atualizar includes gradualmente

### Fase 2: Platform
6. Mover `nv_event.cpp/h` → `platform/android/`
7. Mover `CFPSFix.cpp/h` → `platform/android/fps_fix.cpp`
8. Mover `java/*` → `platform/android/`
9. Mover `StackTrace.h`, `crashlytics.h` → `platform/`

### Fase 3: Audio
10. Mover `audiostream.cpp/h` → `audio/`
11. Mover `voice_new/` → `audio/voice/`
12. Criar `audio/audio_manager.cpp/h`

### Fase 4: Game
13. Reorganizar `game/` internamente:
    - Criar subpastas hooks/, engine/, entities/, etc.
    - Mover arquivos para subpastas apropriadas

### Fase 5: Multiplayer
14. Renomear `net/` → `multiplayer/`
15. Criar subpastas pools/, rpc/, sync/
16. Mover `playertags.cpp` → `multiplayer/features/`

### Fase 6: UI
17. Renomear `gui/` → `ui/`
18. Renomear arquivos (gui.cpp → ui_manager.cpp)
19. Renomear `samp_widgets/` → `screens/`

### Fase 7: Limpeza
20. Atualizar todos os includes restantes
21. Remover globals gradualmente em favor de SAMPCore
22. Testar cada módulo

---

## 9. Arquivos que NÃO Mudam

### Pasta `vendor/` (dependências externas)
```
vendor/
├── armhook/      # Hook library
├── bass/         # Audio library
├── encoding/     # Encoding utils
├── imgui/        # ImGui library
├── inih/         # INI parser
├── quaternion/   # Quaternion math
├── raknet/       # Networking
├── SimpleIni/    # INI library
└── str_obfuscator/ # String obfuscation
```

Estes arquivos não devem ser modificados, apenas referenciados.

---

*Documento gerado para análise de migração*
*Total de arquivos analisados: ~350*
