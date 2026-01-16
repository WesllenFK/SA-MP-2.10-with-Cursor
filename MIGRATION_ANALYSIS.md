# Análise de Dependências Circulares e Padrões de Arquitetura

Este documento analisa as dependências circulares do projeto, propõe soluções com boas práticas, e define claramente a responsabilidade de cada sistema.

---

## 1. Mapa de Dependências Atual (Problemático)

### 1.1 Diagrama de Dependências Circulares

```
                    ┌──────────────────────────────────────────────────────┐
                    │                                                      │
                    ▼                                                      │
              ┌─────────┐                                                  │
         ┌───►│  main   │◄───────────────────────────────────┐             │
         │    └────┬────┘                                    │             │
         │         │ define globals                          │             │
         │         ▼                                         │             │
         │    ┌─────────┐      ┌─────────┐      ┌─────────┐  │             │
         │    │  game   │◄────►│   net   │◄────►│   gui   │──┘             │
         │    └────┬────┘      └────┬────┘      └────┬────┘                │
         │         │                │                │                     │
         │         │                ▼                │                     │
         │         │          ┌─────────┐            │                     │
         │         └─────────►│  voice  │◄───────────┘                     │
         │                    └────┬────┘                                  │
         │                         │                                       │
         │                         ▼                                       │
         │                    ┌─────────┐                                  │
         └────────────────────│  audio  │──────────────────────────────────┘
                              └─────────┘
```

### 1.2 Contagem de Dependências por Módulo

| Módulo | Depende de | É dependência de | Globals que usa |
|--------|-----------|------------------|-----------------|
| **main** | game, net, gui, voice, audio, java | TODOS | Define todos |
| **game** | main, net | net, gui, voice, audio | pGame, pNetGame, pUI |
| **net** | main, game, gui, voice, audio | game, gui, voice | pGame, pNetGame, pUI, pAudioStream |
| **gui** | main, game, net, voice | net, voice | pGame, pNetGame, pUI, pSettings |
| **voice** | main, game, net, gui, audio | gui | pGame, pNetGame, pUI, pAudioStream |
| **audio** | main, game | net, voice | pGame |
| **java** | main, game, net, gui | net, gui | pGame, pNetGame, pUI, pJavaWrapper |

### 1.3 Globals Usados (134 ocorrências!)

```cpp
// Definidos em main.cpp, usados em todo lugar:
extern CGame* pGame;              // 45 arquivos
extern CNetGame* pNetGame;        // 42 arquivos  
extern UI* pUI;                   // 25 arquivos
extern CJavaWrapper* pJavaWrapper; // 8 arquivos
extern CSettings* pSettings;       // 7 arquivos
extern CAudioStream* pAudioStream; // 5 arquivos
extern CPlayerTags* pPlayerTags;   // 2 arquivos
extern MaterialTextGenerator* pMaterialTextGenerator; // 2 arquivos
extern CSnapShotHelper* pSnapShotHelper; // 1 arquivo
```

---

## 2. Responsabilidades de Cada Sistema

### 2.1 Definição Clara de Responsabilidades

| Sistema | Responsabilidade ÚNICA | O que NÃO deve fazer |
|---------|------------------------|----------------------|
| **core** | Tipos básicos, logging, configurações, ponto de acesso centralizado | Lógica de negócio, renderização, rede |
| **game** | Acesso ao engine GTA, hooks, patches, entidades do jogo | Lógica multiplayer, UI, networking |
| **multiplayer** | Protocolo SA-MP, sincronização, pools de rede, RPCs | Renderização, acesso direto ao GTA, UI |
| **ui** | Renderização de interface, widgets, entrada de texto | Lógica de jogo, networking direto |
| **audio** | Streaming de música, sistema de voz | UI, lógica de jogo |
| **platform** | JNI, código Android-específico | Lógica de jogo, multiplayer |

### 2.2 Princípio de Responsabilidade Única (SRP)

```
ERRADO (atual):
┌─────────────────────────────────────────────────────────────┐
│ netrpc.cpp                                                  │
│ - Recebe RPC do servidor                                    │
│ - Decodifica dados                                          │
│ - Cria entidades no jogo (pGame->NewPlayer)                 │
│ - Atualiza UI diretamente (pUI->chat()->addMessage)         │
│ - Toca sons                                                 │
│ - Modifica estado do mundo                                  │
└─────────────────────────────────────────────────────────────┘

CORRETO (proposto):
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   RPC Handler    │────►│  Event/Command   │────►│    Listeners     │
│                  │     │                  │     │                  │
│ - Recebe RPC     │     │ PlayerJoined     │     │ GameListener     │
│ - Decodifica     │     │ ChatMessage      │     │ UIListener       │
│ - Emite evento   │     │ VehicleSpawned   │     │ AudioListener    │
└──────────────────┘     └──────────────────┘     └──────────────────┘
```

