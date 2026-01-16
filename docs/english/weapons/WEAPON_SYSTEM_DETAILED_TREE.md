# GTA SA Weapon System - Detailed Function Tree

**Legend:**
- ✅ = Implemented in APK
- ⚠️ = Partially implemented (via hook/offset)
- ❌ = Not implemented (only in native libGTASA)

---

## RESOURCE SYSTEM

```
CStreaming
│
├── ✅ RequestModel(modelId, flags)
│   └── Adds a model to the streaming request queue
│       to be loaded from the IMG file
│
├── ✅ LoadAllRequestedModels(bPriorityOnly)
│   └── Loads all pending models in the request queue
│       Processes streaming channels until queue is empty
│
├── ✅ TryLoadModel(modelId)
│   └── Tries to load a model synchronously
│       Waits up to 3 seconds for complete loading
│       Returns true if loaded, false if timeout
│
├── ⚠️ LoadInitialWeapons()
│   └── Loads basic weapon models at initialization
│       Called during game boot
│
├── ✅ RemoveModel(modelId)
│   └── Removes a model from memory
│       Frees associated RW (RenderWare) resources
│       Updates used memory counters
│
├── ✅ GetInfo(modelId)
│   └── Returns CStreamingInfo for the model
│       Contains loading state, flags, CD position
│
└── ✅ SetModelIsDeletable(modelId)
    └── Marks model as deletable by streaming system
        Allows removal when memory is low
```

```
CWeaponModelInfo (inherits from CClumpModelInfo)
│
├── ❌ Init()
│   └── Initializes weapon model info structure
│       Configures default values and allocates resources
│
├── ❌ SetClump(RpClump*)
│   └── Sets the 3D model (clump) for this weapon
│       Associates RenderWare geometry to the model
│
└── ❌ GetModelType()
    └── Returns MODEL_INFO_WEAPON (type 8)
        Used to identify it as a weapon model
```

```
CModelInfo
│
├── ✅ GetModelInfo(index)
│   └── Returns CBaseModelInfo pointer for the model
│       Accesses global array ms_modelInfoPtrs[20000]
│
├── ⚠️ AddWeaponModel(index)
│   └── Creates and registers a new CWeaponModelInfo
│       Adds to weapon model store (51 slots)
│
└── ✅ ms_modelInfoPtrs[20000]
    └── Global array with pointers to all models
        Indexed by Model ID
```

---

## CORE WEAPON SYSTEM

