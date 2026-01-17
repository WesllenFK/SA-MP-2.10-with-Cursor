# Player System - Offset Table

**Legend:**
- ✅ = Implemented in APK
- ⚠️ = Partially implemented
- ❌ = Not implemented

> **Note:** 32-bit offsets need `+1` for ARM Thumb mode

---

## CPed - Base Functions

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ⚠️ | `CPed::CPed` | `0x49FA19` | `0x594D1C` | Constructor |
| ⚠️ | `CPed::~CPed` | - | `0x59541C` | Destructor |
| ⚠️ | `CPed::Initialise` | `0x49FA29` | `0x595898` | Initializes ped |
| ⚠️ | `CPed::SetModelIndex` | `0x49FAD5` | `0x595954` | Sets model |
| ❌ | `CPed::DeleteRwObject` | `0x49FC87` | `0x595BB4` | Removes RW object |

## CPed - State and Control

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ⚠️ | `CPed::SetPedState` | `0x4A1EA5` | `0x597FF0` | Sets state |
| ❌ | `CPed::IsPedInControl` | `0x4A18D1` | `0x597A0C` | Is in control |
| ❌ | `CPed::CanBeDeleted` | `0x4A4C89` | `0x59B0D8` | Can be deleted |
| ❌ | `CPed::CanBeArrested` | `0x4A4C0D` | `0x59B044` | Can be arrested |
| ❌ | `CPed::IsPointerValid` | `0x4A7371` | `0x59DF64` | Pointer valid |

## CPed - Movement and Physics

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPed::ProcessControl` | `0x4A2541` | `0x598730` | Per-frame update |
| ❌ | `CPed::UpdatePosition` | `0x4A1999` | `0x597B04` | Updates position |
| ❌ | `CPed::CalculateNewVelocity` | `0x4A1365` | `0x597518` | Calculates velocity |
| ❌ | `CPed::ProcessBuoyancy` | `0x4A1F01` | `0x598078` | Buoyancy |
| ⚠️ | `CPed::Teleport` | - | `0x59DD90` | Teleports |

## CPed - Animation

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPed::PreRenderAfterTest` | `0x4A5981` | `0x59C254` | Post-test render |
| ❌ | `CPed::DoFootLanded` | `0x4A4381` | `0x59A79C` | Foot on ground |
| ❌ | `CPed::RemoveWeaponAnims` | `0x4ADE25` | `0x5A6E7C` | Removes weapon anims |
| ❌ | `CPed::TurnBody` | - | `0x59DCA8` | Turns body |

## CPed - Bones

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ✅ | `CPed::GetBonePosition` | `0x4A4B0D` | `0x59AEE4` | Bone position |
| ❌ | `CPed::GetHeadAndFootPositions` | `0x4A83F1` | `0x59F4C4` | Head and feet |

## CPed - Audio

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPed::Say` | - | `0x5A57C4` | Says phrase |
| ❌ | `CPed::GetPedTalking` | `0x4AC96D` | `0x5A5768` | Is talking |
| ❌ | `CPed::EnablePedSpeech` | `0x4AC97D` | `0x5A5778` | Enables speech |
| ❌ | `CPed::DisablePedSpeech` | `0x4AC975` | `0x5A5770` | Disables speech |

---

## CPlayerPed - Initialization

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ⚠️ | `CPlayerPed::CPlayerPed` | `0x4C367D` | `0x5C0BAC` | Constructor |
| ⚠️ | `CPlayerPed::~CPlayerPed` | `0x4C3901` | `0x5C0EFC` | Destructor |
| ⚠️ | `CPlayerPed::SetupPlayerPed` | `0x4C39A5` | `0x5C0FD4` | Setup |
| ⚠️ | `CPlayerPed::DeactivatePlayerPed` | `0x4C3AD5` | `0x5C1140` | Deactivates |
| ⚠️ | `CPlayerPed::ReactivatePlayerPed` | `0x4C3AED` | `0x5C1158` | Reactivates |
| ⚠️ | `CPlayerPed::RemovePlayerPed` | `0x4C3A61` | `0x5C10B4` | Removes |
| ❌ | `CPlayerPed::SetInitialState` | `0x4C37B5` | `0x5C0D50` | Initial state |
| ⚠️ | `CPlayerPed::Load` | `0x4851E9` | `0x5774B8` | Loads save |
| ⚠️ | `CPlayerPed::Save` | `0x485163` | `0x577390` | Saves |

## CPlayerPed - Control

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPlayerPed::ProcessControl` | `0x4C47E9` | `0x5C2088` | Main update |
| ❌ | `CPlayerPed::ProcessPlayerWeapon` | `0x4C682F` | `0x5C4310` | Weapon |
| ❌ | `CPlayerPed::ProcessWeaponSwitch` | `0x4C5919` | `0x5C3278` | Switch weapon |
| ❌ | `CPlayerPed::ProcessAnimGroups` | `0x4C5E61` | `0x5C37E0` | Anim groups |
| ❌ | `CPlayerPed::SetRealMoveAnim` | `0x4C3B05` | `0x5C1170` | Real anim |