---

## 3. Padrões para Quebrar Dependências Circulares

### 3.1 Padrão 1: Inversão de Dependência (DIP)

**Problema:** `net` depende de `gui` para mostrar mensagens

```cpp
// ATUAL (errado) - net/netrpc.cpp
#include "../gui/gui.h"
extern UI* pUI;

void ClientMessage(RPCParameters* rpcParams) {
    // ...
    pUI->chat()->addClientMessage(szMessage, dwColor);  // Dependência direta!
}
```

**Solução:** Interface abstrata

```cpp
// core/interfaces/i_chat_output.h
class IChatOutput {
public:
    virtual ~IChatOutput() = default;
    virtual void addClientMessage(const char* message, uint32_t color) = 0;
    virtual void addInfoMessage(const char* message) = 0;
};

// multiplayer/netgame.h
class CNetGame {
public:
    void SetChatOutput(IChatOutput* output) { m_chatOutput = output; }
    IChatOutput* GetChatOutput() { return m_chatOutput; }
private:
    IChatOutput* m_chatOutput = nullptr;
};

// multiplayer/rpc/rpc_handlers.cpp
void ClientMessage(RPCParameters* rpcParams) {
    // ...
    if (auto chat = pNetGame->GetChatOutput()) {
        chat->addClientMessage(szMessage, dwColor);  // Via interface!
    }
}

// ui/screens/chat.cpp - implementa a interface
class Chat : public ListBox, public IChatOutput {
public:
    void addClientMessage(const char* message, uint32_t color) override {
        // implementação
    }
};

// Inicialização (em main.cpp ou bootstrap)
pNetGame->SetChatOutput(pUI->chat());
```

### 3.2 Padrão 2: Event Bus (Pub/Sub)

**Problema:** Múltiplos sistemas precisam reagir a eventos

```cpp
// ATUAL (errado) - código espalhado
// Em net/netrpc.cpp:
pUI->chat()->addMessage(...);
pGame->SetWorldTime(...);
pAudioStream->Play(...);
```

**Solução:** Sistema de eventos centralizado

```cpp
// core/events/event_bus.h
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>

class EventBus {
public:
    template<typename EventType>
    using Handler = std::function<void(const EventType&)>;
    
    template<typename EventType>
    static void Subscribe(Handler<EventType> handler) {
        auto& handlers = GetHandlers<EventType>();
        handlers.push_back(handler);
    }
    
    template<typename EventType>
    static void Publish(const EventType& event) {
        auto& handlers = GetHandlers<EventType>();
        for (auto& handler : handlers) {
            handler(event);
        }
    }
    
private:
    template<typename EventType>
    static std::vector<Handler<EventType>>& GetHandlers() {
        static std::vector<Handler<EventType>> handlers;
        return handlers;
    }
};

// multiplayer/events/network_events.h
struct ChatMessageEvent {
    std::string message;
    uint32_t color;
};

struct PlayerJoinedEvent {
    uint16_t playerId;
    std::string playerName;
};

struct WorldTimeChangedEvent {
    int hour;
    int minute;
};

// multiplayer/rpc/rpc_handlers.cpp
void ClientMessage(RPCParameters* rpcParams) {
    // ... decodifica ...
    EventBus::Publish(ChatMessageEvent{szMessage, dwColor});
}

// ui/screens/chat.cpp - subscreve ao evento
void Chat::initialize() {
    EventBus::Subscribe<ChatMessageEvent>([this](const auto& e) {
        this->addClientMessage(e.message, e.color);
    });
}

// game/engine/game.cpp - subscreve ao evento
void CGame::Initialize() {
    EventBus::Subscribe<WorldTimeChangedEvent>([this](const auto& e) {
        this->SetWorldTime(e.hour, e.minute);
    });
}
```

### 3.3 Padrão 3: Service Locator

**Problema:** Globals espalhados por todo código

