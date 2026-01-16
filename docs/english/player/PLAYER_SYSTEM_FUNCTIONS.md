# Player System - Detailed Function Tree

**Legend:**
- ✅ = Implemented in APK
- ⚠️ = Partially implemented (via hook)
- ❌ = Not implemented

---

## 1. CPed (Base Ped Class)

### 1.1 Initialization and Lifecycle
```
CPed
├── ✅ CPed(ePedType)
│   └── Ped constructor, sets type (COP, CIVMALE, GANG1, etc)
│
├── ✅ ~CPed()
│   └── Destructor, frees resources
│
├── ⚠️ Initialise()
│   └── Initializes ped internal structures
│
├── ⚠️ SetModelIndex(uint)
│   └── Sets the ped's 3D model
│       Loads skin via streaming if needed
│
├── ❌ DeleteRwObject()
│   └── Removes RenderWare object
│
└── ⚠️ SetCharCreatedBy(uint8)
    └── Sets who created the ped (GAME, MISSION)
```

### 1.2 State Control
```
CPed
├── ⚠️ SetPedState(ePedState)
│   └── Sets current state (IDLE, ATTACK, DRIVING, etc)
│
├── ❌ IsPedInControl()
│   └── Checks if ped has control of itself
│       False if in ragdoll, cutscene, etc
│
├── ❌ CanBeDeleted()
│   └── Checks if ped can be removed
│
├── ❌ CanBeArrested()
│   └── Checks if can be arrested
│
└── ❌ IsPointerValid()
    └── Checks if ped pointer is valid
```

### 1.3 Movement and Physics
```
CPed
├── ❌ ProcessControl()
│   └── Main per-frame update function
│       Processes physics, collision, AI, animation
│
├── ❌ UpdatePosition()
│   └── Updates position based on velocity
│       Applies gravity and constraints
│
├── ❌ CalculateNewVelocity()
│   └── Calculates new velocity from input
│       Considers terrain and obstacles
│
├── ❌ ProcessBuoyancy()
│   └── Processes water buoyancy
│
├── ⚠️ Teleport(CVector, uint8)
│   └── Teleports ped to position
│
└── ❌ SetMoveState(eMoveState)
    └── Sets movement state (STILL, WALK, RUN, SPRINT)
```

### 1.4 Looking and Aiming
```
CPed
├── ❌ SetLookFlag(CEntity*, bool, bool)
│   └── Sets entity to look at
│
├── ❌ SetAimFlag(CEntity*)
│   └── Sets entity to aim at
│
├── ❌ ClearAimFlag()
│   └── Stops aiming
│
├── ❌ SetWeaponLockOnTarget(CEntity*)
│   └── Sets lock-on target
│
└── ❌ UseFreeAimMagnetize()
    └── Uses free aim with magnetism
```

### 1.5 Animation
```
CPed
├── ❌ PreRenderAfterTest()
│   └── Preparation after visibility tests
│       Updates bones, IK, clothes
│
├── ❌ DoFootLanded(bool, uint8)
│   └── Processes foot touching ground
│       Generates footstep sound
│
├── ❌ RemoveWeaponAnims(int, float)
│   └── Removes weapon animations
│
└── ❌ TurnBody()
    └── Rotates ped body
```

### 1.6 Bone Positions
```
CPed
├── ✅ GetBonePosition(RwV3d&, uint, bool)
│   └── Gets position of specific bone
│       Used for positioning weapons, effects
│
└── ❌ GetHeadAndFootPositions(CVector&, CVector&, bool)
    └── Gets head and feet positions
```

### 1.7 Vehicles
```
CPed
├── ✅ IsInVehicle()
│   └── Checks if in vehicle
│
├── ✅ IsAPassenger()
│   └── Checks if is passenger
│
└── ⚠️ SetPedPositionInCar()
    └── Sets position inside car
```