```
CWeapon (28/32 byte structure)
│
├── Fields:
│   ├── dwType (eWeaponType)     → Weapon type (0-46)
│   ├── dwState (eWeaponState)   → State: ready, reloading, etc
│   ├── dwAmmoInClip             → Ammo in current clip
│   ├── dwAmmo                   → Total available ammo
│   ├── m_nTimer                 → Fire rate timer
│   └── m_pWeaponFxSys           → Visual effects system
│
├── ⚠️ Fire(pEntity, origin, muzzle, target, ...)
│   └── Main fire function
│       Decides which fire type to execute:
│       - FireInstantHit for bullets
│       - FireProjectile for rockets/grenades
│       - FireAreaEffect for flamethrower
│
├── ⚠️ FireInstantHit(pFiringEntity, vecOrigin, vecMuzzle, ...)
│   └── Fires instant bullet (hitscan)
│       Does raycast from barrel to max range
│       Calculates damage and applies to hit target
│       Creates visual effects (tracer, impact)
│
├── ⚠️ FireSniper(pPed, pTargetEntity, vecOrigin)
│   └── Specialized fire for sniper rifles
│       Uses telescopic sight and zoom
│       Higher accuracy and damage at distance
│
├── ❌ FireProjectile(pEntity, vecOrigin, pTarget, vecTarget, speed)
│   └── Fires physical projectile (rocket, grenade)
│       Creates CProjectile entity in world
│       Projectile has its own physics and can be tracked
│
├── ❌ FireFromCar(pVehicle, bLeftHand, bRightHand)
│   └── Fire from inside vehicle
│       Adjusts angles for drive-by
│       Limits allowed weapon types
│
├── ❌ FireAreaEffect(pEntity, vecOrigin, pTarget, vecTarget)
│   └── Area fire (flamethrower, spray)
│       Creates effect cone in front of player
│       Applies continuous damage/effect in area
│
├── ❌ Reload(pPed)
│   └── Reloads the weapon
│       Moves ammo from stock to clip
│       Plays reload animation
│
├── ❌ Update(pPed)
│   └── Per-frame weapon update
│       Manages cooldowns, continuous effects
│       Updates weapon state
│
├── ❌ Initialise(weaponType, ammo, pOwner)
│   └── Initializes weapon instance
│       Sets type, initial ammo, owner
│
├── ❌ Shutdown()
│   └── Finalizes and cleans weapon resources
│       Stops visual effects, frees memory
│
├── ❌ AddGunshell(pEntity, vecPos, vecDir, size)
│   └── Creates ejected shell casing
│       Visual effect of flying casings
│
├── ❌ DoBulletImpact(pShooter, pVictim, vecStart, vecEnd, colPoint, dmg)
│   └── Processes bullet impact
│       Applies damage, creates blood/spark effects
│       Generates damage event for victim
│
├── ❌ GenerateDamageEvent(pVictim, pAttacker, weaponType, damage, ...)
│   └── Generates damage event in event system
│       Notifies AI, statistics, scripts
│
├── ❌ ProcessLineOfSight(start, end, colPoint, entity, ...)
│   └── Does raycast to check line of sight
│       Used to determine if shot hits target
│       Considers walls, vehicles, peds
│
├── ❌ DoDoomAiming(pTarget, vecStart, vecEnd)
│   └── Assisted aiming system (auto-aim)
│       Adjusts shot direction to closest target
│       "Doom aiming" = classic FPS style
│
├── ❌ DoTankDoomAiming(pShooter, pTarget, vecStart, vecEnd)
│   └── Assisted aiming for tank cannon
│       Considers ballistics and distance
│
├── ❌ DoDriveByAutoAiming(pTarget, pVehicle, vecStart, vecEnd, bPassenger)
│   └── Assisted aiming for drive-by
│       Compensates vehicle movement
│       Adjusts for passenger position
│
├── ❌ LaserScopeDot(vecOut, pRange)
│   └── Calculates laser dot position on scope
│       Used by rifles with laser sight
│
├── ❌ PickTargetForHeatSeekingMissile(pos, dir, range, pShooter, ...)
│   └── Selects target for heat-seeking missile
│       Prioritizes aerial vehicles with engine on
│
├── ❌ FindNearestTargetEntityWithScreenCoors(x, y, z, pos, ...)
│   └── Finds closest entity to screen coordinates
│       Used for lock-on in third person
│
├── ❌ IsTypeMelee()
│   └── Returns true if weapon is melee
│       (knife, bat, fist, etc)
│
├── ❌ IsType2Handed()
│   └── Returns true if weapon uses two hands
│       (rifles, shotguns, etc)
│
├── ❌ IsTypeProjectile()
│   └── Returns true if weapon fires projectiles
│       (rockets, grenades, etc)
│
├── ❌ HasWeaponAmmoToBeUsed()
│   └── Checks if has available ammo
│
├── ❌ CanBeUsedFor2Player()
│   └── Checks if weapon can be used in co-op
│
├── ❌ DoWeaponEffect(vecStart, vecEnd)
│   └── Creates shot visual effect (tracer, flash)
│
├── ❌ StopWeaponEffect()
│   └── Stops continuous effects (flamethrower, minigun)
│
├── ❌ TakePhotograph(pTarget, vecPos)
│   └── Special camera function
│       Captures "photo" of target for missions
│
├── ❌ CheckForShootingVehicleOccupant(ppEntity, colPoint, ...)
│   └── Checks if shot hit vehicle occupant
│       Allows killing driver through glass
│
├── ❌ SetUpPelletCol(numPellets, pShooter, pVictim, ...)
│   └── Configures collision for shotgun shots
│       Shotguns fire multiple pellets
│
└── ❌ TargetWeaponRangeMultiplier(pShooter, pTarget)
    └── Calculates range multiplier
        Considers skill and weapon type
```

```
CWeaponInfo (weapon.dat data)
│
├── Fields per weapon:
│   ├── fireType          → Fire type (instant, projectile, area)
│   ├── targetRange       → Maximum range
│   ├── weaponRange       → Effective range
│   ├── damage            → Base damage
│   ├── accuracy          → Base accuracy
│   ├── moveSpeed         → Movement speed with weapon
│   ├── animGroup         → Animation group
│   ├── ammoClip          → Clip size
│   ├── fireOffset        → Weapon barrel offset
│   └── skillLevel        → Skill requirement
│
├── ❌ Initialise()
│   └── Initializes weapon info system
│       Allocates arrays and configures default values
│
├── ❌ LoadWeaponData()
│   └── Loads weapon.dat from disk
│       Parses each line to fill data
│       Called during game initialization
│
├── ⚠️ GetWeaponInfo(weaponType, skillLevel)
│   └── Returns CWeaponInfo* for weapon and skill
│       Each weapon has 3 variants (poor, std, pro)
│
├── ❌ FindWeaponType(name)
│   └── Searches weapon type by string name
│       Ex: "PISTOL" → WEAPON_PISTOL (22)
│
├── ❌ FindWeaponFireType(name)
│   └── Searches fire type by name
│       Ex: "INSTANT_HIT" → type 1
│
├── ❌ GetSkillStatIndex(weaponType)
│   └── Returns skill stat index
│       Used to read/write player skill
│
├── ❌ GetTargetHeadRange()
│   └── Returns headshot range
│       Varies by weapon and skill
│
├── ❌ GetWeaponReloadTime()
│   └── Returns reload time in ms
│
├── ❌ Shutdown()
│   └── Frees system resources
│
└── ms_aWeaponNames[47]
    └── Array with weapon names for debug
```