```cpp
// ATUAL (errado)
extern CGame* pGame;
extern CNetGame* pNetGame;
extern UI* pUI;
// ... usados em 100+ arquivos
```

**Solução:** Ponto de acesso centralizado

```cpp
// core/services.h
class Services {
public:
    // Inicialização
    static void Initialize();
    static void Shutdown();
    
    // Registro
    static void SetGame(CGame* game) { s_game = game; }
    static void SetNetwork(CNetGame* network) { s_network = network; }
    static void SetUI(UI* ui) { s_ui = ui; }
    static void SetAudio(CAudioStream* audio) { s_audio = audio; }
    static void SetSettings(CSettings* settings) { s_settings = settings; }
    static void SetJavaWrapper(CJavaWrapper* java) { s_java = java; }
    
    // Acesso
    static CGame* Game() { return s_game; }
    static CNetGame* Network() { return s_network; }
    static UI* UserInterface() { return s_ui; }
    static CAudioStream* Audio() { return s_audio; }
    static CSettings* Settings() { return s_settings; }
    static CJavaWrapper* Java() { return s_java; }
    
private:
    static inline CGame* s_game = nullptr;
    static inline CNetGame* s_network = nullptr;
    static inline UI* s_ui = nullptr;
    static inline CAudioStream* s_audio = nullptr;
    static inline CSettings* s_settings = nullptr;
    static inline CJavaWrapper* s_java = nullptr;
};

// Macros de compatibilidade para migração gradual
#define pGame Services::Game()
#define pNetGame Services::Network()
#define pUI Services::UserInterface()
// etc.
```

### 3.4 Padrão 4: Callbacks para Comunicação

**Problema:** `audio` precisa saber se o jogo está pausado

```cpp
// ATUAL (errado) - audio/audiostream.cpp
#include "game/game.h"
extern CGame* pGame;

void CAudioStream::Process() {
    if (pGame->IsGamePaused()) {  // Dependência direta de game!
        BASS_SetConfig(5, 0);
    }
}
```

**Solução:** Callback injetado

```cpp
// audio/audiostream.h
class CAudioStream {
public:
    using PauseCheckCallback = std::function<bool()>;
    
    void SetPauseCallback(PauseCheckCallback callback) {
        m_pauseCheck = callback;
    }
    
    void Process() {
        if (m_pauseCheck && m_pauseCheck()) {
            BASS_SetConfig(5, 0);
        }
    }
    
private:
    PauseCheckCallback m_pauseCheck;
};

// Inicialização (bootstrap)
pAudioStream->SetPauseCallback([]() {
    return Services::Game()->IsGamePaused();
});
```

### 3.5 Padrão 5: Forward Declarations

**Problema:** Headers incluem outros headers desnecessariamente

```cpp
// ATUAL (errado) - game/playerped.h
#include "vehicle.h"    // Include completo
#include "object.h"     // Include completo
```

**Solução:** Forward declarations

```cpp
// game/entities/playerped.h
#pragma once

// Forward declarations - não precisa do header completo
class CVehicle;
class CObject;
class CVehicleGTA;

class CPlayerPed {
public:
    CVehicle* GetCurrentVehicle();  // Só declara, não usa internamente
    // ...
private:
    CVehicleGTA* m_pVehicle;  // Ponteiro não precisa de definição completa
};

// game/entities/playerped.cpp
#include "playerped.h"
#include "vehicle.h"    // Aqui sim, precisa da definição completa
#include "object.h"

CVehicle* CPlayerPed::GetCurrentVehicle() {
    // implementação que usa CVehicle
}
```

---

## 4. Nova Arquitetura Proposta

