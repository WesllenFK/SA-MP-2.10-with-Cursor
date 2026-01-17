# Pool System - Offset Table

## Legend

| Symbol | Meaning |
|--------|---------|
| ✅ | Implemented in project |
| ⚠️ | Partially implemented (via hooks) |
| ❌ | Not implemented |

---

## 1. Pool Widgets

### CWidgetPool

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 | ❌ |
| `CWidgetPool::~CWidgetPool` | 0x002bea5d | 0x0037ed8c | ❌ |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 | ❌ |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 | ❌ |
| `CWidgetPool::SetWidgetInfo2` | 0x002be9d5 | 0x0037ecd4 | ❌ |
| `CWidgetPool::GetName` | 0x002bea6d | 0x0037edb0 | ❌ |
| `vtable for CWidgetPool` | 0x00664184 | 0x00819380 | ❌ |

### CWidgetRegionPoolEnglish

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish` | 0x002c1e29 | 0x00382814 | ❌ |
| `CWidgetRegionPoolEnglish::~CWidgetRegionPoolEnglish` | 0x002c1e99 | 0x003828ac | ❌ |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c | ❌ |
| `CWidgetRegionPoolEnglish::IsTouched` | 0x002c1e7d | 0x00382890 | ❌ |
| `CWidgetRegionPoolEnglish::GetName` | 0x002c1ea9 | 0x003828d0 | ❌ |
| `vtable for CWidgetRegionPoolEnglish` | 0x0066499c | 0x0081a3b0 | ❌ |

### CWidgetRegionPoolBallInHand

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand` | 0x002c1c91 | 0x00382650 | ❌ |
| `CWidgetRegionPoolBallInHand::~CWidgetRegionPoolBallInHand` | 0x002c1e07 | 0x003827e4 | ❌ |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 | ❌ |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 | ❌ |
| `CWidgetRegionPoolBallInHand::IsTouched` | 0x002c1d65 | 0x0038272c | ❌ |
| `CWidgetRegionPoolBallInHand::GetName` | 0x002c1e19 | 0x00382808 | ❌ |
| `vtable for CWidgetRegionPoolBallInHand` | 0x00664908 | 0x0081a288 | ❌ |

---

## 2. Physics System (CPhysical)

### Force Application

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::ApplyForce` | 0x003fd619 | 0x004e0f00 | ⚠️ |
| `CPhysical::ApplyMoveForce` | 0x003fd4d9 | 0x004e0de0 | ⚠️ |
| `CPhysical::ApplyTurnForce` | 0x003fd541 | 0x004e0e28 | ⚠️ |
| `CPhysical::ApplyFrictionMoveForce` | 0x003feb0d | 0x004e2218 | ⚠️ |
| `CPhysical::ApplyFrictionTurnForce` | 0x003feb75 | 0x004e2260 | ⚠️ |
| `CPhysical::ApplyFrictionForce` | 0x003fec71 | 0x004e2358 | ⚠️ |

### Velocity

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::ApplySpeed` | 0x003fda95 | 0x004e1300 | ⚠️ |
| `CPhysical::ApplyMoveSpeed` | 0x003fd86d | 0x004e1104 | ⚠️ |
| `CPhysical::ApplyTurnSpeed` | 0x003fd8d5 | 0x004e116c | ⚠️ |
| `CPhysical::GetSpeed` | 0x003fd775 | 0x004e1038 | ⚠️ |

### Friction and Resistance

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::ApplyFriction` (void) | 0x003fe819 | 0x004e1f34 | ⚠️ |
| `CPhysical::ApplyFriction` (float, CColPoint&) | 0x003fe2a9 | 0x004e1a50 | ⚠️ |
| `CPhysical::ApplyFriction` (CPhysical*, float, CColPoint&) | 0x00405f5d | 0x004e9248 | ⚠️ |
| `CPhysical::ApplyGravity` | 0x003fe785 | 0x004e1e74 | ⚠️ |
| `CPhysical::ApplyAirResistance` | 0x003fe9dd | 0x004e2104 | ⚠️ |

### Collision

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::ApplyCollision` (CEntity*) | 0x00401b75 | 0x004e5598 | ⚠️ |
| `CPhysical::ApplyCollision` (CPhysical*) | 0x00401e81 | 0x004e583c | ⚠️ |
| `CPhysical::ApplyCollisionAlt` | 0x00403a6d | 0x004e7108 | ⚠️ |
| `CPhysical::ApplySoftCollision` (CEntity*) | 0x004040ed | 0x004e76fc | ⚠️ |
| `CPhysical::ApplySoftCollision` (CPhysical*) | 0x00404499 | 0x004e7a5c | ⚠️ |
| `CPhysical::ApplySpringCollision` | 0x00405a11 | 0x004e8df4 | ⚠️ |
| `CPhysical::ApplySpringCollisionAlt` | 0x00405ab5 | 0x004e8e7c | ⚠️ |
| `CPhysical::ApplySpringDampening` | 0x00405bbd | 0x004e8f4c | ⚠️ |
| `CPhysical::ApplySpringDampeningOld` | 0x00405ded | 0x004e9120 | ⚠️ |
| `CPhysical::ApplyScriptCollision` | 0x00406c45 | 0x004e9e10 | ⚠️ |