---

## PED WEAPON SYSTEM

```
CPedGTA (Ped structure - ~0x7A4/0x988 bytes)
│
├── Weapon Fields:
│   ├── m_aWeapons[13]       → Array with 13 CWeapon slots
│   ├── m_nActiveWeaponSlot  → Active slot index (0-12)
│   ├── m_nSavedWeapon       → Temporarily saved weapon
│   ├── m_nDelayedWeapon     → Backup weapon
│   ├── m_nDelayedWeaponAmmo → Backup weapon ammo
│   ├── m_nWeaponModelId     → Current 3D model ID (-1 if none)
│   ├── m_nWeaponSkill       → Current skill level
│   ├── m_nWeaponShootingRate→ Fire rate
│   ├── m_nWeaponAccuracy    → Accuracy (0-100)
│   ├── m_pWeaponObject      → RpClump* of weapon model
│   ├── m_pGunflashObject    → RpClump* of gunflash
│   ├── m_pEntLockOnTarget   → Lock-on target entity
│   └── m_vecWeaponPrevPos   → Previous weapon position
│
├── ✅ GiveWeapon(weaponType, ammo)
│   └── Gives a weapon to ped with ammo
│       1. Loads model via CStreaming
│       2. Finds correct slot for type
│       3. Initializes CWeapon in slot
│       4. Sets as current weapon
│
├── ❌ ClearWeapon(weaponType)
│   └── Removes specific weapon from inventory
│       Frees slot and removes visual model
│
├── ⚠️ ClearWeapons()
│   └── Removes all ped weapons
│       Clears all 13 slots
│       Removes visual model from hand
│
├── ⚠️ AddWeaponModel(modelId)
│   └── Attaches weapon 3D model to ped's hand
│       1. Removes previous model if any
│       2. Clones RpClump of model
│       3. Attaches to right hand bone (24)
│       4. Updates m_pWeaponObject
│
├── ⚠️ RemoveWeaponModel(modelId)
│   └── Removes weapon 3D model from hand
│       Detaches from bone and destroys clone
│       Sets m_pWeaponObject = nullptr
│
├── ⚠️ SetCurrentWeapon(weaponType)
│   └── Switches to specified weapon
│       1. Finds weapon slot
│       2. Updates m_nActiveWeaponSlot
│       3. Switches visual model
│       4. Updates animations
│
├── ❌ GetWeaponSlot(weaponType)
│   └── Returns slot index for type
│       Each type has fixed slot (pistols=2, etc)
│
├── ❌ GetWeaponSkill(weaponType)
│   └── Returns skill level for weapon
│       0=poor, 1=std, 2=pro
│
├── ⚠️ SetWeaponSkill(weaponType, skillLevel)
│   └── Sets skill for weapon type
│       Affects accuracy, damage, animations
│
├── ❌ SetWeaponLockOnTarget(pEntity)
│   └── Sets entity as lock-on target
│       Stores in m_pEntLockOnTarget
│
├── ❌ DoWeHaveWeaponAvailable(weaponType)
│   └── Checks if ped has the weapon
│       Returns true if slot has ammo > 0
│
├── ❌ GiveDelayedWeapon(weaponType, ammo)
│   └── Configures backup weapon
│       Ex: cop has nightstick + pistol
│
├── ❌ RequestDelayedWeapon()
│   └── Draws backup weapon
│       Switches from current to backup
│
├── ❌ RemoveWeaponAnims(slot, blendDelta)
│   └── Removes weapon animations
│       Used when holstering weapon or dying
│
├── ❌ CanWeRunAndFireWithWeapon()
│   └── Checks if can run and fire
│       Depends on type (SMGs yes, rifles no)
│
├── ❌ RemoveWeaponWhenEnteringVehicle(flags)
│   └── Holsters weapon when entering vehicle
│       Some weapons are hidden automatically
│
├── ❌ ReplaceWeaponWhenExitingVehicle()
│   └── Restores weapon when exiting vehicle
│       Shows weapon that was hidden
│
├── ✅ SetAmmo(weaponType, ammo)
│   └── Sets ammo for specific weapon
│       Finds slot and updates dwAmmo
│
├── ❌ GrantAmmo(weaponType, ammo)
│   └── Adds ammo (doesn't replace)
│       current_ammo += ammo
│
├── ❌ CreateDeadPedWeaponPickups()
│   └── Creates weapon pickups when dying
│       Drops weapons ped was carrying
│
├── ❌ RemoveWeaponForScriptedCutscene()
│   └── Removes weapon for cutscene
│       Hides model but keeps in inventory
│
└── ❌ ReplaceWeaponForScriptedCutscene()
    └── Restores weapon after cutscene
```

