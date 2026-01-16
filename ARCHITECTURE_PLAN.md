# Plano de Arquitetura: Monólito Modular SA-MP Mobile

## 1. Diagnóstico da Situação Atual

### 1.1 Estrutura Atual
```
app/src/main/cpp/samp/
├── main.cpp/h            # Entry point + globals misturados
├── audiostream.cpp/h     # Audio streaming
├── playertags.cpp/h      # Player tags rendering
├── settings.cpp/h        # Configurações
├── game/                 # ~408 arquivos - Lógica do GTA SA
├── gui/                  # ~51 arquivos - Interface ImGui
├── net/                  # ~28 arquivos - Networking SA-MP
├── voice_new/            # ~53 arquivos - Sistema de voz
├── java/                 # JNI wrappers
├── util/                 # Utilitários
└── vendor/               # Dependências externas (RakNet, ImGui, etc)
```

### 1.2 Problemas Identificados

#### A) Variáveis Globais Excessivas
```cpp
// main.cpp - Estado global exposto diretamente
extern char* g_pszStorage;
extern UI* pUI;
extern CGame* pGame;
extern CNetGame* pNetGame;
extern CPlayerTags* pPlayerTags;
extern CSnapShotHelper* pSnapShotHelper;
extern CAudioStream* pAudioStream;
extern CJavaWrapper* pJavaWrapper;
extern CSettings* pSettings;
```

**Problema**: Qualquer arquivo pode acessar e modificar qualquer estado, criando acoplamento total.

