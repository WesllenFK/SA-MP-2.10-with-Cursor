# Arquitetura Interna dos Subsistemas

Este documento detalha a arquitetura modular interna de cada subsistema proposto, incluindo subcomponentes, interfaces públicas, dependências internas e padrões de design aplicados.

---

## Índice

1. [Módulo CORE](#1-módulo-core)
2. [Módulo GAME](#2-módulo-game)
3. [Módulo MULTIPLAYER](#3-módulo-multiplayer)
4. [Módulo UI](#4-módulo-ui)
5. [Módulo AUDIO](#5-módulo-audio)
6. [Módulo PLATFORM](#6-módulo-platform)
7. [Diagrama de Relacionamento Entre Subsistemas](#7-diagrama-de-relacionamento)
8. [Padrões de Comunicação Interna](#8-padrões-de-comunicação-interna)

---

## 1. Módulo CORE

### 1.1 Propósito
Fundação do projeto. Fornece infraestrutura básica que todos os outros módulos dependem. **NÃO contém lógica de negócio.**

### 1.2 Estrutura de Diretórios

```
core/
├── bootstrap/               # Ciclo de vida da aplicação
│   ├── Bootstrap.h/cpp      # Orquestrador de inicialização/shutdown
│   └── ServiceLocator.h     # Registro centralizado de serviços
│
├── events/                  # Sistema de eventos (Event Bus)
│   ├── EventBus.h           # Pub/Sub central
│   ├── Event.h              # Classe base de eventos
│   └── EventTypes.h         # Forward declarations de todos os eventos
│
├── interfaces/              # Contratos/Interfaces abstratas
│   ├── IGameService.h       # Interface para acesso ao jogo
│   ├── INetworkService.h    # Interface para rede
│   ├── IAudioService.h      # Interface para áudio
│   ├── IUIService.h         # Interface para UI
│   └── ILogger.h            # Interface para logging
│
├── memory/                  # Gerenciamento de memória
│   ├── Memory.h             # Macros SAFE_DELETE, helpers
│   ├── SmartPointers.h      # Typedefs para smart pointers
│   └── MemoryPool.h         # Pool de objetos (opcional)
│
├── types/                   # Tipos básicos compartilhados
│   ├── Types.h              # typedefs (uint8_t, PLAYERID, etc)
│   ├── Vector.h             # CVector, CVector2D
│   ├── Matrix.h             # CMatrix
│   └── Colors.h             # RGBA, cores nomeadas
│
├── logging/                 # Sistema de log
│   ├── Log.h/cpp            # Implementação de logging
│   └── LogMacros.h          # LOGI, LOGW, LOGE, etc
│
├── config/                  # Configurações
│   ├── Settings.h/cpp       # Classe de configurações
│   └── ConfigReader.h       # Parser de configs
│
├── utils/                   # Utilitários gerais
│   ├── Timer.h/cpp          # GetTickCount, timers
│   ├── StringUtils.h        # Manipulação de strings
│   └── Encoding.h           # CP1251, UTF-8, etc
│
└── main.h                   # Header principal do core (inclui essenciais)
```

### 1.3 Componentes Internos

#### 1.3.1 Bootstrap (Ciclo de Vida)

```cpp
// core/bootstrap/Bootstrap.h
class Bootstrap {
public:
    static void Initialize();    // Chamado no JNI_OnLoad
    static void Shutdown();      // Chamado no exit
    
    // Ordem de inicialização controlada
    enum class Phase {
        Platform,    // JNI, Android
        Core,        // Logging, Config
        Game,        // Hooks, Engine
        Multiplayer, // Network
        UI,          // Interface
        Audio        // Som
    };
    
    static void RegisterInitializer(Phase phase, std::function<void()> init);
    static void RegisterShutdown(Phase phase, std::function<void()> shutdown);
    
private:
    static std::map<Phase, std::vector<std::function<void()>>> s_initializers;
    static std::map<Phase, std::vector<std::function<void()>>> s_shutdowns;
};
```

#### 1.3.2 ServiceLocator (Acesso Centralizado)

```cpp
// core/bootstrap/ServiceLocator.h
class Services {
public:
    // Registro (chamado durante inicialização)
    static void Register(IGameService* game);
    static void Register(INetworkService* network);
    static void Register(IAudioService* audio);
    static void Register(IUIService* ui);
    static void Register(ISettings* settings);
    
    // Acesso
    static IGameService* Game() { return s_game; }
    static INetworkService* Network() { return s_network; }
    static IAudioService* Audio() { return s_audio; }
    static IUIService* UI() { return s_ui; }
    static ISettings* Settings() { return s_settings; }
    
    // Cleanup
    static void Clear();

private:
    static inline IGameService* s_game = nullptr;
    static inline INetworkService* s_network = nullptr;
    static inline IAudioService* s_audio = nullptr;
    static inline IUIService* s_ui = nullptr;
    static inline ISettings* s_settings = nullptr;
};

// Macros de compatibilidade para migração gradual
#define pGame    Services::Game()
#define pNetGame Services::Network()
#define pUI      Services::UI()
```

#### 1.3.3 EventBus (Comunicação Desacoplada)

```cpp
// core/events/EventBus.h
class EventBus {
public:
    template<typename TEvent>
    static void Subscribe(std::function<void(const TEvent&)> handler);
    
    template<typename TEvent>
    static void Unsubscribe(size_t handlerId);
    
    template<typename TEvent, typename... Args>
    static void Publish(Args&&... args);
    
    // Para eventos síncronos
    template<typename TEvent>
    static void PublishImmediate(const TEvent& event);

private:
    // Type-erased storage para handlers
    static std::unordered_map<std::type_index, std::vector<std::any>> s_handlers;
};
```

### 1.4 Interfaces Públicas do CORE

| Interface | Descrição | Implementado por |
|-----------|-----------|------------------|
| `IGameService` | Acesso abstrato ao engine | `game/` |
| `INetworkService` | Acesso abstrato à rede | `multiplayer/` |
| `IAudioService` | Acesso abstrato ao áudio | `audio/` |
| `IUIService` | Acesso abstrato à UI | `ui/` |
| `ILogger` | Abstração de logging | `core/logging/` |

### 1.5 Dependências

```
CORE não depende de nenhum outro módulo
     ↓
Todos os módulos dependem de CORE
```

---

## 2. Módulo GAME

### 2.1 Propósito
Encapsula **TODA** interação com o engine do jogo base (GTA SA). Inclui hooks, patches, acesso a entidades e sistemas do engine.

### 2.2 Estrutura de Diretórios

```
game/
├── public/                    # API pública do módulo
│   ├── GameFacade.h           # Fachada principal
│   ├── IGameService.h         # Implementa interface do core
│   └── Events.h               # Eventos emitidos pelo game
│
├── hooks/                     # Sistema de hooks
│   ├── HookManager.h/cpp      # Gerenciador centralizado de hooks
│   ├── Hooks.cpp              # Instalação de hooks
│   ├── Patches.cpp            # Patches de memória
│   └── Trampolines.h          # Definições de trampolines
│
├── memory/                    # Acesso à memória do jogo
│   ├── Offsets.h              # Todos os offsets centralizados
│   ├── Offsets32.h            # Offsets para 32-bit
│   ├── Offsets64.h            # Offsets para 64-bit
│   └── GameMemory.h           # Helpers de leitura/escrita
│
├── engine/                    # Sistemas core do engine
│   ├── CGame.h/cpp            # Classe principal do jogo
│   ├── CWorld.h/cpp           # Mundo, entidades, física
│   ├── CCamera.h/cpp          # Sistema de câmera
│   ├── CTimer.h               # Timer do jogo
│   ├── CStreaming.h/cpp       # Sistema de streaming
│   └── CPools.h/cpp           # Pools nativas do GTA
│
├── entities/                  # Hierarquia de entidades
│   ├── base/                  # Classes base
│   │   ├── CEntityGTA.h       # Entidade base
│   │   ├── CPhysical.h        # Entidade com física
│   │   └── CPlaceable.h       # Posição + matriz
│   │
│   ├── ped/                   # Pedestres
│   │   ├── CPedGTA.h/cpp      # Ped nativo
│   │   ├── CPlayerPed.h/cpp   # Player ped SA-MP
│   │   └── CPedTasks.h        # Tasks de ped
│   │
│   ├── vehicle/               # Veículos
│   │   ├── CVehicleGTA.h/cpp  # Veículo nativo
│   │   ├── CAutomobile.h      # Carros
│   │   └── CHeli.h            # Helicópteros
│   │
│   └── object/                # Objetos
│       ├── CObject.h/cpp      # Objeto genérico
│       └── CBuilding.h        # Construções
│
├── animation/                 # Sistema de animação
│   ├── CAnimManager.h/cpp     # Gerenciador de animações
│   ├── CAnimBlend.h           # Blending de animações
│   └── AnimationData/         # Definições de anims
│       ├── AnimBlock.h
│       └── AnimAssoc.h
│
├── collision/                 # Sistema de colisão
│   ├── CCollision.h/cpp       # Funções de colisão
│   ├── CColModel.h            # Modelo de colisão
│   └── ColData/               # Estruturas de colisão
│       ├── ColSphere.h
│       ├── ColBox.h
│       └── ColTriangle.h
│
├── rendering/                 # Renderização
│   ├── RenderWare/            # Bindings RenderWare
│   │   ├── RwCore.h           # Core RW
│   │   ├── RpWorld.h          # World plugin
│   │   ├── RpClump.h          # Clumps
│   │   └── RwTexture.h        # Texturas
│   │
│   ├── CSprite.h/cpp          # Sprites 2D
│   ├── CFont.h/cpp            # Fontes do jogo
│   └── CFx.h                  # Efeitos visuais
│
├── models/                    # Sistema de modelos
│   ├── CModelInfo.h           # Info de modelos
│   ├── CVehicleModelInfo.h    # Info específica de veículos
│   └── CPedModelInfo.h        # Info específica de peds
│
├── input/                     # Input do jogo
│   ├── CPad.h/cpp             # Gamepad virtual
│   ├── CMultiTouch.h/cpp      # Touch screen
│   └── InputState.h           # Estado de input
│
├── tasks/                     # Task system do GTA
│   ├── CTask.h                # Task base
│   ├── CTaskSimple.h          # Task simples
│   ├── CTaskComplex.h         # Task composta
│   └── CTaskManager.h/cpp     # Gerenciador de tasks
│
└── internal/                  # Headers internos (não exportados)
    ├── GTAStructs.h           # Structs internas do GTA
    └── InternalPatches.h      # Patches internos
```

### 2.3 Componentes Internos

#### 2.3.1 GameFacade (API Pública)

```cpp
// game/public/GameFacade.h
class CGame : public IGameService {
public:
    // Lifecycle
    void Initialize();
    void Shutdown();
    void Process();  // Tick do jogo
    
    // === Entidades ===
    CPlayerPed* CreatePlayerPed(int modelId);
    void DeletePlayerPed(CPlayerPed* ped);
    
    CVehicle* CreateVehicle(int modelId, const CVector& pos, float rotation);
    void DeleteVehicle(CVehicle* vehicle);
    
    CObject* CreateObject(int modelId, const CVector& pos, const CVector& rot);
    void DeleteObject(CObject* object);
    
    // === Mundo ===
    void SetWorldTime(int hour, int minute);
    void SetWeather(int weatherId);
    void SetGravity(float gravity);
    
    // === Câmera ===
    CCamera* GetCamera();
    void SetCameraBehindPlayer();
    
    // === Pools ===
    CPools* GetPools();
    
    // === Input ===
    CPad* GetPad(int index);
    CMultiTouch* GetMultiTouch();
    
    // === Modelo ===
    bool IsModelLoaded(int modelId);
    void RequestModel(int modelId);
    void LoadRequestedModels();
    
    // === Animação ===
    void ApplyAnimation(CPlayerPed* ped, const char* animLib, const char* animName);
    
    // === Encontrar entidades ===
    CPlayerPed* FindPlayerPed();
    CVehicle* FindVehicleByGtaId(int id);
    CObject* FindObjectByGtaId(int id);

private:
    std::unique_ptr<CWorld> m_world;
    std::unique_ptr<CCamera> m_camera;
    std::unique_ptr<CStreaming> m_streaming;
    std::unique_ptr<CPools> m_pools;
    std::unique_ptr<CPad> m_pad[2];
    std::unique_ptr<CMultiTouch> m_multiTouch;
};
```

#### 2.3.2 HookManager (Gerenciamento de Hooks)

```cpp
// game/hooks/HookManager.h
class HookManager {
public:
    static void Initialize();
    static void Shutdown();
    
    // Tipos de hook
    enum class HookType {
        Inline,       // ShadowHook inline
        PLT,          // PLT/GOT hook
        VTable        // Virtual table hook
    };
    
    // Registro de hooks
    template<typename TFunc>
    static bool Hook(uintptr_t address, TFunc detour, TFunc* original);
    
    static bool Unhook(uintptr_t address);
    
    // Patches de memória
    static bool Patch(uintptr_t address, const uint8_t* data, size_t size);
    static bool Nop(uintptr_t address, size_t count);
    
    // Helper para offsets
    static uintptr_t GetOffset(const char* name);  // Busca em Offsets.h
    
private:
    struct HookEntry {
        uintptr_t address;
        void* original;
        HookType type;
    };
    static std::vector<HookEntry> s_hooks;
};
```

#### 2.3.3 Offsets Centralizados

```cpp
// game/memory/Offsets.h
#pragma once

#include "core/types/Types.h"

#if defined(VER_x32)
    #include "Offsets32.h"
#else
    #include "Offsets64.h"
#endif

namespace Offsets {
    // Funções
    inline uintptr_t CWorld_Add;
    inline uintptr_t CWorld_Remove;
    inline uintptr_t CStreaming_RequestModel;
    inline uintptr_t CStreaming_LoadAllRequestedModels;
    
    // Pools
    inline uintptr_t ms_pPedPool;
    inline uintptr_t ms_pVehiclePool;
    inline uintptr_t ms_pObjectPool;
    
    // Variáveis
    inline uintptr_t TheCamera;
    inline uintptr_t FindPlayerPed;
    
    void Initialize(uintptr_t libBase);
}

// game/memory/Offsets32.h
namespace Offsets32 {
    constexpr uintptr_t CWorld_Add = 0x123456;
    constexpr uintptr_t CWorld_Remove = 0x123460;
    // ... todos os offsets 32-bit
}

// game/memory/Offsets64.h
namespace Offsets64 {
    constexpr uintptr_t CWorld_Add = 0x456789;
    constexpr uintptr_t CWorld_Remove = 0x45678D;
    // ... todos os offsets 64-bit
}
```

### 2.4 Eventos Emitidos pelo GAME

```cpp
// game/public/Events.h
namespace GameEvents {
    struct GameInitialized {};
    struct GameShutdown {};
    
    struct EntityCreated { CEntityGTA* entity; };
    struct EntityDestroyed { CEntityGTA* entity; };
    
    struct PlayerPedCreated { CPlayerPed* ped; };
    struct VehicleCreated { CVehicle* vehicle; };
    
    struct WorldTimeChanged { int hour; int minute; };
    struct WeatherChanged { int weatherId; };
    
    struct ModelLoaded { int modelId; };
    struct AnimationPlayed { CPlayerPed* ped; const char* anim; };
    
    struct CameraChanged { CVector pos; CVector lookAt; };
}
```

### 2.5 Dependências Internas

```
game/public/          ← API para outros módulos
     ↑
game/engine/          ← Sistemas core
     ↑
game/entities/        ← Usa engine
     ↑
game/hooks/           ← Intercepta engine
     ↑
game/memory/          ← Base para hooks
```

---

## 3. Módulo MULTIPLAYER

### 3.1 Propósito
Toda lógica de rede e multiplayer (SA-MP). Gerencia conexão, sincronização, pools de entidades de rede e RPCs.

### 3.2 Estrutura de Diretórios

```
multiplayer/
├── public/                    # API pública
│   ├── NetworkFacade.h        # Fachada principal
│   ├── INetworkService.h      # Implementa interface do core
│   └── Events.h               # Eventos de rede
│
├── connection/                # Gerenciamento de conexão
│   ├── CNetGame.h/cpp         # Classe principal de rede
│   ├── ConnectionState.h      # Estados de conexão
│   └── ServerInfo.h           # Info do servidor
│
├── protocol/                  # Protocolo SA-MP
│   ├── RakNet/                # Wrapper RakNet
│   │   ├── RakClient.h        # Cliente RakNet
│   │   └── BitStream.h        # Serialização
│   │
│   ├── PacketEnums.h          # IDs de pacotes
│   └── Compression.h          # Compressão de dados
│
├── rpc/                       # Remote Procedure Calls
│   ├── RPCHandler.h/cpp       # Dispatcher de RPCs
│   ├── RPCRegistry.h          # Registro de handlers
│   │
│   ├── handlers/              # Handlers específicos
│   │   ├── PlayerRPCs.cpp     # RPCs de player
│   │   ├── VehicleRPCs.cpp    # RPCs de veículo
│   │   ├── ObjectRPCs.cpp     # RPCs de objeto
│   │   ├── WorldRPCs.cpp      # RPCs de mundo
│   │   ├── ChatRPCs.cpp       # RPCs de chat
│   │   └── DialogRPCs.cpp     # RPCs de dialog
│   │
│   └── outgoing/              # RPCs enviados
│       ├── ClientRPCs.h/cpp   # RPCs do cliente
│       └── SyncPackets.h/cpp  # Pacotes de sync
│
├── sync/                      # Sincronização
│   ├── SyncManager.h/cpp      # Gerenciador de sync
│   ├── LocalPlayerSync.h/cpp  # Sync do player local
│   ├── RemotePlayerSync.h/cpp # Sync de players remotos
│   ├── VehicleSync.h/cpp      # Sync de veículos
│   └── SyncData.h             # Estruturas de sync
│
├── pools/                     # Pools de entidades de rede
│   ├── PoolManager.h          # Gerenciador de pools
│   │
│   ├── CPlayerPool.h/cpp      # Pool de players
│   ├── CVehiclePool.h/cpp     # Pool de veículos
│   ├── CObjectPool.h/cpp      # Pool de objetos
│   ├── CPickupPool.h/cpp      # Pool de pickups
│   ├── CTextPool.h/cpp        # Pool de text draws
│   ├── CLabelPool.h/cpp       # Pool de 3D text labels
│   ├── CActorPool.h/cpp       # Pool de actors
│   ├── CGangZonePool.h/cpp    # Pool de gang zones
│   └── CMenuPool.h/cpp        # Pool de menus
│
├── entities/                  # Entidades multiplayer
│   ├── CLocalPlayer.h/cpp     # Player local
│   ├── CRemotePlayer.h/cpp    # Player remoto
│   ├── CNetVehicle.h/cpp      # Veículo de rede
│   ├── CNetObject.h/cpp       # Objeto de rede
│   ├── CPickup.h/cpp          # Pickup
│   ├── CTextDraw.h/cpp        # Text draw
│   └── C3DTextLabel.h/cpp     # 3D label
│
└── settings/                  # Configurações de rede
    ├── NetSettings.h          # Settings de rede (do servidor)
    └── NetRates.h             # Taxas de sync
```

### 3.3 Componentes Internos

#### 3.3.1 NetworkFacade (API Pública)

```cpp
// multiplayer/public/NetworkFacade.h
class CNetGame : public INetworkService {
public:
    // Lifecycle
    void Initialize();
    void Shutdown();
    void Process();  // Tick de rede
    
    // === Conexão ===
    bool Connect(const char* ip, int port, const char* nickname, const char* password);
    void Disconnect();
    bool IsConnected() const;
    ConnectionState GetState() const;
    
    // === Pools ===
    CPlayerPool* GetPlayerPool();
    CVehiclePool* GetVehiclePool();
    CObjectPool* GetObjectPool();
    CPickupPool* GetPickupPool();
    CTextPool* GetTextPool();
    CLabelPool* GetLabelPool();
    
    // === Player Local ===
    CLocalPlayer* GetLocalPlayer();
    PLAYERID GetLocalPlayerID() const;
    
    // === RPC ===
    void SendRPC(int rpcId, BitStream* data);
    
    // === Spawn ===
    void RequestSpawn();
    void Spawn();
    
    // === Settings do servidor ===
    const NETSETTINGS* GetNetSettings() const;

private:
    std::unique_ptr<RakClient> m_rakClient;
    std::unique_ptr<RPCHandler> m_rpcHandler;
    std::unique_ptr<SyncManager> m_syncManager;
    
    std::unique_ptr<CPlayerPool> m_playerPool;
    std::unique_ptr<CVehiclePool> m_vehiclePool;
    std::unique_ptr<CObjectPool> m_objectPool;
    std::unique_ptr<CPickupPool> m_pickupPool;
    std::unique_ptr<CTextPool> m_textPool;
    std::unique_ptr<CLabelPool> m_labelPool;
    
    std::unique_ptr<CLocalPlayer> m_localPlayer;
    
    ConnectionState m_state;
    NETSETTINGS m_netSettings;
};
```

#### 3.3.2 RPCHandler (Processamento de RPCs)

```cpp
// multiplayer/rpc/RPCHandler.h
class RPCHandler {
public:
    using HandlerFunc = std::function<void(RPCParameters*)>;
    
    void Initialize();
    void Shutdown();
    
    // Registro de handlers
    void Register(int rpcId, HandlerFunc handler);
    void Unregister(int rpcId);
    
    // Processamento (chamado pelo RakNet)
    void ProcessRPC(int rpcId, RPCParameters* params);
    
private:
    std::unordered_map<int, HandlerFunc> m_handlers;
};

// Exemplo de handler desacoplado
// multiplayer/rpc/handlers/ChatRPCs.cpp
void RegisterChatRPCs(RPCHandler& handler) {
    handler.Register(RPC_ClientMessage, [](RPCParameters* params) {
        BitStream bs(params->input, params->numberOfBitsOfData / 8, false);
        
        uint32_t color;
        uint32_t length;
        char message[256];
        
        bs.Read(color);
        bs.Read(length);
        bs.Read(message, length);
        message[length] = '\0';
        
        // Emite evento em vez de chamar UI diretamente
        EventBus::Publish(ChatMessageReceivedEvent{message, color});
    });
}
```

#### 3.3.3 SyncManager (Sincronização)

```cpp
// multiplayer/sync/SyncManager.h
class SyncManager {
public:
    void Initialize();
    void Process();  // Chamado a cada frame
    
    // Configurações de taxa
    void SetSyncRate(SyncType type, int rateMs);
    
    // Sync manual
    void ForceSyncLocalPlayer();
    void ForceSyncVehicle(int vehicleId);
    
private:
    struct SyncEntry {
        SyncType type;
        int rateMs;
        uint32_t lastSyncTime;
    };
    
    std::vector<SyncEntry> m_syncEntries;
    
    void SyncOnFoot();
    void SyncInCar();
    void SyncPassenger();
    void SyncAim();
    void SyncTrailer();
};
```

### 3.4 Eventos Emitidos pelo MULTIPLAYER

```cpp
// multiplayer/public/Events.h
namespace NetworkEvents {
    // Conexão
    struct ConnectionAttempt { const char* ip; int port; };
    struct Connected { PLAYERID playerId; };
    struct Disconnected { int reason; };
    struct ConnectionFailed { int reason; };
    
    // Players
    struct PlayerConnected { PLAYERID playerId; const char* name; };
    struct PlayerDisconnected { PLAYERID playerId; int reason; };
    struct PlayerSpawned { PLAYERID playerId; };
    struct PlayerDied { PLAYERID playerId; PLAYERID killerId; int reason; };
    struct PlayerStreamIn { PLAYERID playerId; };
    struct PlayerStreamOut { PLAYERID playerId; };
    
    // Veículos
    struct VehicleStreamIn { VEHICLEID vehicleId; };
    struct VehicleStreamOut { VEHICLEID vehicleId; };
    struct PlayerEnterVehicle { PLAYERID playerId; VEHICLEID vehicleId; bool passenger; };
    struct PlayerExitVehicle { PLAYERID playerId; VEHICLEID vehicleId; };
    
    // Chat/Dialogs
    struct ChatMessageReceived { const char* message; uint32_t color; };
    struct DialogReceived { int dialogId; int style; const char* title; const char* body; };
    
    // Mundo
    struct WorldTimeSync { int hour; int minute; };
    struct WeatherSync { int weatherId; };
    struct GravitySync { float gravity; };
    
    // Spawn
    struct SpawnInfoReceived { SPAWNINFO info; };
    struct RequestClassReceived { int classId; };
}
```

### 3.5 Dependências Internas

```
multiplayer/public/   ← API para outros módulos
     ↑
multiplayer/pools/    ← Gerencia entidades
     ↑
multiplayer/sync/     ← Usa pools
     ↑
multiplayer/rpc/      ← Modifica pools via RPCs
     ↑
multiplayer/protocol/ ← Base de comunicação
```

---

## 4. Módulo UI

### 4.1 Propósito
Toda interface do usuário. Renderização, widgets, input de texto, dialogs, chat, HUD.

### 4.2 Estrutura de Diretórios

```
ui/
├── public/                    # API pública
│   ├── UIFacade.h             # Fachada principal
│   ├── IUIService.h           # Implementa interface do core
│   └── Events.h               # Eventos de UI
│
├── core/                      # Infraestrutura de UI
│   ├── UIManager.h/cpp        # Gerenciador principal
│   ├── Screen.h/cpp           # Classe base de tela
│   ├── Renderer.h/cpp         # Wrapper ImGui
│   └── Theme.h/cpp            # Cores, estilos
│
├── widgets/                   # Widgets base
│   ├── Widget.h/cpp           # Widget base
│   ├── Layout.h/cpp           # Container de layout
│   ├── Button.h/cpp           # Botão
│   ├── Label.h/cpp            # Label de texto
│   ├── Image.h/cpp            # Imagem
│   ├── EditBox.h/cpp          # Campo de texto
│   ├── ListBox.h/cpp          # Lista
│   ├── ProgressBar.h/cpp      # Barra de progresso
│   └── ScrollPanel.h/cpp      # Painel com scroll
│
├── screens/                   # Telas específicas
│   ├── SplashScreen.h/cpp     # Tela de splash
│   ├── MainMenu.h/cpp         # Menu principal
│   ├── ServerBrowser.h/cpp    # Lista de servidores
│   ├── SettingsScreen.h/cpp   # Configurações
│   └── GameScreen.h/cpp       # Tela in-game
│
├── hud/                       # HUD elements
│   ├── HUD.h/cpp              # HUD principal
│   ├── Chat.h/cpp             # Chat box
│   ├── PlayerTags.h/cpp       # Name tags
│   ├── Radar.h/cpp            # Mini mapa
│   ├── SpeedoMeter.h/cpp      # Velocímetro
│   └── HealthBar.h/cpp        # Barra de vida
│
├── dialogs/                   # Sistema de dialogs
│   ├── DialogManager.h/cpp    # Gerenciador de dialogs
│   ├── Dialog.h/cpp           # Dialog base
│   ├── MessageBox.h/cpp       # Dialog de mensagem
│   ├── InputDialog.h/cpp      # Dialog com input
│   ├── ListDialog.h/cpp       # Dialog com lista
│   └── TabListDialog.h/cpp    # Dialog com tab list
│
├── input/                     # Input de UI
│   ├── TouchHandler.h/cpp     # Processamento de touch
│   ├── Keyboard.h/cpp         # Teclado virtual
│   └── InputState.h           # Estado de input
│
├── textdraws/                 # TextDraws
│   ├── TextDrawManager.h/cpp  # Gerenciador
│   ├── TextDraw.h/cpp         # TextDraw individual
│   └── TextDrawRenderer.h/cpp # Renderização
│
└── internal/                  # Headers internos
    ├── ImGuiWrapper.h         # Configuração ImGui
    └── FontManager.h          # Gerenciador de fontes
```

### 4.3 Componentes Internos

#### 4.3.1 UIFacade (API Pública)

```cpp
// ui/public/UIFacade.h
class UI : public IUIService {
public:
    // Lifecycle
    void Initialize();
    void Shutdown();
    void Render();    // Chamado a cada frame
    void ProcessInput(const TouchEvent& event);
    
    // === Navegação de Telas ===
    void ShowScreen(ScreenType type);
    void HideScreen(ScreenType type);
    ScreenType GetCurrentScreen() const;
    
    // === Chat ===
    Chat* GetChat();
    void AddChatMessage(const char* message, uint32_t color);
    void ClearChat();
    
    // === Dialogs ===
    void ShowDialog(int dialogId, int style, const char* title, 
                    const char* body, const char* button1, const char* button2);
    void HideDialog();
    bool IsDialogOpen() const;
    
    // === Keyboard ===
    void ShowKeyboard(const char* title = nullptr);
    void HideKeyboard();
    bool IsKeyboardOpen() const;
    
    // === HUD ===
    void ShowHUD();
    void HideHUD();
    void UpdateHUD(int health, int armor, int weaponId, int ammo, int money, int wanted);
    
    // === TextDraws ===
    void ShowTextDraw(int textDrawId);
    void HideTextDraw(int textDrawId);
    void UpdateTextDraw(int textDrawId, const TextDrawData& data);
    
    // === Spawn Screen ===
    void ShowSpawnScreen(const SPAWNINFO& info);
    void HideSpawnScreen();
    
    // === Player Tags ===
    void SetPlayerTagsEnabled(bool enabled);
    void UpdatePlayerTag(PLAYERID playerId, const char* name, float health, bool behind);

private:
    std::unique_ptr<UIManager> m_manager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Chat> m_chat;
    std::unique_ptr<DialogManager> m_dialogManager;
    std::unique_ptr<HUD> m_hud;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<TextDrawManager> m_textDrawManager;
    std::unique_ptr<PlayerTags> m_playerTags;
};
```

#### 4.3.2 Widget Base

```cpp
// ui/widgets/Widget.h
class Widget {
public:
    Widget();
    virtual ~Widget();
    
    // === Hierarquia ===
    Widget* GetParent() const;
    void SetParent(Widget* parent);
    const std::vector<Widget*>& GetChildren() const;
    void AddChild(Widget* child);
    void RemoveChild(Widget* child);
    
    // === Geometria ===
    const ImVec2& GetPosition() const;
    void SetPosition(const ImVec2& pos);
    const ImVec2& GetSize() const;
    void SetSize(const ImVec2& size);
    ImVec2 GetAbsolutePosition() const;
    
    // === Visibilidade ===
    bool IsVisible() const;
    void SetVisible(bool visible);
    
    // === Estado ===
    bool IsFocused() const;
    bool IsActive() const;
    
    // === Eventos (Virtual) ===
    virtual void OnDraw(Renderer* renderer);
    virtual void OnLayout();
    virtual void OnTouchDown(const ImVec2& pos);
    virtual void OnTouchMove(const ImVec2& delta);
    virtual void OnTouchUp();
    virtual void OnFocusGained();
    virtual void OnFocusLost();
    virtual void OnTextInput(const std::string& text);

protected:
    Widget* m_parent = nullptr;
    std::vector<Widget*> m_children;
    ImVec2 m_position;
    ImVec2 m_size;
    bool m_visible = true;
    bool m_focused = false;
    bool m_active = false;
};
```

#### 4.3.3 DialogManager

```cpp
// ui/dialogs/DialogManager.h
class DialogManager {
public:
    void Initialize();
    void Shutdown();
    void Render();
    
    // Mostrar dialog
    void Show(int dialogId, DialogStyle style, const std::string& title,
              const std::string& body, const std::string& button1, 
              const std::string& button2);
    
    void Hide();
    bool IsOpen() const;
    int GetCurrentDialogId() const;
    
    // Resposta
    void SetResponseCallback(std::function<void(int dialogId, int buttonId, 
                             int listItem, const std::string& inputText)> callback);

private:
    std::unique_ptr<Dialog> m_currentDialog;
    std::function<void(int, int, int, const std::string&)> m_responseCallback;
    int m_currentDialogId = -1;
};
```

### 4.4 Eventos Emitidos pela UI

```cpp
// ui/public/Events.h
namespace UIEvents {
    // Input
    struct KeyboardInputReceived { std::string text; };
    struct KeyboardClosed {};
    struct TouchEvent { ImVec2 pos; TouchType type; };
    
    // Dialogs
    struct DialogResponse { int dialogId; int buttonId; int listItem; std::string inputText; };
    struct DialogClosed { int dialogId; };
    
    // Chat
    struct ChatInputSubmitted { std::string message; };
    struct ChatOpened {};
    struct ChatClosed {};
    
    // Spawn
    struct SpawnButtonClicked {};
    struct ClassChangeRequested { int direction; }; // -1 ou +1
    
    // Menu
    struct MenuItemSelected { int menuId; int row; };
    
    // Settings
    struct SettingsChanged { std::string key; std::string value; };
}
```

### 4.5 Dependências Internas

```
ui/public/          ← API para outros módulos
     ↑
ui/screens/         ← Telas compostas
     ↑
ui/hud/             ← Componentes de HUD
ui/dialogs/         ← Sistema de dialogs
     ↑
ui/widgets/         ← Widgets base
     ↑
ui/core/            ← Infraestrutura (renderer, manager)
```

---

## 5. Módulo AUDIO

### 5.1 Propósito
Todo sistema de áudio. Música streaming, sistema de voz, efeitos sonoros.

### 5.2 Estrutura de Diretórios

```
audio/
├── public/                    # API pública
│   ├── AudioFacade.h          # Fachada principal
│   ├── IAudioService.h        # Implementa interface do core
│   └── Events.h               # Eventos de áudio
│
├── core/                      # Infraestrutura de áudio
│   ├── AudioManager.h/cpp     # Gerenciador principal
│   ├── AudioDevice.h/cpp      # Device de áudio (OpenSL ES)
│   └── AudioMixer.h/cpp       # Mixer de canais
│
├── streaming/                 # Audio streaming (músicas)
│   ├── AudioStream.h/cpp      # Stream de áudio
│   ├── BassWrapper.h/cpp      # Wrapper da lib BASS
│   └── StreamBuffer.h         # Buffer de stream
│
├── voice/                     # Sistema de voz
│   ├── VoiceManager.h/cpp     # Gerenciador de voz
│   ├── VoiceNetwork.h/cpp     # Rede de voz (separada)
│   │
│   ├── recording/             # Gravação
│   │   ├── Recorder.h/cpp     # Captura de mic
│   │   └── OpusEncoder.h/cpp  # Encoding Opus
│   │
│   ├── playback/              # Reprodução
│   │   ├── Playback.h/cpp     # Reprodução de voz
│   │   └── OpusDecoder.h/cpp  # Decoding Opus
│   │
│   ├── streams/               # Tipos de streams de voz
│   │   ├── Stream.h/cpp       # Stream base
│   │   ├── GlobalStream.h     # Stream global
│   │   ├── LocalStream.h      # Stream local
│   │   ├── StreamAtPlayer.h   # Stream em player
│   │   ├── StreamAtVehicle.h  # Stream em veículo
│   │   └── StreamAtPoint.h    # Stream em ponto
│   │
│   ├── effects/               # Efeitos de voz
│   │   ├── Effect.h/cpp       # Efeito base
│   │   └── VoiceEffects.h     # Efeitos disponíveis
│   │
│   └── ui/                    # UI de voz
│       ├── MicroIcon.h/cpp    # Ícone do microfone
│       └── SpeakerList.h/cpp  # Lista de falantes
│
└── sfx/                       # Efeitos sonoros (futuro)
    ├── SFXManager.h/cpp       # Gerenciador de SFX
    └── SoundBank.h            # Banco de sons
```

### 5.3 Componentes Internos

#### 5.3.1 AudioFacade (API Pública)

```cpp
// audio/public/AudioFacade.h
class AudioManager : public IAudioService {
public:
    // Lifecycle
    void Initialize();
    void Shutdown();
    void Process();  // Tick de áudio
    
    // === Streaming (Música) ===
    bool PlayStream(const char* url);
    void StopStream();
    void PauseStream();
    void ResumeStream();
    void SetStreamVolume(float volume);
    bool IsStreamPlaying() const;
    
    // === Voice ===
    VoiceManager* GetVoiceManager();
    
    bool IsVoiceEnabled() const;
    void SetVoiceEnabled(bool enabled);
    
    void StartRecording();
    void StopRecording();
    bool IsRecording() const;
    
    void SetMicrophoneVolume(float volume);
    void SetVoiceVolume(float volume);
    
    // === Voice Streams (via servidor) ===
    void CreateVoiceStream(uint32_t streamId, uint32_t flags, uint32_t color, const char* name);
    void DeleteVoiceStream(uint32_t streamId);
    void PushVoicePacket(uint32_t streamId, uint16_t playerId, const uint8_t* data, size_t size);
    
    // === 3D Audio ===
    void SetListenerPosition(const CVector& pos, const CVector& front, const CVector& up);
    
private:
    std::unique_ptr<AudioDevice> m_device;
    std::unique_ptr<AudioStream> m_stream;
    std::unique_ptr<VoiceManager> m_voiceManager;
};
```

#### 5.3.2 VoiceManager (Gerenciamento de Voz)

```cpp
// audio/voice/VoiceManager.h
class VoiceManager {
public:
    void Initialize();
    void Shutdown();
    void Process();
    
    // === Recording ===
    void StartRecording();
    void StopRecording();
    bool IsRecording() const;
    void SetRecordCallback(std::function<void(const uint8_t*, size_t)> callback);
    
    // === Playback ===
    void PlayVoice(uint16_t playerId, const uint8_t* data, size_t size);
    void StopVoice(uint16_t playerId);
    
    // === Streams ===
    void CreateStream(uint32_t id, StreamType type, uint32_t flags, 
                      uint32_t color, const std::string& name);
    void DeleteStream(uint32_t id);
    Stream* GetStream(uint32_t id);
    
    void PushToStream(uint32_t streamId, uint16_t playerId, 
                      const uint8_t* data, size_t size);
    
    // === Effects ===
    void ApplyEffect(uint32_t streamId, uint32_t effectType, const void* params, size_t size);
    void RemoveEffect(uint32_t streamId, uint32_t effectType);
    
    // === UI ===
    MicroIcon* GetMicroIcon();
    SpeakerList* GetSpeakerList();

private:
    std::unique_ptr<Recorder> m_recorder;
    std::unique_ptr<Playback> m_playback;
    std::unique_ptr<VoiceNetwork> m_network;
    std::unique_ptr<MicroIcon> m_microIcon;
    std::unique_ptr<SpeakerList> m_speakerList;
    
    std::map<uint32_t, std::unique_ptr<Stream>> m_streams;
};
```

### 5.4 Eventos Emitidos pelo AUDIO

```cpp
// audio/public/Events.h
namespace AudioEvents {
    // Streaming
    struct StreamStarted { std::string url; };
    struct StreamStopped {};
    struct StreamError { int errorCode; std::string message; };
    struct StreamBuffering { float progress; };
    
    // Voice
    struct RecordingStarted {};
    struct RecordingStopped {};
    struct VoiceDataRecorded { const uint8_t* data; size_t size; };
    
    struct VoicePlaying { uint16_t playerId; };
    struct VoiceStopped { uint16_t playerId; };
    
    struct VoiceStreamCreated { uint32_t streamId; std::string name; };
    struct VoiceStreamDeleted { uint32_t streamId; };
    
    // Mic
    struct MicrophoneMuted {};
    struct MicrophoneUnmuted {};
}
```

### 5.5 Dependências Internas

```
audio/public/        ← API para outros módulos
     ↑
audio/voice/         ← Sistema de voz completo
audio/streaming/     ← Audio streaming
     ↑
audio/core/          ← Infraestrutura (device, mixer)
```

---

## 6. Módulo PLATFORM

### 6.1 Propósito
Código específico de plataforma. JNI, Android APIs, ciclo de vida da aplicação, permissões.

### 6.2 Estrutura de Diretórios

```
platform/
├── public/                    # API pública
│   ├── PlatformFacade.h       # Fachada principal
│   └── Events.h               # Eventos de plataforma
│
├── android/                   # Código Android específico
│   ├── JNI.h/cpp              # Entry points JNI
│   ├── JNIHelper.h/cpp        # Helpers JNI
│   ├── JavaWrapper.h/cpp      # Chamadas Java
│   └── Permissions.h/cpp      # Gerenciamento de permissões
│
├── lifecycle/                 # Ciclo de vida
│   ├── AppState.h/cpp         # Estado da aplicação
│   ├── FocusHandler.h/cpp     # Foco/pausa da app
│   └── BackgroundHandler.h/cpp # Tratamento de background
│
├── input/                     # Input de plataforma
│   ├── NativeInput.h/cpp      # Eventos nativos de input
│   ├── TouchDispatcher.h/cpp  # Distribuição de touch
│   └── Accelerometer.h/cpp    # Sensores (opcional)
│
├── storage/                   # Armazenamento
│   ├── FileSystem.h/cpp       # Acesso a arquivos
│   ├── SharedPrefs.h/cpp      # SharedPreferences
│   └── AssetManager.h/cpp     # Assets da APK
│
├── graphics/                  # Gráficos de plataforma
│   ├── EGLContext.h/cpp       # Contexto EGL
│   ├── GLESInit.h/cpp         # Inicialização OpenGL ES
│   └── SurfaceHandler.h/cpp   # Surface do Android
│
├── threading/                 # Threading
│   ├── ThreadPool.h/cpp       # Pool de threads
│   ├── MainLooper.h/cpp       # Main thread handling
│   └── JNIThreadAttach.h      # Attach/detach de threads JNI
│
└── debug/                     # Debug/Crash
    ├── Crashlytics.h/cpp      # Integração Crashlytics
    ├── StackTrace.h/cpp       # Stack traces
    └── ANRWatchdog.h/cpp      # Detecção de ANR
```

### 6.3 Componentes Internos

#### 6.3.1 PlatformFacade (API Pública)

```cpp
// platform/public/PlatformFacade.h
class Platform {
public:
    // Lifecycle
    static void Initialize(JNIEnv* env, jobject activity);
    static void Shutdown();
    
    // === JNI ===
    static JNIEnv* GetJNIEnv();
    static jobject GetActivity();
    static JavaVM* GetJavaVM();
    
    // === App State ===
    static AppState GetAppState();
    static bool IsPaused();
    static bool IsInBackground();
    
    // === Java Calls ===
    static void ShowToast(const char* message);
    static void ShowKeyboard();
    static void HideKeyboard();
    static std::string GetClipboardText();
    static void SetClipboardText(const char* text);
    static void ExitApp();
    
    // === Permissions ===
    static bool HasPermission(Permission permission);
    static void RequestPermission(Permission permission);
    
    // === Storage ===
    static std::string GetInternalPath();
    static std::string GetExternalPath();
    static std::string GetCachePath();
    
    // === Device Info ===
    static std::string GetDeviceModel();
    static std::string GetAndroidVersion();
    static int GetAPILevel();
    static std::string GetABI();
    
    // === Screen ===
    static int GetScreenWidth();
    static int GetScreenHeight();
    static float GetScreenDensity();
    
    // === Threading ===
    static void RunOnMainThread(std::function<void()> task);
    static void RunOnBackgroundThread(std::function<void()> task);
    
private:
    static inline JavaVM* s_javaVM = nullptr;
    static inline jobject s_activity = nullptr;
    static inline AppState s_appState = AppState::Running;
};
```

#### 6.3.2 JNIHelper (Helpers JNI Seguros)

```cpp
// platform/android/JNIHelper.h
class JNIHelper {
public:
    // Obter JNIEnv de forma segura
    static bool GetEnv(JNIEnv** env, bool* needsDetach);
    static void DetachIfNeeded(bool needsDetach);
    
    // RAII para attach/detach
    class ScopedEnv {
    public:
        ScopedEnv();
        ~ScopedEnv();
        JNIEnv* Get() const { return m_env; }
        operator JNIEnv*() const { return m_env; }
        bool IsValid() const { return m_env != nullptr; }
    private:
        JNIEnv* m_env = nullptr;
        bool m_needsDetach = false;
    };
    
    // Helpers de conversão
    static std::string JStringToStd(JNIEnv* env, jstring jstr);
    static jstring StdToJString(JNIEnv* env, const std::string& str);
    
    // Helpers de array
    static std::vector<uint8_t> JByteArrayToVector(JNIEnv* env, jbyteArray array);
    static jbyteArray VectorToJByteArray(JNIEnv* env, const std::vector<uint8_t>& vec);
    
    // Verificação de exceção
    static bool CheckException(JNIEnv* env, bool clearIfFound = true);
    
    // Referências globais
    static jobject CreateGlobalRef(JNIEnv* env, jobject localRef);
    static void DeleteGlobalRef(JNIEnv* env, jobject globalRef);
};

// Uso:
// JNIHelper::ScopedEnv env;
// if (env.IsValid()) {
//     jstring str = env->NewStringUTF("Hello");
//     // ...
// }
```

### 6.4 Eventos Emitidos pelo PLATFORM

```cpp
// platform/public/Events.h
namespace PlatformEvents {
    // Lifecycle
    struct AppResumed {};
    struct AppPaused {};
    struct AppStopped {};
    struct AppDestroyed {};
    struct LowMemoryWarning {};
    
    // Focus
    struct WindowFocusGained {};
    struct WindowFocusLost {};
    
    // Surface
    struct SurfaceCreated { int width; int height; };
    struct SurfaceChanged { int width; int height; };
    struct SurfaceDestroyed {};
    
    // Input
    struct BackButtonPressed {};
    struct KeyEvent { int keyCode; int action; };
    
    // Permissions
    struct PermissionGranted { Permission permission; };
    struct PermissionDenied { Permission permission; };
}
```

### 6.5 Dependências Internas

```
platform/public/     ← API para outros módulos
     ↑
platform/android/    ← JNI e Java calls
platform/lifecycle/  ← Ciclo de vida
     ↑
platform/storage/    ← Usa Android APIs
platform/input/      ← Usa Android APIs
platform/graphics/   ← Usa EGL/GLES
     ↑
platform/threading/  ← Base para async
```

---

## 7. Diagrama de Relacionamento

### 7.1 Visão Geral das Camadas

```
┌─────────────────────────────────────────────────────────────────────────┐
│                            APPLICATION                                   │
│                                                                         │
│    ┌─────────┐    ┌─────────────┐    ┌─────────┐    ┌─────────┐        │
│    │   UI    │    │ MULTIPLAYER │    │  AUDIO  │    │  GAME   │        │
│    └────┬────┘    └──────┬──────┘    └────┬────┘    └────┬────┘        │
│         │                │                │              │              │
│         │   ┌────────────┼────────────────┼──────────────┘              │
│         │   │            │                │                             │
│         ▼   ▼            ▼                ▼                             │
│    ┌──────────────────────────────────────────────────────────────┐    │
│    │                         CORE                                  │    │
│    │  [Events] [Services] [Interfaces] [Types] [Logging] [Memory] │    │
│    └──────────────────────────────────────────────────────────────┘    │
│                                   │                                     │
│                                   ▼                                     │
│    ┌──────────────────────────────────────────────────────────────┐    │
│    │                       PLATFORM                                │    │
│    │    [JNI] [Android] [Lifecycle] [Threading] [Storage]         │    │
│    └──────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────┘
```

### 7.2 Comunicação Entre Módulos

```
┌─────────┐                              ┌─────────────┐
│   UI    │ ◄───── ChatMessageReceived ──│ MULTIPLAYER │
│         │                              │             │
│         │ ───── DialogResponse ───────►│             │
│         │                              │             │
│         │ ◄──── SpawnInfoReceived ────│             │
│         │ ───── SpawnButtonClicked ──►│             │
└────┬────┘                              └──────┬──────┘
     │                                          │
     │    ┌────────────┐                        │
     │    │            │                        │
     ▼    ▼            │                        ▼
┌─────────┐            │                 ┌─────────┐
│  AUDIO  │ ◄──────────┼─────────────────│  GAME   │
│         │            │                 │         │
│         │ VoiceData  │                 │         │
│         │ via RPC    │                 │ Entity  │
│         │            │                 │ Events  │
└─────────┘            │                 └─────────┘
                       │
              ┌────────┴────────┐
              │      CORE       │
              │   (EventBus)    │
              │   (Services)    │
              └────────┬────────┘
                       │
              ┌────────┴────────┐
              │    PLATFORM     │
              │  (JNI bridge)   │
              └─────────────────┘
```

### 7.3 Matriz de Dependências Detalhada

| Módulo | Depende de | Comunica via |
|--------|------------|--------------|
| **CORE** | PLATFORM (apenas tipos) | Direto (fundação) |
| **PLATFORM** | - | Eventos, Direto |
| **GAME** | CORE | Services, Eventos |
| **MULTIPLAYER** | CORE, GAME (via interface) | Services, Eventos |
| **UI** | CORE | Services, Eventos |
| **AUDIO** | CORE, PLATFORM | Services, Eventos |

---

## 8. Padrões de Comunicação Interna

### 8.1 Dentro de um Módulo

```cpp
// Comunicação interna usa dependência direta (injeção no construtor)

// multiplayer/pools/CPlayerPool.cpp
CPlayerPool::CPlayerPool(CNetGame* netGame, IGameService* gameService)
    : m_netGame(netGame)
    , m_gameService(gameService)
{
}

void CPlayerPool::CreatePlayer(PLAYERID id, const char* name) {
    // Usa game service para criar entidade visual
    auto ped = m_gameService->CreatePlayerPed(0);
    
    m_players[id] = std::make_unique<CRemotePlayer>(id, name, ped);
}
```

### 8.2 Entre Módulos (Via Core)

```cpp
// Comunicação entre módulos usa EventBus ou Interfaces

// multiplayer/rpc/handlers/ChatRPCs.cpp
void OnClientMessage(RPCParameters* params) {
    // Decodifica
    auto [message, color] = DecodeClientMessage(params);
    
    // Emite evento (UI vai escutar)
    EventBus::Publish(ChatMessageReceivedEvent{message, color});
}

// ui/hud/Chat.cpp
Chat::Chat() {
    EventBus::Subscribe<ChatMessageReceivedEvent>([this](const auto& event) {
        AddMessage(event.message, event.color);
    });
}
```

### 8.3 Callbacks Específicos

```cpp
// Para comunicação ponto-a-ponto específica

// audio/streaming/AudioStream.cpp
void AudioStream::SetPauseCallback(std::function<void(bool)> callback) {
    m_pauseCallback = callback;
}

// Chamado quando precisa verificar estado
void AudioStream::CheckPauseState() {
    if (m_pauseCallback) {
        bool shouldPause = m_pauseCallback();
        if (shouldPause && m_isPlaying) {
            Pause();
        }
    }
}

// Em algum lugar durante inicialização
audioStream->SetPauseCallback([]() {
    return Services::Game()->IsGamePaused();
});
```

### 8.4 Service Locator para Acesso Síncrono

```cpp
// Quando precisa de acesso síncrono a outro módulo

// game/engine/CGame.cpp
void CGame::SpawnPlayer() {
    // Usa interface via service locator
    auto netService = Services::Network();
    if (netService && netService->IsConnected()) {
        auto localPlayer = netService->GetLocalPlayer();
        if (localPlayer) {
            // Spawn logic
        }
    }
}
```

---

## Resumo

Este documento define a arquitetura interna detalhada de cada subsistema:

1. **CORE**: Fundação com Events, Services, Interfaces, Types, Logging
2. **GAME**: Engine GTA com Hooks, Memory, Entities, Animation, Collision, Rendering
3. **MULTIPLAYER**: Rede SA-MP com Connection, Protocol, RPC, Sync, Pools
4. **UI**: Interface com Widgets, Screens, HUD, Dialogs, TextDraws
5. **AUDIO**: Som com Streaming, Voice (Recording, Playback, Streams, Effects)
6. **PLATFORM**: Android com JNI, Lifecycle, Storage, Graphics, Threading

Cada módulo:
- Tem uma **API pública** clara via Facade
- Usa **componentes internos** bem definidos
- Emite **eventos** para comunicação desacoplada
- Implementa **interfaces** definidas no Core
- Segue **hierarquia de dependências** unidirecional
