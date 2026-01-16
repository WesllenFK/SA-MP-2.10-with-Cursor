# Pool System - Complete Technical Specification

**Version**: 1.0  
**Last updated**: January 2026  
**Compatibility**: Android NDK, open.mp, SA-MP 0.3.7+

---

## Table of Contents

1. [Overview](#1-overview)
2. [System Architecture](#2-system-architecture)
3. [Data Structures](#3-data-structures)
4. [Streaming System](#4-streaming-system)
5. [Client-Side (C++)](#5-client-side-c)
6. [Server-Side (open.mp/PAWN)](#6-server-side-openmpawn)
7. [Native Hooks](#7-native-hooks)
8. [Offset Table](#8-offset-table)
9. [Error System](#9-error-system)
10. [RPCs and Communication](#10-rpcs-and-communication)
11. [System Files](#11-system-files)
12. [Integration Guide](#12-integration-guide)

---

## 1. Overview

### 1.1 Purpose

Modular pool/billiards system for SA-MP/open.mp that enables:
- Multiple tables at any map position
- Synchronized physics between players
- Aiming and shooting interface
- Distance-based streaming

### 1.2 Design Principles

| Principle | Description |
|-----------|-------------|
| **Modularity** | No external system dependencies (admin, saves, etc.) |
| **Safety** | Never crash; errors are reported and operation is cancelled |
| **Efficiency** | Distance-based streaming; only nearby players receive data |
| **Determinism** | Same shot parameters = same result on all clients |

### 1.3 Requirements

**Client-Side:**
- Android NDK r21+
- C++17
- ImGui (for UI)
- ShadowHook or similar (for hooks)

**Server-Side:**
- open.mp or SA-MP 0.3.7+
- PAWN or open.mp C++ SDK

---

## 2. System Architecture

### 2.1 General Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         GENERAL ARCHITECTURE                        │
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

### 2.2 Layers

| Layer | Location | Responsibility |
|-------|----------|----------------|
| **PoolServerManager** | Server | Manage all tables, coordinate |
| **PoolServerTable** | Server | Single table state |
| **PoolServerStreamer** | Server | Distance control and broadcast |
| **PoolManager** | Client | Manage local tables |
| **PoolCamera** | Client | Aiming camera |
| **PoolPhysics** | Client | Physics hooks |
| **PoolUI** | Client | ImGui interface |
| **PoolError** | Both | Error system |

---

## 3. Data Structures

### 3.1 Constants

```cpp
// pool_constants.h

namespace Pool {
    // Limits
    constexpr uint16_t MAX_TABLES = 100;
    constexpr uint8_t  MAX_BALLS = 16;
    constexpr uint16_t INVALID_TABLE_ID = 0xFFFF;
    constexpr uint16_t INVALID_PLAYER_ID = 0xFFFF;
    
    // Distances (meters)
    constexpr float PLAY_RANGE = 5.0f;      // Can shoot
    constexpr float VIEW_RANGE = 25.0f;     // Sees updates
    constexpr float STREAM_RANGE = 50.0f;   // Objects exist
    
    // Physics
    constexpr float MAX_SHOT_POWER = 20.0f;
    constexpr float MIN_SHOT_POWER = 1.0f;
    constexpr float BALL_RADIUS = 0.028f;   // ~2.8cm (real pool ball)
    constexpr float VELOCITY_THRESHOLD = 0.001f;  // Considered stopped
    
    // Timing
    constexpr uint32_t STREAM_CHECK_INTERVAL = 200;  // ms
    constexpr uint32_t SHOOTER_TIMEOUT = 30000;      // ms
    constexpr uint32_t PHYSICS_SYNC_INTERVAL = 50;   // ms during motion
    
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

### 3.2 Shared Structures

```cpp
// pool_types.h

#pragma once
#include <cstdint>

namespace Pool {

// Simple 3D vector (compatible with GTA's CVector)
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

// Ball state
struct BallState {
    bool active;        // In play?
    Vec3 position;      // Current position
    Vec3 velocity;      // Velocity (for sync during motion)
    
    BallState() : active(true), position(), velocity() {}
};

// Shot data
struct ShotData {
    Vec3 direction;     // Normalized direction
    float power;        // 0.0 - 1.0
    float englishX;     // -1.0 to 1.0 (horizontal spin)
    float englishY;     // -1.0 to 1.0 (vertical spin)
    Vec3 cueBallPos;    // Cue ball position at shot time
    
    ShotData() : direction(), power(0), englishX(0), englishY(0), cueBallPos() {}
};

// Complete table state
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

// Operation result
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

// Convert result to string
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

### 3.3 Callbacks (Interface for External System)

```cpp
// pool_callbacks.h

#pragma once
#include "pool_types.h"

namespace Pool {

// Callback interface that external system should implement
class IPoolCallbacks {
public:
    virtual ~IPoolCallbacks() = default;
    
    // Called when error occurs
    virtual void OnPoolError(Result error, const char* message) = 0;
    
    // Called when player enters table range
    virtual void OnPlayerEnterTableRange(uint16_t playerID, uint16_t tableID) = 0;
    
    // Called when player exits range
    virtual void OnPlayerExitTableRange(uint16_t playerID, uint16_t tableID) = 0;
    
    // Called when shot is executed
    virtual void OnShotExecuted(uint16_t playerID, uint16_t tableID, const ShotData& shot) = 0;
    
    // Called when balls stop moving
    virtual void OnBallsSettled(uint16_t tableID, const TableState& state) = 0;
    
    // Called when ball is pocketed (optional)
    virtual void OnBallPocketed(uint16_t tableID, uint8_t ballIndex) = 0;
};

// Default callback that only logs errors
class DefaultPoolCallbacks : public IPoolCallbacks {
public:
    void OnPoolError(Result error, const char* message) override {
        // Log to debug system
        // Does nothing else
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

## 4. Streaming System

### 4.1 Concept

The system uses three distance zones for optimization:

```
┌─────────────────────────────────────────────────────────────────────┐
│                    STREAMING ZONES                                  │
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
    │   │   │        │ TABLE │            │           │       │
    │   │   │        └───────┘            │           │       │
    │   │   │                             │           │       │
    │   │   │  • Can shoot                │           │       │
    │   │   │  • Sees full UI             │           │       │
    │   │   │  • Receives all updates     │           │       │
    │   │   └─────────────────────────────┘           │       │
    │   │                                             │       │
    │   │  • Sees balls moving                        │       │
    │   │  • Receives physics updates                 │       │
    │   │  • Cannot shoot                             │       │
    │   └─────────────────────────────────────────────┘       │
    │                                                         │
    │  • Objects exist (streamed)                             │
    │  • Does NOT receive physics updates                     │
    │  • Balls stay at last known position                    │
    └─────────────────────────────────────────────────────────┘
    
    OUTSIDE STREAM_RANGE:
    • Objects don't exist locally
    • No processing
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
    
    // Initialization
    Result Initialize(IPoolCallbacks* callbacks = nullptr);
    void Shutdown();
    bool IsInitialized() const { return m_initialized; }
    
    // Update (call every frame)
    void Update(float deltaTime);
    
    // Local table management
    Result OnTableStreamIn(uint16_t tableID, const TableState& state);
    Result OnTableStreamOut(uint16_t tableID);
    Result OnEnterViewRange(uint16_t tableID);
    Result OnExitViewRange(uint16_t tableID);
    Result OnEnterPlayRange(uint16_t tableID);
    Result OnExitPlayRange(uint16_t tableID);
    
    // Shooting
    Result RequestShoot(uint16_t tableID);
    Result OnShootApproved(uint16_t tableID);
    Result OnShootDenied(uint16_t tableID, Result reason);
    Result ExecuteShot(uint16_t tableID, const ShotData& shot);
    Result OnReceiveShot(uint16_t tableID, const ShotData& shot, uint16_t shooterID);
    
    // State
    Result OnReceiveTableState(uint16_t tableID, const TableState& state);
    
    // Position cue ball
    Result StartBallInHand(uint16_t tableID);
    Result ConfirmBallPosition(uint16_t tableID, const Vec3& position);
    
    // Access
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

### 5.2 PoolPhysics

```cpp
// pool_physics.h

#pragma once
#include "pool_types.h"

namespace Pool {

class CPhysical;

class PoolPhysics {
public:
    // Hook initialization
    static Result Initialize();
    static void Shutdown();
    static bool IsInitialized() { return s_initialized; }
    
    // Apply physics
    static Result ApplyForce(CPhysical* physical, const Vec3& force);
    static Result ApplyForceAtPoint(CPhysical* physical, const Vec3& force, const Vec3& point);
    static Result ApplyMoveForce(CPhysical* physical, const Vec3& force);
    static Result ApplyTurnForce(CPhysical* physical, const Vec3& force, const Vec3& point);
    
    // Queries
    static Result GetVelocity(CPhysical* physical, Vec3& outVelocity);
    static Result GetPosition(CPhysical* physical, Vec3& outPosition);
    static Result GetSpeed(CPhysical* physical, float& outSpeed);
    
    // Utilities
    static bool IsStopped(CPhysical* physical);
    
    // Apply complete shot
    static Result ApplyShot(CPhysical* cueBall, const ShotData& shot);
    
private:
    static bool s_initialized;
    static uintptr_t s_libBase;
    
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

---

## 6. Server-Side (open.mp/PAWN)

### 6.1 File Structure

```
gamemodes/
└── pool/
    ├── pool_main.inc           // Main include
    ├── pool_config.inc         // Configurations
    ├── pool_data.inc           // Data structures
    ├── pool_tables.inc         // Table management
    ├── pool_streaming.inc      // Streaming system
    ├── pool_rpc.inc            // Client communication
    └── pool_utils.inc          // Utilities
```

### 6.2 pool_config.inc

```pawn
// pool_config.inc

#if defined _pool_config_included
    #endinput
#endif
#define _pool_config_included

// Limits
#define POOL_MAX_TABLES             100
#define POOL_MAX_BALLS              16

// Distances
#define POOL_PLAY_RANGE             5.0
#define POOL_VIEW_RANGE             25.0
#define POOL_STREAM_RANGE           50.0

// Timing
#define POOL_STREAM_CHECK_MS        200
#define POOL_SHOOTER_TIMEOUT_MS     30000

// Invalid IDs
#define POOL_INVALID_TABLE_ID       0xFFFF
#define POOL_INVALID_PLAYER_ID      0xFFFF

// Model IDs
#define POOL_MODEL_TABLE            3094
#define POOL_MODEL_BALL_CUE         3003
#define POOL_MODEL_BALL_8           3106
#define POOL_MODEL_BALL_SOLID_1     2995
#define POOL_MODEL_BALL_STRIPE_1    3002
```

### 6.3 pool_streaming.inc

```pawn
// pool_streaming.inc

#if defined _pool_streaming_included
    #endinput
#endif
#define _pool_streaming_included

new g_PoolStreamTimer = -1;

stock Pool_StartStreaming() {
    if (g_PoolStreamTimer != -1) return 0;
    g_PoolStreamTimer = SetTimer("Pool_StreamCheck", POOL_STREAM_CHECK_MS, true);
    return 1;
}

stock Pool_StopStreaming() {
    if (g_PoolStreamTimer == -1) return 0;
    KillTimer(g_PoolStreamTimer);
    g_PoolStreamTimer = -1;
    return 1;
}

forward Pool_StreamCheck();
public Pool_StreamCheck() {
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) {
        if (!IsPlayerConnected(playerid)) continue;
        Pool_CheckPlayerRanges(playerid);
    }
}

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
        
        new bool:wasInStream = g_PoolPlayerInfo[playerid][tableID][pinfo_InStreamRange];
        new bool:wasInView = g_PoolPlayerInfo[playerid][tableID][pinfo_InViewRange];
        new bool:wasInPlay = g_PoolPlayerInfo[playerid][tableID][pinfo_InPlayRange];
        
        new bool:isInStream = (dist <= POOL_STREAM_RANGE);
        new bool:isInView = (dist <= POOL_VIEW_RANGE);
        new bool:isInPlay = (dist <= POOL_PLAY_RANGE);
        
        // Handle range transitions
        if (isInStream && !wasInStream) Pool_OnPlayerEnterStreamRange(playerid, tableID);
        if (isInView && !wasInView) Pool_OnPlayerEnterViewRange(playerid, tableID);
        if (isInPlay && !wasInPlay) Pool_OnPlayerEnterPlayRange(playerid, tableID);
        if (!isInPlay && wasInPlay) Pool_OnPlayerExitPlayRange(playerid, tableID);
        if (!isInView && wasInView) Pool_OnPlayerExitViewRange(playerid, tableID);
        if (!isInStream && wasInStream) Pool_OnPlayerExitStreamRange(playerid, tableID);
        
        // Update state
        g_PoolPlayerInfo[playerid][tableID][pinfo_InStreamRange] = isInStream;
        g_PoolPlayerInfo[playerid][tableID][pinfo_InViewRange] = isInView;
        g_PoolPlayerInfo[playerid][tableID][pinfo_InPlayRange] = isInPlay;
    }
}
```

---

## 7. Native Hooks

### 7.1 Required Hooks Summary

| Hook | Function | Usage |
|------|----------|-------|
| `CPhysical::ApplyForce` | Apply force with point | Shot with english |
| `CPhysical::ApplyMoveForce` | Apply linear force | Simple shot |
| `CPhysical::ApplyTurnForce` | Apply torque | Ball spin |
| `CPhysical::GetSpeed` | Get velocity | Detect stopped ball |
| `CCamera::SetPosition` | Position camera | Aiming camera |
| `CCamera::SetLookAt` | Aim camera | Aiming camera |

### 7.2 Hook Implementation

```cpp
// pool_physics.cpp

Result PoolPhysics::Initialize() {
    if (s_initialized) return Result::Success;
    
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD);
    POOL_CHECK(handle != nullptr, Result::ErrorNativeHookFailed, "Failed to get libGTASA handle");
    
    Dl_info info;
    if (dladdr(handle, &info)) {
        s_libBase = reinterpret_cast<uintptr_t>(info.dli_fbase);
    }
    
    POOL_CHECK(s_libBase != 0, Result::ErrorNativeHookFailed, "Failed to get libGTASA base");
    
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

    POOL_CHECK(s_natives.ApplyForce != 0, Result::ErrorNativeHookFailed, "ApplyForce offset invalid");
    POOL_CHECK(s_natives.ApplyMoveForce != 0, Result::ErrorNativeHookFailed, "ApplyMoveForce offset invalid");
    
    return Result::Success;
}

Result PoolPhysics::ApplyShot(CPhysical* cueBall, const ShotData& shot) {
    POOL_CHECK(s_initialized, Result::ErrorNativeHookFailed, "Physics not initialized");
    POOL_CHECK(ValidatePointer(cueBall), Result::ErrorInvalidParameters, "Invalid cue ball pointer");
    POOL_CHECK(shot.power > 0.0f && shot.power <= 1.0f, Result::ErrorInvalidParameters, "Invalid power");
    
    float forceMagnitude = MIN_SHOT_POWER + (shot.power * (MAX_SHOT_POWER - MIN_SHOT_POWER));
    Vec3 force = shot.direction.Normalized() * forceMagnitude;
    
    Vec3 hitOffset(
        shot.englishX * BALL_RADIUS,
        shot.englishY * BALL_RADIUS,
        0.0f
    );
    
    Vec3 position;
    Result result = GetPosition(cueBall, position);
    POOL_CHECK(result == Result::Success, result, "Failed to get cue ball position");
    
    Vec3 hitPoint = position + hitOffset;
    
    if (shot.englishX != 0.0f || shot.englishY != 0.0f) {
        return ApplyForceAtPoint(cueBall, force, hitPoint);
    } else {
        return ApplyMoveForce(cueBall, force);
    }
}
```

---

## 8. Offset Table

### 8.1 CPhysical

| Function | Offset 32-bit | Offset 64-bit | Signature |
|----------|---------------|---------------|-----------|
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

| Function | Offset 32-bit | Offset 64-bit | Usage |
|----------|---------------|---------------|-------|
| `CColSphere::Set` | 0x002e1cc5 | 0x003a4d0c | Set sphere |
| `CColSphere::IntersectRay` | 0x002e1ce1 | 0x003a4d30 | Raycast |
| `CColSphere::IntersectSphere` | 0x002e2035 | 0x003a504c | Ball-ball collision |
| `CCollision::TestSphereSphere` | 0x002d9a11 | 0x0039c648 | Test collision |
| `CCollision::ProcessSphereSphere` | 0x002d9a19 | 0x0039c68c | Process collision |
| `CCollision::TestLineSphere` | 0x002dabbd | 0x0039d5c4 | Aim raycast |
| `CCollision::ProcessLineSphere` | 0x002db031 | 0x0039d8e8 | Process raycast |

### 8.3 CObject

| Function | Offset 32-bit | Offset 64-bit | Usage |
|----------|---------------|---------------|-------|
| `CObject::CObject` | 0x00452c6d | 0x0053b828 | Constructor |
| `CObject::Create` | 0x0045343d | 0x0053c19c | Create object |
| `CObject::ProcessControl` | 0x00453825 | 0x0053c790 | Physics loop |
| `CObject::SetIsStatic` | 0x00455681 | 0x0053e8a8 | Set static |
| `CObject::Render` | 0x00454f55 | 0x0053e0f0 | Render |

### 8.4 Pool Widgets

| Function | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |

### 8.5 Global Variables

| Variable | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `MI_POOL_CUE_BALL` | 0x006aad08 | 0x00876314 |
| `CWorld::SnookerTableMin` | 0x0096b9e8 | 0x00bdcb14 |
| `CWorld::SnookerTableMax` | 0x0096b9dc | 0x00bdcb08 |
| `CTheScripts::bMiniGameInProgress` | 0x00819d80 | 0x009ff3a0 |

---

## 9. Error System

### 9.1 Error Codes

| Code | Name | Description | Action |
|------|------|-------------|--------|
| 0 | `Success` | Operation successful | Continue |
| 1 | `ErrorTableNotFound` | Table doesn't exist | Cancel, log |
| 2 | `ErrorPlayerNotInRange` | Player out of range | Cancel, notify |
| 3 | `ErrorTableInMotion` | Balls still moving | Cancel, notify |
| 4 | `ErrorAlreadyShooting` | Another player shooting | Cancel, notify |
| 5 | `ErrorInvalidParameters` | Invalid parameters | Cancel, log |
| 6 | `ErrorNativeHookFailed` | Native hook failure | Cancel, log warning |
| 7 | `ErrorObjectCreationFailed` | Failed to create object | Cancel, log |
| 8 | `ErrorUnknown` | Unknown error | Cancel, log |

### 9.2 Error Handling Macros

```cpp
// pool_error.h

// Check with early return
#define POOL_CHECK(condition, result, context) \
    do { \
        if (!(condition)) { \
            Pool::PoolError::Report(Pool::ErrorLevel::Error, result, context); \
            return result; \
        } \
    } while(0)

// Check that only logs (doesn't return)
#define POOL_WARN_IF(condition, result, context) \
    do { \
        if (condition) { \
            Pool::PoolError::Report(Pool::ErrorLevel::Warning, result, context); \
        } \
    } while(0)
```

---

## 10. RPCs and Communication

### 10.1 RPC IDs

| ID | Direction | Name | Description |
|----|-----------|------|-------------|
| 100 | S→C | `TABLE_STATE` | Complete state |
| 101 | S→C | `ENTER_VIEW` | Entered view range |
| 102 | S→C | `EXIT_VIEW` | Exited view range |
| 103 | S→C | `ENTER_PLAY` | Entered play range |
| 104 | S→C | `EXIT_PLAY` | Exited play range |
| 105 | S→C | `SHOOT_APPROVED` | Shot approved |
| 106 | S→C | `SHOOT_DENIED` | Shot denied |
| 107 | S→C | `SHOT_BROADCAST` | Shot broadcast |
| 108 | S→C | `BALLS_UPDATE` | Position update |
| 109 | S→C | `RESET` | Table reset |
| 200 | C→S | `REQUEST_SHOOT` | Request to shoot |
| 201 | C→S | `EXECUTE_SHOT` | Execute shot |
| 202 | C→S | `BALLS_SETTLED` | Balls stopped |
| 203 | C→S | `PLACE_CUE` | Position cue ball |
| 204 | C→S | `REQUEST_RESET` | Request reset |

---

## 11. System Files

### 11.1 Directory Structure

```
app/src/main/cpp/samp/game/Pool/
├── pool_constants.h        // Global constants
├── pool_types.h            // Types and structures
├── pool_callbacks.h        // Callback interface
├── pool_error.h            // Error system
├── pool_error.cpp
├── pool_manager.h          // Main manager
├── pool_manager.cpp
├── pool_table_local.h      // Local table
├── pool_table_local.cpp
├── pool_camera.h           // Aiming camera
├── pool_camera.cpp
├── pool_physics.h          // Physics hooks
├── pool_physics.cpp
├── pool_ui.h               // ImGui interface
├── pool_ui.cpp
├── pool_rpc.h              // RPC definitions
└── pool_rpc.cpp            // Serialization/deserialization
```

---

## 12. Integration Guide

### 12.1 Initialization

```cpp
#include "Pool/pool_manager.h"

class MyPoolCallbacks : public Pool::IPoolCallbacks {
public:
    void OnPoolError(Pool::Result error, const char* message) override {
        MyLogger::Error("Pool", message);
    }
    // Implement other methods...
};

MyPoolCallbacks g_poolCallbacks;

void InitializePool() {
    Pool::Result result = Pool::PoolManager::Instance().Initialize(&g_poolCallbacks);
    
    if (result != Pool::Result::Success) {
        MyLogger::Error("Pool", "Failed to initialize: %s", 
                       Pool::ResultToString(result));
    }
}
```

### 12.2 Update Loop

```cpp
void GameUpdate(float deltaTime) {
    if (Pool::PoolManager::Instance().IsInitialized()) {
        Pool::PoolManager::Instance().Update(deltaTime);
    }
}
```

### 12.3 ImGui Rendering

```cpp
void RenderImGui() {
    Pool::PoolManager::Instance().GetUI().Render();
}
```

### 12.4 Server-Side (PAWN)

```pawn
#include "pool/pool_main.inc"

public OnGameModeInit() {
    Pool_StartStreaming();
    Pool_CreateTable(1000.0, 2000.0, 15.0, 0.0);
    return 1;
}

public OnGameModeExit() {
    Pool_StopStreaming();
    return 1;
}
```

---

## Conclusion

This document defines a modular and safe pool system for SA-MP/open.mp. Key features:

1. **Modular**: No external dependencies
2. **Safe**: Errors handled without crashes
3. **Efficient**: Distance-based streaming
4. **Complete**: Client-side + Server-side

For implementation, recommend starting with:
1. Data structures and constants
2. Error system
3. PoolPhysics (hooks)
4. Basic PoolManager
5. Server-side streaming
6. UI and camera

---

*Document generated for SA-MP 2.10 project*
