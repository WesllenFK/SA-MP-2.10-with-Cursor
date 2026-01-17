# Player/Character System - GTA San Andreas

## Index
1. [Overview](#1-overview)
2. [Class Hierarchy](#2-class-hierarchy)
3. [CPedGTA Structure](#3-cpedgta-structure)
4. [Model System](#4-model-system)
5. [Animation System](#5-animation-system)
6. [Audio System](#6-audio-system)
7. [Creation Flow](#7-creation-flow)
8. [Project Implementation](#8-project-implementation)

---

## 1. Overview

The character system in GTA San Andreas is composed of multiple layers that manage:
- **3D Model** (Skinned mesh with skeleton)
- **Animations** (Animation blending via HAnim hierarchy)
- **Physics** (Collision, gravity, movement)
- **AI/Tasks** (Task system for behavior)
- **Audio** (Voice, footsteps, effects)
- **Clothes** (CJ's dynamic clothing system)

### Main Components:
- `CEntity` → `CPhysical` → `CPed` → `CPlayerPed`
- `CPedModelInfo` - Ped model information
- `CAnimManager` / `CAnimBlendAssociation` - Animation system
- `CAEPedAudioEntity` / `CAEPedSpeechAudioEntity` - Audio
- `CClothes` / `CClothesBuilder` - Clothing system
- `CTaskManager` - Task/action manager
- `CPedIntelligence` - AI and decision making

---

## 2. Class Hierarchy

```
CPlaceable
    └── CEntity
            └── CPhysical
                    └── CPed (CPedGTA)
                            └── CPlayerPed
```

### Structure Sizes:
| Class | 32-bit | 64-bit |
|-------|--------|--------|
| CPlaceable | 0x18 | 0x20 |
| CEntity | 0x38 | 0x48 |
| CPhysical | 0x138 | 0x190 |
| CPedGTA | 0x7A4 | 0x988 |

---

## 3. CPedGTA Structure

### 3.1 Audio Fields
```cpp
uint8_t m_PedAudioEntity[0x15C/0x1A8];       // General ped audio
uint8_t m_PedSpeechAudioEntity[0x100/0x130]; // Speech/voice
uint8_t m_PedWeaponAudioEntity[0xA8/0xC8];   // Weapon sounds
```

### 3.2 AI and State Fields
```cpp
CPedIntelligence*  m_pIntelligence;   // AI system
CPlayerPedData*    m_pPlayerData;     // Player-specific data
ePedCreatedBy      m_nCreatedBy;      // Creator (GAME, MISSION, etc)
ePedState          m_nPedState;       // Current state (IDLE, ATTACK, etc)
eMoveState         m_nMoveState;      // Movement state
```

### 3.3 Physics/Collision Fields
```cpp
CStoredCollPoly    m_storedCollPoly;           // Collision polygon
CPhysical*         m_pGroundPhysical;          // Ground object
CVector            m_vecGroundOffset;          // Ground offset
CVector            m_vecGroundNormal;          // Ground normal
CEntityGTA*        m_pEntityStandingOn;        // Entity standing on
float              m_fHitHeadHeight;           // Head hit height
```

### 3.4 Health Fields
```cpp
float m_fHealth;     // Current health
float m_fMaxHealth;  // Maximum health
float m_fArmour;     // Armor
```

### 3.5 Rotation/Movement Fields
```cpp
float m_fCurrentRotation;   // Current rotation
float m_fAimingRotation;    // Aiming rotation
float m_fHeadingChangeRate; // Direction change rate
float m_fMoveAnim;          // Movement animation speed
```

### 3.6 Vehicle Fields
```cpp
CVehicleGTA* pVehicle;          // Current vehicle
CVehicleGTA* m_VehDeadInFrontOf; // Vehicle died in front of
```

### 3.7 Animation Fields
```cpp
std::array<AnimBlendFrameData*, TOTAL_PED_NODES> m_apBones; // Bone array
AssocGroupId m_nAnimGroup;           // Current animation group
CVector2D    m_vecAnimMovingShiftLocal; // Local animation shift
CVector2D    m_vecAnimMovingShift;   // Global shift
CPedIK       m_pedIK;                // Inverse Kinematics system
```

### 3.8 Ped Flags (16 bytes of flags)
```cpp
// 1st byte
bool bIsStanding : 1;          // Is standing
bool bWasStanding : 1;         // Was standing
bool bIsLooking : 1;           // Is looking
bool bIsRestoringLook : 1;     // Restoring look
bool bIsAimingGun : 1;         // Aiming gun
bool bIsRestoringGun : 1;      // Restoring gun position
bool bCanPointGunAtTarget : 1; // Can point gun
bool bIsTalking : 1;           // Is talking

// 2nd byte
bool bInVehicle : 1;           // In vehicle
bool bIsInTheAir : 1;          // In the air
bool bIsLanding : 1;           // Landing
bool bHitSomethingLastFrame : 1; // Collided last frame
bool bRenderPedInCar : 1;      // Render inside car
bool bUpdateAnimHeading : 1;   // Update heading from anim
bool bRemoveHead : 1;          // Waiting to remove head

// 3rd byte
bool bFiringWeapon : 1;        // Firing weapon
bool bPedIsBleeding : 1;       // Ped is bleeding
bool bStopAndShoot : 1;        // Stop and shoot
bool bIsPedDieAnimPlaying : 1; // Death animation playing
bool bStayInSamePlace : 1;     // Stay in same place
bool bBeingChasedByPolice : 1; // Being chased by police

// ... (more flags for duck, drown, fight, etc)
```

---

## 4. Model System

### 4.1 CPedModelInfo
Stores ped model information (inherits from CClumpModelInfo).

```cpp
class CPedModelInfo : public CClumpModelInfo {
    void Init();                              // Initialize
    void SetClump(RpClump* clump);           // Set 3D model
    void DeleteRwObject();                    // Remove RW object
    void SetFaceTexture(RwTexture*);         // Face texture
    void IncrementVoice();                    // Next voice
    void CreateHitColModelSkinned(RpClump*); // Create skinned collision
    void AnimatePedColModelSkinned(RpClump*);// Animate collision
    ModelInfoType GetModelType();            // Return type (PED)
};
```

### 4.2 Ped Bones (ePedNode)
```cpp
enum ePedNode : int32 {
    PED_NODE_UPPER_TORSO     = 1,   // Upper torso
    PED_NODE_HEAD            = 2,   // Head
    PED_NODE_LEFT_ARM        = 3,   // Left arm
    PED_NODE_RIGHT_ARM       = 4,   // Right arm
    PED_NODE_LEFT_HAND       = 5,   // Left hand
    PED_NODE_RIGHT_HAND      = 6,   // Right hand (weapon)
    PED_NODE_LEFT_LEG        = 7,   // Left leg
    PED_NODE_RIGHT_LEG       = 8,   // Right leg
    PED_NODE_LEFT_FOOT       = 9,   // Left foot
    PED_NODE_RIGHT_FOOT      = 10,  // Right foot
    PED_NODE_RIGHT_LOWER_LEG = 11,  // Right shin
    PED_NODE_LEFT_LOWER_LEG  = 12,  // Left shin
    PED_NODE_LEFT_LOWER_ARM  = 13,  // Left forearm
    PED_NODE_RIGHT_LOWER_ARM = 14,  // Right forearm
    PED_NODE_LEFT_CLAVICLE   = 15,  // Left clavicle
    PED_NODE_RIGHT_CLAVICLE  = 16,  // Right clavicle
    PED_NODE_NECK            = 17,  // Neck
    PED_NODE_JAW             = 18,  // Jaw
    TOTAL_PED_NODES          = 19
};
```

---

## 5. Animation System

### 5.1 CAnimManager
Manages all game animations.

```cpp
class CAnimManager {
    // Static structures
    static CAnimBlendHierarchy ms_aAnimations[]; // All animations
    static CAnimBlendAssocGroup ms_aAnimAssocGroups[]; // Groups
    static CAnimBlock ms_aAnimBlocks[]; // Animation blocks
    
    // Add animation
    static CAnimBlendAssociation* AddAnimation(RpClump*, AssocGroupId, AnimationId);
    
    // Blend animations
    static CAnimBlendAssociation* BlendAnimation(RpClump*, AssocGroupId, AnimationId, float);
    
    // Get animation
    static CAnimBlendHierarchy* GetAnimation(char const*, CAnimBlock const*);
    static CAnimBlendAssociation* GetAnimAssociation(AssocGroupId, AnimationId);
    
    // Block management
    static void LoadAnimFile(char const*);
    static void RegisterAnimBlock(char const*);
    static void RemoveAnimBlock(int);
    
    // Initialization
    static void Initialise();
    static void Shutdown();
};
```

### 5.2 CPedIK (Inverse Kinematics)
System that adjusts bones in real-time for aiming, looking, etc.

```cpp
class CPedIK {
    uint32 m_flags;          // State flags
    float  m_torsoOrien;     // Torso orientation
    float  m_headOrien;      // Head orientation
    
    void RotateTorso(RwFrame*, float, bool); // Rotate torso
    void PitchForSlope();                     // Adjust for terrain
    void GetWorldMatrix(RwFrame*, RwMatrix*); // Bone matrix
};
```

---

## 6. Audio System

### 6.1 CAEPedAudioEntity
Manages general ped sounds (footsteps, impacts, etc).

```cpp
class CAEPedAudioEntity {
    void Initialise(CEntity*);          // Initialize
    void Terminate();                    // Finalize
    void Service();                      // Per-frame update
    
    // Sound events
    void HandleFootstepEvent(int, float, float, uint8); // Footsteps
    void HandleLandingEvent(int);        // Landing
    void HandlePedHit(int, CPhysical*, uint8, float, uint32); // Impact
    
    // JetPack
    void TurnOnJetPack();
    void TurnOffJetPack();
    void UpdateJetPack(float, float);
    
    // Other
    void PlayWindRush(float, float);     // Wind (falling)
    void HandleSwimSplash(int);          // Water splash
};
```

### 6.2 CAEPedSpeechAudioEntity
Manages ped speech/voice.

```cpp
class CAEPedSpeechAudioEntity {
    void Initialise(CEntity*);
    
    // Speech
    void AddSayEvent(int, short, uint32, float, uint8, uint8, uint8);
    void PlayLoadedSound();
    void StopCurrentSpeech();
    
    // Control
    void EnablePedSpeech();
    void DisablePedSpeech(short);
    bool GetPedTalking();
    
    // Types and mood
    int GetPedType();
    int GetCurrentCJMood();
    int GetVoiceForMood(short);
};
```

---

## 7. Creation Flow

### 7.1 Local Player Creation
```
CPlayerPed::CPlayerPed(playerNum, skin, x, y, z, rotation)
    │
    ├──▶ CPlayerPed::SetupPlayerPed(playerNum)
    │         └── Configures player internal structures
    │
    ├──▶ CPlayerPed::DeactivatePlayerPed(playerNum)
    │         └── Temporarily deactivates
    │
    ├──▶ FindPlayerPed(playerNum)
    │         └── Gets created ped pointer
    │
    ├──▶ CTheScripts::ClearSpaceForMissionEntity(pos, ped)
    │         └── Clears space for ped
    │
    ├──▶ CPlayerPed::ReactivatePlayerPed(playerNum)
    │         └── Reactivates player
    │
    ├──▶ CWorld::Add(ped)
    │         └── Adds to world
    │
    ├──▶ SetModelIndex(skin)
    │         │
    │         ├──▶ CStreaming::RequestModel(skinId, flags)
    │         ├──▶ CStreaming::LoadAllRequestedModels(false)
    │         └──▶ CPed::SetModelIndex(modelId) [native]
    │
    └──▶ SetMatrix(pos, rotation)
             └── Sets initial position and rotation
```

---

## 8. Project Implementation

### 8.1 Functions Implemented in APK

| Status | Function | Description |
|:------:|----------|-------------|
| ✅ | `CPlayerPed::CPlayerPed()` | Default constructor (local player) |
| ✅ | `CPlayerPed::CPlayerPed(int,int,float,float,float,float)` | Full constructor |
| ✅ | `CPlayerPed::~CPlayerPed()` | Destructor |
| ✅ | `CPlayerPed::IsInVehicle()` | Checks if in vehicle |
| ✅ | `CPlayerPed::SetModelIndex()` | Sets model/skin |
| ✅ | `CPlayerPed::SetHealth()` / `GetHealth()` | Health |
| ✅ | `CPlayerPed::SetArmour()` / `GetArmour()` | Armor |
| ✅ | `CPlayerPed::GiveWeapon()` | Gives weapon to ped |
| ✅ | `CPlayerPed::ClearWeapons()` | Removes all weapons |
| ✅ | `CPlayerPed::SetAmmo()` | Sets ammo |
| ✅ | `CPlayerPed::SetRotation()` | Sets rotation |
| ✅ | `CPlayerPed::ApplyAnimation()` | Applies animation |
| ✅ | `CPlayerPed::GetBonePosition()` | Bone position |
| ✅ | `CPlayerPed::PutDirectlyInVehicle()` | Puts in vehicle |
| ✅ | `CPlayerPed::EnterVehicle()` | Enters vehicle |
| ✅ | `CPlayerPed::ExitCurrentVehicle()` | Exits vehicle |
| ✅ | `CPlayerPed::FireInstant()` | Fires weapon |
| ✅ | `CPlayerPed::StartJetpack()` / `StopJetpack()` | Jetpack |
| ✅ | `CPlayerPed::StartDancing()` / `StopDancing()` | Dancing |
| ✅ | `CPlayerPed::AttachObject()` / `DeattachObject()` | Attached objects |
| ✅ | `CPlayerPed::SetWeaponSkill()` | Sets weapon skill |

### 8.2 Important Native Offsets

| Function | 32-bit | 64-bit |
|----------|--------|--------|
| `CPed::Initialise` | `0x49FA19` | `0x595898` |
| `CPed::SetModelIndex` | `0x49FAD5` | `0x595954` |
| `CPed::ProcessControl` | `0x4A2541` | `0x598730` |
| `CPed::UpdatePosition` | `0x4A1999` | `0x597B04` |
| `CPed::GetBonePosition` | `0x4A4B0D` | `0x59AEE4` |
| `CPlayerPed::SetupPlayerPed` | `0x4C39A5` | `0x5C0FD4` |
| `CPlayerPed::DeactivatePlayerPed` | `0x4C3AD5` | `0x5C1140` |
| `CPlayerPed::ReactivatePlayerPed` | `0x4C3AED` | `0x5C1158` |
| `CPlayerPed::ProcessControl` | `0x4C47E9` | `0x5C2088` |
| `CPlayerPed::SetRealMoveAnim` | `0x4C3B05` | `0x5C1170` |
| `CPlayerPed::ProcessAnimGroups` | `0x4C5E61` | `0x5C37E0` |
| `CPlayerPed::FindWeaponLockOnTarget` | `0x4C6D65` | `0x5C494C` |
| `CPlayerPed::Load` | `0x4851E9` | `0x5774B8` |
| `CPlayerPed::Save` | `0x485163` | `0x577390` |

---

## Diagram: Ped Life Cycle

```
┌─────────────────────────────────────────────────────────────┐
│                    LIFE CYCLE - CPed                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐               │
│  │  Create  │───▶│ Process  │───▶│  Delete  │               │
│  │  (Init)  │    │ (Update) │    │(Cleanup) │               │
│  └──────────┘    └────┬─────┘    └──────────┘               │
│                       │                                      │
│                       ▼                                      │
│  ┌────────────────────────────────────────────────────┐     │
│  │               ProcessControl() - Per Frame         │     │
│  ├────────────────────────────────────────────────────┤     │
│  │  1. UpdatePosition()    → Physics, gravity          │     │
│  │  2. ProcessBuoyancy()   → Water buoyancy           │     │
│  │  3. CalculateNewVelocity() → Velocity              │     │
│  │  4. CalculateNewOrientation() → Rotation           │     │
│  │  5. ProcessEntityCollision() → Collisions          │     │
│  │  6. CTaskManager::ManageTasks() → AI/Behavior      │     │
│  │  7. UpdateMovement() → Movement animation          │     │
│  │  8. PreRenderAfterTest() → Prepare render          │     │
│  └────────────────────────────────────────────────────┘     │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

**Document created for GTA San Andreas player/character system analysis.**
**Based on libGTASA version 2.1 dumps (32-bit and 64-bit).**