### Processing

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::ProcessControl` | 0x003fedcd | 0x004e2668 | ⚠️ |
| `CPhysical::ProcessCollision` | 0x003fef49 | 0x004e2668 | ⚠️ |
| `CPhysical::ProcessShift` | 0x00401141 | - | ⚠️ |
| `CPhysical::ProcessEntityCollision` | 0x0040696d | 0x004e9a48 | ⚠️ |
| `CPhysical::ProcessCollisionSectorList` | 0x003ff7fd | 0x004e2fd0 | ⚠️ |
| `CPhysical::ProcessCollisionSectorList_SimpleCar` | 0x00400c8d | 0x004e44c0 | ⚠️ |
| `CPhysical::CheckCollision` | 0x003ff655 | 0x004e2e30 | ⚠️ |
| `CPhysical::CheckCollision_SimpleCar` | 0x003ff5f9 | 0x004e2db0 | ⚠️ |
| `CPhysical::TestCollision` | 0x004019bd | 0x004e532c | ⚠️ |
| `CPhysical::SkipPhysics` | 0x003feedd | - | ⚠️ |

### Records and Management

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::AddCollisionRecord` | 0x00401a79 | 0x004e5438 | ⚠️ |
| `CPhysical::GetHasCollidedWith` | 0x00401b0d | - | ⚠️ |
| `CPhysical::GetHasCollidedWithAnyObject` | 0x00401b43 | - | ⚠️ |
| `CPhysical::SetDamagedPieceRecord` | 0x003fd3d5 | - | ⚠️ |
| `CPhysical::AddToMovingList` | 0x003fd335 | - | ⚠️ |
| `CPhysical::RemoveFromMovingList` | 0x003fd37d | - | ⚠️ |
| `CPhysical::RemoveAndAdd` | 0x003fd0ad | - | ⚠️ |

### Lifecycle

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::CPhysical` | 0x003fcc81 | - | ⚠️ |
| `CPhysical::~CPhysical` | 0x003fcda5 | - | ⚠️ |
| `CPhysical::Add` | 0x003fce3d | - | ⚠️ |
| `CPhysical::Remove` | 0x003fd02d | - | ⚠️ |

### Attachment

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::AttachEntityToEntity` (CVector) | 0x00406d13 | - | ⚠️ |
| `CPhysical::AttachEntityToEntity` (RtQuat) | 0x00407869 | - | ⚠️ |
| `CPhysical::PositionAttachedEntity` | 0x00406ddd | - | ⚠️ |
| `CPhysical::DettachEntityFromEntity` | 0x00407a69 | - | ⚠️ |
| `CPhysical::DettachAutoAttachedEntity` | 0x00407cd1 | - | ⚠️ |

### Utilities

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CPhysical::GetBoundRect` | 0x003fcde9 | - | ⚠️ |
| `CPhysical::GetLightingTotal` | 0x00407e29 | - | ⚠️ |
| `CPhysical::GetLightingFromCol` | 0x00407d71 | - | ⚠️ |
| `CPhysical::CanPhysicalBeDamaged` | 0x00407f21 | - | ⚠️ |
| `CPhysical::UnsetIsInSafePosition` | 0x003fe725 | - | ⚠️ |
| `CPhysical::RemoveRefsToEntity` | 0x00406af1 | - | ⚠️ |
| `CPhysical::PlacePhysicalRelativeToOtherPhysical` | 0x00406b69 | - | ⚠️ |

---

## 3. Sphere Collision

### CColSphere

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CColSphere::Set` | 0x002e1cc5 | 0x003a4d0c | ⚠️ |
| `CColSphere::IntersectRay` | 0x002e1ce1 | 0x003a4d30 | ⚠️ |
| `CColSphere::IntersectEdge` | 0x002e1e03 | 0x003a4e1c | ⚠️ |
| `CColSphere::IntersectPoint` | 0x002e1ff5 | 0x003a5010 | ⚠️ |
| `CColSphere::IntersectSphere` | 0x002e2035 | 0x003a504c | ⚠️ |

