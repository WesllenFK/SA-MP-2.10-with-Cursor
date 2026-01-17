# Pool System - GTA San Andreas

## Overview

The GTA San Andreas pool system is a complete minigame that simulates a pool/billiards game with realistic sphere collision physics, surface friction, and specific touch controls for mobile devices.

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                       POOL SYSTEM                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   OBJECTS    │    │   PHYSICS    │    │  COLLISION   │      │
│  │              │    │              │    │              │      │
│  │ • Table      │    │ • CPhysical  │    │ • CColSphere │      │
│  │ • Balls      │    │ • Friction   │    │ • SphereSphere│     │
│  │ • Cue        │    │ • Velocity   │    │ • LineSphere │      │
│  │ • Triangle   │    │ • Force      │    │ • Raycasting │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│          │                  │                   │               │
│          └──────────────────┼───────────────────┘               │
│                             ▼                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   WIDGETS    │    │   SCRIPTS    │    │    AUDIO     │      │
│  │              │    │              │    │              │      │
│  │ • English    │    │ • MiniGame   │    │ • Collisions │      │
│  │ • BallInHand │    │ • InProgress │    │ • Shots      │      │
│  │ • Pool       │    │ • Rules      │    │ • Pocketed   │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Main Components

### 1. Game Objects

#### Pool Models (eModelID.h)

| Model | ID | Description |
|-------|-----|-------------|
| `MODEL_POOLCUE` | 338 | Pool cue (weapon) |
| `MODEL_K_POOLTABLESM` | 2964 | Small pool table |
| `MODEL_K_POOLTRIANGLE01` | 2965 | Rack triangle |
| `MODEL_K_POOLBALLSTP01-07` | 2995-3001 | Striped balls (1-7) |
| `MODEL_K_POOLBALLSPT01-07` | 3002, 3100-3105 | Solid balls |
| `MODEL_K_POOLBALLCUE` | 3003 | Cue ball (white) |
| `MODEL_K_POOLBALL8` | 3106 | 8-ball (black) |
| `MODEL_K_POOLTABLEB` | 3094 | Large pool table |
| `MODEL_K_POOLQ` | 3122 | Alternative cue |
| `MODEL_K_POOLQ2` | 3004 | Decorative cue |

#### Global Variable

```cpp
// Dump 32-bit: 0x006aad08
// Dump 64-bit: 0x00876314 (memory offset: 0x00886314)
MI_POOL_CUE_BALL  // Cue ball Model ID
```

---

### 2. Physics System

Pool physics uses GTA SA's `CPhysical` system, with specific functions for sphere collision simulation.

#### Movement Physics

```cpp
// Apply force on shot
CPhysical::ApplyMoveForce(CVector force);

// Force with application point (for english effect)
CPhysical::ApplyForce(CVector force, CVector point, bool);

// Velocity and direction
CPhysical::ApplySpeed();
CPhysical::ApplyMoveSpeed();
CPhysical::ApplyTurnSpeed();
CPhysical::ApplyTurnForce(CVector force, CVector point);
```

#### Table Friction

```cpp
// General friction
CPhysical::ApplyFriction();
CPhysical::ApplyFriction(float friction, CColPoint& colPoint);
CPhysical::ApplyFriction(CPhysical* other, float friction, CColPoint& colPoint);

// Friction force
CPhysical::ApplyFrictionMoveForce(CVector force);
CPhysical::ApplyFrictionTurnForce(CVector force, CVector point);
CPhysical::ApplyFrictionForce(CVector force, CVector point);

// Air resistance (minimal for pool balls)
CPhysical::ApplyAirResistance();
```

#### Gravity and Control

```cpp
CPhysical::ApplyGravity();
CPhysical::ProcessControl();
CPhysical::ProcessCollision();
CPhysical::SkipPhysics();
```

---

### 3. Collision System

Pool heavily depends on sphere-sphere and sphere-line collision systems.

#### CColSphere

