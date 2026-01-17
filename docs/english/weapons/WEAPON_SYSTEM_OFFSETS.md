# GTA SA Weapon System - Complete Offset Table

**Legend:**
- ✅ = Implemented in APK
- ⚠️ = Partially implemented (via hook)
- ❌ = Not implemented

> **Note:** 32-bit offsets need `+1` for ARM Thumb mode

---

## CStreaming (Streaming System)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ✅ | `RequestModel` | `0x2D299D` | `0x3924BC` | Adds model to request queue |
| ✅ | `LoadAllRequestedModels` | `0x2D46A9` | `0x396ABC` | Loads all pending models |
| ✅ | `FlushChannels` | `0x2D4879` | `0x396D1C` | Finishes channel loading |
| ✅ | `RemoveModel` | `0x2D0129` | `0x391FF0` | Removes model from memory |
| ✅ | `MakeSpaceFor` | `0x2D39E5` | `0x396008` | Frees memory for new model |
| ⚠️ | `LoadInitialWeapons` | `0x2D6BEF` | `0x399654` | Loads weapons at init |
| ✅ | `SetModelIsDeletable` | `0x2D6788` | `0x399090` | Marks model as deletable |
| ✅ | `InitImageList` | `0x2CF681` | `0x391320` | Initializes IMG file list |
| ✅ | `AddImageToList` | `0x2CF7D1` | `0x391410` | Adds IMG to list |
| ✅ | `ProcessLoadingChannel` | `0x2D2411` | `0x394298` | Processes streaming channel |
| ✅ | `ConvertBufferToObject` | `0x2D2FD0` | `0x395114` | Converts buffer to RW object |

---

## CWeapon (Weapon)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ⚠️ | `Fire` | `0x5DB9F1` | `0x700C64` | Fires weapon |
| ⚠️ | `FireInstantHit` | `0x5DC179` | `0x701494` | Hitscan fire |
| ⚠️ | `FireSniper` | `0x5DD741` | `0x702B20` | Sniper fire |
| ❌ | `FireProjectile` | `0x5DDECD` | `0x703274` | Projectile fire |
| ❌ | `FireFromCar` | `0x5DEFA1` | `0x7042D4` | Vehicle fire |
| ❌ | `FireAreaEffect` | `0x5DE6A9` | `0x703A2C` | Flamethrower/spray |
| ❌ | `Reload` | `0x5DB853` | `0x700A2C` | Reloads weapon |
| ❌ | `Update` | `0x5E1F19` | `0x707730` | Updates state |
| ❌ | `Initialise` | `0x5DB901` | `0x700B10` | Initializes weapon |
| ❌ | `DoBulletImpact` | `0x5E07D9` | `0x705C7C` | Processes impact |
| ❌ | `GenerateDamageEvent` | `0x5E1395` | `0x706888` | Generates damage event |
| ❌ | `DoDoomAiming` | `0x5DFF25` | `0x705290` | Auto-aim |
| ❌ | `DoTankDoomAiming` | `0x5E1B4D` | `0x707284` | Tank auto-aim |
| ❌ | `DoDriveByAutoAiming` | `0x5DFB81` | `0x704EEC` | Drive-by auto-aim |

---

## CWeaponInfo (Weapon Data)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `Initialise` | `0x4746B9` | `0x560E20` | Initializes system |
| ⚠️ | `GetWeaponInfo` | `0x5E42E9` | `0x709BA8` | Gets weapon info |
| ❌ | `LoadWeaponData` | `0x474751` | `0x560EA8` | Loads weapon.dat |
| ❌ | `FindWeaponType` | `0x5E4335` | `0x709C24` | Finds type by name |
| ❌ | `GetSkillStatIndex` | `0x5E4319` | `0x709BFC` | Skill index |

---