#### B) Dependências Cruzadas
```cpp
// netgame.cpp depende de:
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

**Problema**: O módulo de rede conhece GUI, voz, audio e JNI diretamente.

#### C) Potenciais Dependências Circulares
```
game/ ←→ net/           # Game precisa de net (sync), net precisa de game (entities)
gui/  ←→ net/           # GUI mostra dados de net, net notifica GUI
gui/  ←→ game/          # GUI controla game, game atualiza GUI
voice/ ←→ net/          # Voice usa network, network controla voice
```

#### D) Mistura de Responsabilidades
- `main.cpp`: Inicialização + logging + signal handlers + JNI handlers
- `gui.cpp`: Renderização + lógica de negócio + estado
- `netgame.cpp`: Conexão + pools + processamento de pacotes

#### E) Camada game/ Desorganizada
```
game/
├── Arquivos raiz (game.cpp, playerped.h, vehicle.h, etc)
├── Core/               # Estruturas de dados básicas
├── Entity/             # Entidades GTA
├── Animation/          # Sistema de animações
├── Collision/          # Sistema de colisão
├── RW/                 # RenderWare bindings
├── Tasks/              # AI Tasks
├── Models/             # Model info
├── Textures/           # Sistema de texturas
├── Widgets/            # Widgets do jogo (toque)
└── ... (muitas outras pastas)
```

---

## 2. Proposta: Arquitetura Monólito Modular

### 2.1 Princípios

1. **Módulos Independentes**: Cada módulo compila separadamente
2. **Dependências Unidirecionais**: Dependências fluem em uma direção
3. **Interfaces Claras**: Comunicação via interfaces/abstrações
4. **Inversão de Dependência**: Módulos de alto nível não dependem de baixo nível

### 2.2 Estrutura Proposta

```
app/src/main/cpp/
├── core/                      # MÓDULO CORE (sem dependências externas)
│   ├── types/                 # Tipos básicos (Vector, Matrix, Quaternion)
│   ├── memory/                # Memory management (Pool, Allocators)
│   ├── containers/            # Containers (Lists, Maps, etc)
│   ├── math/                  # Math utilities
│   ├── logging/               # Sistema de logging
│   └── interfaces/            # Interfaces base (IUpdatable, IRenderable)
│
├── platform/                  # MÓDULO PLATFORM (Android/JNI)
│   ├── android/               # Android-specific code
│   │   ├── jni_bridge.cpp/h   # JNI wrapper limpo
│   │   └── storage.cpp/h      # File system access
│   ├── input/                 # Input handling (multitouch)
│   └── hooks/                 # Hooking framework
│
├── engine/                    # MÓDULO ENGINE (Abstração do GTA SA)
│   ├── entities/              # Entidades abstratas
│   │   ├── entity.h           # Interface base
│   │   ├── ped.cpp/h          # Ped abstração
│   │   ├── vehicle.cpp/h      # Vehicle abstração
│   │   └── object.cpp/h       # Object abstração
│   ├── world/                 # World management
│   ├── camera/                # Camera system
│   ├── rendering/             # Rendering abstractions
│   ├── animation/             # Animation system
│   ├── collision/             # Collision system
│   └── game_interface.h       # Interface principal do engine
│
├── gta_bindings/              # MÓDULO GTA BINDINGS (Acesso direto ao GTA)
│   ├── gta_entities/          # GTA entity implementations
│   ├── gta_pools/             # GTA pools access
│   ├── gta_renderware/        # RenderWare bindings
│   ├── gta_patches/           # Patches e hooks do GTA
│   └── gta_bridge.cpp/h       # Bridge para o engine
│
├── network/                   # MÓDULO NETWORK
│   ├── transport/             # RakNet wrapper
│   │   ├── connection.cpp/h
│   │   └── packet_handler.cpp/h
│   ├── protocol/              # Protocolo SA-MP
│   │   ├── packets/           # Definições de pacotes
│   │   ├── rpc/               # RPC handlers
│   │   └── sync/              # Sync handlers
│   ├── pools/                 # Network entity pools
│   │   ├── player_pool.cpp/h
│   │   ├── vehicle_pool.cpp/h
│   │   └── ...
│   ├── events/                # Eventos de rede (interfaces)
│   │   └── network_events.h   # INetworkEventListener
│   └── network_manager.cpp/h  # Fachada principal
│
├── multiplayer/               # MÓDULO MULTIPLAYER (Lógica SA-MP)
│   ├── local_player.cpp/h     # Jogador local
│   ├── remote_player.cpp/h    # Jogadores remotos
│   ├── sync/                  # Sincronização
│   │   ├── onfoot_sync.cpp/h
│   │   ├── vehicle_sync.cpp/h
│   │   └── aim_sync.cpp/h
│   ├── features/              # Features SA-MP
│   │   ├── textdraws.cpp/h
│   │   ├── gangzones.cpp/h
│   │   ├── pickups.cpp/h
│   │   └── ...
│   └── multiplayer_manager.h  # Fachada
│
├── voice/                     # MÓDULO VOICE
│   ├── audio/                 # Audio processing
│   │   ├── playback.cpp/h
│   │   ├── record.cpp/h
│   │   └── effects.cpp/h
│   ├── streams/               # Voice streams
│   │   ├── stream.cpp/h
│   │   ├── local_stream.cpp/h
│   │   └── global_stream.cpp/h
│   ├── network/               # Voice networking
│   └── voice_manager.cpp/h    # Fachada
│
├── ui/                        # MÓDULO UI
│   ├── imgui_backend/         # ImGui rendering
│   ├── widgets/               # Widgets base
│   │   ├── widget.cpp/h
│   │   ├── button.cpp/h
│   │   ├── label.cpp/h
│   │   └── ...
│   ├── screens/               # Telas/Componentes SA-MP
│   │   ├── chat.cpp/h
│   │   ├── dialog.cpp/h
│   │   ├── spawn_screen.cpp/h
│   │   ├── keyboard.cpp/h
│   │   └── ...
│   ├── settings/              # UI settings
│   └── ui_manager.cpp/h       # Fachada
│
├── audio/                     # MÓDULO AUDIO
│   ├── bass_wrapper/          # BASS library wrapper
│   ├── audio_stream.cpp/h     # Audio streaming
│   └── audio_manager.cpp/h
│
├── config/                    # MÓDULO CONFIG
│   ├── settings.cpp/h
│   └── config_manager.cpp/h
│
├── services/                  # MÓDULO SERVICES (Locator Pattern)
│   ├── service_locator.cpp/h  # Registro de serviços
│   └── interfaces/            # Interfaces de serviços
│       ├── i_game_service.h
│       ├── i_network_service.h
│       ├── i_ui_service.h
│       └── ...
│
├── app/                       # MÓDULO APPLICATION
│   ├── application.cpp/h      # Entry point limpo
│   ├── game_loop.cpp/h        # Main loop
│   └── bootstrap.cpp/h        # Inicialização/DI
│
└── vendor/                    # DEPENDÊNCIAS EXTERNAS (não modificar)
    ├── raknet/
    ├── imgui/
    ├── bass/
    └── ...