```
CPlayerPed (High-level wrapper)
│
├── ✅ GiveWeapon(weaponId, ammo)
│   └── Wrapper that calls CPedGTA::GiveWeapon
│       Checks if m_pPed is valid first
│
├── ✅ GetCurrentWeapon()
│   └── Returns current weapon type (0-46)
│       Reads from ped's active slot
│
├── ✅ GetCurrentWeaponSlot()
│   └── Returns CWeapon* pointer of active slot
│       &m_pPed->m_aWeapons[m_nActiveWeaponSlot]
│
├── ✅ FindWeaponSlot(weaponType)
│   └── Searches slot that contains weapon
│       Iterates through 13 slots searching
│
├── ✅ SetAmmo(weaponType, ammo)
│   └── Sets ammo via FindWeaponSlot
│
├── ✅ SetArmedWeapon(weaponType, unk)
│   └── Switches current weapon (not implemented)
│
├── ✅ ClearWeapons()
│   └── Removes all weapons via native hook
│
├── ✅ FireInstant()
│   └── Fires current weapon
│       1. Saves camera state
│       2. Configures remote player aim
│       3. Calls CWeapon::FireInstantHit
│       4. Restores camera state
│
├── ✅ GetWeaponInfoForFire(bLeftWrist, vecBonePos, vecOut)
│   └── Gets position and offset for fire
│       Calculates barrel position based on bone
│
├── ✅ GetCurrentWeaponFireOffset()
│   └── Returns fire offset from weapon.dat
│       Relative position of weapon barrel
│
├── ✅ ProcessBulletData(btData)
│   └── Processes bullet data for sync
│       Prepares BULLET_SYNC_DATA for sending
│       Identifies hit type (player/vehicle/object)
│
├── ✅ SetWeaponSkill(weaponType, skill)
│   └── Sets skill via stats system
│       Updates local or remote depending on player
│
├── ❌ ProcessPlayerWeapon(pPad)
│   └── Processes player weapon input
│       Checks fire, aim, switch buttons
│
├── ❌ ProcessWeaponSwitch(pPad)
│   └── Processes weapon switch via input
│       Scroll or number keys
│
├── ❌ FindWeaponLockOnTarget()
│   └── Searches valid target for lock-on
│       Prioritizes threats and proximity
│
├── ❌ FindNextWeaponLockOnTarget(pCurrent, bForward)
│   └── Cycles to next/previous target
│
├── ❌ DoesPlayerWantNewWeapon(weaponType, bForce)
│   └── Checks if player wants to switch
│
├── ❌ MakeChangesForNewWeapon(weaponType)
│   └── Applies visual/animation changes
│       when switching weapons
│
├── ❌ ClearWeaponTarget()
│   └── Removes current lock-on
│
├── ❌ GetWeaponRadiusOnScreen()
│   └── Returns crosshair radius on screen
│       Used to draw crosshair
│
├── ❌ UpdateCameraWeaponModes(pPad)
│   └── Updates camera mode based on weapon
│       Switches between aiming modes
│
├── ❌ CanIKReachThisTarget(vecTarget, pWeapon, bCheck)
│   └── Checks if IK can aim at target
│       Torso/arm rotation limits
│
└── ❌ DoesTargetHaveToBeBroken(pEntity, pWeapon)
    └── Checks if should break lock-on
        Target died, out of range, etc
```

---

## AIMING SYSTEM

```
CWeaponEffects
│
├── ❌ Init()
│   └── Initializes aim effects system
│       Allocates structures for crosshairs
│
├── ❌ Render()
│   └── Renders crosshairs and markers
│       Draws 2D sprites at target position
│
├── ❌ MarkTarget(index, vecPos, r, g, b, a, size, type)
│   └── Marks position with crosshair
│       Creates visual indicator on target
│
├── ❌ IsLockedOn(index)
│   └── Returns true if slot has lock-on
│
├── ❌ ClearCrossHair(index)
│   └── Removes specific crosshair
│
├── ❌ ClearCrossHairs()
│   └── Removes all crosshairs
│
├── ❌ ClearCrossHairImmediately(index)
│   └── Removes crosshair without fade
│
├── ❌ ClearAllCrosshairs()
│   └── Clears all markers
│
└── ❌ Shutdown()
    └── Finalizes effects system
```

