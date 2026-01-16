# Basketball System - GTA San Andreas

## Index
1. [Overview](#1-overview)
2. [System Architecture](#2-system-architecture)
3. [Object System](#3-object-system)
4. [Physics System](#4-physics-system)
5. [Animation/Tasks System](#5-animationtasks-system)
6. [UI/Widgets System](#6-uiwidgets-system)
7. [Script System](#7-script-system)
8. [Gameplay Flow](#8-gameplay-flow)
9. [Project Implementation](#9-project-implementation)

---

## 1. Overview

The basketball system in GTA San Andreas is a **minigame** that uses multiple game systems:

- **CObject** - The basketball as a physical object
- **CPhysical** - Ball physics (gravity, collision, bounce)
- **CTaskSimpleHoldEntity** - Holding the ball
- **CTaskSimpleThrowControl/Projectile** - Throwing the ball
- **Widgets** - UI controls for mobile
- **CTheScripts** - Minigame active flag

### Main Components:
- `CObject` / `CObjectGta` - Ball object
- `CPhysical` - Physics system
- `CTaskSimpleHoldEntity` - Task for holding objects
- `CTaskSimpleThrowControl` - Throw control task
- `CTaskSimpleThrowProjectile` - Throw execution task
- `CWidgetButtonBasketballShoot` - Shoot button (mobile)
- `CWidgetRegionBasketBallJump` - Jump region (mobile)

---

## 2. System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                    BASKETBALL SYSTEM - GTA SA                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐           │
│  │   CObject    │───▶│  CPhysical   │───▶│  Collision   │           │
│  │ (Basketball) │    │  (Physics)   │    │   (Hoop)     │           │
│  └──────────────┘    └──────────────┘    └──────────────┘           │
│         │                   │                   │                    │
│         ▼                   ▼                   ▼                    │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐           │
│  │ Hold Entity  │    │    Apply     │    │   Script     │           │
│  │    Task      │    │Force/Gravity │    │   (Score)    │           │
│  └──────────────┘    └──────────────┘    └──────────────┘           │
│         │                                                            │
│         ▼                                                            │
│  ┌──────────────┐    ┌──────────────┐                               │
│  │    Throw     │    │   Widgets    │                               │
│  │    Task      │◀───│  (UI Touch)  │                               │
│  └──────────────┘    └──────────────┘                               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Object System

### 3.1 Ball Model
```cpp
// Basketball model ID
enum eModelID {
    MODEL_BASKETBALL = 2114,  // 3D ball model
};

// Global variable with model ID
MI_BASKETBALL // 0x6AAD06 (32-bit) / 0x876312 (64-bit)
```

### 3.2 CObjectGta (Object Structure)

```cpp
class CObjectGta : public CPhysical {
    uintptr*    m_pControlCodeList;     // Control code list
    uint8       m_nObjectType;          // Type (OBJECT_GAME, etc)
    uint8       m_nBonusValue;          // Bonus value
    uint16      m_wCostValue;           // Cost
    
    // Object flags (32 bits)
    union {
        struct {
            uint32 bIsPickup : 1;               // Is pickup
            uint32 bIsTargetable : 1;           // Can be targeted
            uint32 bIsLiftable : 1;             // Can be lifted
            uint32 bCanBeAttachedToMagnet : 1;  // Can attach to magnet
            // ...
        } objectFlags;
        uint32 m_nObjectFlags;
    };
    
    uint8       m_nColDamageEffect;     // Collision damage effect
    float       m_fHealth;              // Object health
    float       m_fScale;               // Scale
    uintptr     *m_pObjectInfo;         // Object info
};
// Size: 0x184 (32-bit) / 0x1F8 (64-bit)
```

### 3.3 Main CObject Functions

| Function | Description |
|----------|-------------|
| `CObject::Create(int, bool)` | Creates object by model ID |
| `CObject::Init()` | Initializes structures |
| `CObject::ProcessControl()` | Per-frame update |
| `CObject::ObjectDamage(...)` | Processes damage |
| `CObject::Teleport(CVector, uint8)` | Teleports |
| `CObject::SetIsStatic(bool)` | Sets static/dynamic |
| `CObject::Explode()` | Explodes object |
| `CObject::Render()` | Renders |

---

## 4. Physics System

### 4.1 CPhysical - Physics Functions

The basketball uses `CPhysical` system for realistic movement.

```cpp
class CPhysical : public CEntity {
    // Apply forces
    void ApplyForce(CVector force, CVector point, bool);
    void ApplyMoveForce(CVector force);
    void ApplyTurnForce(CVector force, CVector point);
    
    // Speed
    void ApplySpeed();
    void ApplyMoveSpeed();
    void ApplyTurnSpeed();
    
    // Gravity and resistance
    void ApplyGravity();
    void ApplyAirResistance();
    void ApplyFriction();
    
    // Collision
    void ApplyCollision(CEntity*, CColPoint&, float&);
    void ApplyCollision(CPhysical*, CColPoint&, float&, float&);
    
    // Springs (for bouncing)
    void ApplySpringCollision(float, CVector&, CVector&, float, float, float&);
    void ApplySpringDampening(float, float, CVector&, CVector&, CVector&);
};
```

### 4.2 Ball Physics

When the ball is thrown:

1. **ApplyMoveForce** - Applies initial throw force
2. **ApplyGravity** - Gravity pulls ball down
3. **ApplyAirResistance** - Air resistance decelerates
4. **ApplyCollision** - Collision with hoop, floor, etc
5. **ApplySpringCollision** - Bounce effect

---

## 5. Animation/Tasks System

### 5.1 Related Tasks

Basketball uses the Task system to manage ped actions.

#### CTaskSimpleThrowControl
Controls the throw.

```cpp
class CTaskSimpleThrowControl : public CTaskSimple {
    CEntity* m_pTargetEntity;   // Target (hoop)
    CVector  m_vecTarget;       // Target position
    
    bool ProcessPed(CPed*);
    bool MakeAbortable(CPed*, int, CEvent const*);
};
```

#### CTaskSimpleThrowProjectile
Executes the physical throw.

```cpp
class CTaskSimpleThrowProjectile : public CTaskSimple {
    CEntity* m_pProjectile;     // Object being thrown
    CVector  m_vecTarget;       // Target position
    
    bool ProcessPed(CPed*);
    void ControlThrow(bool, CEntity*, CVector*);
    void StartAnim(CPed*);
    
    static void FinishAnimThrowProjectileCB(CAnimBlendAssociation*, void*);
};
```

---

## 6. UI/Widgets System

### 6.1 Basketball Widgets (Mobile)

For touch control on mobile, the game has specific widgets.

#### CWidgetButtonBasketballShoot
Shoot button.

| Function | 32-bit | 64-bit |
|----------|--------|--------|
| Constructor | `0x2B51F1` | `0x37461C` |
| Update | `0x2B5219` | `0x374658` |
| GetName | `0x2B53B9` | `0x374814` |

#### CWidgetRegionBasketBallJump
Basketball jump region.

| Function | 32-bit | 64-bit |
|----------|--------|--------|
| Constructor | `0x2BFF19` | `0x380588` |
| Update | `0x2BFF4D` | `0x3805D0` |
| OnInitialTouch | `0x2BFF69` | `0x380600` |
| IsTouched | `0x2BFF7B` | `0x380628` |

### 6.2 Widget IDs

```cpp
enum WidgetPosition {
    WIDGET_POSITION_BASKETBALL_SHOOT = 0x34,
    WIDGET_POSITION_BASKETBALL_TRICK = 0x35,
    WIDGET_POSITION_SPRINT_AND_BASKETBALL_JUMP = 0x77,
};

enum WidgetId {
    WIDGET_BASKETBALL_SHOOT = 0x57,
    WIDGET_BASKETBALL_TRICK = 0x58,
    WIDGET_BASKETBALL_JUMP = 0xA9,
};
```

---

## 7. Script System

### 7.1 Minigame Flag

The game uses flags to indicate a minigame is active.

```cpp
class CTheScripts {
    static bool bMiniGameInProgress;              // Minigame active
    static bool bDisplayNonMiniGameHelpMessages;  // Show help
};
```

| Variable | 32-bit | 64-bit |
|----------|--------|--------|
| `bMiniGameInProgress` | `0x819D80` | `0x9FF3A0` |
| `bDisplayNonMiniGameHelpMessages` | `0x819D81` | `0x9FF3A1` |

### 7.2 CScriptedBrainTaskStore

Stores scripted tasks for peds.

| Function | 32-bit | 64-bit | Description |
|----------|--------|--------|-------------|
| `SetTask` | `0x4F197D` | `0x5F94A0` | Sets task |
| `GetTask` | `0x4F19F9` | `0x5F955C` | Gets task |
| `Clear(CPed*)` | `0x4EE425` | `0x5F4870` | Clears task |

---

## 8. Gameplay Flow

### 8.1 Starting Minigame

```
Player enters basketball area
    │
    ├──▶ CTheScripts::bMiniGameInProgress = true
    │
    ├──▶ CObject::Create(MODEL_BASKETBALL, ...)
    │         └── Creates basketball
    │
    ├──▶ Shows basketball widgets
    │         ├── WIDGET_BASKETBALL_SHOOT
    │         └── WIDGET_BASKETBALL_JUMP
    │
    └──▶ Activates minigame scripts
```

### 8.2 Throw Flow

```
Player presses shoot button
    │
    ├──▶ CWidgetButtonBasketballShoot::Update()
    │
    ├──▶ CTaskSimpleHoldEntity::CanThrowEntity()
    │
    ├──▶ CTaskSimpleThrowProjectile::StartAnim()
    │
    ├──▶ CPhysical::ApplyMoveForce()
    │         └── Initial ball force
    │
    ├──▶ CPhysical::ApplyGravity() [per frame]
    │
    ├──▶ Collision detected with hoop
    │         │
    │         ├── Hit: Script increments score
    │         └── Miss: Ball falls
    │
    └──▶ CTaskSimpleThrowProjectile::FinishAnimCB()
```

---

## 9. Project Implementation

### 9.1 Implementation Status

| Component | Status | Description |
|-----------|:------:|-------------|
| CObject wrapper | ✅ | Basic wrapper implemented |
| CObjectGta struct | ✅ | Structure defined |
| Physics hooks | ⚠️ | Partial (via native calls) |
| Widget classes | ❌ | Not implemented |
| Task classes | ❌ | Not implemented |
| Script flags | ❌ | Not implemented |

---

## Offset Table

### CObject

| Function | 32-bit | 64-bit | Description |
|----------|--------|--------|-------------|
| `CObject::CObject()` | `0x452C6D` | `0x53B828` | Empty constructor |
| `CObject::CObject(int, bool)` | `0x453011` | `0x53BC64` | By model ID |
| `CObject::~CObject` | `0x4530FD` | `0x53BDA0` | Destructor |
| `CObject::Init` | `0x452C99` | `0x53B868` | Initialize |
| `CObject::Create(int, bool)` | `0x45343D` | `0x53C19C` | Create by ID |
| `CObject::ProcessControl` | `0x453825` | `0x53C790` | Update |
| `CObject::ObjectDamage` | `0x453F71` | `0x53CF40` | Damage |
| `CObject::Render` | `0x454F55` | `0x53E0F0` | Render |
| `CObject::Teleport` | `0x455BB9` | `0x53EEDC` | Teleport |
| `CObject::Explode` | `0x4557A1` | `0x53EA24` | Explode |
| `CObject::Load` | `0x484FAD` | `0x577274` | Load |
| `CObject::Save` | `0x484F41` | `0x5771E4` | Save |

### CPhysical (Ball relevant)

| Function | 32-bit | 64-bit | Description |
|----------|--------|--------|-------------|
| `ApplyMoveForce` | `0x3FD4D9` | `0x4E0DE0` | Movement force |
| `ApplyTurnForce` | `0x3FD541` | `0x4E0E28` | Rotation force |
| `ApplySpeed` | `0x3FDA95` | `0x4E1300` | Apply velocity |
| `ApplyGravity` | `0x3FE785` | `0x4E1E74` | Gravity |
| `ApplyAirResistance` | `0x3FE9DD` | `0x4E2104` | Air resistance |
| `ApplyCollision` | `0x401B75` | `0x4E5598` | Collision |
| `ApplySpringCollision` | `0x405A11` | `0x4E8DF4` | Bounce |

### Throw Tasks

| Function | 32-bit | Description |
|----------|--------|-------------|
| `CTaskSimpleHoldEntity::CanThrowEntity` | `0x5421C9` | Can throw |
| `CTaskSimpleThrowControl::ProcessPed` | `0x4DEF91` | Process |
| `CTaskSimpleThrowProjectile::ProcessPed` | `0x4DEAF9` | Process |
| `CTaskSimpleThrowProjectile::ControlThrow` | `0x4DEE05` | Control |
| `CTaskSimpleThrowProjectile::StartAnim` | `0x4DECB1` | Start anim |

---

**Document created for GTA San Andreas basketball system analysis.**
**Based on libGTASA version 2.1 dumps (32-bit and 64-bit).**