```

### 2.3 Diagrama de Dependências

```
                    ┌─────────────────┐
                    │      app        │
                    │  (Application)  │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
              ▼              ▼              ▼
        ┌──────────┐  ┌───────────┐  ┌──────────┐
        │ services │  │multiplayer│  │   ui     │
        └────┬─────┘  └─────┬─────┘  └────┬─────┘
             │              │              │
             │       ┌──────┴──────┐       │
             │       │             │       │
             ▼       ▼             ▼       ▼
        ┌──────────────┐     ┌───────────────┐
        │   network    │     │    voice      │
        └──────┬───────┘     └───────┬───────┘
               │                     │
               └──────────┬──────────┘
                          │
                          ▼
                   ┌─────────────┐
                   │   engine    │
                   └──────┬──────┘
                          │
              ┌───────────┼───────────┐
              │           │           │
              ▼           ▼           ▼
        ┌───────────┐ ┌────────┐ ┌─────────┐
        │gta_bindings│ │platform│ │  audio  │
        └─────┬─────┘ └────┬───┘ └────┬────┘
              │            │          │
              └────────────┴──────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │    core     │
                    └─────────────┘
```

**Regras de Dependência:**
- Setas apontam para dependências
- Módulos só podem depender de módulos abaixo na hierarquia
- Comunicação lateral via interfaces/eventos

---

## 3. Padrões de Design a Implementar

### 3.1 Service Locator (Substituir Globals)

```cpp
// services/service_locator.h
class ServiceLocator {
public:
    static void Initialize();
    static void Shutdown();
    
    // Registro de serviços
    template<typename T>
    static void Register(std::shared_ptr<T> service);
    
    // Obtenção de serviços
    template<typename T>
    static T* Get();
    
private:
    static std::unordered_map<std::type_index, std::shared_ptr<void>> services;
};

// Uso:
// Antes: pNetGame->GetPlayerPool()
// Depois: ServiceLocator::Get<INetworkService>()->GetPlayerPool()
```

### 3.2 Event Bus (Comunicação Entre Módulos)

```cpp
// core/events/event_bus.h
class EventBus {
public:
    template<typename EventType>
    static void Subscribe(std::function<void(const EventType&)> handler);
    
    template<typename EventType>
    static void Publish(const EventType& event);
};

// Eventos definidos por módulo:
// network/events/network_events.h
struct PlayerConnectedEvent {
    uint16_t playerId;
    std::string playerName;
};

struct PlayerDisconnectedEvent {
    uint16_t playerId;
    uint8_t reason;
};

// Uso no UI:
EventBus::Subscribe<PlayerConnectedEvent>([](const auto& e) {
    chat->addMessage("Player connected: " + e.playerName);
});
```

### 3.3 Interfaces para Inversão de Dependência

```cpp
// services/interfaces/i_game_service.h
class IGameService {
public:
    virtual ~IGameService() = default;
    virtual IPlayerPed* GetLocalPlayer() = 0;
    virtual void SetWorldTime(int hour, int minute) = 0;
    virtual void SetWeather(int weatherId) = 0;
    // ...
};

// engine/game_service.cpp
class GameService : public IGameService {
    // Implementação usando gta_bindings
};
```

### 3.4 Fachadas por Módulo

```cpp
// network/network_manager.h
class NetworkManager {
public:
    // API pública limpa
    bool Connect(const std::string& host, int port, const std::string& password);
    void Disconnect();
    void Process();
    
    // Pools
    IPlayerPool* GetPlayerPool();
    IVehiclePool* GetVehiclePool();
    