### 4.1 Camadas de Dependência (Unidirecionais)

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              APPLICATION                                 │
│                     (main.cpp, bootstrap, game loop)                     │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                              FEATURES                                    │
│              (multiplayer, ui, audio - lógica de alto nível)             │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                               ENGINE                                     │
│                    (game - acesso ao engine do jogo)                     │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                              PLATFORM                                    │
│                        (android, jni, hooks)                             │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                                CORE                                      │
│                (types, logging, events, interfaces, services)            │
└─────────────────────────────────────────────────────────────────────────┘
```

**Regra:** Cada camada só pode depender de camadas ABAIXO dela.

### 4.2 Estrutura de Diretórios Final

```
samp/
├── core/                           # CAMADA MAIS BAIXA
│   ├── types.h                     # Tipos básicos (CVector, etc)
│   ├── logging.cpp/h               # Sistema de log unificado
│   ├── services.cpp/h              # Service Locator
│   ├── events/
│   │   ├── event_bus.h             # Sistema de eventos
│   │   └── events.h                # Definições de eventos
│   └── interfaces/
│       ├── i_chat_output.h         # Interface para chat
│       ├── i_game_service.h        # Interface para game
│       └── i_network_service.h     # Interface para network
│
├── platform/                       # CAMADA DE PLATAFORMA
│   ├── android/
│   │   ├── jni_bridge.cpp/h
│   │   ├── nv_event.cpp/h
│   │   └── storage.cpp/h
│   ├── hooks/
│   │   ├── hook_manager.cpp/h      # Gerenciador de hooks
│   │   └── patch.cpp/h             # Patches de memória
│   └── crashlytics.h
│
├── game/                           # CAMADA ENGINE
│   ├── engine/
│   │   ├── game.cpp/h              # CGame - acesso ao engine
│   │   ├── world.cpp/h             # Mundo, tempo, clima
│   │   ├── camera.cpp/h
│   │   ├── streaming.cpp/h
│   │   └── pools.cpp/h             # Pools do GTA
│   ├── entities/
│   │   ├── playerped.cpp/h
│   │   ├── vehicle.cpp/h
│   │   ├── object.cpp/h
│   │   └── actor.cpp/h
│   ├── hooks/                      # Hooks específicos do game
│   │   ├── render_hooks.cpp/h
│   │   ├── entity_hooks.cpp/h
│   │   └── game_hooks.cpp/h
│   ├── rendering/
│   │   ├── RW/                     # RenderWare
│   │   ├── font.cpp/h
│   │   └── sprite2d.cpp/h
│   ├── physics/
│   │   └── Collision/
│   ├── animation/
│   │   └── Animation/
│   └── input/
│       ├── pad.cpp/h
│       └── multitouch.cpp/h
│
├── multiplayer/                    # CAMADA FEATURES
│   ├── network/
│   │   ├── netgame.cpp/h           # Gerenciador de rede
│   │   ├── connection.cpp/h        # Conexão RakNet
│   │   └── packet_handler.cpp/h
│   ├── pools/
│   │   ├── player_pool.cpp/h
│   │   ├── vehicle_pool.cpp/h
│   │   └── [outros pools]
│   ├── sync/
│   │   ├── sync_data.h             # Structs de sincronização
│   │   ├── local_player.cpp/h
│   │   └── remote_player.cpp/h
│   ├── rpc/
│   │   ├── rpc_registry.cpp/h      # Registro de RPCs
│   │   ├── game_rpc.cpp/h          # RPCs de jogo
│   │   ├── player_rpc.cpp/h        # RPCs de jogador
│   │   └── world_rpc.cpp/h         # RPCs de mundo
│   ├── events/
│   │   └── network_events.h        # Eventos de rede
│   └── features/
│       ├── playertags.cpp/h
│       ├── checkpoints.cpp/h
│       └── material_text.cpp/h
│
├── ui/                             # CAMADA FEATURES
│   ├── ui_manager.cpp/h            # Gerenciador de UI
│   ├── imgui_wrapper.cpp/h
│   ├── imgui_renderer.cpp/h
│   ├── ui_settings.cpp/h
│   ├── widgets/
│   │   ├── widget.cpp/h
│   │   ├── button.cpp/h
│   │   ├── label.cpp/h
│   │   └── [outros widgets]
│   ├── screens/
│   │   ├── chat.cpp/h              # Implementa IChatOutput
│   │   ├── dialog.cpp/h
│   │   ├── keyboard.cpp/h
│   │   ├── spawn.cpp/h
│   │   └── scoreboard.cpp/h
│   └── events/
│       └── ui_events.h             # Eventos de UI
│
├── audio/                          # CAMADA FEATURES
│   ├── audio_manager.cpp/h         # Gerenciador de áudio
│   ├── audio_stream.cpp/h
│   └── voice/
│       ├── voice_manager.cpp/h
│       ├── playback.cpp/h
│       ├── record.cpp/h
│       └── streams/
│
├── config/                         # CAMADA FEATURES
│   └── settings.cpp/h
│
└── vendor/                         # DEPENDÊNCIAS EXTERNAS
    └── [não modificar]