### CCollision - Sphere vs Sphere

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CCollision::TestSphereSphere` | 0x002d9a11 | 0x0039c648 | ⚠️ |
| `CCollision::ProcessSphereSphere` | 0x002d9a19 | 0x0039c68c | ⚠️ |

### CCollision - Line vs Sphere

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CCollision::TestLineSphere` | 0x002dabbd | 0x0039d5c4 | ⚠️ |
| `CCollision::ProcessLineSphere` | 0x002db031 | 0x0039d8e8 | ⚠️ |

### CCollision - SphereCast

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CCollision::SphereCastVsSphere` | 0x002e0135 | 0x003a2e40 | ⚠️ |
| `CCollision::SphereCastVsEntity` | 0x002deb51 | 0x003a1660 | ⚠️ |
| `CCollision::SphereCastVsBBox` | 0x002e00b1 | 0x003a2da4 | ⚠️ |
| `CCollision::SphereCastVsCaches` | 0x002dfd75 | 0x003a29e0 | ⚠️ |
| `CCollision::SphereCastVersusVsPoly` | 0x002e0bb5 | 0x003a3724 | ⚠️ |

### CCollision - Sphere vs Box/Triangle

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CCollision::ProcessSphereBox` | 0x002d9d11 | 0x0039c960 | ⚠️ |
| `CCollision::TestSphereTriangle` | 0x002da2ef | 0x0039ce4c | ⚠️ |
| `CCollision::ProcessSphereTriangle` | 0x002da6b9 | 0x0039d164 | ⚠️ |

### CCollision - Other

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CCollision::SphereVsEntity` | 0x002e1cc5 | 0x003a3934 | ⚠️ |
| `CCollision::GetBoundingBoxFromTwoSpheres` | 0x002de955 | 0x003a1438 | ⚠️ |
| `CCollision::BuildCacheOfCameraCollision` | 0x002df7e5 | 0x003a23d8 | ⚠️ |
| `CCollision::CameraConeCastVsWorldCollision` | 0x002dfc0d | 0x003a2868 | ⚠️ |
| `CCollision::CheckCameraCollisionBuildings` | 0x002de9e9 | 0x003a14c0 | ⚠️ |
| `CCollision::CheckCameraCollisionVehicles` | 0x002df251 | 0x003a1e64 | ⚠️ |
| `CCollision::CheckCameraCollisionObjects` | 0x002df529 | 0x003a20c8 | ⚠️ |

---

## 4. Objects (CObject)

### Lifecycle

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CObject::CObject` (void) | 0x00452c6d | 0x0053b828 | ⚠️ |
| `CObject::CObject` (int, bool) | 0x00453011 | 0x0053bc64 | ⚠️ |
| `CObject::CObject` (CDummyObject*) | 0x00453059 | 0x0053bcc4 | ⚠️ |
| `CObject::~CObject` | 0x004530fd | 0x0053bda0 | ⚠️ |
| `CObject::Init` | 0x00452c99 | 0x0053b868 | ⚠️ |
| `CObject::Create` (int, bool) | 0x0045343d | 0x0053c19c | ⚠️ |
| `CObject::Create` (CDummyObject*) | 0x00453661 | 0x0053c4e4 | ⚠️ |

### Rendering

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CObject::PreRender` | 0x00454d0d | 0x0053de48 | ⚠️ |
| `CObject::Render` | 0x00454f55 | 0x0053e0f0 | ⚠️ |
| `CObject::SetupLighting` | 0x00410539 | 0x004f59d4 | ⚠️ |
| `CObject::RemoveLighting` | 0x00410615 | 0x004f5adc | ⚠️ |

### Physics and Control

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CObject::ProcessControl` | 0x00453825 | 0x0053c790 | ⚠️ |
| `CObject::ProcessControlLogic` | 0x004543d5 | 0x0053d434 | ⚠️ |
| `CObject::SetIsStatic` | 0x00455681 | 0x0053e8a8 | ⚠️ |
| `CObject::SpecialEntityPreCollisionStuff` | 0x00454fd5 | 0x0053e18c | ⚠️ |
| `CObject::SpecialEntityCalcCollisionSteps` | 0x00455285 | 0x0053e4dc | ⚠️ |

