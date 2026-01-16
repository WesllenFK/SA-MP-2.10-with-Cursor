# Pool System - Detailed Function Tree

## Status Legend

| Symbol | Meaning |
|--------|---------|
| ✅ | Implemented in project |
| ⚠️ | Partially implemented (via hooks) |
| ❌ | Not implemented |

---

## 1. Pool Objects (CObject)

```
CObject ⚠️
├── Lifecycle
│   ├── CObject::CObject() ⚠️
│   │   └── Default constructor
│   ├── CObject::CObject(int modelId, bool) ⚠️
│   │   └── Creates object with specific model
│   ├── CObject::CObject(CDummyObject*) ⚠️
│   │   └── Creates object from dummy
│   ├── CObject::~CObject() ⚠️
│   │   └── Destructor
│   ├── CObject::Init() ⚠️
│   │   └── Initializes object
│   └── CObject::Create(int modelId, bool) ⚠️
│       └── Creates and returns new object
│
├── Rendering
│   ├── CObject::PreRender() ⚠️
│   │   └── Pre-render preparation
│   ├── CObject::Render() ⚠️
│   │   └── Renders object
│   ├── CObject::SetupLighting() ⚠️
│   │   └── Sets up lighting
│   └── CObject::RemoveLighting(bool) ⚠️
│       └── Removes lighting
│
├── Physics and Control
│   ├── CObject::ProcessControl() ⚠️
│   │   └── Processes control logic
│   ├── CObject::ProcessControlLogic() ⚠️
│   │   └── Specific control logic
│   ├── CObject::SetIsStatic(bool) ⚠️
│   │   └── Sets if object is static
│   └── CObject::SpecialEntityPreCollisionStuff(...) ⚠️
│       └── Special pre-collision preparation
│
├── Management
│   ├── CObject::CanBeDeleted() ⚠️
│   │   └── Checks if can be deleted
│   ├── CObject::CanBeTargetted() ⚠️
│   │   └── Checks if can be targeted
│   ├── CObject::SetObjectTargettable(bool) ⚠️
│   │   └── Sets if can be targeted
│   ├── CObject::AddToControlCodeList() ⚠️
│   │   └── Adds to control list
│   └── CObject::RemoveFromControlCodeList() ⚠️
│       └── Removes from control list
│
└── Persistence
    ├── CObject::Save() ⚠️
    │   └── Saves object state
    └── CObject::Load() ⚠️
        └── Loads object state
```

---

## 2. Physics System (CPhysical)