## CPlayerPed - Targeting

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPlayerPed::FindWeaponLockOnTarget` | `0x4C6D65` | `0x5C494C` | Finds target |
| ❌ | `CPlayerPed::FindNextWeaponLockOnTarget` | `0x4C7D4D` | `0x5C5AAC` | Next target |
| ❌ | `CPlayerPed::EvaluateTarget` | `0x4C739D` | `0x5C5020` | Evaluates |
| ❌ | `CPlayerPed::HandleTapToTarget` | `0x4C849D` | `0x5C623C` | Tap to target |
| ❌ | `CPlayerPed::HandleMeleeTargeting` | `0x4C8D8D` | `0x5C6BE0` | Melee |
| ❌ | `CPlayerPed::ClearWeaponTarget` | `0x4C58E5` | `0x5C3228` | Clears target |
| ❌ | `CPlayerPed::CanIKReachThisTarget` | `0x4C7321` | `0x5C4F90` | IK reaches |
| ❌ | `CPlayerPed::DoesTargetHaveToBeBroken` | `0x4C8EFD` | `0x5C6D9C` | Break lock |

## CPlayerPed - Group

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPlayerPed::TellGroupToStartFollowingPlayer` | `0x4C4555` | `0x5C1D34` | Follow |
| ❌ | `CPlayerPed::MakeThisPedJoinOurGroup` | `0x4C9D59` | `0x5C7E00` | Join group |
| ❌ | `CPlayerPed::DisbandPlayerGroup` | `0x4C9CED` | `0x5C7D6C` | Disband |

## CPlayerPed - Stamina

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPlayerPed::HandlePlayerBreath` | `0x4C56F5` | `0x5C3010` | Breath |
| ❌ | `CPlayerPed::HandleSprintEnergy` | `0x4C5809` | `0x5C3140` | Sprint energy |
| ❌ | `CPlayerPed::ResetSprintEnergy` | `0x4CA401` | `0x5C868C` | Resets energy |

## CPlayerPed - State

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPlayerPed::SetWantedLevel` | `0x4C9719` | `0x5C76D0` | Wanted level |
| ❌ | `CPlayerPed::Busted` | `0x4C970D` | `0x5C76C0` | Busted |
| ❌ | `CPlayerPed::PlayerWantsToAttack` | `0x4CA189` | `0x5C8358` | Wants attack |

---

## CPedModelInfo

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CPedModelInfo::SetClump` | `0x3866C5` | `0x45D514` | Sets clump |
| ❌ | `CPedModelInfo::CreateHitColModelSkinned` | `0x386711` | `0x45D578` | Skinned collision |
| ❌ | `CPedModelInfo::AnimatePedColModelSkinned` | `0x386959` | `0x45D8D0` | Animates collision |

---

## CAnimManager

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CAnimManager::Initialise` | `0x471F95` | - | Initializes |
| ❌ | `CAnimManager::Shutdown` | `0x38DCA1` | `0x466668` | Shuts down |
| ⚠️ | `CAnimManager::AddAnimation` | `0x38E069` | `0x466B64` | Adds anim |
| ⚠️ | `CAnimManager::BlendAnimation` | `0x38E341` | - | Blends |
| ⚠️ | `CAnimManager::LoadAnimFile` | `0x38E629` | - | Loads IFP |
| ❌ | `CAnimManager::GetAnimAssociation` | `0x38E039` | `0x466B34` | Gets association |

---

## CAEPedAudioEntity

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ❌ | `CAEPedAudioEntity::Initialise` | `0x399A51` | - | Initializes |
| ❌ | `CAEPedAudioEntity::HandleFootstepEvent` | `0x39852D` | - | Footsteps |
| ❌ | `CAEPedAudioEntity::HandleLandingEvent` | `0x398BB1` | - | Landing |
| ❌ | `CAEPedAudioEntity::TurnOnJetPack` | `0x399EB1` | - | Jetpack on |
| ❌ | `CAEPedAudioEntity::TurnOffJetPack` | `0x39A005` | - | Jetpack off |

---

## CTaskManager

| Status | Function | 32-bit | 64-bit | Description |
|:------:|----------|:------:|:------:|-------------|
| ✅ | `CTaskManager::CTaskManager` | `0x5338B1` | - | Constructor |
| ❌ | `CTaskManager::SetTask` | `0x53397B` | - | Sets task |
| ❌ | `CTaskManager::ManageTasks` | `0x533E23` | - | Manages |
| ✅ | `CTaskManager::FlushImmediately` | `0x53411F` | - | Flush immediate |

---

## Summary

| Category | Total | ✅ | ⚠️ | ❌ |
|----------|:-----:|:--:|:--:|:--:|
| CPed Base | 52 | 1 | 7 | 44 |
| CPlayerPed | 45 | 0 | 8 | 37 |
| CPedModelInfo | 9 | 0 | 0 | 9 |
| CAnimManager | 20 | 0 | 3 | 17 |
| CAEPed*Audio | 26 | 0 | 0 | 26 |
| CTaskManager | 13 | 3 | 0 | 10 |
| **TOTAL** | **165** | **4** | **18** | **143** |

---

## How to Use

### 32-bit (ARM Thumb)
```cpp
// +1 for Thumb mode
auto SetupPlayerPed = (void(*)(int))(g_libGTASA + 0x4C39A5 + 1);
SetupPlayerPed(playerNum);
```

### 64-bit (ARM64)
```cpp
auto SetupPlayerPed = (void(*)(int))(g_libGTASA + 0x5C0FD4);
SetupPlayerPed(playerNum);
```

### Multi-version Macro
```cpp
#define SETUP_PLAYER_PED_OFFSET (VER_x32 ? 0x4C39A5 + 1 : 0x5C0FD4)
```