```
CCamera (Weapon functions)
│
├── ✅ SetBehindPlayer()
│   └── Positions camera behind player
│       Used when exiting vehicle, dying, etc
│
├── ✅ GetPosition()
│   └── Returns current camera position
│
├── ❌ IsTargetingActive()
│   └── Checks if in aiming mode
│       True when holding aim button
│
├── ❌ UpdateAimingCoors(vecTarget)
│   └── Updates aiming coordinates
│       Moves camera smoothly to target
│
├── ❌ UpdateTargetEntity()
│   └── Updates camera target entity
│
├── ❌ SetColVarsAimWeapon(mode)
│   └── Configures collision for aim mode
│       Adjusts camera clipping
│
├── ❌ ClearPlayerWeaponMode()
│   └── Clears player weapon mode
│       Returns to normal camera
│
├── ❌ SetNewPlayerWeaponMode(primary, secondary, mode)
│   └── Sets new weapon camera mode
│       Ex: over-the-shoulder aim, first person
│
├── ❌ Using1stPersonWeaponMode()
│   └── Returns true if in first person
│       Sniper, rocket launcher use this
│
├── ❌ Find3rdPersonQuickAimPitch()
│   └── Calculates pitch for quick aim
│       Used when aiming without lock-on
│
├── ❌ CameraPedAimModeSpecialCases(pPed)
│   └── Handles special aim cases
│       Adjustments for jetpack, bicycle, etc
│
├── ❌ Find3rdPersonCamTargetVector(angle, source, dir, pos)
│   └── Calculates aim vector in 3rd person
│       Projects ray from camera to world
│
├── ❌ Enable1rstPersonWeaponsCamera()
│   └── Activates first person camera
│       For sniper and telescopic sights
│
└── ❌ HandleCameraMotionForDuckingDuringAim(pPed, ...)
    └── Adjusts camera when crouching while aiming
        Height and angle compensation
```

---

## DAMAGE SYSTEM

```
CPedDamageResponseCalculator
│
├── ❌ CPedDamageResponseCalculator(pAttacker, damage, weaponType, pieceType, bSpeak)
│   └── Constructor that calculates damage response
│       Considers armor, body part, weapon type
│       Calculates final damage and ped reaction
│
└── ❌ IsBleedingWeapon(pPed)
    └── Checks if weapon causes bleeding
        Knives, axes cause bleeding
        Affects visually and can cause slow death
```

```
CDamageManager (Vehicle damage)
│
├── ❌ ApplyDamage(pAutomobile, component, intensity, damageType)
│   └── Applies damage to vehicle component
│       Calculates deformation and can destroy part
│
├── ❌ SetDoorStatus(door, status)
│   └── Sets door state
│       0=ok, 1=dented, 2=loose, 3=removed
│
├── ❌ SetPanelStatus(panel, status)
│   └── Sets panel state
│       Hood, trunk, fenders
│
├── ❌ SetWheelStatus(wheel, status)
│   └── Sets wheel state
│       0=ok, 1=flat, 2=removed
│
├── ❌ SetLightStatus(light, status)
│   └── Sets headlight state
│       0=working, 1=broken
│
├── ❌ SetEngineStatus(status)
│   └── Sets engine state
│       Affects performance and can catch fire
│
├── ❌ FuckCarCompletely(bExplode)
│   └── Completely destroys vehicle
│       All parts damaged
│
├── ❌ ProgressDoorDamage(door, pAutomobile)
│   └── Progresses door damage
│       Dented → Loose → Removed
│
└── ❌ GetCarNodeIndexFromDoor(door)
    └── Converts door enum to node index
        Used to find correct geometry
```

---

## PROJECTILE SYSTEM

```
CProjectileInfo
│
├── ❌ Initialise()
│   └── Initializes projectile pool
│       Allocates array for 32 simultaneous projectiles
│
├── ❌ AddProjectile(pCreator, weaponType, vecPos, speed, vecDir, pTarget)
│   └── Creates new projectile in world
│       1. Finds free slot in pool
│       2. Creates CProjectile entity
│       3. Sets physics (velocity, gravity)
│       4. Configures tracking if guided missile
│
├── ❌ Update()
│   └── Updates all active projectiles
│       Moves projectiles, checks collisions
│       Explodes if hit something or timeout
│
├── ❌ RemoveProjectile(pInfo, pProjectile)
│   └── Removes specific projectile
│       Destroys entity and frees slot
│
├── ❌ RemoveAllProjectiles()
│   └── Removes all projectiles
│       Called when loading save
│
├── ❌ RemoveDetonatorProjectiles()
│   └── Removes remote control bombs
│       Called when using detonator
│
├── ❌ IsProjectileInRange(x1, y1, z1, x2, y2, z2, bExplode)
│   └── Checks if projectile in area
│       Optionally explodes found projectiles
│
├── ❌ RemoveFXSystem(index)
│   └── Removes projectile effects system
│       Smoke, fire, etc
│
├── ❌ RemoveIfThisIsAProjectile(pObject)
│   └── Removes if object is projectile
│       Safety check
│
├── ❌ GetProjectileInfo(index)
│   └── Returns projectile info by index
│
└── ms_apProjectile[32]
    └── Array of pointers to active projectiles
```