## CPed (Ped Weapons)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ✅ | `GiveWeapon` | `0x49F589` | `0x59525C` | Gives weapon to ped |
| ⚠️ | `ClearWeapons` | `0x49F837` | `0x595604` | Removes all weapons |
| ❌ | `ClearWeapon` | `0x4A52D9` | `0x59B998` | Removes specific weapon |
| ⚠️ | `AddWeaponModel` | `0x4A4CE9` | `0x59B174` | Attaches 3D model |
| ⚠️ | `RemoveWeaponModel` | `0x4A4DF5` | `0x59B2B4` | Removes 3D model |
| ⚠️ | `SetCurrentWeapon` | `0x4A521D` | `0x59B86C` | Switches weapon |
| ❌ | `GetWeaponSlot` | `0x4A5179` | `0x59B78C` | Returns weapon slot |
| ⚠️ | `SetWeaponSkill` | `0x4A56E7` | `0x59BF24` | Sets skill |
| ✅ | `SetAmmo` | `0x4A5283` | `0x59B918` | Sets ammo |
| ❌ | `GrantAmmo` | `0x4A5235` | `0x59B8A0` | Adds ammo |
| ❌ | `GetBonePosition` | `0x4A4B0D` | `0x59AEE4` | Bone position |

---

## CExplosion (Explosions)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `Initialise` | `0x5D78C1` | `0x6FCE60` | Initializes pool |
| ❌ | `AddExplosion` | `0x5D7A1D` | `0x6FCFAC` | Creates explosion |
| ❌ | `Update` | `0x5D89DD` | `0x6FDC88` | Updates |

---

## CProjectileInfo (Projectiles)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `Initialise` | `0x5D9205` | `0x6FF5C0` | Initializes pool |
| ❌ | `AddProjectile` | `0x5D9305` | `0x6FF6B0` | Creates projectile |
| ❌ | `Update` | `0x5D9F39` | `0x700210` | Updates all |
| ❌ | `RemoveProjectile` | `0x5D9D19` | `0x6FEFC8` | Removes |

---

## CFireManager (Fire)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `Init` | `0x3F11AD` | `0x4D30CC` | Initializes |
| ❌ | `Update` | `0x3F1629` | `0x4D361C` | Updates |
| ❌ | `StartFire` | `0x3F2619` | `0x4D4550` | Starts fire |
| ❌ | `ExtinguishPoint` | `0x3F2CA5` | `0x4D4D8C` | Extinguishes |

---

## CAEWeaponAudioEntity (Audio)

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `Initialise` | `0x3B9EB1` | `0x497B00` | Initializes |
| ❌ | `WeaponFire` | `0x3B9F35` | `0x497BA4` | Fire sound |
| ❌ | `WeaponReload` | `0x3BAEED` | `0x498D90` | Reload sound |
| ❌ | `PlayGunSounds` | `0x3BA20D` | `0x497E98` | Various sounds |

---

## Coverage Summary

| Category | Total | ✅ | ⚠️ | ❌ |
|----------|:-----:|:--:|:--:|:--:|
| CStreaming | 19 | 14 | 1 | 4 |
| CWeapon | 38 | 0 | 3 | 35 |
| CWeaponInfo | 9 | 0 | 1 | 8 |
| CPed | 26 | 2 | 5 | 19 |
| Others | 92 | 0 | 0 | 92 |
| **TOTAL** | **184** | **16** | **10** | **158** |

---

## How to Use Offsets

### 32-bit (ARM Thumb)
```cpp
// Add +1 for Thumb mode
auto func = (void(*)(...))(g_libGTASA + 0x49F588 + 1);
func(...);

// Or using CHook
CHook::CallFunction<void>(g_libGTASA + 0x49F588 + 1, args...);
```

### 64-bit (ARM64)
```cpp
// No modification
auto func = (void(*)(...))(g_libGTASA + 0x59525C);
func(...);

// Or using CHook
CHook::CallFunction<void>(g_libGTASA + 0x59525C, args...);
```

### Version Macro
```cpp
#define WEAPON_FIRE_OFFSET (VER_x32 ? 0x5DB9F1 + 1 : 0x700C64)
```