```
CPhysical ⚠️
├── Lifecycle
│   ├── CPhysical::CPhysical() ⚠️
│   │   └── Constructor
│   ├── CPhysical::~CPhysical() ⚠️
│   │   └── Destructor
│   ├── CPhysical::Add() ⚠️
│   │   └── Adds to world
│   └── CPhysical::Remove() ⚠️
│       └── Removes from world
│
├── Force Application
│   ├── CPhysical::ApplyForce(CVector, CVector, bool) ⚠️
│   │   └── Applies force with application point
│   │       Used for "english" effect on shot
│   ├── CPhysical::ApplyMoveForce(CVector) ⚠️
│   │   └── Applies linear movement force
│   │       Main function for shots
│   ├── CPhysical::ApplyTurnForce(CVector, CVector) ⚠️
│   │   └── Applies rotation force
│   │       Generates spin on ball
│   ├── CPhysical::ApplyFrictionMoveForce(CVector) ⚠️
│   │   └── Applies linear friction force
│   ├── CPhysical::ApplyFrictionTurnForce(CVector, CVector) ⚠️
│   │   └── Applies rotational friction force
│   └── CPhysical::ApplyFrictionForce(CVector, CVector) ⚠️
│       └── Applies general friction force
│
├── Velocity
│   ├── CPhysical::ApplySpeed() ⚠️
│   │   └── Applies accumulated velocity
│   ├── CPhysical::ApplyMoveSpeed() ⚠️
│   │   └── Applies linear velocity
│   ├── CPhysical::ApplyTurnSpeed() ⚠️
│   │   └── Applies angular velocity
│   └── CPhysical::GetSpeed(CVector) ⚠️
│       └── Gets velocity at point
│
├── Friction
│   ├── CPhysical::ApplyFriction() ⚠️
│   │   └── Applies general friction
│   │       Slows down balls on table
│   ├── CPhysical::ApplyFriction(float, CColPoint&) ⚠️
│   │   └── Applies friction with collision point
│   └── CPhysical::ApplyFriction(CPhysical*, float, CColPoint&) ⚠️
│       └── Applies friction between two physicals
│
├── Gravity and Resistance
│   ├── CPhysical::ApplyGravity() ⚠️
│   │   └── Applies gravity
│   │       Keeps balls on table
│   └── CPhysical::ApplyAirResistance() ⚠️
│       └── Applies air resistance
│       Minimal for pool balls
│
├── Collision
│   ├── CPhysical::CheckCollision() ⚠️
│   │   └── Checks collisions
│   ├── CPhysical::CheckCollision_SimpleCar() ⚠️
│   │   └── Simplified check
│   ├── CPhysical::TestCollision(bool) ⚠️
│   │   └── Tests collision
│   ├── CPhysical::ApplyCollision(CEntity*, CColPoint&, float&) ⚠️
│   │   └── Applies collision with entity
│   ├── CPhysical::ApplyCollision(CPhysical*, CColPoint&, float&, float&) ⚠️
│   │   └── Applies collision between physicals
│   │       Used for ball-ball collision
│   ├── CPhysical::ApplyCollisionAlt(...) ⚠️
│   │   └── Alternative collision
│   ├── CPhysical::ApplySoftCollision(CEntity*, ...) ⚠️
│   │   └── Soft collision with entity
│   ├── CPhysical::ApplySoftCollision(CPhysical*, ...) ⚠️
│   │   └── Soft collision between physicals
│   ├── CPhysical::ApplySpringCollision(...) ⚠️
│   │   └── Spring collision (table edges)
│   ├── CPhysical::ApplySpringCollisionAlt(...) ⚠️
│   │   └── Alternative spring collision
│   ├── CPhysical::ApplySpringDampening(...) ⚠️
│   │   └── Spring dampening
│   ├── CPhysical::ApplySpringDampeningOld(...) ⚠️
│   │   └── Legacy dampening
│   └── CPhysical::ApplyScriptCollision(...) ⚠️
│       └── Script-driven collision
│
├── Processing
│   ├── CPhysical::ProcessControl() ⚠️
│   │   └── Processes physical control
│   ├── CPhysical::ProcessCollision() ⚠️
│   │   └── Processes collisions
│   ├── CPhysical::ProcessShift() ⚠️
│   │   └── Processes displacement
│   ├── CPhysical::ProcessEntityCollision(CEntity*, CColPoint*) ⚠️
│   │   └── Processes collision with entity
│   ├── CPhysical::ProcessCollisionSectorList(int, int) ⚠️
│   │   └── Processes sector list
│   ├── CPhysical::ProcessCollisionSectorList_SimpleCar(...) ⚠️
│   │   └── Simplified version
│   └── CPhysical::SkipPhysics() ⚠️
│       └── Skips physics processing
│
├── Records
│   ├── CPhysical::AddCollisionRecord(CEntity*) ⚠️
│   │   └── Records collision
│   ├── CPhysical::GetHasCollidedWith(CEntity*) ⚠️
│   │   └── Checks if collided with entity
│   ├── CPhysical::GetHasCollidedWithAnyObject() ⚠️
│   │   └── Checks collision with any object
│   └── CPhysical::SetDamagedPieceRecord(...) ⚠️
│       └── Records damaged piece
│
├── Attachment
│   ├── CPhysical::AttachEntityToEntity(...) ⚠️
│   │   └── Attaches entity to another
│   ├── CPhysical::PositionAttachedEntity() ⚠️
│   │   └── Positions attached entity
│   ├── CPhysical::DettachEntityFromEntity(...) ⚠️
│   │   └── Detaches entity
│   └── CPhysical::DettachAutoAttachedEntity() ⚠️
│       └── Auto-detaches
│
└── Utilities
    ├── CPhysical::GetBoundRect() ⚠️
    │   └── Gets bounding rectangle
    ├── CPhysical::GetLightingTotal() ⚠️
    │   └── Gets total lighting
    ├── CPhysical::GetLightingFromCol(bool) ⚠️
    │   └── Gets lighting from collision
    ├── CPhysical::CanPhysicalBeDamaged(...) ⚠️
    │   └── Checks if can be damaged
    ├── CPhysical::AddToMovingList() ⚠️
    │   └── Adds to moving list
    ├── CPhysical::RemoveFromMovingList() ⚠️
    │   └── Removes from moving list
    ├── CPhysical::RemoveAndAdd() ⚠️
    │   └── Removes and adds again
    ├── CPhysical::RemoveRefsToEntity(CEntity*) ⚠️
    │   └── Removes references
    ├── CPhysical::UnsetIsInSafePosition() ⚠️
    │   └── Unsets safe position
    └── CPhysical::PlacePhysicalRelativeToOtherPhysical(...) ⚠️
        └── Positions relative to another physical
```

