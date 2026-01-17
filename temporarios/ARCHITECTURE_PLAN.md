# Plano de Arquitetura: Monólito Modular

## Contexto

Este plano é aplicável a projetos de **clients multiplayer para jogos** (SA-MP, MTA, FiveM-style, etc.) que modificam um jogo base através de hooks e patches.

---

## 1. Diagnóstico Atual (SA-MP Mobile)

### Estrutura Existente
```
samp/
├── main.cpp              # Entry point + globals + JNI + logging misturados
├── game/                 # ~408 arquivos - Engine GTA + hooks + patches + entidades
├── gui/                  # ~51 arquivos - Interface usuário
├── net/                  # ~28 arquivos - Rede + pools de entidades multiplayer
├── voice_new/            # ~53 arquivos - Sistema de voz
├── java/                 # JNI
├── vendor/               # Libs externas
└── [arquivos soltos]     # audiostream, playertags, settings, etc.
```

### Problemas Principais

1. **Globals expostos em todo lugar**
```cpp
extern UI* pUI;
extern CGame* pGame;
extern CNetGame* pNetGame;
// ... qualquer arquivo acessa qualquer coisa
```

2. **Pastas muito grandes sem organização interna** - `game/` tem 408 arquivos

3. **Responsabilidades misturadas** - `main.cpp` faz inicialização, logging, JNI, signal handlers

4. **Arquivos soltos na raiz** - `audiostream`, `playertags`, `settings` deveriam estar organizados

---

## 2. Nova Estrutura Proposta

### Princípio: 5-6 Módulos Coesos