```

---

## 5. Tabela de Dependências Permitidas

### 5.1 Matriz de Dependências

| Módulo | core | platform | game | multiplayer | ui | audio | config |
|--------|------|----------|------|-------------|-----|-------|--------|
| **core** | - | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **platform** | ✅ | - | ❌ | ❌ | ❌ | ❌ | ❌ |
| **game** | ✅ | ✅ | - | ❌ | ❌ | ❌ | ❌ |
| **multiplayer** | ✅ | ❌ | ✅ | - | ❌ | ❌ | ✅ |
| **ui** | ✅ | ❌ | ❌ | ❌ | - | ❌ | ✅ |
| **audio** | ✅ | ✅ | ❌ | ❌ | ❌ | - | ✅ |
| **config** | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | - |
| **app** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

✅ = Pode depender
❌ = NÃO pode depender (usar interfaces/eventos)

### 5.2 Como Módulos se Comunicam

| De → Para | Mecanismo |
|-----------|-----------|
| multiplayer → ui | `EventBus::Publish<ChatMessageEvent>` ou `IChatOutput` |
| multiplayer → game | Acesso direto (game é camada inferior) |
| multiplayer → audio | `EventBus::Publish<PlayAudioEvent>` |
| ui → multiplayer | `EventBus::Publish<SendChatEvent>` |
| ui → game | Via `Services::Game()` (para dados de leitura) |
| audio → game | Callback injetado (`SetPauseCallback`) |
| game → multiplayer | ❌ NÃO PERMITIDO (inversão via eventos) |

---

## 6. Eventos do Sistema

### 6.1 Lista de Eventos

```cpp
// core/events/events.h

// ===== EVENTOS DE REDE =====
struct PlayerConnectedEvent {
    uint16_t playerId;
    std::string playerName;
    bool isNPC;
};

struct PlayerDisconnectedEvent {
    uint16_t playerId;
    uint8_t reason;
};

struct ChatMessageReceivedEvent {
    std::string message;
    uint32_t color;
    uint16_t fromPlayerId;  // INVALID_PLAYER_ID se for do servidor
};

struct VehicleSpawnedEvent {
    uint16_t vehicleId;
    int modelId;
    CVector position;
};

struct WorldTimeChangedEvent {
    int hour;
    int minute;
};

struct WeatherChangedEvent {
    int weatherId;
};

// ===== EVENTOS DE UI =====
struct SendChatMessageEvent {
    std::string message;
};

struct SendCommandEvent {
    std::string command;
};

struct DialogResponseEvent {
    uint16_t dialogId;
    uint8_t buttonId;
    uint16_t listItem;
    std::string inputText;
};

struct KeyboardInputEvent {
    std::string input;
};

// ===== EVENTOS DE JOGO =====
struct PlayerDeathEvent {
    uint16_t killerId;
    uint8_t reason;
};

struct PlayerSpawnedEvent {
    CVector position;
    float rotation;
    int skin;
};

struct VehicleEnterEvent {
    uint16_t vehicleId;
    bool asPassenger;
};

struct VehicleExitEvent {
    uint16_t vehicleId;
};

// ===== EVENTOS DE ÁUDIO =====
struct PlayAudioStreamEvent {
    std::string url;
    CVector position;
    float radius;
    bool usePosition;
};

struct StopAudioStreamEvent {};
```

### 6.2 Exemplo de Fluxo de Evento

```
┌────────────────┐     ┌────────────────┐     ┌────────────────┐
│   Servidor     │     │  RPC Handler   │     │   Event Bus    │
│                │────►│                │────►│                │
│ ClientMessage  │     │ Decodifica     │     │ Publish        │
│ RPC            │     │ Cria evento    │     │ ChatMessage    │
└────────────────┘     └────────────────┘     └───────┬────────┘
                                                      │
                       ┌──────────────────────────────┼──────────────────────────────┐
                       │                              │                              │
                       ▼                              ▼                              ▼
              ┌────────────────┐             ┌────────────────┐             ┌────────────────┐
              │    Chat UI     │             │   Log System   │             │  Voice (mute)  │
              │                │             │                │             │                │
              │ addMessage()   │             │ log()          │             │ checkMention() │
              └────────────────┘             └────────────────┘             └────────────────┘
```

---

## 7. Interfaces do Sistema

### 7.1 Interfaces Principais

```cpp
// core/interfaces/i_game_service.h
class IGameService {
public:
    virtual ~IGameService() = default;
    