```cpp
// Basic structure
class CColSphere {
    CVector m_vecCenter;    // Sphere center
    float   m_fRadius;      // Radius
    uint8   m_nMaterial;    // Surface material
    uint8   m_nFlags;       // Collision flags
    uint8   m_nLighting;    // Lighting
};

// Main methods
void Set(float radius, CVector const& center, uint8 material, uint8 flags, uint8 lighting);
bool IntersectRay(CVector const& origin, CVector const& dir, CVector& hit1, CVector& hit2) const;
bool IntersectEdge(CVector const& p1, CVector const& p2, CVector& hit1, CVector& hit2) const;
bool IntersectPoint(CVector const& point) const;
bool IntersectSphere(CColSphere const& other) const;
```

#### Collision Functions (CCollision)

| Function | Description |
|----------|-------------|
| `TestSphereSphere` | Tests if two spheres touch |
| `ProcessSphereSphere` | Processes sphere collision with response |
| `TestLineSphere` | Tests if line intersects sphere |
| `ProcessLineSphere` | Processes line-sphere intersection |
| `SphereCastVsSphere` | Sphere raycast against sphere |
| `SphereCastVsEntity` | Sphere raycast against entity |

---

### 4. Interface Widgets (Mobile)

#### Widget Positions (TouchInterface.h)

| Constant | Value | Usage |
|----------|-------|-------|
| `WIDGET_POSITION_POOL_ENGLISH` | 0x16 | Ball effect control |
| `WIDGET_POSITION_POOL` | 0x81 | Main pool widget |

#### Widget IDs

| Constant | Value | Usage |
|----------|-------|-------|
| `WIDGET_POOL_ENGLISH` | 0x23 | English effect on shot |
| `WIDGET_POOL_BALL_IN_HAND` | 0x24 | Position cue ball |
| `WIDGET_POOL` | 0xB5 | Pool interface |

---

### 5. Widget Classes

#### CWidgetPool

Main widget that manages the minigame interface.

```cpp
class CWidgetPool {
public:
    CWidgetPool(char const* name, WidgetPosition const& pos);
    ~CWidgetPool();
    
    void Update();
    void Draw();
    void SetWidgetInfo2(int, int, int, int, char*, char*);
    char* GetName();
};
```

| Function | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetPool::SetWidgetInfo2` | 0x002be9d5 | 0x0037ecd4 |
| `CWidgetPool::GetName` | 0x002bea6d | 0x0037edb0 |
| `CWidgetPool::~CWidgetPool` | 0x002bea5d | 0x0037ed8c |

#### CWidgetRegionPoolEnglish

Controls the "english" effect (spin applied to cue ball).

```cpp
class CWidgetRegionPoolEnglish {
public:
    CWidgetRegionPoolEnglish(char const* name, WidgetPosition const& pos);
    ~CWidgetRegionPoolEnglish();
    
    void GetWidgetValue2(float* x, float* y);  // Gets touch offset
    bool IsTouched(CVector2D* touchPos);
    char* GetName();
};
```

| Function | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish` | 0x002c1e29 | 0x00382814 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolEnglish::IsTouched` | 0x002c1e7d | 0x00382890 |
| `CWidgetRegionPoolEnglish::GetName` | 0x002c1ea9 | 0x003828d0 |
| `CWidgetRegionPoolEnglish::~CWidgetRegionPoolEnglish` | 0x002c1e99 | 0x003828ac |

#### CWidgetRegionPoolBallInHand

Allows player to position cue ball after foul.

```cpp
class CWidgetRegionPoolBallInHand {
public:
    CWidgetRegionPoolBallInHand(char const* name, WidgetPosition const& pos);
    ~CWidgetRegionPoolBallInHand();
    
    void GetWidgetValue2(float* x, float* y);
    void SetWidgetValue3(float x, float y, float z);  // Sets ball position
    bool IsTouched(CVector2D* touchPos);
    char* GetName();
};
```

| Function | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand` | 0x002c1c91 | 0x00382650 |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |
| `CWidgetRegionPoolBallInHand::IsTouched` | 0x002c1d65 | 0x0038272c |
| `CWidgetRegionPoolBallInHand::GetName` | 0x002c1e19 | 0x00382808 |
| `CWidgetRegionPoolBallInHand::~CWidgetRegionPoolBallInHand` | 0x002c1e07 | 0x003827e4 |