```
CProjectile (Physical entity)
│
├── ❌ CProjectile(modelId)
│   └── Projectile entity constructor
│       Inherits from CObject
│       Configures special collision
│
└── ❌ ~CProjectile()
    └── Destructor
        Frees resources and effects
```

---

## BULLET SYSTEM

```
CBulletInfo
│
├── ❌ Initialise()
│   └── Initializes bullet system
│       Allocates array for bullet tracking
│       Used for physical bullets (not hitscan)
│
├── ❌ AddBullet(pShooter, weaponType, vecPos, vecVelocity)
│   └── Adds bullet to system
│       For weapons with visible bullets
│       (minigun, etc)
│
├── ❌ Update()
│   └── Updates all bullets
│       Moves, checks collisions, removes old ones
│
└── ❌ Shutdown()
    └── Finalizes bullet system
```

```
CBulletTraces (Visual traces)
│
├── ❌ Init()
│   └── Initializes tracer system
│       Allocates structures for traces
│
├── ❌ AddTrace(vecStart, vecEnd, width, color, type)
│   └── Adds bullet trace
│       Luminous line from barrel to impact
│       Version 1: initial and final position
│
├── ❌ AddTrace(vecStart, vecEnd, time, pEntity)
│   └── Adds trace with duration
│       Version 2: with lifetime
│
├── ❌ Update()
│   └── Updates active traces
│       Gradual fade out, removes old ones
│
├── ❌ Render()
│   └── Renders all traces
│       Draws lines/sprites
│
└── aTraces[16]
    └── Array of active traces
```

---

## EXPLOSION SYSTEM

```
CExplosion
│
├── ❌ Initialise()
│   └── Initializes explosion pool
│       Allocates array for 48 explosions
│
├── ❌ AddExplosion(pCreator, pVictim, type, vecPos, time, bMakeSound, camShake, bVisible)
│   └── Creates new explosion
│       1. Finds free slot
│       2. Configures type (grenade, car, barrel, etc)
│       3. Applies damage in radius
│       4. Creates visual and audio effects
│       5. Pushes nearby objects/peds
│
├── ❌ Update()
│   └── Updates active explosions
│       Expands radius, creates secondary fire
│       Removes finished explosions
│
├── ❌ GetExplosionType(index)
│   └── Returns explosion type
│       EXPLOSION_GRENADE, EXPLOSION_CAR, etc
│
├── ❌ GetExplosionPosition(index)
│   └── Returns explosion position
│
├── ❌ GetExplosionActiveCounter(index)
│   └── Returns active frame counter
│
├── ❌ ResetExplosionActiveCounter(index)
│   └── Resets counter to zero
│
├── ❌ DoesExplosionMakeSound(index)
│   └── Checks if explosion makes sound
│       Some are silent
│
├── ❌ TestForExplosionInArea(type, x1, y1, z1, x2, y2, z2)
│   └── Tests if explosion of type in area
│       Used by scripts
│
├── ❌ RemoveAllExplosionsInArea(vecCenter, radius)
│   └── Removes explosions in radius
│
├── ❌ ClearAllExplosions()
│   └── Clears all explosions
│
└── ❌ Shutdown()
    └── Finalizes explosion system
```

---

## FIRE SYSTEM

```
CFire
│
├── ❌ Initialise()
│   └── Initializes fire instance
│       Configures particles, size, damage
│
├── ❌ ProcessFire()
│   └── Processes fire per frame
│       Spreads to nearby objects
│       Applies damage to peds/vehicles
│       Consumes fuel and can go out
│
├── ❌ Extinguish()
│   └── Extinguishes the fire
│       Removes particles, stops damage
│
└── ❌ CreateFxSysForStrength(vecPos, matrix)
    └── Creates particle system based on strength
        Bigger fire = more particles
```