```
samp/
├── core/                 # Fundação do projeto
│   ├── main.cpp/h        # Entry point limpo
│   ├── types.h           # Tipos básicos compartilhados
│   ├── logging.cpp/h     # Sistema de log unificado
│   ├── settings.cpp/h    # Configurações
│   ├── memory.h          # Macros de memória (SAFE_DELETE, etc)
│   └── timer.cpp/h       # Timers e GetTickCount
│
├── game/                 # TUDO relacionado ao jogo base
│   ├── hooks/            # Todos os hooks em um lugar
│   │   ├── hooks.cpp/h   # Instalação dos hooks
│   │   └── patches.cpp/h # Patches de memória
│   │
│   ├── engine/           # Acesso ao engine do jogo
│   │   ├── game.cpp/h    # Classe principal CGame
│   │   ├── world.cpp/h   # Mundo, tempo, clima
│   │   ├── camera.cpp/h  # Sistema de câmera
│   │   ├── streaming.cpp/h
│   │   └── pools.cpp/h   # Pools do GTA (peds, vehicles, etc)
│   │
│   ├── entities/         # Entidades do jogo
│   │   ├── ped.cpp/h     # CPedGTA, CPlayerPed
│   │   ├── vehicle.cpp/h # CVehicleGTA, CVehicle
│   │   ├── object.cpp/h  # CObject
│   │   └── actor.cpp/h   # CActor
│   │
│   ├── rendering/        # Renderização
│   │   ├── renderware.cpp/h  # Bindings RenderWare
│   │   ├── textures.cpp/h
│   │   ├── sprites.cpp/h
│   │   └── font.cpp/h
│   │
│   ├── physics/          # Física e colisão
│   │   ├── collision.cpp/h
│   │   └── collision_data.h
│   │
│   ├── animation/        # Sistema de animação
│   │   ├── anim_manager.cpp/h
│   │   └── anim_blend.cpp/h
│   │
│   └── input/            # Input do jogo
│       ├── pad.cpp/h
│       └── multitouch.cpp/h
│
├── multiplayer/          # Lógica multiplayer (SA-MP específico)
│   ├── netgame.cpp/h     # Conexão e gerenciamento
│   ├── local_player.cpp/h
│   ├── remote_player.cpp/h
│   │
│   ├── pools/            # Pools de entidades de rede
│   │   ├── player_pool.cpp/h
│   │   ├── vehicle_pool.cpp/h
│   │   ├── object_pool.cpp/h
│   │   ├── pickup_pool.cpp/h
│   │   ├── textdraw_pool.cpp/h
│   │   ├── textlabel_pool.cpp/h
│   │   ├── gangzone_pool.cpp/h
│   │   ├── actor_pool.cpp/h
│   │   └── menu_pool.cpp/h
│   │
│   ├── sync/             # Sincronização
│   │   ├── sync_data.h   # Structs de sync
│   │   ├── player_sync.cpp/h
│   │   ├── vehicle_sync.cpp/h
│   │   └── aim_sync.cpp/h
│   │
│   ├── rpc/              # Handlers de RPC
│   │   ├── rpc_handlers.cpp/h
│   │   └── script_rpc.cpp/h
│   │
│   └── features/         # Features específicas SA-MP
│       ├── player_tags.cpp/h
│       ├── material_text.cpp/h
│       └── checkpoints.cpp/h
│
├── ui/                   # Interface do usuário
│   ├── ui_manager.cpp/h  # Gerenciador principal
│   ├── imgui_renderer.cpp/h
│   │
│   ├── widgets/          # Widgets base
│   │   ├── widget.cpp/h
│   │   ├── button.cpp/h
│   │   ├── label.cpp/h
│   │   ├── editbox.cpp/h
│   │   ├── listbox.cpp/h
│   │   └── layout.cpp/h
│   │
│   └── screens/          # Telas/componentes específicos
│       ├── chat.cpp/h
│       ├── dialog.cpp/h
│       ├── keyboard.cpp/h
│       ├── spawn_screen.cpp/h
│       ├── scoreboard.cpp/h
│       └── splash.cpp/h
│
├── audio/                # Audio (streaming + voz)
│   ├── audio_manager.cpp/h   # Gerenciador principal
│   ├── audio_stream.cpp/h    # Streaming de música
│   │
│   └── voice/            # Sistema de voz
│       ├── voice_manager.cpp/h
│       ├── playback.cpp/h
│       ├── record.cpp/h
│       ├── streams.cpp/h
│       └── effects.cpp/h
│
├── platform/             # Código específico da plataforma
│   ├── android/
│   │   ├── jni_bridge.cpp/h  # Interface JNI limpa
│   │   ├── java_wrapper.cpp/h
│   │   └── storage.cpp/h
│   │
│   └── crashlytics.cpp/h
│
└── vendor/               # Dependências externas (não modificar)
    ├── raknet/
    ├── imgui/
    ├── bass/
    └── ...
```

---

## 3. Detalhamento dos Módulos

### 3.1 `core/` - Fundação

**Responsabilidade**: Código base que todo o projeto usa.

```cpp
// core/main.h - Header principal limpo
#pragma once

#include "types.h"
#include "logging.h"
#include "memory.h"

// Acesso centralizado (substituindo globals)
class SAMPCore {
public:
    static void Initialize();
    static void Shutdown();
    
    static class CGame* Game();
    static class CNetGame* Network();
    static class UI* UserInterface();
    static class AudioManager* Audio();
    static class CSettings* Settings();
    
private:
    static inline CGame* s_game = nullptr;
    static inline CNetGame* s_network = nullptr;
    static inline UI* s_ui = nullptr;
    // ...
};

// Atalhos opcionais para migração gradual
#define pGame SAMPCore::Game()
#define pNetGame SAMPCore::Network()
#define pUI SAMPCore::UserInterface()
```

**Por que centralizar aqui?**
- Um único ponto de acesso a subsistemas
- Ordem de inicialização/destruição controlada
- Migração gradual (macros de compatibilidade)

---

### 3.2 `game/` - Engine e Modificações do Jogo

**Responsabilidade**: Tudo relacionado ao jogo base - hooks, patches, acesso ao engine, entidades.

