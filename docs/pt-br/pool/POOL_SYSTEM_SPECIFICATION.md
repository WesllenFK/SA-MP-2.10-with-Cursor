# Sistema de Sinuca - Especificação Técnica Completa

**Versão**: 1.0  
**Última atualização**: Janeiro 2026  
**Compatibilidade**: Android NDK, open.mp, SA-MP 0.3.7+

---

## Índice

1. [Visão Geral](#1-visão-geral)
2. [Arquitetura do Sistema](#2-arquitetura-do-sistema)
3. [Estruturas de Dados](#3-estruturas-de-dados)
4. [Sistema de Streaming](#4-sistema-de-streaming)
5. [Client-Side (C++)](#5-client-side-c)
6. [Server-Side (open.mp/PAWN)](#6-server-side-openmpawn)
7. [Hooks Nativos](#7-hooks-nativos)
8. [Tabela de Offsets](#8-tabela-de-offsets)
9. [Sistema de Erros](#9-sistema-de-erros)
10. [RPCs e Comunicação](#10-rpcs-e-comunicação)
11. [Arquivos do Sistema](#11-arquivos-do-sistema)
12. [Guia de Integração](#12-guia-de-integração)

---

## 1. Visão Geral

### 1.1 Objetivo

Sistema modular de sinuca (pool/bilhar) para SA-MP/open.mp que permite:
- Múltiplas mesas em qualquer posição do mapa
- Física sincronizada entre jogadores
- Interface de mira e tacada
- Streaming baseado em distância

### 1.2 Princípios de Design

| Princípio | Descrição |
|-----------|-----------|
| **Modularidade** | Não depende de sistemas externos (admin, saves, etc.) |
| **Segurança** | Nunca crashar; erros são reportados e operação é cancelada |
| **Eficiência** | Streaming por distância; apenas jogadores próximos recebem dados |
| **Determinismo** | Mesmos parâmetros de tacada = mesmo resultado em todos clientes |

### 1.3 Requisitos

**Client-Side:**
- Android NDK r21+
- C++17
- ImGui (para UI)
- ShadowHook ou similar (para hooks)

**Server-Side:**
- open.mp ou SA-MP 0.3.7+
- PAWN ou SDK C++ do open.mp

---

## 2. Arquitetura do Sistema

### 2.1 Diagrama Geral

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ARQUITETURA GERAL                           │
└─────────────────────────────────────────────────────────────────────┘

                              ┌─────────────┐
                              │   open.mp   │
                              │   SERVER    │
                              └──────┬──────┘
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
                    ▼                ▼                ▼
            ┌───────────┐    ┌───────────┐    ┌───────────┐
            │PoolServer │    │PoolServer │    │PoolServer │
            │ Manager   │    │  Table    │    │ Streamer  │
            └───────────┘    └───────────┘    └───────────┘
                    │                │                │
                    └────────────────┼────────────────┘
                                     │
                              ┌──────┴──────┐
                              │     RPC     │
                              │   NETWORK   │
                              └──────┬──────┘
                                     │
                    ┌────────────────┼────────────────┐
                    │                │                │
                    ▼                ▼                ▼
            ┌───────────┐    ┌───────────┐    ┌───────────┐
            │   Pool    │    │   Pool    │    │   Pool    │
            │  Manager  │    │  Camera   │    │ Physics   │
            └───────────┘    └───────────┘    └───────────┘
                    │                │                │
                    ▼                ▼                ▼
            ┌───────────┐    ┌───────────┐    ┌───────────┐
            │  PoolUI   │    │  PoolUI   │    │  Native   │
            │  (ImGui)  │    │  Render   │    │   Hooks   │
            └───────────┘    └───────────┘    └───────────┘
                                     │
                              ┌──────┴──────┐
                              │   libGTASA  │
                              │   (Native)  │
                              └─────────────┘
```

### 2.2 Camadas

| Camada | Localização | Responsabilidade |
|--------|-------------|------------------|
| **PoolServerManager** | Server | Gerenciar todas as mesas, coordenar |
| **PoolServerTable** | Server | Estado de uma mesa específica |
| **PoolServerStreamer** | Server | Controle de distância e broadcast |
| **PoolManager** | Client | Gerenciar mesas locais |
| **PoolCamera** | Client | Câmera de mira |
| **PoolPhysics** | Client | Hooks de física |
| **PoolUI** | Client | Interface ImGui |
| **PoolError** | Ambos | Sistema de erros |

---

## 3. Estruturas de Dados

### 3.1 Constantes

```cpp
// pool_constants.h

namespace Pool {
    // Limites
    constexpr uint16_t MAX_TABLES = 100;
    constexpr uint8_t  MAX_BALLS = 16;
    constexpr uint16_t INVALID_TABLE_ID = 0xFFFF;
    constexpr uint16_t INVALID_PLAYER_ID = 0xFFFF;
    
    // Distâncias (metros)
    constexpr float PLAY_RANGE = 5.0f;      // Pode tacar
    constexpr float VIEW_RANGE = 25.0f;     // Vê atualizações
    constexpr float STREAM_RANGE = 50.0f;   // Objetos existem
    
    // Física
    constexpr float MAX_SHOT_POWER = 20.0f;
    constexpr float MIN_SHOT_POWER = 1.0f;
    constexpr float BALL_RADIUS = 0.028f;   // ~2.8cm (bola de sinuca real)
    constexpr float VELOCITY_THRESHOLD = 0.001f;  // Considerado parado
    
    // Timing
    constexpr uint32_t STREAM_CHECK_INTERVAL = 200;  // ms
    constexpr uint32_t SHOOTER_TIMEOUT = 30000;      // ms
    constexpr uint32_t PHYSICS_SYNC_INTERVAL = 50;   // ms durante movimento
    
    // Model IDs
    constexpr uint16_t MODEL_TABLE_LARGE = 3094;     // K_POOLTABLEB
    constexpr uint16_t MODEL_TABLE_SMALL = 2964;     // K_POOLTABLESM
    constexpr uint16_t MODEL_BALL_CUE = 3003;        // K_POOLBALLCUE
    constexpr uint16_t MODEL_BALL_8 = 3106;          // K_POOLBALL8
    constexpr uint16_t MODEL_BALL_SOLID_1 = 2995;    // K_POOLBALLSTP01
    constexpr uint16_t MODEL_BALL_STRIPE_1 = 3002;   // K_POOLBALLSPT01
    constexpr uint16_t MODEL_CUE_STICK = 338;        // POOLCUE
}
```

### 3.2 Estruturas Compartilhadas

```cpp
// pool_types.h

#pragma once
#include <cstdint>

namespace Pool {

// Vetor 3D simples (compatível com CVector do GTA)
struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    float Length() const { 
        return sqrtf(x*x + y*y + z*z); 
    }
    
    Vec3 Normalized() const {
        float len = Length();
        if (len < 0.0001f) return Vec3(0, 0, 0);
        return Vec3(x/len, y/len, z/len);
    }
    
    Vec3 operator*(float f) const { return Vec3(x*f, y*f, z*f); }
    Vec3 operator+(const Vec3& v) const { return Vec3(x+v.x, y+v.y, z+v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x-v.x, y-v.y, z-v.z); }
};

// Estado de uma bola
struct BallState {
    bool active;        // Em jogo?
    Vec3 position;      // Posição atual
    Vec3 velocity;      // Velocidade (para sync durante movimento)
    
    BallState() : active(true), position(), velocity() {}
};

// Dados de uma tacada
struct ShotData {
    Vec3 direction;     // Direção normalizada
    float power;        // 0.0 - 1.0
    float englishX;     // -1.0 a 1.0 (spin horizontal)
    float englishY;     // -1.0 a 1.0 (spin vertical)
    Vec3 cueBallPos;    // Posição da bola branca no momento
    
    ShotData() : direction(), power(0), englishX(0), englishY(0), cueBallPos() {}
};

// Estado completo de uma mesa
struct TableState {
    uint16_t tableID;
    Vec3 position;
    float rotation;
    BallState balls[MAX_BALLS];
    bool inMotion;
    uint16_t currentShooter;
    uint32_t lastUpdateTime;
    
    TableState() : tableID(INVALID_TABLE_ID), position(), rotation(0),
                   inMotion(false), currentShooter(INVALID_PLAYER_ID), 
                   lastUpdateTime(0) {
        for (int i = 0; i < MAX_BALLS; i++) {
            balls[i] = BallState();
        }
    }
};

// Resultado de operação
enum class Result : uint8_t {
    Success = 0,
    ErrorTableNotFound,
    ErrorPlayerNotInRange,
    ErrorTableInMotion,
    ErrorAlreadyShooting,
    ErrorInvalidParameters,
    ErrorNativeHookFailed,
    ErrorObjectCreationFailed,
    ErrorUnknown
};

// Converter resultado para string
inline const char* ResultToString(Result r) {
    switch (r) {
        case Result::Success: return "Success";
        case Result::ErrorTableNotFound: return "Table not found";
        case Result::ErrorPlayerNotInRange: return "Player not in range";
        case Result::ErrorTableInMotion: return "Table in motion";
        case Result::ErrorAlreadyShooting: return "Already shooting";
        case Result::ErrorInvalidParameters: return "Invalid parameters";
        case Result::ErrorNativeHookFailed: return "Native hook failed";
        case Result::ErrorObjectCreationFailed: return "Object creation failed";
        default: return "Unknown error";
    }
}

} // namespace Pool
```

### 3.3 Callbacks (Interface para Sistema Externo)

```cpp
// pool_callbacks.h

#pragma once
#include "pool_types.h"

namespace Pool {

// Interface de callbacks que o sistema externo deve implementar
class IPoolCallbacks {
public:
    virtual ~IPoolCallbacks() = default;
    
    // Chamado quando ocorre um erro
    virtual void OnPoolError(Result error, const char* message) = 0;
    
    // Chamado quando jogador entra no range de uma mesa
    virtual void OnPlayerEnterTableRange(uint16_t playerID, uint16_t tableID) = 0;
    
    // Chamado quando jogador sai do range
    virtual void OnPlayerExitTableRange(uint16_t playerID, uint16_t tableID) = 0;
    
    // Chamado quando uma tacada é executada
    virtual void OnShotExecuted(uint16_t playerID, uint16_t tableID, const ShotData& shot) = 0;
    
    // Chamado quando bolas param de mover
    virtual void OnBallsSettled(uint16_t tableID, const TableState& state) = 0;
    
    // Chamado quando bola é encaçapada (opcional)
    virtual void OnBallPocketed(uint16_t tableID, uint8_t ballIndex) = 0;
};

// Callback padrão que apenas loga erros
class DefaultPoolCallbacks : public IPoolCallbacks {
public:
    void OnPoolError(Result error, const char* message) override {
        // Log para sistema de debug
        // Não faz nada além de logar
    }
    
    void OnPlayerEnterTableRange(uint16_t, uint16_t) override {}
    void OnPlayerExitTableRange(uint16_t, uint16_t) override {}
    void OnShotExecuted(uint16_t, uint16_t, const ShotData&) override {}
    void OnBallsSettled(uint16_t, const TableState&) override {}
    void OnBallPocketed(uint16_t, uint8_t) override {}
};

} // namespace Pool
```

---

## 4. Sistema de Streaming

### 4.1 Conceito

O sistema usa três zonas de distância para otimização:

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ZONAS DE STREAMING                               │
└─────────────────────────────────────────────────────────────────────┘

    STREAM_RANGE (50m)
    ┌─────────────────────────────────────────────────────────┐
    │                                                         │
    │   VIEW_RANGE (25m)                                      │
    │   ┌─────────────────────────────────────────────┐       │
    │   │                                             │       │
    │   │   PLAY_RANGE (5m)                           │       │
    │   │   ┌─────────────────────────────┐           │       │
    │   │   │                             │           │       │
    │   │   │        ┌───────┐            │           │       │
    │   │   │        │ MESA  │            │           │       │
    │   │   │        └───────┘            │           │       │
    │   │   │                             │           │       │
    │   │   │  • Pode tacar               │           │       │
    │   │   │  • Vê UI completa           │           │       │
    │   │   │  • Recebe todos updates     │           │       │
    │   │   └─────────────────────────────┘           │       │
    │   │                                             │       │
    │   │  • Vê bolas movendo                         │       │
    │   │  • Recebe updates de física                 │       │
    │   │  • NÃO pode tacar                           │       │
    │   └─────────────────────────────────────────────┘       │
    │                                                         │
    │  • Objetos existem (streamed)                           │
    │  • NÃO recebe updates de física                         │
    │  • Bolas ficam na última posição conhecida              │
    └─────────────────────────────────────────────────────────┘
    
    FORA DO STREAM_RANGE:
    • Objetos não existem localmente
    • Nenhum processamento
```

### 4.2 Estrutura do Streamer (Server)

```cpp
// pool_streamer.h (conceito - implementação real em PAWN/open.mp)

namespace Pool {

struct PlayerTableInfo {
    bool inStreamRange;
    bool inViewRange;
    bool inPlayRange;
    uint32_t lastUpdate;
};

class PoolStreamer {
public:
    // Chamado periodicamente pelo servidor
    void Update();
    
    // Verificar ranges de um jogador
    void CheckPlayerRanges(uint16_t playerID);
    
    // Obter jogadores em cada range
    std::vector<uint16_t> GetPlayersInPlayRange(uint16_t tableID);
    std::vector<uint16_t> GetPlayersInViewRange(uint16_t tableID);
    
private:
    // Mapa: tableID -> (playerID -> info)
    std::unordered_map<uint16_t, std::unordered_map<uint16_t, PlayerTableInfo>> m_playerInfo;
    
    float CalculateDistance(uint16_t playerID, uint16_t tableID);
    void OnPlayerEnterRange(uint16_t playerID, uint16_t tableID, float distance);
    void OnPlayerExitRange(uint16_t playerID, uint16_t tableID);
};

} // namespace Pool
```

---

## 5. Client-Side (C++)

### 5.1 PoolManager

```cpp
// pool_manager.h

#pragma once
#include "pool_types.h"
#include "pool_callbacks.h"
#include "pool_table_local.h"
#include "pool_camera.h"
#include "pool_physics.h"
#include "pool_ui.h"
#include "pool_error.h"
#include <unordered_map>
#include <memory>

namespace Pool {

class PoolManager {
public:
    // Singleton
    static PoolManager& Instance() {
        static PoolManager instance;
        return instance;
    }
    
    // Inicialização
    Result Initialize(IPoolCallbacks* callbacks = nullptr);
    void Shutdown();
    bool IsInitialized() const { return m_initialized; }
    
    // Atualização (chamar todo frame)
    void Update(float deltaTime);
    
    // Gerenciamento de mesas locais
    Result OnTableStreamIn(uint16_t tableID, const TableState& state);
    Result OnTableStreamOut(uint16_t tableID);
    Result OnEnterViewRange(uint16_t tableID);
    Result OnExitViewRange(uint16_t tableID);
    Result OnEnterPlayRange(uint16_t tableID);
    Result OnExitPlayRange(uint16_t tableID);
    
    // Tacada
    Result RequestShoot(uint16_t tableID);
    Result OnShootApproved(uint16_t tableID);
    Result OnShootDenied(uint16_t tableID, Result reason);
    Result ExecuteShot(uint16_t tableID, const ShotData& shot);
    Result OnReceiveShot(uint16_t tableID, const ShotData& shot, uint16_t shooterID);
    
    // Estado
    Result OnReceiveTableState(uint16_t tableID, const TableState& state);
    
    // Posicionar bola branca
    Result StartBallInHand(uint16_t tableID);
    Result ConfirmBallPosition(uint16_t tableID, const Vec3& position);
    
    // Acesso
    LocalPoolTable* GetTable(uint16_t tableID);
    uint16_t GetActiveTableID() const { return m_activeTableID; }
    bool IsPlayerShooting() const { return m_isShooting; }
    
    // Callbacks
    void SetCallbacks(IPoolCallbacks* callbacks) { m_callbacks = callbacks; }
    
private:
    PoolManager();
    ~PoolManager();
    PoolManager(const PoolManager&) = delete;
    PoolManager& operator=(const PoolManager&) = delete;
    
    bool m_initialized;
    IPoolCallbacks* m_callbacks;
    DefaultPoolCallbacks m_defaultCallbacks;
    
    std::unordered_map<uint16_t, std::unique_ptr<LocalPoolTable>> m_tables;
    uint16_t m_activeTableID;
    bool m_isShooting;
    
    PoolCamera m_camera;
    PoolUI m_ui;
    
    void ReportError(Result error, const char* context);
    void CheckBallsMotion();
};

} // namespace Pool
```

### 5.2 LocalPoolTable

```cpp
// pool_table_local.h

#pragma once
#include "pool_types.h"

namespace Pool {

// Forward declarations
class CObject;
class CPhysical;

struct LocalBall {
    bool active;
    uint16_t objectID;          // ID do objeto SA-MP
    CObject* pObject;           // Ponteiro nativo
    CPhysical* pPhysical;       // Ponteiro para física
    Vec3 position;
    Vec3 velocity;
    
    LocalBall() : active(false), objectID(0), pObject(nullptr), 
                  pPhysical(nullptr), position(), velocity() {}
};

class LocalPoolTable {
public:
    LocalPoolTable(uint16_t tableID);
    ~LocalPoolTable();
    
    // Inicialização
    Result Create(const TableState& state);
    void Destroy();
    
    // Estado
    uint16_t GetTableID() const { return m_tableID; }
    bool IsInViewRange() const { return m_inViewRange; }
    bool IsInPlayRange() const { return m_inPlayRange; }
    bool IsInMotion() const { return m_inMotion; }
    
    void SetInViewRange(bool value) { m_inViewRange = value; }
    void SetInPlayRange(bool value) { m_inPlayRange = value; }
    
    // Física
    Result ApplyShot(const ShotData& shot);
    void UpdatePhysics(float deltaTime);
    bool CheckAllBallsStopped();
    
    // Acesso às bolas
    LocalBall* GetBall(uint8_t index);
    LocalBall* GetCueBall() { return GetBall(0); }
    
    // Posição
    const Vec3& GetPosition() const { return m_position; }
    float GetRotation() const { return m_rotation; }
    
    // Sincronização
    void ApplyState(const TableState& state);
    TableState GetCurrentState() const;
    
private:
    uint16_t m_tableID;
    uint16_t m_tableObjectID;
    CObject* m_pTableObject;
    
    Vec3 m_position;
    float m_rotation;
    
    LocalBall m_balls[MAX_BALLS];
    
    bool m_inViewRange;
    bool m_inPlayRange;
    bool m_inMotion;
    
    Result CreateTableObject(const Vec3& pos, float rot);
    Result CreateBallObjects(const TableState& state);
    void DestroyAllObjects();
    
    bool ValidateNativePointers();
};

} // namespace Pool
```

### 5.3 PoolCamera

```cpp
// pool_camera.h

#pragma once
#include "pool_types.h"

namespace Pool {

class LocalPoolTable;

class PoolCamera {
public:
    PoolCamera();
    ~PoolCamera();
    
    // Ativação
    Result Activate(LocalPoolTable* table);
    void Deactivate();
    bool IsActive() const { return m_active; }
    
    // Atualização
    void Update(float deltaTime);
    
    // Controles
    void AddRotation(float delta);
    void SetHeight(float height);
    void SetDistance(float distance);
    
    // Mira
    Vec3 GetAimDirection() const;
    float GetAimAngle() const { return m_angle; }
    
    // Renderização auxiliar
    void RenderAimLine();
    
private:
    bool m_active;
    LocalPoolTable* m_table;
    
    float m_angle;          // Rotação horizontal (radianos)
    float m_height;         // Altura acima da mesa
    float m_distance;       // Distância da bola branca
    
    // Limites
    static constexpr float MIN_HEIGHT = 1.0f;
    static constexpr float MAX_HEIGHT = 5.0f;
    static constexpr float MIN_DISTANCE = 1.0f;
    static constexpr float MAX_DISTANCE = 4.0f;
    
    // Hooks de câmera
    static bool s_hooksInstalled;
    static Result InstallCameraHooks();
    static void RemoveCameraHooks();
    
    void ApplyCameraTransform();
    void RestoreNormalCamera();
};

} // namespace Pool
```

### 5.4 PoolPhysics

```cpp
// pool_physics.h

#pragma once
#include "pool_types.h"

namespace Pool {

// Forward declarations
class CPhysical;
class CVector;

class PoolPhysics {
public:
    // Inicialização de hooks
    static Result Initialize();
    static void Shutdown();
    static bool IsInitialized() { return s_initialized; }
    
    // Aplicar física
    static Result ApplyForce(CPhysical* physical, const Vec3& force);
    static Result ApplyForceAtPoint(CPhysical* physical, const Vec3& force, const Vec3& point);
    static Result ApplyMoveForce(CPhysical* physical, const Vec3& force);
    static Result ApplyTurnForce(CPhysical* physical, const Vec3& force, const Vec3& point);
    
    // Consultas
    static Result GetVelocity(CPhysical* physical, Vec3& outVelocity);
    static Result GetPosition(CPhysical* physical, Vec3& outPosition);
    static Result GetSpeed(CPhysical* physical, float& outSpeed);
    
    // Utilitários
    static bool IsStopped(CPhysical* physical);
    
    // Aplicar tacada completa
    static Result ApplyShot(CPhysical* cueBall, const ShotData& shot);
    
private:
    static bool s_initialized;
    static uintptr_t s_libBase;
    
    // Ponteiros para funções nativas
    struct NativeFunctions {
        uintptr_t ApplyForce;
        uintptr_t ApplyMoveForce;
        uintptr_t ApplyTurnForce;
        uintptr_t GetSpeed;
        uintptr_t ApplyFriction;
        uintptr_t ApplyGravity;
        
        NativeFunctions() : ApplyForce(0), ApplyMoveForce(0), ApplyTurnForce(0),
                           GetSpeed(0), ApplyFriction(0), ApplyGravity(0) {}
    };
    
    static NativeFunctions s_natives;
    
    static Result ResolveOffsets();
    static bool ValidatePointer(void* ptr);
};

} // namespace Pool
```

### 5.5 PoolUI

```cpp
// pool_ui.h

#pragma once
#include "pool_types.h"

namespace Pool {

class LocalPoolTable;

class PoolUI {
public:
    PoolUI();
    ~PoolUI();
    
    // Estado
    void Show(LocalPoolTable* table);
    void Hide();
    bool IsVisible() const { return m_visible; }
    
    // Renderização (chamar dentro do frame ImGui)
    void Render();
    
    // Acesso aos valores
    float GetPower() const { return m_power; }
    float GetEnglishX() const { return m_englishX; }
    float GetEnglishY() const { return m_englishY; }
    
    // Reset
    void ResetValues();
    
    // Callbacks
    void SetOnShootCallback(std::function<void(float power, float engX, float engY)> callback) {
        m_onShoot = callback;
    }
    
private:
    bool m_visible;
    LocalPoolTable* m_table;
    
    float m_power;
    float m_englishX;
    float m_englishY;
    
    bool m_isCharging;      // Segurando botão de força
    uint32_t m_chargeStart; // Quando começou a carregar
    
    std::function<void(float, float, float)> m_onShoot;
    
    void RenderEnglishControl();
    void RenderPowerControl();
    void RenderShootButton();
    void RenderBallInHandUI();
};

} // namespace Pool
```

### 5.6 PoolError

```cpp
// pool_error.h

#pragma once
#include "pool_types.h"
#include <functional>
#include <string>

namespace Pool {

// Níveis de erro
enum class ErrorLevel : uint8_t {
    Debug,      // Apenas log
    Warning,    // Log + continua
    Error,      // Log + cancela operação
    Critical    // Log + pode crashar se ignorado
};

// Handler de erro
using ErrorHandler = std::function<void(ErrorLevel level, Result result, const char* message)>;

class PoolError {
public:
    // Configurar handler customizado
    static void SetErrorHandler(ErrorHandler handler) {
        s_handler = handler;
    }
    
    // Reportar erro
    static void Report(ErrorLevel level, Result result, const char* context) {
        char message[256];
        snprintf(message, sizeof(message), "[Pool] %s: %s", context, ResultToString(result));
        
        if (s_handler) {
            s_handler(level, result, message);
        } else {
            DefaultHandler(level, result, message);
        }
    }
    
    // Verificação com early return
    #define POOL_CHECK(condition, result, context) \
        do { \
            if (!(condition)) { \
                Pool::PoolError::Report(Pool::ErrorLevel::Error, result, context); \
                return result; \
            } \
        } while(0)
    
    // Verificação que apenas loga (não retorna)
    #define POOL_WARN_IF(condition, result, context) \
        do { \
            if (condition) { \
                Pool::PoolError::Report(Pool::ErrorLevel::Warning, result, context); \
            } \
        } while(0)
    
private:
    static ErrorHandler s_handler;
    
    static void DefaultHandler(ErrorLevel level, Result result, const char* message) {
        // Log padrão - implementar conforme sistema de log do projeto
        // Exemplo: __android_log_print(ANDROID_LOG_ERROR, "Pool", "%s", message);
    }
};

} // namespace Pool
```

---

## 6. Server-Side (open.mp/PAWN)

### 6.1 Estrutura de Arquivos

```
gamemodes/
└── pool/
    ├── pool_main.inc           // Include principal
    ├── pool_config.inc         // Configurações
    ├── pool_data.inc           // Estruturas de dados
    ├── pool_tables.inc         // Gerenciamento de mesas
    ├── pool_streaming.inc      // Sistema de streaming
    ├── pool_rpc.inc            // Comunicação com clientes
    └── pool_utils.inc          // Utilitários
```

### 6.2 pool_config.inc

```pawn
// pool_config.inc

#if defined _pool_config_included
    #endinput
#endif
#define _pool_config_included

// Limites
#define POOL_MAX_TABLES             100
#define POOL_MAX_BALLS              16

// Distâncias
#define POOL_PLAY_RANGE             5.0
#define POOL_VIEW_RANGE             25.0
#define POOL_STREAM_RANGE           50.0

// Timing
#define POOL_STREAM_CHECK_MS        200
#define POOL_SHOOTER_TIMEOUT_MS     30000

// IDs inválidos
#define POOL_INVALID_TABLE_ID       0xFFFF
#define POOL_INVALID_PLAYER_ID      0xFFFF

// Model IDs
#define POOL_MODEL_TABLE            3094
#define POOL_MODEL_BALL_CUE         3003
#define POOL_MODEL_BALL_8           3106
#define POOL_MODEL_BALL_SOLID_1     2995
#define POOL_MODEL_BALL_STRIPE_1    3002
```

### 6.3 pool_data.inc

```pawn
// pool_data.inc

#if defined _pool_data_included
    #endinput
#endif
#define _pool_data_included

// Estado de uma bola
enum E_POOL_BALL {
    bool:pball_Active,
    Float:pball_PosX,
    Float:pball_PosY,
    Float:pball_PosZ,
    Float:pball_VelX,
    Float:pball_VelY,
    Float:pball_VelZ
};

// Estado de uma mesa
enum E_POOL_TABLE {
    bool:ptable_Exists,
    Float:ptable_PosX,
    Float:ptable_PosY,
    Float:ptable_PosZ,
    Float:ptable_Rotation,
    ptable_ObjectID,
    ptable_BallObjects[POOL_MAX_BALLS],
    bool:ptable_InMotion,
    ptable_CurrentShooter,
    ptable_ShooterTimeout
};

// Dados de tacada
enum E_POOL_SHOT {
    Float:pshot_DirX,
    Float:pshot_DirY,
    Float:pshot_DirZ,
    Float:pshot_Power,
    Float:pshot_EnglishX,
    Float:pshot_EnglishY,
    Float:pshot_CuePosX,
    Float:pshot_CuePosY,
    Float:pshot_CuePosZ
};

// Informação de jogador por mesa
enum E_POOL_PLAYER_INFO {
    bool:pinfo_InStreamRange,
    bool:pinfo_InViewRange,
    bool:pinfo_InPlayRange
};

// Dados globais
new g_PoolTables[POOL_MAX_TABLES][E_POOL_TABLE];
new g_PoolBalls[POOL_MAX_TABLES][POOL_MAX_BALLS][E_POOL_BALL];
new g_PoolPlayerInfo[MAX_PLAYERS][POOL_MAX_TABLES][E_POOL_PLAYER_INFO];
new g_PoolTableCount = 0;
```

### 6.4 pool_tables.inc

```pawn
// pool_tables.inc

#if defined _pool_tables_included
    #endinput
#endif
#define _pool_tables_included

#include "pool_config.inc"
#include "pool_data.inc"

// Criar mesa
stock Pool_CreateTable(Float:x, Float:y, Float:z, Float:rotation = 0.0) {
    // Encontrar slot livre
    new tableID = -1;
    for (new i = 0; i < POOL_MAX_TABLES; i++) {
        if (!g_PoolTables[i][ptable_Exists]) {
            tableID = i;
            break;
        }
    }
    
    if (tableID == -1) {
        return POOL_INVALID_TABLE_ID; // Sem slots
    }
    
    // Configurar mesa
    g_PoolTables[tableID][ptable_Exists] = true;
    g_PoolTables[tableID][ptable_PosX] = x;
    g_PoolTables[tableID][ptable_PosY] = y;
    g_PoolTables[tableID][ptable_PosZ] = z;
    g_PoolTables[tableID][ptable_Rotation] = rotation;
    g_PoolTables[tableID][ptable_InMotion] = false;
    g_PoolTables[tableID][ptable_CurrentShooter] = POOL_INVALID_PLAYER_ID;
    
    // Criar objeto da mesa (não dinâmico - todos veem)
    g_PoolTables[tableID][ptable_ObjectID] = CreateDynamicObject(
        POOL_MODEL_TABLE, x, y, z, 0.0, 0.0, rotation,
        -1, -1, -1, POOL_STREAM_RANGE
    );
    
    // Inicializar bolas na formação padrão
    Pool_ResetBalls(tableID);
    
    g_PoolTableCount++;
    
    return tableID;
}

// Destruir mesa
stock Pool_DestroyTable(tableID) {
    if (tableID < 0 || tableID >= POOL_MAX_TABLES) return 0;
    if (!g_PoolTables[tableID][ptable_Exists]) return 0;
    
    // Destruir objetos
    DestroyDynamicObject(g_PoolTables[tableID][ptable_ObjectID]);
    
    for (new i = 0; i < POOL_MAX_BALLS; i++) {
        if (g_PoolBalls[tableID][i][pball_Active]) {
            DestroyDynamicObject(g_PoolTables[tableID][ptable_BallObjects][i]);
        }
    }
    
    // Limpar dados
    g_PoolTables[tableID][ptable_Exists] = false;
    g_PoolTableCount--;
    
    return 1;
}

// Resetar bolas para formação inicial
stock Pool_ResetBalls(tableID) {
    if (!g_PoolTables[tableID][ptable_Exists]) return 0;
    
    new Float:baseX = g_PoolTables[tableID][ptable_PosX];
    new Float:baseY = g_PoolTables[tableID][ptable_PosY];
    new Float:baseZ = g_PoolTables[tableID][ptable_PosZ] + 0.82; // Altura da mesa
    new Float:rot = g_PoolTables[tableID][ptable_Rotation];
    
    // Bola branca
    Pool_SetBall(tableID, 0, POOL_MODEL_BALL_CUE, baseX - 0.6, baseY, baseZ);
    
    // Formação de triângulo para as outras bolas
    // (implementar posições específicas conforme layout da mesa)
    
    // Bola 8 no centro
    Pool_SetBall(tableID, 8, POOL_MODEL_BALL_8, baseX + 0.4, baseY, baseZ);
    
    return 1;
}

// Configurar uma bola
stock Pool_SetBall(tableID, ballIndex, modelID, Float:x, Float:y, Float:z) {
    if (ballIndex < 0 || ballIndex >= POOL_MAX_BALLS) return 0;
    
    // Destruir objeto antigo se existir
    if (g_PoolBalls[tableID][ballIndex][pball_Active]) {
        DestroyDynamicObject(g_PoolTables[tableID][ptable_BallObjects][ballIndex]);
    }
    
    // Criar novo
    g_PoolBalls[tableID][ballIndex][pball_Active] = true;
    g_PoolBalls[tableID][ballIndex][pball_PosX] = x;
    g_PoolBalls[tableID][ballIndex][pball_PosY] = y;
    g_PoolBalls[tableID][ballIndex][pball_PosZ] = z;
    g_PoolBalls[tableID][ballIndex][pball_VelX] = 0.0;
    g_PoolBalls[tableID][ballIndex][pball_VelY] = 0.0;
    g_PoolBalls[tableID][ballIndex][pball_VelZ] = 0.0;
    
    g_PoolTables[tableID][ptable_BallObjects][ballIndex] = CreateDynamicObject(
        modelID, x, y, z, 0.0, 0.0, 0.0,
        -1, -1, -1, POOL_STREAM_RANGE
    );
    
    return 1;
}

// Obter mesa mais próxima de um jogador
stock Pool_GetNearestTable(playerid, &Float:distance = 0.0) {
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    
    new nearestID = POOL_INVALID_TABLE_ID;
    new Float:nearestDist = 999999.0;
    
    for (new i = 0; i < POOL_MAX_TABLES; i++) {
        if (!g_PoolTables[i][ptable_Exists]) continue;
        
        new Float:dist = GetDistanceBetweenPoints(
            px, py, pz,
            g_PoolTables[i][ptable_PosX],
            g_PoolTables[i][ptable_PosY],
            g_PoolTables[i][ptable_PosZ]
        );
        
        if (dist < nearestDist) {
            nearestDist = dist;
            nearestID = i;
        }
    }
    
    distance = nearestDist;
    return nearestID;
}
```

### 6.5 pool_streaming.inc

```pawn
// pool_streaming.inc

#if defined _pool_streaming_included
    #endinput
#endif
#define _pool_streaming_included

// Timer para verificação de distância
new g_PoolStreamTimer = -1;

// Iniciar sistema de streaming
stock Pool_StartStreaming() {
    if (g_PoolStreamTimer != -1) return 0;
    g_PoolStreamTimer = SetTimer("Pool_StreamCheck", POOL_STREAM_CHECK_MS, true);
    return 1;
}

// Parar sistema
stock Pool_StopStreaming() {
    if (g_PoolStreamTimer == -1) return 0;
    KillTimer(g_PoolStreamTimer);
    g_PoolStreamTimer = -1;
    return 1;
}

// Verificação periódica
forward Pool_StreamCheck();
public Pool_StreamCheck() {
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) {
        if (!IsPlayerConnected(playerid)) continue;
        
        Pool_CheckPlayerRanges(playerid);
    }
}

// Verificar ranges de um jogador
stock Pool_CheckPlayerRanges(playerid) {
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);
    
    for (new tableID = 0; tableID < POOL_MAX_TABLES; tableID++) {
        if (!g_PoolTables[tableID][ptable_Exists]) continue;
        
        new Float:dist = GetDistanceBetweenPoints(
            px, py, pz,
            g_PoolTables[tableID][ptable_PosX],
            g_PoolTables[tableID][ptable_PosY],
            g_PoolTables[tableID][ptable_PosZ]
        );
        
        // Verificar mudanças de range
        new bool:wasInStream = g_PoolPlayerInfo[playerid][tableID][pinfo_InStreamRange];
        new bool:wasInView = g_PoolPlayerInfo[playerid][tableID][pinfo_InViewRange];
        new bool:wasInPlay = g_PoolPlayerInfo[playerid][tableID][pinfo_InPlayRange];
        
        new bool:isInStream = (dist <= POOL_STREAM_RANGE);
        new bool:isInView = (dist <= POOL_VIEW_RANGE);
        new bool:isInPlay = (dist <= POOL_PLAY_RANGE);
        
        // Entrou no stream range
        if (isInStream && !wasInStream) {
            Pool_OnPlayerEnterStreamRange(playerid, tableID);
        }
        
        // Entrou no view range
        if (isInView && !wasInView) {
            Pool_OnPlayerEnterViewRange(playerid, tableID);
        }
        
        // Entrou no play range
        if (isInPlay && !wasInPlay) {
            Pool_OnPlayerEnterPlayRange(playerid, tableID);
        }
        
        // Saiu do play range
        if (!isInPlay && wasInPlay) {
            Pool_OnPlayerExitPlayRange(playerid, tableID);
        }
        
        // Saiu do view range
        if (!isInView && wasInView) {
            Pool_OnPlayerExitViewRange(playerid, tableID);
        }
        
        // Saiu do stream range
        if (!isInStream && wasInStream) {
            Pool_OnPlayerExitStreamRange(playerid, tableID);
        }
        
        // Atualizar estado
        g_PoolPlayerInfo[playerid][tableID][pinfo_InStreamRange] = isInStream;
        g_PoolPlayerInfo[playerid][tableID][pinfo_InViewRange] = isInView;
        g_PoolPlayerInfo[playerid][tableID][pinfo_InPlayRange] = isInPlay;
    }
}

// Callbacks de range
forward Pool_OnPlayerEnterStreamRange(playerid, tableID);
public Pool_OnPlayerEnterStreamRange(playerid, tableID) {
    // Enviar RPC com estado da mesa
    Pool_SendTableState(playerid, tableID);
}

forward Pool_OnPlayerEnterViewRange(playerid, tableID);
public Pool_OnPlayerEnterViewRange(playerid, tableID) {
    // Notificar cliente que pode ver atualizações
    Pool_SendRPC(playerid, POOL_RPC_ENTER_VIEW, tableID);
}

forward Pool_OnPlayerEnterPlayRange(playerid, tableID);
public Pool_OnPlayerEnterPlayRange(playerid, tableID) {
    // Notificar cliente que pode jogar
    Pool_SendRPC(playerid, POOL_RPC_ENTER_PLAY, tableID);
}

forward Pool_OnPlayerExitPlayRange(playerid, tableID);
public Pool_OnPlayerExitPlayRange(playerid, tableID) {
    // Se era o shooter, liberar
    if (g_PoolTables[tableID][ptable_CurrentShooter] == playerid) {
        g_PoolTables[tableID][ptable_CurrentShooter] = POOL_INVALID_PLAYER_ID;
    }
    
    Pool_SendRPC(playerid, POOL_RPC_EXIT_PLAY, tableID);
}

forward Pool_OnPlayerExitViewRange(playerid, tableID);
public Pool_OnPlayerExitViewRange(playerid, tableID) {
    Pool_SendRPC(playerid, POOL_RPC_EXIT_VIEW, tableID);
}

forward Pool_OnPlayerExitStreamRange(playerid, tableID);
public Pool_OnPlayerExitStreamRange(playerid, tableID) {
    // Cliente vai destruir objetos locais
}

// Obter jogadores em um range específico
stock Pool_GetPlayersInViewRange(tableID, players[], maxPlayers) {
    new count = 0;
    
    for (new i = 0; i < MAX_PLAYERS && count < maxPlayers; i++) {
        if (!IsPlayerConnected(i)) continue;
        if (g_PoolPlayerInfo[i][tableID][pinfo_InViewRange]) {
            players[count++] = i;
        }
    }
    
    return count;
}

// Broadcast para todos no view range
stock Pool_BroadcastToViewRange(tableID, rpcID, {Float,_}:...) {
    new players[MAX_PLAYERS];
    new count = Pool_GetPlayersInViewRange(tableID, players, MAX_PLAYERS);
    
    for (new i = 0; i < count; i++) {
        // Enviar RPC para cada jogador
        Pool_SendRPC(players[i], rpcID, tableID);
    }
    
    return count;
}
```

### 6.6 pool_rpc.inc

```pawn
// pool_rpc.inc

#if defined _pool_rpc_included
    #endinput
#endif
#define _pool_rpc_included

// IDs de RPC
enum {
    // Server -> Client
    POOL_RPC_TABLE_STATE = 100,     // Estado completo da mesa
    POOL_RPC_ENTER_VIEW,            // Entrou no view range
    POOL_RPC_EXIT_VIEW,             // Saiu do view range
    POOL_RPC_ENTER_PLAY,            // Entrou no play range
    POOL_RPC_EXIT_PLAY,             // Saiu do play range
    POOL_RPC_SHOOT_APPROVED,        // Tacada aprovada
    POOL_RPC_SHOOT_DENIED,          // Tacada negada
    POOL_RPC_SHOT_BROADCAST,        // Broadcast de tacada
    POOL_RPC_BALLS_UPDATE,          // Atualização de posições
    POOL_RPC_RESET,                 // Mesa resetada
    
    // Client -> Server
    POOL_RPC_REQUEST_SHOOT = 200,   // Pedir para tacar
    POOL_RPC_EXECUTE_SHOT,          // Executar tacada
    POOL_RPC_BALLS_SETTLED,         // Bolas pararam
    POOL_RPC_PLACE_CUE_BALL,        // Posicionar bola branca
    POOL_RPC_REQUEST_RESET          // Pedir reset
};

// Processar pedido de tacada
stock Pool_OnRequestShoot(playerid, tableID) {
    // Validações
    if (!g_PoolTables[tableID][ptable_Exists]) {
        Pool_SendRPC(playerid, POOL_RPC_SHOOT_DENIED, tableID);
        return 0;
    }
    
    if (!g_PoolPlayerInfo[playerid][tableID][pinfo_InPlayRange]) {
        Pool_SendRPC(playerid, POOL_RPC_SHOOT_DENIED, tableID);
        return 0;
    }
    
    if (g_PoolTables[tableID][ptable_InMotion]) {
        Pool_SendRPC(playerid, POOL_RPC_SHOOT_DENIED, tableID);
        return 0;
    }
    
    if (g_PoolTables[tableID][ptable_CurrentShooter] != POOL_INVALID_PLAYER_ID) {
        Pool_SendRPC(playerid, POOL_RPC_SHOOT_DENIED, tableID);
        return 0;
    }
    
    // Aprovar
    g_PoolTables[tableID][ptable_CurrentShooter] = playerid;
    g_PoolTables[tableID][ptable_ShooterTimeout] = GetTickCount() + POOL_SHOOTER_TIMEOUT_MS;
    
    Pool_SendRPC(playerid, POOL_RPC_SHOOT_APPROVED, tableID);
    
    return 1;
}

// Processar tacada executada
stock Pool_OnExecuteShot(playerid, tableID, shotData[E_POOL_SHOT]) {
    // Validar que é o shooter
    if (g_PoolTables[tableID][ptable_CurrentShooter] != playerid) {
        return 0;
    }
    
    // Marcar em movimento
    g_PoolTables[tableID][ptable_InMotion] = true;
    g_PoolTables[tableID][ptable_CurrentShooter] = POOL_INVALID_PLAYER_ID;
    
    // Broadcast para todos no view range (exceto quem tacou)
    new players[MAX_PLAYERS];
    new count = Pool_GetPlayersInViewRange(tableID, players, MAX_PLAYERS);
    
    for (new i = 0; i < count; i++) {
        if (players[i] != playerid) {
            Pool_SendShotData(players[i], tableID, shotData);
        }
    }
    
    return 1;
}

// Processar bolas pararam
stock Pool_OnBallsSettled(playerid, tableID, ballsData[][E_POOL_BALL]) {
    // Salvar novo estado
    for (new i = 0; i < POOL_MAX_BALLS; i++) {
        g_PoolBalls[tableID][i] = ballsData[i];
        
        // Atualizar posição do objeto
        if (ballsData[i][pball_Active]) {
            SetDynamicObjectPos(
                g_PoolTables[tableID][ptable_BallObjects][i],
                ballsData[i][pball_PosX],
                ballsData[i][pball_PosY],
                ballsData[i][pball_PosZ]
            );
        }
    }
    
    g_PoolTables[tableID][ptable_InMotion] = false;
    
    return 1;
}

// Enviar estado da mesa
stock Pool_SendTableState(playerid, tableID) {
    // Montar dados e enviar via RPC
    // Implementação depende do sistema de RPC usado
}

// Enviar dados de tacada
stock Pool_SendShotData(playerid, tableID, shotData[E_POOL_SHOT]) {
    // Implementação depende do sistema de RPC usado
}

// Enviar RPC genérico
stock Pool_SendRPC(playerid, rpcID, tableID, {Float,_}:...) {
    // Implementação depende do sistema de RPC usado
    // Para open.mp, usar pawn-natives ou plugin
}
```

---

## 7. Hooks Nativos

### 7.1 Resumo de Hooks Necessários

| Hook | Função | Uso |
|------|--------|-----|
| `CPhysical::ApplyForce` | Aplicar força com ponto | Tacada com english |
| `CPhysical::ApplyMoveForce` | Aplicar força linear | Tacada simples |
| `CPhysical::ApplyTurnForce` | Aplicar torque | Spin da bola |
| `CPhysical::GetSpeed` | Obter velocidade | Detectar bola parada |
| `CCamera::SetPosition` | Posicionar câmera | Câmera de mira |
| `CCamera::SetLookAt` | Apontar câmera | Câmera de mira |

### 7.2 Implementação dos Hooks

```cpp
// pool_physics.cpp

#include "pool_physics.h"
#include "pool_error.h"
#include <dlfcn.h>

namespace Pool {

bool PoolPhysics::s_initialized = false;
uintptr_t PoolPhysics::s_libBase = 0;
PoolPhysics::NativeFunctions PoolPhysics::s_natives;

Result PoolPhysics::Initialize() {
    if (s_initialized) return Result::Success;
    
    // Obter base da biblioteca
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD);
    POOL_CHECK(handle != nullptr, Result::ErrorNativeHookFailed, "Failed to get libGTASA handle");
    
    Dl_info info;
    if (dladdr(handle, &info)) {
        s_libBase = reinterpret_cast<uintptr_t>(info.dli_fbase);
    }
    
    POOL_CHECK(s_libBase != 0, Result::ErrorNativeHookFailed, "Failed to get libGTASA base");
    
    // Resolver offsets
    Result result = ResolveOffsets();
    POOL_CHECK(result == Result::Success, result, "Failed to resolve offsets");
    
    s_initialized = true;
    return Result::Success;
}

Result PoolPhysics::ResolveOffsets() {
#ifdef __LP64__
    // 64-bit offsets
    s_natives.ApplyForce = s_libBase + 0x004e0f00;
    s_natives.ApplyMoveForce = s_libBase + 0x004e0de0;
    s_natives.ApplyTurnForce = s_libBase + 0x004e0e28;
    s_natives.GetSpeed = s_libBase + 0x004e1038;
    s_natives.ApplyFriction = s_libBase + 0x004e1f34;
    s_natives.ApplyGravity = s_libBase + 0x004e1e74;
#else
    // 32-bit offsets
    s_natives.ApplyForce = s_libBase + 0x003fd619;
    s_natives.ApplyMoveForce = s_libBase + 0x003fd4d9;
    s_natives.ApplyTurnForce = s_libBase + 0x003fd541;
    s_natives.GetSpeed = s_libBase + 0x003fd775;
    s_natives.ApplyFriction = s_libBase + 0x003fe819;
    s_natives.ApplyGravity = s_libBase + 0x003fe785;
#endif

    // Validar que todos foram resolvidos
    POOL_CHECK(s_natives.ApplyForce != 0, Result::ErrorNativeHookFailed, "ApplyForce offset invalid");
    POOL_CHECK(s_natives.ApplyMoveForce != 0, Result::ErrorNativeHookFailed, "ApplyMoveForce offset invalid");
    
    return Result::Success;
}

bool PoolPhysics::ValidatePointer(void* ptr) {
    if (ptr == nullptr) return false;
    
    // Verificação adicional de memória válida
    // Implementar conforme necessidade (mincore, /proc/self/maps, etc.)
    
    return true;
}

Result PoolPhysics::ApplyMoveForce(CPhysical* physical, const Vec3& force) {
    POOL_CHECK(s_initialized, Result::ErrorNativeHookFailed, "Physics not initialized");
    POOL_CHECK(ValidatePointer(physical), Result::ErrorInvalidParameters, "Invalid physical pointer");
    
    // Converter Vec3 para CVector do GTA
    struct CVector { float x, y, z; };
    CVector gForce = { force.x, force.y, force.z };
    
    // Chamar função nativa
    typedef void (*ApplyMoveForce_t)(CPhysical*, CVector);
    auto fn = reinterpret_cast<ApplyMoveForce_t>(s_natives.ApplyMoveForce);
    
    // Usar try-catch ou signal handler para capturar crashes
    fn(physical, gForce);
    
    return Result::Success;
}

Result PoolPhysics::ApplyForceAtPoint(CPhysical* physical, const Vec3& force, const Vec3& point) {
    POOL_CHECK(s_initialized, Result::ErrorNativeHookFailed, "Physics not initialized");
    POOL_CHECK(ValidatePointer(physical), Result::ErrorInvalidParameters, "Invalid physical pointer");
    
    struct CVector { float x, y, z; };
    CVector gForce = { force.x, force.y, force.z };
    CVector gPoint = { point.x, point.y, point.z };
    
    typedef void (*ApplyForce_t)(CPhysical*, CVector, CVector, bool);
    auto fn = reinterpret_cast<ApplyForce_t>(s_natives.ApplyForce);
    
    fn(physical, gForce, gPoint, true);
    
    return Result::Success;
}

Result PoolPhysics::GetVelocity(CPhysical* physical, Vec3& outVelocity) {
    POOL_CHECK(s_initialized, Result::ErrorNativeHookFailed, "Physics not initialized");
    POOL_CHECK(ValidatePointer(physical), Result::ErrorInvalidParameters, "Invalid physical pointer");
    
    // Acessar m_vecMoveSpeed diretamente da estrutura
    // Offset depende da estrutura CPhysical
    
#ifdef __LP64__
    constexpr size_t MOVE_SPEED_OFFSET = 0x70;  // Verificar offset correto
#else
    constexpr size_t MOVE_SPEED_OFFSET = 0x44;  // Verificar offset correto
#endif

    float* velocity = reinterpret_cast<float*>(
        reinterpret_cast<uintptr_t>(physical) + MOVE_SPEED_OFFSET
    );
    
    outVelocity.x = velocity[0];
    outVelocity.y = velocity[1];
    outVelocity.z = velocity[2];
    
    return Result::Success;
}

bool PoolPhysics::IsStopped(CPhysical* physical) {
    Vec3 velocity;
    Result result = GetVelocity(physical, velocity);
    
    if (result != Result::Success) {
        return true;  // Assumir parado em caso de erro
    }
    
    return velocity.Length() < VELOCITY_THRESHOLD;
}

Result PoolPhysics::ApplyShot(CPhysical* cueBall, const ShotData& shot) {
    POOL_CHECK(s_initialized, Result::ErrorNativeHookFailed, "Physics not initialized");
    POOL_CHECK(ValidatePointer(cueBall), Result::ErrorInvalidParameters, "Invalid cue ball pointer");
    POOL_CHECK(shot.power > 0.0f && shot.power <= 1.0f, Result::ErrorInvalidParameters, "Invalid power");
    
    // Calcular força
    float forceMagnitude = MIN_SHOT_POWER + (shot.power * (MAX_SHOT_POWER - MIN_SHOT_POWER));
    Vec3 force = shot.direction.Normalized() * forceMagnitude;
    
    // Calcular ponto de impacto (para english)
    Vec3 hitOffset(
        shot.englishX * BALL_RADIUS,
        shot.englishY * BALL_RADIUS,
        0.0f
    );
    
    Vec3 position;
    Result result = GetPosition(cueBall, position);
    POOL_CHECK(result == Result::Success, result, "Failed to get cue ball position");
    
    Vec3 hitPoint = position + hitOffset;
    
    // Aplicar força
    if (shot.englishX != 0.0f || shot.englishY != 0.0f) {
        // Com english - usar ApplyForce com ponto
        return ApplyForceAtPoint(cueBall, force, hitPoint);
    } else {
        // Sem english - tacada reta
        return ApplyMoveForce(cueBall, force);
    }
}

void PoolPhysics::Shutdown() {
    s_initialized = false;
    s_libBase = 0;
    s_natives = NativeFunctions();
}

} // namespace Pool
```

---

## 8. Tabela de Offsets

### 8.1 CPhysical

| Função | Offset 32-bit | Offset 64-bit | Assinatura |
|--------|---------------|---------------|------------|
| `ApplyForce` | 0x003fd619 | 0x004e0f00 | `void(CVector, CVector, bool)` |
| `ApplyMoveForce` | 0x003fd4d9 | 0x004e0de0 | `void(CVector)` |
| `ApplyTurnForce` | 0x003fd541 | 0x004e0e28 | `void(CVector, CVector)` |
| `ApplySpeed` | 0x003fda95 | 0x004e1300 | `void()` |
| `ApplyMoveSpeed` | 0x003fd86d | 0x004e1104 | `void()` |
| `ApplyTurnSpeed` | 0x003fd8d5 | 0x004e116c | `void()` |
| `ApplyGravity` | 0x003fe785 | 0x004e1e74 | `void()` |
| `ApplyFriction` | 0x003fe819 | 0x004e1f34 | `void()` |
| `ApplyFriction (colpoint)` | 0x003fe2a9 | 0x004e1a50 | `void(float, CColPoint&)` |
| `ApplyFriction (physical)` | 0x00405f5d | 0x004e9248 | `void(CPhysical*, float, CColPoint&)` |
| `ApplyAirResistance` | 0x003fe9dd | 0x004e2104 | `void()` |
| `ApplyCollision` | 0x00401b75 | 0x004e5598 | `bool(CEntity*, CColPoint&, float&)` |
| `ApplyCollision (physical)` | 0x00401e81 | 0x004e583c | `bool(CPhysical*, CColPoint&, float&, float&)` |
| `GetSpeed` | 0x003fd775 | 0x004e1038 | `CVector(CVector)` |
| `ProcessControl` | 0x003fedcd | 0x004e2668 | `void()` |
| `ProcessCollision` | 0x003fef49 | 0x004e2668 | `void()` |
| `CheckCollision` | 0x003ff655 | 0x004e2e30 | `bool()` |
| `TestCollision` | 0x004019bd | 0x004e532c | `bool(bool)` |

### 8.2 CColSphere / CCollision

| Função | Offset 32-bit | Offset 64-bit | Uso |
|--------|---------------|---------------|-----|
| `CColSphere::Set` | 0x002e1cc5 | 0x003a4d0c | Definir esfera |
| `CColSphere::IntersectRay` | 0x002e1ce1 | 0x003a4d30 | Raycast |
| `CColSphere::IntersectSphere` | 0x002e2035 | 0x003a504c | Colisão bola-bola |
| `CCollision::TestSphereSphere` | 0x002d9a11 | 0x0039c648 | Testar colisão |
| `CCollision::ProcessSphereSphere` | 0x002d9a19 | 0x0039c68c | Processar colisão |
| `CCollision::TestLineSphere` | 0x002dabbd | 0x0039d5c4 | Raycast de mira |
| `CCollision::ProcessLineSphere` | 0x002db031 | 0x0039d8e8 | Processar raycast |

### 8.3 CObject

| Função | Offset 32-bit | Offset 64-bit | Uso |
|--------|---------------|---------------|-----|
| `CObject::CObject` | 0x00452c6d | 0x0053b828 | Construtor |
| `CObject::Create` | 0x0045343d | 0x0053c19c | Criar objeto |
| `CObject::ProcessControl` | 0x00453825 | 0x0053c790 | Loop de física |
| `CObject::SetIsStatic` | 0x00455681 | 0x0053e8a8 | Definir estático |
| `CObject::Render` | 0x00454f55 | 0x0053e0f0 | Renderizar |

### 8.4 Widgets de Pool

| Função | Offset 32-bit | Offset 64-bit |
|--------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |

### 8.5 Variáveis Globais

| Variável | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `MI_POOL_CUE_BALL` | 0x006aad08 | 0x00876314 |
| `CWorld::SnookerTableMin` | 0x0096b9e8 | 0x00bdcb14 |
| `CWorld::SnookerTableMax` | 0x0096b9dc | 0x00bdcb08 |
| `CTheScripts::bMiniGameInProgress` | 0x00819d80 | 0x009ff3a0 |

---

## 9. Sistema de Erros

### 9.1 Códigos de Erro

| Código | Nome | Descrição | Ação |
|--------|------|-----------|------|
| 0 | `Success` | Operação bem-sucedida | Continuar |
| 1 | `ErrorTableNotFound` | Mesa não existe | Cancelar, logar |
| 2 | `ErrorPlayerNotInRange` | Jogador fora do alcance | Cancelar, notificar |
| 3 | `ErrorTableInMotion` | Bolas ainda movendo | Cancelar, notificar |
| 4 | `ErrorAlreadyShooting` | Outro jogador tacando | Cancelar, notificar |
| 5 | `ErrorInvalidParameters` | Parâmetros inválidos | Cancelar, logar |
| 6 | `ErrorNativeHookFailed` | Falha em hook nativo | Cancelar, logar warning |
| 7 | `ErrorObjectCreationFailed` | Falha ao criar objeto | Cancelar, logar |
| 8 | `ErrorUnknown` | Erro desconhecido | Cancelar, logar |

### 9.2 Tratamento de Erros

```cpp
// Exemplo de uso seguro
Result PoolManager::ExecuteShot(uint16_t tableID, const ShotData& shot) {
    // Validações
    auto* table = GetTable(tableID);
    if (!table) {
        ReportError(Result::ErrorTableNotFound, "ExecuteShot");
        return Result::ErrorTableNotFound;
    }
    
    if (!table->IsInPlayRange()) {
        ReportError(Result::ErrorPlayerNotInRange, "ExecuteShot");
        return Result::ErrorPlayerNotInRange;
    }
    
    if (table->IsInMotion()) {
        ReportError(Result::ErrorTableInMotion, "ExecuteShot");
        return Result::ErrorTableInMotion;
    }
    
    // Validar parâmetros
    if (shot.power <= 0.0f || shot.power > 1.0f) {
        ReportError(Result::ErrorInvalidParameters, "ExecuteShot - invalid power");
        return Result::ErrorInvalidParameters;
    }
    
    // Executar
    Result result = table->ApplyShot(shot);
    if (result != Result::Success) {
        ReportError(result, "ExecuteShot - ApplyShot failed");
        return result;
    }
    
    // Notificar servidor
    // ...
    
    return Result::Success;
}

void PoolManager::ReportError(Result error, const char* context) {
    // Logar erro
    PoolError::Report(
        error == Result::ErrorNativeHookFailed ? ErrorLevel::Warning : ErrorLevel::Error,
        error,
        context
    );
    
    // Notificar sistema externo via callback
    if (m_callbacks) {
        char message[256];
        snprintf(message, sizeof(message), "%s: %s", context, ResultToString(error));
        m_callbacks->OnPoolError(error, message);
    }
}
```

---

## 10. RPCs e Comunicação

### 10.1 Formato de Pacotes

```cpp
// Estrutura base de RPC
struct PoolRPCHeader {
    uint8_t rpcID;
    uint16_t tableID;
    uint16_t timestamp;     // Para ordenação
};

// Tacada
struct PoolRPCShot {
    PoolRPCHeader header;
    float dirX, dirY, dirZ;
    float power;
    float englishX, englishY;
    float cuePosX, cuePosY, cuePosZ;
};

// Estado de mesa
struct PoolRPCTableState {
    PoolRPCHeader header;
    float tableX, tableY, tableZ;
    float rotation;
    uint8_t ballCount;
    // Seguido por array de BallState
};

// Estado de bola (compacto)
struct PoolRPCBallState {
    uint8_t index;
    uint8_t flags;          // bit 0: active
    int16_t posX, posY, posZ;   // Posição * 100 (2 casas decimais)
};
```

### 10.2 IDs de RPC

| ID | Direção | Nome | Descrição |
|----|---------|------|-----------|
| 100 | S→C | `TABLE_STATE` | Estado completo |
| 101 | S→C | `ENTER_VIEW` | Entrou view range |
| 102 | S→C | `EXIT_VIEW` | Saiu view range |
| 103 | S→C | `ENTER_PLAY` | Entrou play range |
| 104 | S→C | `EXIT_PLAY` | Saiu play range |
| 105 | S→C | `SHOOT_APPROVED` | Tacada aprovada |
| 106 | S→C | `SHOOT_DENIED` | Tacada negada |
| 107 | S→C | `SHOT_BROADCAST` | Broadcast de tacada |
| 108 | S→C | `BALLS_UPDATE` | Atualização posições |
| 109 | S→C | `RESET` | Mesa resetada |
| 200 | C→S | `REQUEST_SHOOT` | Pedir para tacar |
| 201 | C→S | `EXECUTE_SHOT` | Executar tacada |
| 202 | C→S | `BALLS_SETTLED` | Bolas pararam |
| 203 | C→S | `PLACE_CUE` | Posicionar bola branca |
| 204 | C→S | `REQUEST_RESET` | Pedir reset |

---

## 11. Arquivos do Sistema

### 11.1 Estrutura de Diretórios

```
app/src/main/cpp/samp/game/Pool/
├── pool_constants.h        // Constantes globais
├── pool_types.h            // Tipos e estruturas
├── pool_callbacks.h        // Interface de callbacks
├── pool_error.h            // Sistema de erros
├── pool_error.cpp
├── pool_manager.h          // Manager principal
├── pool_manager.cpp
├── pool_table_local.h      // Mesa local
├── pool_table_local.cpp
├── pool_camera.h           // Câmera de mira
├── pool_camera.cpp
├── pool_physics.h          // Hooks de física
├── pool_physics.cpp
├── pool_ui.h               // Interface ImGui
├── pool_ui.cpp
├── pool_rpc.h              // Definições de RPC
└── pool_rpc.cpp            // Serialização/deserialização
```

### 11.2 Dependências

```cmake
# CMakeLists.txt (parcial)

# Pool system
set(POOL_SOURCES
    Pool/pool_error.cpp
    Pool/pool_manager.cpp
    Pool/pool_table_local.cpp
    Pool/pool_camera.cpp
    Pool/pool_physics.cpp
    Pool/pool_ui.cpp
    Pool/pool_rpc.cpp
)

# Adicionar ao target principal
target_sources(samp PRIVATE ${POOL_SOURCES})

# Dependências
# - ImGui (para UI)
# - ShadowHook ou similar (para hooks)
```

---

## 12. Guia de Integração

### 12.1 Inicialização

```cpp
// Em algum ponto de inicialização do app

#include "Pool/pool_manager.h"

// Callback customizado (opcional)
class MyPoolCallbacks : public Pool::IPoolCallbacks {
public:
    void OnPoolError(Pool::Result error, const char* message) override {
        // Logar no sistema de log do seu app
        MyLogger::Error("Pool", message);
    }
    
    void OnShotExecuted(uint16_t playerID, uint16_t tableID, 
                        const Pool::ShotData& shot) override {
        // Notificar outros sistemas se necessário
    }
    
    // Implementar outros métodos...
};

MyPoolCallbacks g_poolCallbacks;

void InitializePool() {
    Pool::Result result = Pool::PoolManager::Instance().Initialize(&g_poolCallbacks);
    
    if (result != Pool::Result::Success) {
        // Sistema não vai funcionar, mas não vai crashar
        MyLogger::Error("Pool", "Failed to initialize: %s", 
                       Pool::ResultToString(result));
    }
}
```

### 12.2 Loop de Update

```cpp
// No loop principal do jogo

void GameUpdate(float deltaTime) {
    // Outros updates...
    
    // Pool update
    if (Pool::PoolManager::Instance().IsInitialized()) {
        Pool::PoolManager::Instance().Update(deltaTime);
    }
}
```

### 12.3 Renderização ImGui

```cpp
// No loop de renderização ImGui

void RenderImGui() {
    // Outras janelas...
    
    // Pool UI
    Pool::PoolManager::Instance().GetUI().Render();
}
```

### 12.4 Recebimento de RPCs

```cpp
// Quando receber RPC do servidor

void OnReceiveRPC(uint8_t rpcID, BitStream& bs) {
    switch (rpcID) {
        case POOL_RPC_TABLE_STATE: {
            Pool::TableState state;
            // Deserializar...
            Pool::PoolManager::Instance().OnReceiveTableState(state.tableID, state);
            break;
        }
        
        case POOL_RPC_SHOT_BROADCAST: {
            uint16_t tableID, shooterID;
            Pool::ShotData shot;
            // Deserializar...
            Pool::PoolManager::Instance().OnReceiveShot(tableID, shot, shooterID);
            break;
        }
        
        // Outros RPCs...
    }
}
```

### 12.5 Server-Side (PAWN)

```pawn
// gamemode.pwn

#include "pool/pool_main.inc"

public OnGameModeInit() {
    // Iniciar sistema de pool
    Pool_StartStreaming();
    
    // Criar algumas mesas
    Pool_CreateTable(1000.0, 2000.0, 15.0, 0.0);
    Pool_CreateTable(1050.0, 2000.0, 15.0, 90.0);
    
    return 1;
}

public OnGameModeExit() {
    Pool_StopStreaming();
    return 1;
}

public OnPlayerConnect(playerid) {
    // Inicializar dados do jogador para pool
    for (new i = 0; i < POOL_MAX_TABLES; i++) {
        g_PoolPlayerInfo[playerid][i][pinfo_InStreamRange] = false;
        g_PoolPlayerInfo[playerid][i][pinfo_InViewRange] = false;
        g_PoolPlayerInfo[playerid][i][pinfo_InPlayRange] = false;
    }
    return 1;
}
```

---

## Conclusão

Este documento define um sistema de sinuca modular e seguro para SA-MP/open.mp. Características principais:

1. **Modular**: Não depende de sistemas externos
2. **Seguro**: Erros são tratados sem crashes
3. **Eficiente**: Streaming por distância
4. **Completo**: Client-side + Server-side

Para implementação, recomenda-se começar por:
1. Estruturas de dados e constantes
2. Sistema de erros
3. PoolPhysics (hooks)
4. PoolManager básico
5. Server-side streaming
6. UI e câmera

---

*Documento gerado para o projeto SA-MP 2.10*