```
CFireManager
│
├── ❌ Init()
│   └── Initializes fire manager
│       Pool of 60 simultaneous fires
│
├── ❌ Update()
│   └── Updates all fires
│       Processes each active CFire
│
├── ❌ StartFire(vecPos, size, bAllowSpreading, pCreator, burnTime, strength, bSmall)
│   └── Starts fire at position
│       Version 1: fire on ground/object
│
├── ❌ StartFire(pEntity, pAttacker, size, bAllowSpreading, burnTime, strength)
│   └── Starts fire on entity
│       Version 2: ped/vehicle catching fire
│
├── ❌ StartScriptFire(vecPos, pTarget, size, strength, bPropagate, duration)
│   └── Starts script-controlled fire
│       Persistent, doesn't go out by itself
│
├── ❌ FindNearestFire(vecPos, bCheckOnlyScript, bCheckOnlyNoScript)
│   └── Finds nearest fire
│       Used by firefighters
│
├── ❌ ExtinguishPoint(vecPos, radius)
│   └── Extinguishes fires in radius
│       Basic version
│
├── ❌ ExtinguishPointWithWater(vecPos, radius, waterAmount)
│   └── Extinguishes fires with water amount
│       Affects progressively
│
├── ❌ GetNextFreeFire(bScriptFire)
│   └── Returns next free slot
│
├── ❌ GetNumFiresInArea(x1, y1, z1, x2, y2, z2)
│   └── Counts fires in area
│
├── ❌ GetNumFiresInRange(vecPos, radius)
│   └── Counts fires in radius
│
├── ❌ PlentyFiresAvailable()
│   └── Checks if free slots available
│
├── ❌ RemoveScriptFire(scriptFireIndex)
│   └── Removes specific script fire
│
├── ❌ RemoveAllScriptFires()
│   └── Removes all script fires
│
├── ❌ IsScriptFireExtinguished(index)
│   └── Checks if script fire extinguished
│
└── ❌ Shutdown()
    └── Finalizes manager
```

---

## ANIMATION SYSTEM

```
CTaskSimpleUseGun
│
├── ❌ ProcessPed(pPed)
│   └── Processes ped using weapon
│       Updates IK, animations, state
│
├── ❌ ControlGun(pPed, pTarget, mode)
│   └── Controls ped's weapon
│       Manages aiming and firing
│
├── ❌ ControlGunMove(vecMove)
│   └── Controls movement with weapon
│       Adjusts speed and animations
│
├── ❌ AimGun(pPed)
│   └── Executes weapon aiming
│       Rotates ped and arms to target
│
├── ❌ FireGun(pPed, bLeftHand)
│   └── Executes fire
│       Calls CWeapon::Fire and plays anim
│
├── ❌ StartAnim(pPed)
│   └── Starts weapon animation
│       Selects anim based on type
│
├── ❌ ClearAnim(pPed)
│   └── Clears weapon animation
│
├── ❌ SetMoveAnim(pPed)
│   └── Sets movement animation
│       Walk/run with weapon
│
├── ❌ SetPedPosition(pPed)
│   └── Adjusts ped position
│       Compensation for animations
│
├── ❌ RemoveStanceAnims(pPed, blendDelta)
│   └── Removes weapon stance anims
│
├── ❌ RequirePistolWhip(pPed, pTarget)
│   └── Checks if should do melee
│       Pistol whip when too close
│
├── ❌ StartCountDown(time, bAutoFire)
│   └── Starts countdown to fire
│       Used for burst fire
│
├── ❌ FinishGunAnimCB(pAssoc, pData)
│   └── Callback when finishing gun anim
│
├── ❌ PlayerPassiveControlGun()
│   └── Passive weapon control
│       When player isn't aiming
│
├── ❌ AbortIK(pPed)
│   └── Aborts IK system
│       Resets bone rotations
│
├── ❌ Reset(pPed, pTarget, vecTarget, mode, time)
│   └── Resets task with new parameters
│
└── ❌ MakeAbortable(pPed, priority, pEvent)
    └── Allows aborting task
        Checks if can be interrupted
```

```
CTaskSimpleGunControl
│
├── ❌ ProcessPed(pPed)
│   └── Processes gun control
│       Higher level task than UseGun
│
├── ❌ MakeAbortable(pPed, priority, pEvent)
│   └── Allows aborting task
│
└── ❌ ChangeFiringTask(mode, pTarget, vecPos, vecDir)
    └── Changes firing task
        Switches between fire modes
```

```
CTaskManager
│
├── ✅ GetActiveTask()
│   └── Returns main active task
│       From primary tasks array
│
├── ✅ GetSimplestActiveTask()
│   └── Returns simplest active task
│       Goes down subtask hierarchy
│
└── ✅ FlushImmediately()
    └── Removes all tasks immediately
        Used when teleporting/respawning
```

```
CPedIK (Inverse Kinematics)
│
├── Fields:
│   ├── m_flags        → IK state flags
│   ├── m_torsoOrien   → Torso orientation
│   ├── m_headOrien    → Head orientation
│   └── ms_*Info       → Static limit info
│
├── ❌ RotateTorso(pFrame, orientation, bUpdatePos)
│   └── Rotates torso for aiming
│       Applies rotation to torso bone
│       Respects rotation limits
│
├── ❌ PitchForSlope()
│   └── Adjusts pitch for terrain
│       Body inclination on hills
│
└── ❌ GetWorldMatrix(pFrame, pMatrix)
    └── Gets bone world matrix
        Used for position calculations
```