**Estrutura interna:**

```
game/
├── hooks/        # ONDE instalar hooks/patches
├── engine/       # ACESSO ao engine (ler/escrever estado do jogo)
├── entities/     # ENTIDADES (wrappers para entidades GTA)
├── rendering/    # RENDERIZAÇÃO (texturas, sprites, fonts)
├── physics/      # FÍSICA (colisão)
├── animation/    # ANIMAÇÕES
└── input/        # INPUT (controles)
```

**Exemplo de organização de hooks:**

```cpp
// game/hooks/hooks.h
namespace GameHooks {
    void InstallAll();
    void InstallRenderHooks();
    void InstallInputHooks();
    void InstallEntityHooks();
}

// game/hooks/hooks.cpp
#include "hooks.h"
#include "../engine/game.h"

void GameHooks::InstallAll() {
    InstallRenderHooks();
    InstallInputHooks();
    InstallEntityHooks();
}

// Hooks de render
void (*CSprite2d_Draw_orig)(...);
void CSprite2d_Draw_hook(...) {
    // Modificação
    CSprite2d_Draw_orig(...);
}

void GameHooks::InstallRenderHooks() {
    CHook::Install(g_libGTASA + OFFSET_SPRITE2D_DRAW, 
                   CSprite2d_Draw_hook, 
                   &CSprite2d_Draw_orig);
}
```

---

### 3.3 `multiplayer/` - Lógica Multiplayer

**Responsabilidade**: Tudo específico do multiplayer - rede, sincronização, pools de rede.

**Diferença de `game/`:**
- `game/` = acesso ao jogo original
- `multiplayer/` = lógica adicionada pelo mod

```cpp
// multiplayer/netgame.h
class CNetGame {
public:
    CNetGame(const char* host, int port, const char* name, const char* pass);
    ~CNetGame();
    
    void Process();
    void Disconnect();
    
    // Pools
    CPlayerPool* GetPlayerPool() { return m_pools.players.get(); }
    CVehiclePool* GetVehiclePool() { return m_pools.vehicles.get(); }
    // ...
    
private:
    struct {
        std::unique_ptr<CPlayerPool> players;
        std::unique_ptr<CVehiclePool> vehicles;
        std::unique_ptr<CObjectPool> objects;
        // ...
    } m_pools;
    
    std::unique_ptr<RakClientInterface> m_rakClient;
};
```

---

### 3.4 `ui/` - Interface do Usuário

**Responsabilidade**: Toda a interface visual - ImGui, widgets, telas.

```cpp
// ui/ui_manager.h
class UI {
public:
    UI(ImVec2 displaySize, const std::string& fontPath);
    
    void Render();
    bool HandleTouch(int type, int x, int y);
    
    // Acesso aos componentes
    Chat* GetChat() { return m_chat.get(); }
    Dialog* GetDialog() { return m_dialog.get(); }
    Keyboard* GetKeyboard() { return m_keyboard.get(); }
    
private:
    std::unique_ptr<Chat> m_chat;
    std::unique_ptr<Dialog> m_dialog;
    std::unique_ptr<Keyboard> m_keyboard;
    // ...
};
```

---

### 3.5 `audio/` - Áudio e Voz

**Responsabilidade**: Streaming de áudio e sistema de voz.

```cpp
// audio/audio_manager.h
class AudioManager {
public:
    void Initialize();
    void Process();
    void Shutdown();
    
    // Streaming
    void PlayStream(const std::string& url);
    void StopStream();
    
    // Voz
    VoiceManager* Voice() { return m_voice.get(); }
    
private:
    std::unique_ptr<AudioStream> m_stream;
    std::unique_ptr<VoiceManager> m_voice;
};
```

---

### 3.6 `platform/` - Código de Plataforma

**Responsabilidade**: Código específico Android/JNI, crashlytics.