    // Mundo
    virtual void SetWorldTime(int hour, int minute) = 0;
    virtual void GetWorldTime(int* hour, int* minute) = 0;
    virtual void SetWeather(int weatherId) = 0;
    virtual void SetGravity(float gravity) = 0;
    
    // Jogador local
    virtual CPlayerPed* GetLocalPlayer() = 0;
    virtual bool IsGamePaused() = 0;
    virtual bool IsGameLoaded() = 0;
    
    // Entidades
    virtual CPlayerPed* CreatePlayer(int skin, CVector pos, float rotation) = 0;
    virtual CVehicle* CreateVehicle(int model, CVector pos, float rotation) = 0;
    virtual CObject* CreateObject(int model, CVector pos, CVector rot) = 0;
    
    // Streaming
    virtual void RequestModel(int modelId) = 0;
    virtual void LoadRequestedModels() = 0;
    virtual bool IsModelLoaded(int modelId) = 0;
};

// core/interfaces/i_network_service.h
class INetworkService {
public:
    virtual ~INetworkService() = default;
    
    virtual bool IsConnected() = 0;
    virtual void Disconnect() = 0;
    
    virtual void SendChatMessage(const char* message) = 0;
    virtual void SendCommand(const char* command) = 0;
    
    virtual CPlayerPool* GetPlayerPool() = 0;
    virtual CVehiclePool* GetVehiclePool() = 0;
};

// core/interfaces/i_chat_output.h
class IChatOutput {
public:
    virtual ~IChatOutput() = default;
    
    virtual void AddClientMessage(const std::string& message, uint32_t color) = 0;
    virtual void AddInfoMessage(const std::string& message) = 0;
    virtual void AddChatMessage(const std::string& nick, const std::string& message, uint32_t nickColor) = 0;
};

// core/interfaces/i_audio_service.h
class IAudioService {
public:
    virtual ~IAudioService() = default;
    
    virtual void PlayStream(const std::string& url, CVector pos, float radius) = 0;
    virtual void StopStream() = 0;
    virtual void SetVolume(float volume) = 0;
};
```

### 7.2 Implementação das Interfaces

```cpp
// game/engine/game.cpp
class CGame : public IGameService {
public:
    void SetWorldTime(int hour, int minute) override {
        // implementação
    }
    // ... outras implementações
};

// ui/screens/chat.cpp
class Chat : public ListBox, public IChatOutput {
public:
    void AddClientMessage(const std::string& message, uint32_t color) override {
        // implementação
    }
    // ... outras implementações
};

// Registro no Services
Services::SetGame(pGame);  // pGame implementa IGameService
Services::SetChatOutput(pUI->chat());  // chat implementa IChatOutput
```

---

## 8. Migração: Antes e Depois

### 8.1 Exemplo: netrpc.cpp

**ANTES (problemático):**
```cpp
// net/netrpc.cpp
#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../gui/gui.h"
#include "../vendor/encoding/encoding.h"
#include "../settings.h"

extern UI* pUI;
extern CGame *pGame;
extern CNetGame *pNetGame;
extern CSettings *pSettings;

void ClientMessage(RPCParameters* rpcParams) {
    // ...decodifica...
    
    pUI->chat()->addClientMessage(szMessage, dwColor);  // Dependência direta!
    
    if (pSettings->Get().bSoundEnabled) {
        // toca som
    }
}

void SetWorldTime(RPCParameters* rpcParams) {
    // ...decodifica...
    
    pGame->SetWorldTime(hour, minute);  // OK - game é camada inferior
    pGame->ToggleThePassingOfTime(false);
}
```

**DEPOIS (correto):**
```cpp
// multiplayer/rpc/world_rpc.cpp
#include "core/services.h"
#include "core/events/event_bus.h"
#include "multiplayer/events/network_events.h"

void ClientMessage(RPCParameters* rpcParams) {
    // ...decodifica...
    
    // Emite evento - quem quiser escuta
    EventBus::Publish(ChatMessageReceivedEvent{
        .message = szMessage,
        .color = dwColor,
        .fromPlayerId = INVALID_PLAYER_ID
    });
}

void SetWorldTime(RPCParameters* rpcParams) {
    // ...decodifica...
    
    // Acesso direto OK - game é camada inferior
    Services::Game()->SetWorldTime(hour, minute);
    Services::Game()->ToggleThePassingOfTime(false);
    
    // Também emite evento para quem precisar saber
    EventBus::Publish(WorldTimeChangedEvent{hour, minute});
}
```

### 8.2 Exemplo: gui.cpp

**ANTES:**
```cpp
// gui/gui.cpp
#include "../main.h"
#include "../game/game.h"
#include "../net/netgame.h"
#include "../voice_new/Plugin.h"
// ... muitos includes