    // Envio
    void SendChatMessage(const std::string& message);
    void SendCommand(const std::string& command);
    
private:
    // Implementação escondida
    std::unique_ptr<RakClientWrapper> m_client;
    std::unique_ptr<PlayerPool> m_playerPool;
    // ...
};
```

---

## 4. Plano de Migração (Fases)

### Fase 1: Preparação (Sem quebrar código existente)

1. **Criar estrutura de pastas**
   - Criar pastas vazias conforme nova estrutura
   - Não mover nenhum arquivo ainda

2. **Criar módulo `core`**
   - Extrair tipos básicos (Vector, Matrix, Quaternion) para core/types
   - Extrair sistema de logging para core/logging
   - Criar interfaces base

3. **Criar módulo `services`**
   - Implementar ServiceLocator básico
   - Registrar serviços existentes como wrappers dos globals

### Fase 2: Extração de Módulos Independentes

4. **Migrar `config`**
   - Mover CSettings para config/
   - Adaptar para usar ServiceLocator

5. **Migrar `audio`**
   - Mover CAudioStream para audio/
   - Criar AudioManager como fachada

6. **Migrar `platform`**
   - Mover código JNI para platform/android
   - Mover hooks para platform/hooks

### Fase 3: Refatorar Módulos Complexos

7. **Separar `gta_bindings` de `engine`**
   - Criar interfaces em engine/
   - Mover implementações GTA para gta_bindings/
   - Bridge conecta as camadas

8. **Refatorar `network`**
   - Extrair transporte (RakNet) para transport/
   - Criar interfaces de eventos
   - Pools viram entidades network

9. **Refatorar `ui`**
   - Separar widgets genéricos de telas SA-MP
   - Criar UIManager como fachada
   - Comunicação via eventos

10. **Refatorar `voice`**
    - Separar audio processing de network
    - Criar VoiceManager

### Fase 4: Integração Final

11. **Criar `multiplayer`**
    - Mover lógica de LocalPlayer
    - Mover lógica de sincronização
    - Integrar com network e engine

12. **Criar `app`**
    - Mover inicialização de main.cpp para application.cpp
    - Bootstrap configura todos os serviços
    - Main loop limpo

13. **Limpeza**
    - Remover globals restantes
    - Remover arquivos obsoletos
    - Documentar APIs públicas

---

## 5. Estrutura de CMake Modular

```cmake
# CMakeLists.txt principal
cmake_minimum_required(VERSION 3.18)
project(samp_mobile)

# Módulos como bibliotecas estáticas
add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(engine)
add_subdirectory(gta_bindings)
add_subdirectory(network)
add_subdirectory(multiplayer)
add_subdirectory(voice)
add_subdirectory(ui)
add_subdirectory(audio)
add_subdirectory(config)
add_subdirectory(services)
add_subdirectory(app)

# Biblioteca final
add_library(samp SHARED
    $<TARGET_OBJECTS:app>
)

target_link_libraries(samp
    app
    multiplayer
    network
    ui
    voice
    audio
    engine
    gta_bindings
    platform
    config
    services
    core
)
```

```cmake
# Exemplo: network/CMakeLists.txt
add_library(network STATIC
    transport/connection.cpp
    transport/packet_handler.cpp
    protocol/rpc/rpc_handlers.cpp
    pools/player_pool.cpp
    pools/vehicle_pool.cpp
    network_manager.cpp
)

target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Dependências
target_link_libraries(network
    PUBLIC core
    PUBLIC engine
    PRIVATE vendor_raknet
)
```

---

## 6. Benefícios Esperados

### Manutenibilidade
- Código organizado por responsabilidade
- Fácil localizar onde fazer mudanças
- Menos risco de quebrar funcionalidades não relacionadas

### Testabilidade
- Módulos podem ser testados isoladamente
- Interfaces permitem mocks
- Menos estado global = menos side effects

### Escalabilidade
- Novos recursos adicionados em módulos específicos
- Compilação incremental mais rápida
- Múltiplos desenvolvedores podem trabalhar em paralelo

### Reutilização
- Módulos como `core`, `platform` podem ser reutilizados
- Padrões consistentes em todo o projeto

---

## 7. Riscos e Mitigações

| Risco | Probabilidade | Impacto | Mitigação |
|-------|---------------|---------|-----------|
| Regressões durante migração | Alta | Alto | Migração incremental, testes manuais após cada fase |
| Aumento inicial de complexidade | Média | Médio | Documentação clara, padrões consistentes |
| Overhead de abstração | Baixa | Baixo | Usar inline para hot paths, profiling |
| Dependências circulares acidentais | Média | Alto | CI com verificação de dependências |

---

## 8. Próximos Passos

1. **Revisar** este plano e ajustar conforme necessário
2. **Aprovar** a abordagem geral
3. **Iniciar Fase 1**: Criar estrutura básica + módulo core
4. **Iterar** através das fases, validando a cada passo

---

*Documento gerado em: Janeiro 2026*
*Versão: 1.0*