```cpp
// platform/android/jni_bridge.h
class JNIBridge {
public:
    static void Initialize(JNIEnv* env, jobject activity);
    
    // Métodos para chamar Java
    static void ShowKeyboard();
    static void HideKeyboard();
    static void ShowLoadingScreen();
    static void HideLoadingScreen();
    static void ShowDialog(int type, const char* title, const char* text);
    
    static const char* GetStoragePath();
    
private:
    static JavaVM* s_jvm;
    static jobject s_activity;
};
```

---

## 4. Comunicação Entre Módulos

### 4.1 Acesso Direto (Simples)

Para a maioria dos casos, acesso direto via `SAMPCore`:

```cpp
// Em qualquer lugar
SAMPCore::Network()->SendChatMessage("Hello");
SAMPCore::UserInterface()->GetChat()->AddMessage("Test");
SAMPCore::Game()->SetWorldTime(12, 0);
```

### 4.2 Callbacks (Quando Necessário)

Para notificações entre módulos sem acoplamento forte:

```cpp
// multiplayer/netgame.h
class CNetGame {
public:
    // Callback para quando conectar
    std::function<void(const char* hostname)> OnConnect;
    
    // Callback para mensagem recebida
    std::function<void(const char* msg, uint32_t color)> OnChatMessage;
};

// Em algum lugar da inicialização
pNetGame->OnChatMessage = [](const char* msg, uint32_t color) {
    SAMPCore::UserInterface()->GetChat()->AddMessage(msg, color);
};
```

---

## 5. Migração Gradual

### Fase 1: Organizar sem Quebrar

1. **Criar pastas** da nova estrutura
2. **Mover arquivos** para suas novas casas
3. **Ajustar includes** (`#include "../old/path.h"` → `#include "new/path.h"`)
4. **Manter globals** funcionando com macros de compatibilidade

### Fase 2: Consolidar Módulos

5. **Criar `SAMPCore`** como ponto central
6. **Migrar globals** gradualmente para `SAMPCore`
7. **Agrupar hooks** dispersos em `game/hooks/`

### Fase 3: Limpar

8. **Remover macros** de compatibilidade
9. **Remover código morto**
10. **Documentar APIs** públicas

---

## 6. Padrão de Arquivos

### Headers (.h)
```cpp
#pragma once

// Includes externos primeiro
#include <string>
#include <memory>

// Includes do projeto
#include "core/types.h"

class MinhaClasse {
public:
    // API pública
    
private:
    // Implementação
};
```

### Source (.cpp)
```cpp
#include "minha_classe.h"

// Includes adicionais necessários para implementação
#include "core/logging.h"

// Implementação
```

---

## 7. Aplicação a Outros Projetos

Esta estrutura é adaptável para qualquer client multiplayer:

| Módulo | SA-MP Mobile | MTA-like | FiveM-like |
|--------|--------------|----------|------------|
| `core/` | Fundação | Fundação | Fundação |
| `game/` | GTA SA hooks | GTA SA hooks | GTA V hooks |
| `multiplayer/` | Protocolo SA-MP | Protocolo MTA | Protocolo FiveM |
| `ui/` | ImGui mobile | CEF/CEGUI | NUI/CEF |
| `audio/` | BASS + voz | OpenAL | XAudio2 |
| `platform/` | Android/JNI | Windows | Windows |

**O que muda:**
- `game/` - Hooks específicos do jogo
- `multiplayer/` - Protocolo de rede específico
- `platform/` - Sistema operacional

**O que permanece igual:**
- `core/` - Padrões de organização
- Estrutura de pastas
- Comunicação entre módulos

---

## 8. Resumo Visual