---

## 3. Sphere Collision (CColSphere / CCollision)

```
CColSphere ⚠️
├── Configuration
│   └── CColSphere::Set(float, CVector const&, uint8, uint8, uint8) ⚠️
│       └── Sets radius, center, material, flags, lighting
│
└── Intersection
    ├── CColSphere::IntersectRay(CVector const&, CVector const&, CVector&, CVector&) ⚠️
    │   └── Tests intersection with ray
    │       Used for aiming shot
    ├── CColSphere::IntersectEdge(CVector const&, CVector const&, CVector&, CVector&) ⚠️
    │   └── Tests intersection with edge
    │       Collision with table edges
    ├── CColSphere::IntersectPoint(CVector const&) ⚠️
    │   └── Tests if point is inside
    │       Pocket detection
    └── CColSphere::IntersectSphere(CColSphere const&) ⚠️
        └── Tests intersection with another sphere
        Ball-ball collision

CCollision ⚠️
├── Sphere-Sphere Tests
│   ├── CCollision::TestSphereSphere(CColSphere const&, CColSphere const&) ⚠️
│   │   └── Tests if two spheres touch
│   │       Returns bool
│   └── CCollision::ProcessSphereSphere(CColSphere const&, CColSphere const&, CColPoint&, float&) ⚠️
│       └── Processes collision between spheres
│       Calculates contact point and depth
│
├── Line-Sphere Tests
│   ├── CCollision::TestLineSphere(CColLine const&, CColSphere const&) ⚠️
│   │   └── Tests if line intersects sphere
│   │       Used for aim raycast
│   └── CCollision::ProcessLineSphere(CColLine const&, CColSphere const&, CColPoint&, float&) ⚠️
│       └── Processes line-sphere intersection
│       Calculates impact point
│
├── SphereCast
│   ├── CCollision::SphereCastVsSphere(CColSphere*, CColSphere*, CColSphere*) ⚠️
│   │   └── Sphere raycast against sphere
│   │       Trajectory prediction
│   ├── CCollision::SphereCastVsEntity(CColSphere*, CColSphere*, CEntity*) ⚠️
│   │   └── Sphere raycast against entity
│   ├── CCollision::SphereCastVsBBox(CColSphere*, CColSphere*, CColBox*) ⚠️
│   │   └── Sphere raycast against box
│   │       Edge collision
│   ├── CCollision::SphereCastVsCaches(...) ⚠️
│   │   └── Raycast using cache
│   └── CCollision::SphereCastVersusVsPoly(...) ⚠️
│       └── Raycast against polygon
│
├── Sphere-Box/Triangle
│   ├── CCollision::ProcessSphereBox(CColSphere const&, CColBox const&, CColPoint&, float&) ⚠️
│   │   └── Processes sphere-box collision
│   │       Collision with table edges
│   ├── CCollision::TestSphereTriangle(...) ⚠️
│   │   └── Tests sphere against triangle
│   └── CCollision::ProcessSphereTriangle(...) ⚠️
│       └── Processes triangle collision
│
├── Entity
│   └── CCollision::SphereVsEntity(CColSphere*, CEntity*) ⚠️
│       └── Tests sphere against entity
│
└── Camera
    ├── CCollision::BuildCacheOfCameraCollision(...) ⚠️
    │   └── Builds cache for camera
    ├── CCollision::CheckCameraCollisionBuildings(...) ⚠️
    │   └── Checks collision with buildings
    ├── CCollision::CheckCameraCollisionVehicles(...) ⚠️
    │   └── Checks collision with vehicles
    ├── CCollision::CheckCameraCollisionObjects(...) ⚠️
    │   └── Checks collision with objects
    ├── CCollision::CameraConeCastVsWorldCollision(...) ⚠️
    │   └── Camera cone cast
    └── CCollision::GetBoundingBoxFromTwoSpheres(...) ⚠️
        └── Gets bounding box from two spheres
```

---

## 4. Pool Widgets