---

### 6. Table Bounds

```cpp
// Global variables that define the physical table boundaries
// Used to detect when balls leave the play area

// Dump 32-bit
CWorld::SnookerTableMin  // 0x0096b9e8 - Minimum corner (X, Y, Z)
CWorld::SnookerTableMax  // 0x0096b9dc - Maximum corner (X, Y, Z)

// Dump 64-bit
CWorld::SnookerTableMin  // 0x00bdcb14
CWorld::SnookerTableMax  // 0x00bdcb08
```

---

### 7. Minigame Control

```cpp
// Global flag indicating if a minigame is active
CTheScripts::bMiniGameInProgress  // 32-bit: 0x00819d80 | 64-bit: 0x009ff3a0

// Flag for displaying help messages outside minigame
CTheScripts::bDisplayNonMiniGameHelpMessages  // 32-bit: 0x00819d81 | 64-bit: 0x009ff3a1
```

---

### 8. Weapon - Pool Cue

The cue also functions as a melee weapon.

```cpp
// eWeaponType.h
WEAPON_POOL_CUE = 7  // Cue weapon slot

// common.h
#define WEAPON_MODEL_POOLSTICK 338  // Cue 3D model
```

---

## Gameplay Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                    MINIGAME FLOW                                │
└─────────────────────────────────────────────────────────────────┘

[1. START]
    │
    ▼
[Player interacts with pool table]
    │
    ▼
[CTheScripts::bMiniGameInProgress = true]
    │
    ▼
[2. POSITIONING] ◄────────────────────────────────────────┐
    │                                                      │
    ▼                                                      │
[Pool widgets activated]                                   │
    │                                                      │
    ▼                                                      │
[3. AIM AND POWER]                                         │
    │                                                      │
    ├── CWidgetPool::Update()                              │
    ├── CWidgetRegionPoolEnglish::GetWidgetValue2()        │
    │   (Get english effect)                               │
    │                                                      │
    ▼                                                      │
[4. SHOT]                                                  │
    │                                                      │
    ├── CPhysical::ApplyForce() on cue ball                │
    ├── Direction + Power + English                        │
    │                                                      │
    ▼                                                      │
[5. PHYSICS SIMULATION]                                    │
    │                                                      │
    ├── Physics loop:                                      │
    │   ├── CPhysical::ApplySpeed()                        │
    │   ├── CPhysical::ApplyFriction()                     │
    │   ├── CCollision::TestSphereSphere()                 │
    │   ├── CCollision::ProcessSphereSphere()              │
    │   └── Check bounds (SnookerTableMin/Max)             │
    │                                                      │
    ▼                                                      │
[6. CHECK RESULT]                                          │
    │                                                      │
    ├── Ball pocketed? ───► Update score                   │
    ├── Cue ball pocketed? ───► Foul                       │
    │   └── CWidgetRegionPoolBallInHand active             │
    ├── 8-ball pocketed early? ───► Loss                   │
    │                                                      │
    ▼                                                      │
[7. NEXT TURN]                                             │
    │                                                      │
    ├── Switch player (if multiplayer)                     │
    └──────────────────────────────────────────────────────┘

[8. GAME END]
    │
    ├── 8-ball pocketed correctly ───► Victory
    │
    ▼
