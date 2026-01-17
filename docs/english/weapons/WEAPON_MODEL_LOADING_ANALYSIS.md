# Weapon Model Loading System Analysis - GTA San Andreas

## Index
1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Main Classes](#main-classes)
4. [Loading Flow](#loading-flow)
5. [Data Structures](#data-structures)
6. [Native Functions from Dump](#native-functions-from-dump)
7. [Project Implementation](#project-implementation)
8. [Technical Considerations](#technical-considerations)

---

## 1. Overview

The weapon model loading system in GTA San Andreas is based on the game's **streaming system**. Weapon models are treated as DFF resources (3D models) that are loaded on demand when a ped needs to equip a weapon.

### Main Components:
- **CStreaming** - Main streaming system
- **CWeaponModelInfo** - Weapon model specific information
- **CWeaponInfo** - Weapon gameplay data (damage, range, etc.)
- **CWeapon** - Weapon instance in use
- **CPed::AddWeaponModel/RemoveWeaponModel** - Visual model management on ped

---

## 2. System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                    WEAPON SYSTEM - GTA SA                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │  CStreaming  │───▶│CWeaponModel- │───▶│  RpClump     │          │
│  │  RequestModel│    │    Info      │    │ (3D Model)   │          │
│  └──────────────┘    └──────────────┘    └──────────────┘          │
│         │                   │                   │                   │
│         ▼                   ▼                   ▼                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │   IMG File   │    │  CWeaponInfo │    │CPed::m_pWeap-│          │
│  │  (GTA3.IMG)  │    │(weapon.dat)  │    │  onObject    │          │
│  └──────────────┘    └──────────────┘    └──────────────┘          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Main Classes

### 3.1 CWeapon (Structure)
```cpp
// Location: app/src/main/cpp/samp/game/common.h
struct CWeapon
{
    eWeaponType dwType;        // Weapon type (enum)
    eWeaponState dwState;      // Current weapon state
    union {
        uint32_t dwAmmoInClip;
        uint32_t m_nAmmoInClip;
    };
    uint32_t dwAmmo;           // Total ammo
    uint32_t m_nTimer;         // Internal timer
    bool m_bFirstPersonWeaponModeSelected;
    bool m_bDontPlaceInHand;
    uint8_t pad[2];
    uintptr_t *m_pWeaponFxSys; // Visual effects system
};
// Size: 28 bytes (32-bit) / 32 bytes (64-bit)
```

### 3.2 CPedGTA - Weapon Related Fields
```cpp
// Location: app/src/main/cpp/samp/game/Entity/CPedGTA.h

// RenderWare object pointers
RpClump* *m_pWeaponObject;      // Current weapon 3D object
RpClump* *m_pGunflashObject;    // Gunflash object
RpClump* *m_pGogglesObject;     // Goggles object (night vision, etc.)

// Weapons array (13 slots)
CWeapon m_aWeapons[13];

// State information
eWeaponType m_nSavedWeapon;     // Temporarily saved weapon
eWeaponType m_nDelayedWeapon;   // Backup weapon (e.g., cop's pistol)
uint32 m_nDelayedWeaponAmmo;
uint8_t m_nActiveWeaponSlot;    // Active slot (0-12)
uint8 m_nWeaponShootingRate;
uint8 m_nWeaponAccuracy;
eWeaponSkill m_nWeaponSkill;    // Skill level
int32 m_nWeaponModelId;         // Loaded model ID (-1 if none)
```

### 3.3 eWeaponType (Enum)
```cpp
// Location: app/src/main/cpp/samp/game/Enums/eWeaponType.h
enum eWeaponType : uint32_t {
    WEAPON_UNARMED = 0,
    WEAPON_BRASSKNUCKLE,       // 1
    WEAPON_GOLFCLUB,           // 2
    WEAPON_NIGHTSTICK,         // 3
    WEAPON_KNIFE,              // 4
    WEAPON_BASEBALLBAT,        // 5
    // ... (melee weapons)
    
    WEAPON_PISTOL = 22,        // First weapon with skill
    WEAPON_PISTOL_SILENCED,    // 23
    WEAPON_DESERT_EAGLE,       // 24
    WEAPON_SHOTGUN,            // 25
    WEAPON_SAWNOFF_SHOTGUN,    // 26
    WEAPON_SPAS12_SHOTGUN,     // 27
    WEAPON_MICRO_UZI,          // 28
    WEAPON_MP5,                // 29
    WEAPON_AK47,               // 30
    WEAPON_M4,                 // 31
    WEAPON_TEC9,               // 32
    WEAPON_COUNTRYRIFLE,       // 33
    WEAPON_SNIPERRIFLE,        // 34
    WEAPON_RLAUNCHER,          // 35
    WEAPON_RLAUNCHER_HS,       // 36
    WEAPON_FLAMETHROWER,       // 37
    WEAPON_MINIGUN,            // 38
    // ...
    WEAPON_PARACHUTE = 46,
    WEAPON_LAST_WEAPON = 47,
};
```

---

## 4. Loading Flow

### 4.1 Complete Flow: GiveWeapon
```
CPed::GiveWeapon(weaponType, ammo)
    │
    ├──▶ GameGetWeaponModelIDFromWeaponID(weaponType)
    │         └──▶ Maps weaponType → modelId (330-372)
    │
    ├──▶ CStreaming::TryLoadModel(modelId)
    │         │
    │         ├──▶ CStreaming::RequestModel(modelId, flags)
    │         │         └──▶ Adds to request list
    │         │
    │         ├──▶ CStreaming::LoadAllRequestedModels(false)
    │         │         │
    │         │         ├──▶ CStreaming::RequestModelStream(chIdx)
    │         │         │         └──▶ CdStreamRead() - Reads from IMG
    │         │         │
    │         │         └──▶ CStreaming::ProcessLoadingChannel(chIdx)
    │         │                   └──▶ ConvertBufferToObject()
    │         │                             └──▶ CWeaponModelInfo::SetClump()
    │         │
    │         └──▶ Waits until model is loaded
    │
    └──▶ CPed::GiveWeapon (native)
              │
              ├──▶ Finds appropriate slot for weapon
              │
              ├──▶ Initializes CWeapon in slot
              │
              └──▶ CPed::SetCurrentWeapon(weaponType)
                        │
                        └──▶ CPed::AddWeaponModel(modelId)
                                  │
                                  ├──▶ Removes previous model if any
                                  │
                                  └──▶ Attaches RpClump to ped's hand
```

### 4.2 Native Functions (from 64-bit Dump)

| Function | Address | Size | Description |
|----------|---------|------|-------------|
| `CPed::GiveWeapon` | 0x59525C | 420 bytes | Gives weapon to ped |
| `CPed::AddWeaponModel` | 0x59B174 | 320 bytes | Adds visual model |
| `CPed::RemoveWeaponModel` | 0x59B2B4 | 216 bytes | Removes visual model |
| `CPed::SetWeaponSkill` | 0x59BF24 | 20 bytes | Sets skill |
| `CStreaming::LoadInitialWeapons` | 0x399654 | 48 bytes | Loads initial weapons |
| `CWeaponModelInfo::Init` | 0x460B7C | 36 bytes | Initializes model info |
| `CWeaponModelInfo::SetClump` | 0x460BA0 | 124 bytes | Sets model clump |
| `CWeaponInfo::LoadWeaponData` | 0x560EA8 | 1812 bytes | Loads weapon.dat |
| `CWeaponInfo::GetWeaponInfo` | 0x709BA8 | 84 bytes | Gets weapon info |
| `CModelInfo::AddWeaponModel` | 0x45CB04 | 92 bytes | Adds model to store |

### 4.3 Native Functions (from 32-bit Dump)

| Function | Address |
|----------|---------|
| `CPed::GiveWeapon` | 0x49F589 |
| `CPed::AddWeaponModel` | 0x4A4CE9 |
| `CPed::RemoveWeaponModel` | 0x4A4DF5 |
| `CStreaming::LoadInitialWeapons` | 0x2D6BEF |
| `CWeaponModelInfo::Init` | 0x389135 |
| `CWeaponModelInfo::SetClump` | 0x389145 |
| `CWeaponInfo::LoadWeaponData` | 0x474751 |
| `CWeaponInfo::GetWeaponInfo` | 0x5E42E9 |
| `CModelInfo::AddWeaponModel` | 0x386071 |

---

## 5. Data Structures

### 5.1 WeaponType → ModelID Mapping
```cpp
// Game internal mapping
// WeaponType 0-46 → ModelID 330-372 (approximately)

// Examples:
WEAPON_PISTOL (22)        → Model ID 346
WEAPON_SHOTGUN (25)       → Model ID 349
WEAPON_AK47 (30)          → Model ID 355
WEAPON_M4 (31)            → Model ID 356
WEAPON_SNIPERRIFLE (34)   → Model ID 358

// Function: GameGetWeaponModelIDFromWeaponID
// Returns -1 for weapons without model (UNARMED, etc.)
// Returns 350 or 365 for special weapons
```

### 5.2 CWeaponModelInfo (Derived from CClumpModelInfo)
```cpp
// Weapon model store: 51 slots
// Variable: ms_weaponModelStore (0x00B01360 in 64-bit dump)

class CWeaponModelInfo : public CClumpModelInfo {
    // Inherits all CClumpModelInfo fields
    // Adds weapon-specific information
    
    // vtable at 0x82F440 (64-bit)
    virtual ModelInfoType GetModelType() override; // Returns MODEL_INFO_WEAPON
    virtual void Init() override;
    virtual void SetClump(RpClump* clump) override;
};
```

### 5.3 Weapon Slots in Ped
```
Slot 0: Fists / Brass knuckles
Slot 1: Melee weapons (bat, nightstick, etc.)
Slot 2: Pistols
Slot 3: Shotguns
Slot 4: Submachine guns
Slot 5: Assault rifles
Slot 6: Sniper rifles
Slot 7: Heavy weapons (rocket launcher, minigun)
Slot 8: Throwables (grenades, molotov)
Slot 9: Special equipment (spray, extinguisher)
Slot 10: Gifts (flowers, dildo)
Slot 11: Accessories (parachute, goggles)
Slot 12: Detonator
```

---

## 6. Streaming System

### 6.1 CStreaming - Overview
```cpp
// Location: app/src/main/cpp/samp/game/Streaming.h

class CStreaming {
public:
    static CStreamingInfo ms_aInfoForModel[26316]; // Info for all resources
    static tStreamingChannel ms_channel[2];        // 2 streaming channels
    static tStreamingFileDesc ms_files[8];         // Open IMG files
    
    // Main functions
    static void RequestModel(int32 modelId, int32 flags);
    static void LoadAllRequestedModels(bool bPriorityOnly);
    static bool TryLoadModel(int modelId);
    static void RemoveModel(int32 modelId);
};
```

### 6.2 Streaming Flags
```cpp
#define STREAMING_GAME_REQUIRED     0x02  // Required by game
#define STREAMING_MISSION_REQUIRED  0x04  // Required by mission
#define STREAMING_KEEP_IN_MEMORY    0x08  // Keep in memory
#define STREAMING_PRIORITY_REQUEST  0x10  // Priority request
```

### 6.3 Loading States
```cpp
enum eStreamingLoadState : uint8 {
    LOADSTATE_NOT_LOADED = 0,  // Not loaded
    LOADSTATE_LOADED = 1,      // Loaded in memory
    LOADSTATE_REQUESTED = 2,   // Requested, waiting
    LOADSTATE_READING = 3,     // Being read from disk
    LOADSTATE_FINISHING = 4,   // Finishing (large models)
};
```

---

## 7. Project Implementation

### 7.1 CPedGTA::GiveWeapon
```cpp
// Location: app/src/main/cpp/samp/game/Entity/CPedGTA.cpp

void CPedGTA::GiveWeapon(int iWeaponID, int iAmmo)
{
    int iModelID = 0;
    iModelID = GameGetWeaponModelIDFromWeaponID(iWeaponID);
    
    // Validation
    if (iModelID == -1 || iModelID == 350 || iModelID == 365) 
        return;

    // Load model if needed
    if (!CStreaming::TryLoadModel(iModelID))
        return;

    // Call native function
    CHook::CallFunction<void>(
        g_libGTASA + (VER_x32 ? 0x0049F588 + 1 : 0x59525C), 
        this, iWeaponID, iAmmo
    );
    
    // Set as current weapon
    CHook::CallFunction<void>(
        g_libGTASA + (VER_x32 ? 0x004A521C + 1 : 0x59B86C), 
        this, iWeaponID
    );
}
```

### 7.2 CStreaming::TryLoadModel
```cpp
// Location: app/src/main/cpp/samp/game/Streaming.cpp

bool CStreaming::TryLoadModel(int modelId) {
    if(!CStreaming::GetInfo(modelId).IsLoaded()) {
        // Request model
        CStreaming::RequestModel(modelId, 
            STREAMING_GAME_REQUIRED | STREAMING_KEEP_IN_MEMORY);
        
        // Load all requested models
        CStreaming::LoadAllRequestedModels(false);
        
        // Wait for loading with timeout
        uint32 count = 0;
        while (!CStreaming::GetInfo(modelId).IsLoaded()) {
            count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (count > 30) { // 3 second timeout
                return false;
            }
        }
    }
    return true;
}
```

---

## 8. Technical Considerations

### 8.1 Common Issues

1. **Model doesn't load**
   - Check if modelId is in valid range (330-372)
   - Check if IMG file is accessible
   - Check available memory for streaming

2. **Invisible weapon on ped**
   - Check if `AddWeaponModel` was called
   - Check if hand bone is correct
   - Check if clump was attached correctly

3. **Crash when switching weapons**
   - Check if previous model was removed
   - Check pending references to model
   - Check if streaming is not overloaded

### 8.2 Best Practices

1. **Always verify loading**
   ```cpp
   if (!CStreaming::TryLoadModel(modelId))
       return; // Don't proceed if failed
   ```

2. **Use appropriate flags**
   ```cpp
   // For mission weapons
   RequestModel(id, STREAMING_MISSION_REQUIRED | STREAMING_KEEP_IN_MEMORY);
   
   // For temporary weapons
   RequestModel(id, STREAMING_GAME_REQUIRED);
   ```

3. **Release unused models**
   ```cpp
   CStreaming::SetModelIsDeletable(modelId);
   // or
   CStreaming::RemoveModelIfNoRefs(modelId);
   ```

### 8.3 Important Offsets (64-bit)

| Field/Function | Offset |
|---------------|--------|
| g_libGTASA + CPed::GiveWeapon | +0x59525C |
| g_libGTASA + CPed::AddWeaponModel | +0x59B174 |
| g_libGTASA + CPed::RemoveWeaponModel | +0x59B2B4 |
| g_libGTASA + CPed::SetCurrentWeapon | +0x59B86C |
| g_libGTASA + CWeaponInfo::GetWeaponInfo | +0x709BA8 |
| CPedGTA::m_aWeapons | offset varies by version |
| CPedGTA::m_nActiveWeaponSlot | offset varies by version |
| CPedGTA::m_nWeaponModelId | offset varies by version |

### 8.4 Important Offsets (32-bit)

| Field/Function | Offset |
|---------------|--------|
| g_libGTASA + CPed::GiveWeapon | +0x49F588+1 |
| g_libGTASA + CPed::AddWeaponModel | +0x4A4CE9+1 |
| g_libGTASA + CPed::RemoveWeaponModel | +0x4A4DF5+1 |
| g_libGTASA + CPed::SetCurrentWeapon | +0x4A521C+1 |
| g_libGTASA + CWeaponInfo::GetWeaponInfo | +0x5E42E8+1 |

---

## Memory Diagram - Weapon Array in Ped

```
CPedGTA (Offset: ~0x700 in 32-bit)
    │
    ├── m_aWeapons[0]  ─────┬── dwType (4 bytes)
    │   (28 bytes)          ├── dwState (4 bytes)
    │                       ├── dwAmmoInClip (4 bytes)
    │                       ├── dwAmmo (4 bytes)
    │                       ├── m_nTimer (4 bytes)
    │                       ├── flags (2 bytes)
    │                       ├── padding (2 bytes)
    │                       └── m_pWeaponFxSys (4/8 bytes)
    │
    ├── m_aWeapons[1]  (28 bytes)
    ├── m_aWeapons[2]  (28 bytes)
    │   ...
    └── m_aWeapons[12] (28 bytes)
    
    Total: 13 slots × 28 bytes = 364 bytes
```

---

**Document created for GTA San Andreas weapon loading system analysis.**
**Based on libGTASA version 2.1 dumps (32-bit and 64-bit).**