```
┌─────────────────────────────────────────────────────┐
│                      core/                          │
│         (tipos, logging, settings, SAMPCore)        │
└─────────────────────────────────────────────────────┘
                         │
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│   game/     │  │ multiplayer/│  │  platform/  │
│             │  │             │  │             │
│ hooks/      │  │ netgame     │  │ android/    │
│ engine/     │  │ pools/      │  │ jni_bridge  │
│ entities/   │  │ sync/       │  │             │
│ rendering/  │  │ rpc/        │  └─────────────┘
│ physics/    │  │             │
│ animation/  │  └──────┬──────┘
│ input/      │         │
└──────┬──────┘         │
       │                │
       └────────┬───────┘
                ▼
        ┌─────────────┐
        │    ui/      │
        │ widgets/    │
        │ screens/    │
        └─────────────┘
                │
                ▼
        ┌─────────────┐
        │   audio/    │
        │ streaming   │
        │ voice/      │
        └─────────────┘
```

---

## 9. Checklist de Migração

**Status Atual:** ~30% completo (Última atualização: 2024-12-19)

### Fase 1: Infraestrutura Core
- [ ] Criar `core/services/ServiceLocator.h/cpp` (Service Locator)
- [ ] Criar `core/events/EventBus.h/cpp` (Sistema de eventos)
- [ ] Criar `core/interfaces/` (IGameService, INetworkService, IChatOutput, IAudioService)
- [ ] Criar `core/bootstrap/Bootstrap.h/cpp` (Inicialização controlada)
- [ ] Mover `main.cpp` conteúdo para `core/bootstrap/`
- [ ] Criar `SAMPCore` como singleton central

### Fase 2: Reorganização Game Module
- [x] Agrupar hooks em `game/hooks/` ✅
- [x] Mover entidades para `game/entities/` ✅ (actor, object, playerped, vehicle)
- [x] Criar `game/engine/` ✅ (game.cpp movido)
- [x] Criar `game/input/` ✅ (pad.cpp movido)
- [ ] Mover `RW/` para `game/rendering/RW/` ⚠️ (ainda em `game/RW/`)
- [ ] Mover `Collision/` para `game/physics/Collision/` ⚠️ (ainda em `game/Collision/`)
- [ ] Organizar arquivos restantes de `game/` em subdiretórios

### Fase 3: Reorganização Multiplayer Module
- [ ] Mover `net/netgame.*` para `multiplayer/connection/`
- [ ] Mover `net/netrpc.*`, `net/scriptrpc.*` para `multiplayer/rpc/`
- [ ] Mover pools de rede para `multiplayer/pools/`
- [ ] Mover `net/localplayer.*`, `net/remoteplayer.*` para `multiplayer/sync/`
- [ ] Remover diretório `net/` antigo

### Fase 4: Reorganização UI Module
- [ ] Mover `gui/gui.*` para `ui/core/ui_manager.*`
- [ ] Mover `gui/samp_widgets/` para `ui/screens/`
- [ ] Mover `gui/imguiwrapper.*` para `ui/core/imgui_wrapper.*`
- [ ] Organizar `ui/` com widgets e screens
- [ ] Remover diretório `gui/` antigo

### Fase 5: Reorganização Audio Module
- [ ] Mover `voice_new/*` para `audio/voice/`
- [ ] Mover `audiostream.*` para `audio/audio_stream.*`
- [ ] Consolidar audio e voice em `audio/`

### Fase 6: Reorganização Platform Module
- [x] Criar estrutura `platform/android/`, `platform/debug/`, `platform/lifecycle/` ✅
- [ ] Mover JNI para `platform/android/jni_bridge.*`
- [ ] Mover `crashlytics.*` para `platform/debug/`

### Fase 7: Refatoração e Limpeza
- [ ] Remover globals gradualmente (substituir por Service Locator)
- [ ] Migrar chamadas diretas para eventos (Event Bus)
- [ ] Implementar interfaces em classes existentes
- [ ] Atualizar includes em todos os arquivos ⚠️ (parcial - muitos erros ainda)
- [ ] Remover macros de compatibilidade (após migração completa)
- [ ] Modularizar CMakeLists.txt
- [ ] Testar cada fase

---

*Versão: 2.0 - Simplificada*