extern CNetGame* pNetGame;
extern CGame* pGame;

void UI::render() {
    if (pNetGame && pNetGame->GetTextDrawPool()) {
        pNetGame->GetTextDrawPool()->Draw();  // Dependência de net!
    }
    
    if (pPlayerTags) {
        pPlayerTags->Render(m_renderer);  // Dependência de playertags
    }
}
```

**DEPOIS:**
```cpp
// ui/ui_manager.cpp
#include "core/services.h"
#include "core/events/event_bus.h"
#include "ui/events/ui_events.h"

void UI::Initialize() {
    // Subscreve a eventos
    EventBus::Subscribe<RenderTextDrawsEvent>([this](const auto& e) {
        // Textdraws são passados no evento, não busca de outro módulo
        for (const auto& td : e.textdraws) {
            RenderTextDraw(td);
        }
    });
}

void UI::Render() {
    // Renderiza apenas UI
    DrawWidgets();
    
    // Outros módulos são notificados para renderizar via evento
    EventBus::Publish(UIRenderEvent{m_renderer});
}
```

---

## 9. Checklist de Migração por Arquivo

### 9.1 Para CADA arquivo .cpp:

- [ ] Remover includes de módulos de nível superior
- [ ] Substituir globals por `Services::*()` 
- [ ] Substituir chamadas diretas por eventos quando apropriado
- [ ] Usar interfaces em vez de classes concretas
- [ ] Adicionar forward declarations onde possível

### 9.2 Para CADA arquivo .h:

- [ ] Minimizar includes (usar forward declarations)
- [ ] Não incluir headers de outros módulos
- [ ] Declarar interfaces em vez de dependências concretas

### 9.3 Arquivos que Precisam de Mais Atenção

| Arquivo | Globals Usados | Dependências Cruzadas | Prioridade |
|---------|----------------|----------------------|------------|
| `hooks.cpp` | 5 | game, net, gui, java | 🔴 Alta |
| `netrpc.cpp` | 4 | game, gui, settings | 🔴 Alta |
| `netgame.cpp` | 4 | game, gui, audio, voice | 🔴 Alta |
| `gui.cpp` | 4 | game, net, voice, playertags | 🔴 Alta |
| `SpeakerList.cpp` | 4 | game, gui, net, audio | 🔴 Alta |
| `localplayer.cpp` | 4 | game, gui, voice, java | 🟡 Média |
| `scriptrpc.cpp` | 4 | game, audio | 🟡 Média |
| `chat.cpp` | 5 | game, net, java | 🟡 Média |
| `playertags.cpp` | 2 | game, net | 🟢 Baixa |
| `audiostream.cpp` | 1 | game | 🟢 Baixa |

---

## 10. Ordem de Implementação

### Fase 1: Infraestrutura (sem quebrar código)
1. Criar `core/services.h` com Service Locator
2. Criar `core/events/event_bus.h`
3. Criar interfaces em `core/interfaces/`
4. Adicionar macros de compatibilidade (`#define pGame Services::Game()`)

### Fase 2: Registrar Services
5. Em `main.cpp`, registrar todos os serviços no `Services`
6. Testar que macros de compatibilidade funcionam

### Fase 3: Migrar Eventos (um por vez)
7. Criar `ChatMessageReceivedEvent`, migrar de netrpc.cpp
8. Criar `WorldTimeChangedEvent`, migrar
9. Continuar com outros eventos

### Fase 4: Implementar Interfaces
10. `CGame` implementa `IGameService`
11. `Chat` implementa `IChatOutput`
12. Substituir chamadas diretas por interfaces

### Fase 5: Remover Compatibilidade
13. Remover macros `#define pGame`
14. Atualizar todos os arquivos para usar `Services::`
15. Remover globals de `main.cpp`

### Fase 6: Reorganizar Arquivos
16. Mover arquivos para nova estrutura de pastas
17. Atualizar todos os includes
18. Atualizar CMakeLists.txt

---

*Documento de Análise de Dependências v2.0*
*Total de dependências circulares identificadas: 47*
*Total de globals a eliminar: 134 ocorrências*