[CTheScripts::bMiniGameInProgress = false]
```

---

## English Shot Physics

The "english" system allows applying lateral spin to the cue ball, affecting its trajectory after collisions.

```
┌─────────────────────────────────────────────────────────────────┐
│                    ENGLISH EFFECT                               │
└─────────────────────────────────────────────────────────────────┘

        Top view of cue ball:
        
              [TOP]
                │
     [LEFT] ────┼──── [RIGHT]
                │
            [BOTTOM]

    Touch center:   Straight shot, no effect
    Touch left:     Ball curves left after collision
    Touch right:    Ball curves right after collision
    Touch top:      "Follow" - ball continues after collision
    Touch bottom:   "Draw" - ball returns after collision

    Implementation:
    CWidgetRegionPoolEnglish::GetWidgetValue2(&offsetX, &offsetY);
    
    // Apply force with offset from center
    CVector forcePoint = ballCenter + CVector(offsetX, offsetY, 0);
    CPhysical::ApplyForce(force, forcePoint, true);
```

---

## Complete Offset Table

### Physics (CPhysical)

| Function | Offset 32-bit | Offset 64-bit | Description |
|----------|---------------|---------------|-------------|
| `ApplyForce` | 0x003fd619 | 0x004e0f00 | Apply force with point |
| `ApplyMoveForce` | 0x003fd4d9 | 0x004e0de0 | Apply movement force |
| `ApplyTurnForce` | 0x003fd541 | 0x004e0e28 | Apply rotation force |
| `ApplySpeed` | 0x003fda95 | 0x004e1300 | Apply velocity |
| `ApplyMoveSpeed` | 0x003fd86d | 0x004e1104 | Apply linear velocity |
| `ApplyTurnSpeed` | 0x003fd8d5 | 0x004e116c | Apply angular velocity |
| `ApplyGravity` | 0x003fe785 | 0x004e1e74 | Apply gravity |
| `ApplyFriction` | 0x003fe819 | 0x004e1f34 | Apply general friction |
| `ApplyFriction (colPoint)` | 0x003fe2a9 | 0x004e1a50 | Friction with point |
| `ApplyFriction (physical)` | 0x00405f5d | 0x004e9248 | Friction between physicals |
| `ApplyFrictionMoveForce` | 0x003feb0d | 0x004e2218 | Linear friction force |
| `ApplyFrictionTurnForce` | 0x003feb75 | 0x004e2260 | Angular friction force |
| `ApplyFrictionForce` | 0x003fec71 | 0x004e2358 | General friction force |
| `ApplyAirResistance` | 0x003fe9dd | 0x004e2104 | Air resistance |
| `ApplyCollision` | 0x00401b75 | 0x004e5598 | Collision with entity |
| `ApplyCollision (physical)` | 0x00401e81 | 0x004e583c | Collision between physicals |
| `ApplyCollisionAlt` | 0x00403a6d | 0x004e7108 | Alternative collision |
| `ApplySoftCollision` | 0x004040ed | 0x004e76fc | Soft collision |
| `ProcessControl` | 0x003fedcd | 0x004e2668 (nearby) | Process control |
| `ProcessCollision` | 0x003fef49 | 0x004e2668 | Process collision |
| `CheckCollision` | 0x003ff655 | 0x004e2e30 | Check collision |
| `TestCollision` | 0x004019bd | 0x004e532c | Test collision |
| `GetSpeed` | 0x003fd775 | 0x004e1038 | Get velocity |

### Sphere Collision (CCollision / CColSphere)

| Function | Offset 32-bit | Offset 64-bit | Description |
|----------|---------------|---------------|-------------|
| `CColSphere::Set` | 0x002e1cc5 | 0x003a4d0c | Set sphere |
| `CColSphere::IntersectRay` | 0x002e1ce1 | 0x003a4d30 | Intersect ray |
| `CColSphere::IntersectEdge` | 0x002e1e03 | 0x003a4e1c | Intersect edge |
| `CColSphere::IntersectPoint` | 0x002e1ff5 | 0x003a5010 | Intersect point |
| `CColSphere::IntersectSphere` | 0x002e2035 | 0x003a504c | Intersect sphere |
| `TestSphereSphere` | 0x002d9a19 (nearby) | 0x0039c648 | Test sphere-sphere |
| `ProcessSphereSphere` | 0x002d9a19 | 0x0039c68c | Process sphere-sphere |
| `TestLineSphere` | 0x002dabbd | 0x0039d5c4 | Test line-sphere |
| `ProcessLineSphere` | 0x002db031 | 0x0039d8e8 | Process line-sphere |
| `SphereCastVsSphere` | 0x002e0135 | 0x003a2e40 | Raycast sphere-sphere |
| `SphereCastVsEntity` | 0x002deb51 | 0x003a1660 | Raycast sphere-entity |

### Pool Widgets

| Function | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetPool::SetWidgetInfo2` | 0x002be9d5 | 0x0037ecd4 |
| `CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish` | 0x002c1e29 | 0x00382814 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolEnglish::IsTouched` | 0x002c1e7d | 0x00382890 |
| `CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand` | 0x002c1c91 | 0x00382650 |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |
| `CWidgetRegionPoolBallInHand::IsTouched` | 0x002c1d65 | 0x0038272c |

### Global Variables

| Variable | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `MI_POOL_CUE_BALL` | 0x006aad08 | 0x00876314 |
| `CWorld::SnookerTableMin` | 0x0096b9e8 | 0x00bdcb14 |
| `CWorld::SnookerTableMax` | 0x0096b9dc | 0x00bdcb08 |
| `CTheScripts::bMiniGameInProgress` | 0x00819d80 | 0x009ff3a0 |
| `CTheScripts::bDisplayNonMiniGameHelpMessages` | 0x00819d81 | 0x009ff3a1 |

---

## Project Implementation

### Implementation Status

| Component | Status | Notes |
|-----------|--------|-------|
| Ball models | ✅ | Defined in eModelID.h |
| Cue as weapon | ✅ | WEAPON_POOL_CUE in eWeaponType.h |
| Widget IDs | ✅ | Defined in TouchInterface.h |
| CPhysical | ⚠️ | Partially via hooks |
| CColSphere | ⚠️ | Partially via hooks |
| CWidgetPool | ❌ | Not implemented |
| CWidgetRegionPoolEnglish | ❌ | Not implemented |
| CWidgetRegionPoolBallInHand | ❌ | Not implemented |
| Table bounds | ❌ | Not implemented |

### Relevant Project Files

```
app/src/main/cpp/samp/game/
├── Enums/
│   ├── eModelID.h          # Pool model IDs
│   ├── eWeaponType.h       # WEAPON_POOL_CUE
│   ├── eSurfaceType.h      # SURFACE_P_POOLSIDE
│   └── HIDMapping.h        # Input mappings
├── Widgets/
│   └── TouchInterface.h    # Pool widgets
├── Entity/
│   └── Object.h            # CObjectGta (ball base)
├── object.h                # CObject wrapper
└── common.h                # WEAPON_MODEL_POOLSTICK
```

---

## SA-MP Considerations

### Multiplayer Implementation Challenges

1. **Physics Synchronization**
   - Multiple balls with independent physics
   - Chain collisions (domino effect)
   - Friction and velocity must be consistent

2. **Suggested Network Structure**

```cpp
struct PoolGameState {
    uint16_t player1ID;
    uint16_t player2ID;
    uint8_t  currentPlayer;     // 0 or 1
    uint8_t  gameType;          // 8-ball, 9-ball, etc.
    
    struct BallState {
        uint8_t  ballID;        // 0-15 (0 = cue ball)
        CVector  position;
        CVector  velocity;
        bool     pocketed;
    } balls[16];
    
    uint8_t  player1Balls;      // Bitmask of P1's balls
    uint8_t  player2Balls;      // Bitmask of P2's balls
    bool     ball8Assignable;   // If 8-ball can be assigned
};
```

3. **Server-Side Validation**
   - Verify valid shots
   - Calculate physics on server
   - Broadcast state to clients

---

## References

- Dumps: `dumps_libGTASA_32and64/`
- Source code: `app/src/main/cpp/samp/game/`
- Widget files: `FrontEnd/WidgetPool.cpp`, `FrontEnd/WidgetRegionPoolBallInHand.cpp`, `FrontEnd/WidgetRegionPoolEnglish.cpp`