```
CWidgetPool ❌
├── Lifecycle
│   ├── CWidgetPool::CWidgetPool(char const*, WidgetPosition const&) ❌
│   │   └── Constructor with name and position
│   └── CWidgetPool::~CWidgetPool() ❌
│       └── Destructor
│
├── Update
│   ├── CWidgetPool::Update() ❌
│   │   └── Updates widget state
│   │       Called each frame
│   └── CWidgetPool::Draw() ❌
│       └── Renders widget
│
└── Configuration
    ├── CWidgetPool::SetWidgetInfo2(int, int, int, int, char*, char*) ❌
    │   └── Sets widget info
    └── CWidgetPool::GetName() ❌
        └── Returns widget name

CWidgetRegionPoolEnglish ❌
├── Lifecycle
│   ├── CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish(char const*, WidgetPosition const&) ❌
│   │   └── Constructor
│   └── CWidgetRegionPoolEnglish::~CWidgetRegionPoolEnglish() ❌
│       └── Destructor
│
├── Interaction
│   ├── CWidgetRegionPoolEnglish::GetWidgetValue2(float*, float*) ❌
│   │   └── Gets X/Y touch offset
│   │       Determines english effect on ball
│   │       Values: -1.0 to 1.0 for each axis
│   └── CWidgetRegionPoolEnglish::IsTouched(CVector2D*) ❌
│       └── Checks if being touched
│
└── Utility
    └── CWidgetRegionPoolEnglish::GetName() ❌
        └── Returns widget name

CWidgetRegionPoolBallInHand ❌
├── Lifecycle
│   ├── CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand(char const*, WidgetPosition const&) ❌
│   │   └── Constructor
│   └── CWidgetRegionPoolBallInHand::~CWidgetRegionPoolBallInHand() ❌
│       └── Destructor
│
├── Interaction
│   ├── CWidgetRegionPoolBallInHand::GetWidgetValue2(float*, float*) ❌
│   │   └── Gets touch position
│   │       For positioning cue ball
│   ├── CWidgetRegionPoolBallInHand::SetWidgetValue3(float, float, float) ❌
│   │   └── Sets ball position
│   │       Used after foul
│   └── CWidgetRegionPoolBallInHand::IsTouched(CVector2D*) ❌
│       └── Checks if being touched
│
└── Utility
    └── CWidgetRegionPoolBallInHand::GetName() ❌
        └── Returns widget name
```

---

## 5. Minigame Control

```
CTheScripts ⚠️
├── bMiniGameInProgress ⚠️
│   └── Flag: minigame is active
│       true during pool game
│
└── bDisplayNonMiniGameHelpMessages ⚠️
    └── Flag: display external messages
    false during minigame

CWorld ❌
├── SnookerTableMin ❌
│   └── CVector: minimum table corner
│       Lower X, Y, Z limit
│
└── SnookerTableMax ❌
    └── CVector: maximum table corner
    Upper X, Y, Z limit
```

---

## 6. Surfaces and Materials

```
eSurfaceType ✅
└── SURFACE_P_POOLSIDE = 127 ✅
    └── Surface type: pool table edge
    Defines friction and collision sound

SurfaceInfos_c ⚠️
└── SurfaceInfos_c::GetFrictionEffect(unsigned int) ⚠️
    └── Gets friction effect for surface
    Used to slow down balls
```

---

## 7. Weapon - Pool Cue

```
eWeaponType ✅
└── WEAPON_POOL_CUE = 7 ✅
    └── Pool cue melee weapon slot

eModelID ✅
├── MODEL_POOLCUE = 338 ✅
│   └── Cue 3D model
│
└── MODEL_K_POOLQ = 3122 ✅
    └── Alternative cue model

common.h ✅
└── WEAPON_MODEL_POOLSTICK = 338 ✅
    └── Defines cue model for weapon system
```

---

## Implementation Summary

| Category | Total | ✅ | ⚠️ | ❌ |
|----------|-------|-----|-----|-----|
| CObject | 20 | 0 | 20 | 0 |
| CPhysical | 45 | 0 | 45 | 0 |
| CColSphere | 4 | 0 | 4 | 0 |
| CCollision | 15 | 0 | 15 | 0 |
| CWidgetPool | 6 | 0 | 0 | 6 |
| CWidgetRegionPoolEnglish | 5 | 0 | 0 | 5 |
| CWidgetRegionPoolBallInHand | 6 | 0 | 0 | 6 |
| CTheScripts | 2 | 0 | 2 | 0 |
| CWorld | 2 | 0 | 0 | 2 |
| Enums/Defines | 6 | 6 | 0 | 0 |
| **Total** | **111** | **6** | **86** | **19** |