---

## AUDIO SYSTEM

```
CAEWeaponAudioEntity
│
├── ❌ Initialise()
│   └── Initializes weapon audio entity
│       Configures sound slots
│
├── ❌ WeaponFire(weaponType, pEntity, slot)
│   └── Plays fire sound
│       Selects sound based on weapon
│       Considers distance and obstacles
│
├── ❌ WeaponReload(weaponType, pEntity, slot)
│   └── Plays reload sound
│
├── ❌ PlayGunSounds(pEntity, sfx1, sfx2, sfx3, sfx4, sfx5, slot, vol, freq, rolloff)
│   └── Plays multiple weapon sounds
│       Fire, echo, shell casing, etc
│
├── ❌ PlayCameraSound(pEntity, slot, volume)
│   └── Camera photo sound
│
├── ❌ PlayGoggleSound(sfx, slot)
│   └── Goggles sound (night/thermal vision)
│
├── ❌ PlayWeaponLoopSound(pEntity, sfx, slot, vol, freq, delay)
│   └── Continuous weapon sound
│       Minigun, flamethrower
│
├── ❌ PlayMiniGunStopSound(pEntity)
│   └── Sound when stopping minigun
│       Motor deceleration
│
├── ❌ PlayMiniGunFireSounds(pEntity, slot)
│   └── Minigun fire sounds
│
├── ❌ PlayChainsawStopSound(pEntity)
│   └── Sound when stopping chainsaw
│
├── ❌ PlayFlameThrowerSounds(pEntity, sfx1, sfx2, slot, vol, freq)
│   └── Flamethrower sounds
│
├── ❌ PlayFlameThrowerIdleGasLoop(pEntity)
│   └── Flamethrower idle gas sound
│
├── ❌ StopFlameThrowerIdleGasLoop()
│   └── Stops flamethrower idle sound
│
├── ❌ ReportStealthKill(weaponType, pEntity, slot)
│   └── Reports stealth kill for audio
│       Special assassination sound
│
├── ❌ ReportChainsawEvent(pEntity, slot)
│   └── Reports chainsaw event
│       Cutting sounds
│
├── ❌ UpdateParameters(pSound, slot)
│   └── Updates sound parameters
│       Volume, position, etc
│
├── ❌ Reset()
│   └── Resets audio state
│
└── ❌ Terminate()
    └── Finalizes audio entity
```

---

## COLLISION SYSTEM

```
CCollision
│
├── ✅ ProcessLineOfSight(line, matrix, model, colPoint, range, ...)
│   └── Processes line of sight against model
│       Complete raycast with collision result
│       Used by weapons to check hits
│
├── ⚠️ ProcessLineSphere(line, sphere, colPoint, range)
│   └── Tests line against sphere
│       Simplified collision
│
├── ⚠️ ProcessLineBox(line, box, colPoint, range)
│   └── Tests line against box
│       For rectangular objects
│
├── ⚠️ ProcessLineTriangle(line, verts, triangle, plane, colPoint, range, ...)
│   └── Tests line against triangle
│       Lowest level collision
│
├── ⚠️ ProcessColModels(mat1, model1, mat2, model2, spheres, colPoints, depths, ...)
│   └── Processes collision between two models
│       Returns contact points
│
└── ⚠️ ProcessVerticalLine(line, matrix, model, colPoint, range, ...)
    └── Optimized vertical line
        For checking ground
```

```
CColPoint (Result structure)
│
├── Fields:
│   ├── m_vecPoint      → Collision point
│   ├── m_vecNormal     → Surface normal
│   ├── m_nSurfaceTypeA → Surface type A
│   ├── m_nSurfaceTypeB → Surface type B
│   ├── m_fDepth        → Penetration depth
│   └── m_nPieceTypeA/B → Hit piece type
│
└── Used for:
    - Determining where bullet hit
    - Calculating ricochet angle
    - Identifying material for effects
    - Applying damage to correct part
```

---

## Final Summary

```
APK IMPLEMENTATION:
├── ✅ Total Implemented:        ~32 functions/classes
├── ⚠️ Total Partial (hooks):    ~27 functions/classes  
├── ❌ Total Not Implemented:    ~104 functions/classes
│
└── Coverage: ~20% of complete system
    
    Project focuses on:
    1. Model streaming (to load weapons)
    2. Data structures (CWeapon, slots)
    3. High-level interface (CPlayerPed)
    4. Synchronization system (BULLET_SYNC)
    
    The rest uses direct calls to libGTASA via hooks.
```