### Management

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CObject::CanBeDeleted` | 0x004537f9 | 0x0053c750 | ⚠️ |
| `CObject::CanBeTargetted` | 0x00455b87 | 0x0053eea4 | ⚠️ |
| `CObject::SetObjectTargettable` | 0x00455b71 | 0x0053ee90 | ⚠️ |
| `CObject::AddToControlCodeList` | 0x004537bd | 0x0053c700 | ⚠️ |
| `CObject::RemoveFromControlCodeList` | 0x004532c5 | 0x0053bf98 | ⚠️ |
| `CObject::RefModelInfo` | 0x00455b91 | 0x0053eeb0 | ⚠️ |

### Persistence

| Function | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `CObject::Save` | 0x00484f41 | 0x005771e4 | ⚠️ |
| `CObject::Load` | 0x00484fad | - | ⚠️ |

---

## 5. Global Variables

| Variable | Offset 32-bit | Offset 64-bit | Status |
|----------|---------------|---------------|--------|
| `MI_POOL_CUE_BALL` | 0x006aad08 | 0x00876314 | ❌ |
| `CWorld::SnookerTableMin` | 0x0096b9e8 | 0x00bdcb14 | ❌ |
| `CWorld::SnookerTableMax` | 0x0096b9dc | 0x00bdcb08 | ❌ |
| `CTheScripts::bMiniGameInProgress` | 0x00819d80 | 0x009ff3a0 | ⚠️ |
| `CTheScripts::bDisplayNonMiniGameHelpMessages` | 0x00819d81 | 0x009ff3a1 | ⚠️ |
| `SurfaceInfos_c::GetFrictionEffect` | 0x0041ded7 | - | ⚠️ |

---

## 6. Source Files (from 64-bit dump)

| File | Description |
|------|-------------|
| `../FrontEnd/WidgetPool.cpp` | Main pool widget |
| `../FrontEnd/WidgetRegionPoolBallInHand.cpp` | Ball positioning widget |
| `../FrontEnd/WidgetRegionPoolEnglish.cpp` | English effect widget |

---

## Usage Notes

### For Physics Hooks (Shot)

```cpp
// Offset 32-bit: 0x003fd4d9
// Offset 64-bit: 0x004e0de0
typedef void (*ApplyMoveForce_t)(CPhysical* pThis, CVector force);
ApplyMoveForce_t ApplyMoveForce = (ApplyMoveForce_t)(g_libGTASA + OFFSET);

// Usage: apply force on ball
CVector shotForce = direction * power;
ApplyMoveForce(pBall->m_pEntity, shotForce);
```

### For English Hooks

```cpp
// Offset 32-bit: 0x003fd619
// Offset 64-bit: 0x004e0f00
typedef void (*ApplyForce_t)(CPhysical* pThis, CVector force, CVector point, bool);
ApplyForce_t ApplyForce = (ApplyForce_t)(g_libGTASA + OFFSET);

// Usage: apply force with english effect
CVector englishOffset(offsetX * ballRadius, offsetY * ballRadius, 0);
CVector hitPoint = ballCenter + englishOffset;
ApplyForce(pBall->m_pEntity, shotForce, hitPoint, true);
```

### For Ball-Ball Collision

```cpp
// Offset 32-bit: 0x002d9a19
// Offset 64-bit: 0x0039c68c
typedef bool (*ProcessSphereSphere_t)(CColSphere const&, CColSphere const&, CColPoint&, float&);
ProcessSphereSphere_t ProcessSphereSphere = (ProcessSphereSphere_t)(g_libGTASA + OFFSET);

// Usage: check collision between two balls
CColPoint colPoint;
float depth;
if (ProcessSphereSphere(ball1Sphere, ball2Sphere, colPoint, depth)) {
    // Collision detected
}
```

### For Table Bounds

```cpp
// Offsets 32-bit
CVector* SnookerTableMin = (CVector*)(g_libGTASA + 0x0096b9e8);
CVector* SnookerTableMax = (CVector*)(g_libGTASA + 0x0096b9dc);

// Check if ball is on table
bool IsOnTable(CVector ballPos) {
    return ballPos.x >= SnookerTableMin->x && ballPos.x <= SnookerTableMax->x &&
           ballPos.y >= SnookerTableMin->y && ballPos.y <= SnookerTableMax->y;
}
```