### 1.8 Attachments/Objects
```
CPed
├── ❌ AttachPedToEntity(CEntity*, CVector, ushort, float, eWeaponType)
│   └── Attaches ped to entity
│
├── ❌ GiveObjectToPedToHold(int, uint8)
│   └── Gives object for ped to hold
│
├── ❌ PutOnGoggles()
│   └── Puts on goggles (night vision, etc)
│
└── ❌ TakeOffGoggles()
    └── Removes goggles
```

### 1.9 Damage and Death
```
CPed
├── ❌ RemoveBodyPart(int, int8)
│   └── Removes body part (dismemberment)
│
├── ❌ KillPedWithCar(CVehicle*, float, bool)
│   └── Kills ped with car
│
└── ❌ CreateDeadPedMoney()
    └── Creates dead ped's money
```

### 1.10 Speech/Audio
```
CPed
├── ❌ Say(short, uint32, float, uint8, uint8, uint8)
│   └── Ped says a phrase
│
├── ❌ GetPedTalking()
│   └── Checks if talking
│
├── ❌ EnablePedSpeech()
│   └── Enables speech
│
└── ❌ DisablePedSpeech(short)
    └── Disables speech
```

---

## 2. CPlayerPed (Player Extension)

### 2.1 Initialization
```
CPlayerPed
├── ✅ CPlayerPed(int, bool)
│   └── Player constructor
│
├── ⚠️ SetupPlayerPed(int)
│   └── Configures player structures
│
├── ⚠️ DeactivatePlayerPed(int)
│   └── Temporarily deactivates player
│
├── ⚠️ ReactivatePlayerPed(int)
│   └── Reactivates player
│
├── ⚠️ RemovePlayerPed(int)
│   └── Removes player from game
│
└── ❌ SetInitialState(bool)
    └── Sets initial state after spawn
```

### 2.2 Movement Control
```
CPlayerPed
├── ❌ ProcessControl()
│   └── Main per-frame update
│
├── ❌ ProcessPlayerWeapon(CPad*)
│   └── Processes weapon input
│
├── ❌ ProcessWeaponSwitch(CPad*)
│   └── Processes weapon switching
│
├── ❌ ProcessAnimGroups()
│   └── Processes animation groups
│
├── ❌ SetRealMoveAnim()
│   └── Sets real movement animation
│
└── ❌ SetPlayerMoveBlendRatio(CVector*)
    └── Sets movement blend ratio
```

### 2.3 Targeting System
```
CPlayerPed
├── ❌ FindWeaponLockOnTarget()
│   └── Finds lock-on target
│       Prioritizes nearby threats
│
├── ❌ FindNextWeaponLockOnTarget(CEntity*, bool)
│   └── Next lock-on target
│
├── ❌ EvaluateTarget(CEntity*, CEntity**, float*, float, float, bool)
│   └── Evaluates target quality
│
├── ❌ HandleTapToTarget(float, float, bool)
│   └── Tap-to-target system
│
├── ❌ HandleMeleeTargeting()
│   └── Melee combat targeting
│
├── ❌ Compute3rdPersonMouseTarget(bool)
│   └── Computes mouse target in 3rd person
│
├── ❌ ClearWeaponTarget()
│   └── Clears weapon target
│
├── ❌ CanIKReachThisTarget(CVector, CWeapon*, bool)
│   └── Checks if IK can reach target
│
└── ❌ DoesTargetHaveToBeBroken(CEntity*, CWeapon*)
    └── Checks if should break lock-on
```

### 2.4 Group System
```
CPlayerPed
├── ❌ TellGroupToStartFollowingPlayer(bool, bool, bool)
│   └── Orders group to follow
│
├── ❌ MakeThisPedJoinOurGroup(CPed*)
│   └── Adds ped to group
│
├── ❌ DisbandPlayerGroup()
│   └── Disbands group
│
├── ❌ MakePlayerGroupDisappear()
│   └── Makes group disappear
│
└── ❌ MakePlayerGroupReappear()
    └── Makes group reappear
```

### 2.5 Stamina and Energy
```
CPlayerPed
├── ❌ HandlePlayerBreath(bool, float)
│   └── Manages breath (diving)
│
├── ❌ ResetPlayerBreath()
│   └── Resets breath
│
├── ❌ HandleSprintEnergy(bool, float)
│   └── Manages sprint energy
│
└── ❌ ControlButtonSprint(eSprintType)
    └── Controls sprint button
```

### 2.6 State and Wanted Level
```
CPlayerPed
├── ❌ SetWantedLevel(int)
│   └── Sets wanted level
│
├── ❌ Busted()
│   └── Player was busted
│
├── ❌ PlayerWantsToAttack()
│   └── Player wants to attack
│
├── ❌ ClearAdrenaline()
│   └── Clears adrenaline effect
│
└── ❌ KeepAreaAroundPlayerClear()
    └── Keeps area around player clear
```

---

## 3. CPedModelInfo (Ped Model)

```
CPedModelInfo
├── ❌ SetClump(RpClump*)
│   └── Sets model clump
│
├── ❌ CreateHitColModelSkinned(RpClump*)
│   └── Creates skinned collision model
│
├── ❌ AnimatePedColModelSkinned(RpClump*)
│   └── Animates skinned collision
│
└── ❌ IncrementVoice()
    └── Next available voice
```

---

## 4. CAnimManager (Animation Manager)

```
CAnimManager
├── ❌ Initialise()
│   └── Initializes animation system
│
├── ⚠️ AddAnimation(RpClump*, AssocGroupId, AnimationId)
│   └── Adds animation to clump
│
├── ⚠️ BlendAnimation(RpClump*, AssocGroupId, AnimationId, float)
│   └── Blends animation
│
├── ❌ GetAnimAssociation(AssocGroupId, AnimationId)
│   └── Gets animation association
│
├── ⚠️ LoadAnimFile(char const*)
│   └── Loads IFP file
│
├── ❌ RegisterAnimBlock(char const*)
│   └── Registers animation block
│
├── ❌ CreateAnimAssocGroups()
│   └── Creates association groups
│
└── ❌ UncompressAnimation(CAnimBlendHierarchy*)
    └── Uncompresses animation
```

---

## 5. CTaskManager (Task Manager)

```
CTaskManager
├── ✅ GetActiveTask()
│   └── Returns main active task
│
├── ✅ GetSimplestActiveTask()
│   └── Returns simplest active task
│
├── ✅ FlushImmediately()
│   └── Removes all tasks immediately
│
├── ❌ SetTask(CTask*, int, bool)
│   └── Sets primary task
│
├── ❌ ManageTasks()
│   └── Manages all tasks per frame
│
└── ❌ Flush()
    └── Clears all tasks
```

---

## 6. Clothing System (CClothes / CClothesBuilder)

```
CClothes
├── ❌ RebuildPlayer(CPlayerPed*, bool)
│   └── Rebuilds player model with clothes
│
├── ❌ ConstructPedModel(uint, CPedClothesDesc&, ...)
│   └── Constructs complete model
│
└── ❌ RequestMotionGroupAnims()
    └── Requests motion group animations
```

```
CClothesBuilder
├── ❌ CreateSkinnedClump(RpClump*, RwTexDictionary*, ...)
│   └── Creates skinned clump with clothes
│
├── ❌ BlendGeometry(RpClump*, char const*, ...)
│   └── Blends geometry
│
└── ❌ BlendTextures(RwTexture*, RwTexture*, ...)
    └── Blends textures
```

---

## Coverage Summary

| Category | Total | ✅ | ⚠️ | ❌ |
|----------|:-----:|:--:|:--:|:--:|
| CPed Basic | 65 | 5 | 8 | 52 |
| CPlayerPed | 55 | 2 | 6 | 47 |
| CPedModelInfo | 10 | 0 | 0 | 10 |
| CAnimManager | 22 | 0 | 3 | 19 |
| CTaskManager | 13 | 3 | 0 | 10 |
| CClothes | 21 | 0 | 0 | 21 |
| **TOTAL** | **186** | **10** | **17** | **159** |

---

**Coverage: ~15% of complete character system**
