# 🔫 Sistema de Armas GTA SA - Tabela Completa de Offsets

**Legenda:**
- ✅ = Implementado no APK
- ⚠️ = Parcialmente implementado (via hook)
- ❌ = Não implementado

> **Nota:** Offsets 32-bit precisam de `+1` para ARM Thumb mode

---

## 📦 CStreaming (Sistema de Streaming)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ✅ | `RequestModel` | `0x2D299D` | `0x3924BC` | Adiciona modelo à fila de requisições |
| ✅ | `LoadAllRequestedModels` | `0x2D46A9` | `0x396ABC` | Carrega todos os modelos pendentes |
| ✅ | `FlushChannels` | `0x2D4879` | `0x396D1C` | Finaliza carregamento dos canais |
| ✅ | `RemoveModel` | `0x2D0129` | `0x391FF0` | Remove modelo da memória |
| ✅ | `MakeSpaceFor` | `0x2D39E5` | `0x396008` | Libera memória para novo modelo |
| ⚠️ | `LoadInitialWeapons` | `0x2D6BEF` | `0x399654` | Carrega armas na inicialização |
| ✅ | `SetModelIsDeletable` | `0x2D6788` | `0x399090` | Marca modelo como deletável |
| ✅ | `InitImageList` | `0x2CF681` | `0x391320` | Inicializa lista de arquivos IMG |
| ✅ | `AddImageToList` | `0x2CF7D1` | `0x391410` | Adiciona IMG à lista |
| ✅ | `GetNextFileOnCd` | `0x2D3A39` | `0x396080` | Obtém próximo arquivo para carregar |
| ✅ | `ProcessLoadingChannel` | `0x2D2411` | `0x394298` | Processa canal de streaming |
| ✅ | `RequestModelStream` | `0x2D3FBD` | `0x396628` | Inicia leitura de modelos |
| ✅ | `RetryLoadFile` | `0x2D2315` | `0x394220` | Retenta carregamento após erro |
| ✅ | `PurgeRequestList` | `0x2D21F9` | `0x394080` | Limpa lista de requisições |
| ✅ | `ConvertBufferToObject` | `0x2D2FD0` | `0x395114` | Converte buffer em objeto RW |
| ✅ | `FinishLoadingLargeFile` | `0x2D36B0` | `0x395948` | Finaliza modelo grande |
| ❌ | `LoadCdDirectory` | `0x46C0AD` | `0x5578FC` | Carrega diretório do CD |
| ❌ | `StreamZoneModels` | `0x2D193D` | `0x393608` | Stream de modelos por zona |
| ❌ | `UpdateMemoryUsed` | `0x2D0555` | `0x392474` | Atualiza contador de memória |

### Variáveis Globais CStreaming

| Nome | 32-bit | 64-bit | Descrição |
|------|:------:|:------:|-----------|
| `ms_aInfoForModel` | `0x712338` | `0x8F21C0` | Array de CStreamingInfo |
| `ms_memoryUsed` | `0x792B7C` | `0x972BD8` | Memória usada |
| `ms_memoryAvailable` | `0x685FA0` | `0x850408` | Memória disponível |
| `ms_channel` | `0x792C40` | `0x972C80` | Canais de streaming |
| `ms_numModelsRequested` | `0x792B78` | `0x9729D8` | Nº de modelos requisitados |
| `ms_bLoadingBigModel` | `0x792D7C` | `0x972A98` | Flag de modelo grande |
| `ms_pStreamingBuffer` | `0x792D74` | `0x972BD8` | Buffer de streaming |

---

## 🔫 CWeapon (Arma)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ⚠️ | `Fire` | `0x5DB9F1` | `0x700C64` | Dispara arma (2096 bytes) |
| ⚠️ | `FireInstantHit` | `0x5DC179` | `0x701494` | Disparo hitscan (5772 bytes) |
| ⚠️ | `FireSniper` | `0x5DD741` | `0x702B20` | Disparo sniper (820 bytes) |
| ❌ | `FireProjectile` | `0x5DDECD` | `0x703274` | Disparo de projétil (1976 bytes) |
| ❌ | `FireFromCar` | `0x5DEFA1` | `0x7042D4` | Disparo de veículo (324 bytes) |
| ❌ | `FireAreaEffect` | `0x5DE6A9` | `0x703A2C` | Lança-chamas/spray (852 bytes) |
| ❌ | `FireM16_1stPerson` | `0x5DDA79` | `0x702E54` | M16 primeira pessoa (1056 bytes) |
| ❌ | `FireInstantHitFromCar` | `0x5DF099` | `0x704418` | Drive-by (1392 bytes) |
| ❌ | `FireInstantHitFromCar2` | `0x5E19F5` | `0x707098` | Drive-by v2 (492 bytes) |
| ❌ | `Reload` | `0x5DB853` | `0x700A2C` | Recarga arma (96 bytes) |
| ❌ | `Update` | `0x5E1F19` | `0x707730` | Atualiza estado (1256 bytes) |
| ❌ | `Initialise` | `0x5DB901` | `0x700B10` | Inicializa arma (224 bytes) |
| ❌ | `Shutdown` | `0x5DB99D` | `0x700BF0` | Finaliza arma (116 bytes) |
| ❌ | `InitialiseWeapons` | `0x5DB889` | `0x700A8C` | Inicializa sistema (60 bytes) |
| ❌ | `ShutdownWeapons` | `0x5DB8BD` | `0x700AC8` | Finaliza sistema (44 bytes) |
| ❌ | `UpdateWeapons` | `0x5DB8E9` | `0x700AF4` | Atualiza todas (28 bytes) |
| ❌ | `StaticUpdate` | `0x5E2345` | `0x707C50` | Update estático (76 bytes) |
| ❌ | `DoBulletImpact` | `0x5E07D9` | `0x705C7C` | Processa impacto (3084 bytes) |
| ❌ | `GenerateDamageEvent` | `0x5E1395` | `0x706888` | Gera evento de dano (896 bytes) |
| ❌ | `ProcessLineOfSight` | `0x5DF749` | `0x704988` | Raycast (172 bytes) |
| ❌ | `DoDoomAiming` | `0x5DFF25` | `0x705290` | Auto-aim (1180 bytes) |
| ❌ | `DoTankDoomAiming` | `0x5E1B4D` | `0x707284` | Auto-aim tanque (1196 bytes) |
| ❌ | `DoDriveByAutoAiming` | `0x5DFB81` | `0x704EEC` | Auto-aim drive-by (932 bytes) |
| ❌ | `LaserScopeDot` | `0x5E1871` | `0x706EBC` | Ponto laser (476 bytes) |
| ❌ | `PickTargetForHeatSeekingMissile` | `0x5E161D` | `0x706C08` | Alvo míssil (340 bytes) |
| ❌ | `EvaluateTargetForHeatSeekingMissile` | `0x5E3029` | `0x708AA8` | Avalia alvo (304 bytes) |
| ❌ | `FindNearestTargetEntityWithScreenCoors` | `0x5E2C75` | `0x7086D0` | Alvo na tela (984 bytes) |
| ❌ | `TargetWeaponRangeMultiplier` | `0x5DF799` | `0x704A34` | Mult. alcance (228 bytes) |
| ❌ | `CheckForShootingVehicleOccupant` | `0x5DF845` | `0x704B18` | Verifica ocupante (980 bytes) |
| ❌ | `SetUpPelletCol` | `0x5E0431` | `0x7058D4` | Config. pellets (936 bytes) |
| ❌ | `AddGunshell` | `0x5E02B9` | `0x70572C` | Ejeta cartucho (424 bytes) |
| ❌ | `DoWeaponEffect` | `0x5E1755` | `0x706D5C` | Efeito visual (352 bytes) |
| ❌ | `StopWeaponEffect` | `0x5E2325` | `0x707C18` | Para efeito (56 bytes) |
| ❌ | `TakePhotograph` | `0x5DEA19` | `0x703D80` | Tira foto (1364 bytes) |
| ❌ | `IsTypeMelee` | `0x5E25D9` | `0x707F34` | É corpo-a-corpo (40 bytes) |
| ❌ | `IsType2Handed` | `0x5E25F1` | `0x707F5C` | Usa 2 mãos (44 bytes) |
| ❌ | `IsTypeProjectile` | `0x5E260F` | `0x707F88` | É projétil (48 bytes) |
| ❌ | `HasWeaponAmmoToBeUsed` | `0x5E2C49` | `0x70869C` | Tem munição (52 bytes) |
| ❌ | `CanBeUsedFor2Player` | `0x5E2631` | `0x707FB8` | Para 2 jogadores (52 bytes) |

### Variáveis Globais CWeapon

| Nome | 32-bit | 64-bit |
|------|:------:|:------:|
| `ms_bTakePhoto` | `0xA861D1` | `0xD2B028` |
| `ms_bPhotographHasBeenTaken` | `0xA861D0` | `0xD2B028` |
| `ms_PelletTestCol` | `0xA861D8` | `0xD2B030` |
| `m_nTakePhotoFrames` | `0xA861D4` | `0xD2B02C` |
| `ms_fExtinguisherAimAngle` | `0x6B2C60` | `0x88F690` |

---

## 📋 CWeaponInfo (Dados de Armas)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `Initialise` | `0x4746B9` | `0x560E20` | Inicializa sistema (136 bytes) |
| ⚠️ | `GetWeaponInfo` | `0x5E42E9` | `0x709BA8` | Obtém info da arma (84 bytes) |
| ❌ | `LoadWeaponData` | `0x474751` | `0x560EA8` | Carrega weapon.dat (1812 bytes) |
| ❌ | `FindWeaponType` | `0x5E4335` | `0x709C24` | Busca tipo por nome (84 bytes) |
| ❌ | `FindWeaponFireType` | `0x474DB9` | `0x5615BC` | Busca tipo de tiro (184 bytes) |
| ❌ | `GetSkillStatIndex` | `0x5E4319` | `0x709BFC` | Índice de skill (40 bytes) |
| ❌ | `GetTargetHeadRange` | `0x5E4369` | `0x709C78` | Alcance headshot (36 bytes) |
| ❌ | `GetWeaponReloadTime` | `0x5E4391` | `0x709C9C` | Tempo de recarga (156 bytes) |
| ❌ | `Shutdown` | `0x5E42E5` | `0x709BA4` | Finaliza (4 bytes) |

### Variáveis Globais CWeaponInfo

| Nome | 32-bit | 64-bit |
|------|:------:|:------:|
| `ms_aWeaponNames` | `0x6B2C98` | `0x88F6C8` |
| `ms_aWeaponAimOffsets` | `0xA88588` | `0xD2D3FC` |
| `aWeaponInfo` | - | `0xD2B0FC` |

---

## 👤 CPed (Armas no Ped)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ✅ | `GiveWeapon` | `0x49F589` | `0x59525C` | Dá arma ao ped (420 bytes) |
| ⚠️ | `ClearWeapons` | `0x49F837` | `0x595604` | Remove todas armas |
| ❌ | `ClearWeapon` | `0x4A52D9` | `0x59B998` | Remove arma específica (160 bytes) |
| ⚠️ | `AddWeaponModel` | `0x4A4CE9` | `0x59B174` | Anexa modelo 3D (320 bytes) |
| ⚠️ | `RemoveWeaponModel` | `0x4A4DF5` | `0x59B2B4` | Remove modelo 3D (216 bytes) |
| ⚠️ | `SetCurrentWeapon(type)` | `0x4A521D` | `0x59B86C` | Troca arma por tipo (52 bytes) |
| ⚠️ | `SetCurrentWeapon(slot)` | `0x4A5189` | `0x59B8A0` | Troca arma por slot |
| ❌ | `GetWeaponSlot` | `0x4A5179` | `0x59B78C` | Retorna slot da arma (36 bytes) |
| ❌ | `GetWeaponSkill(type)` | `0x4A5653` | `0x59BE58` | Skill por tipo (204 bytes) |
| ❌ | `GetWeaponSkill()` | `0x4A12E3` | `0x597464` | Skill atual |
| ⚠️ | `SetWeaponSkill` | `0x4A56E7` | `0x59BF24` | Define skill (20 bytes) |
| ❌ | `SetWeaponLockOnTarget` | `0x4A8345` | `0x59F3EC` | Define lock-on (216 bytes) |
| ❌ | `DoWeHaveWeaponAvailable` | `0x4A533F` | `0x59B9C8` | Verifica disponibilidade |
| ❌ | `GiveDelayedWeapon` | `0x4A5097` | `0x59B6BC` | Dá arma de reserva (208 bytes) |
| ❌ | `RequestDelayedWeapon` | `0x4A2D49` | `0x598670` | Saca arma de reserva |
| ✅ | `SetAmmo` | `0x4A5283` | `0x59B918` | Define munição (128 bytes) |
| ❌ | `GrantAmmo` | `0x4A5235` | `0x59B8A0` | Adiciona munição (120 bytes) |
| ❌ | `DoGunFlash` | `0x4A54DD` | `0x59BCB0` | Flash do tiro (168 bytes) |
| ❌ | `SetGunFlashAlpha` | `0x4A5575` | `0x59BD68` | Alpha do flash |
| ❌ | `ResetGunFlashAlpha` | `0x4A562D` | `0x59BE2C` | Reseta flash (44 bytes) |
| ❌ | `RemoveWeaponAnims` | `0x4ADE25` | `0x5A6E7C` | Remove anims (156 bytes) |
| ❌ | `RemoveWeaponWhenEnteringVehicle` | `0x4A536D` | `0x59BA48` | Remove ao entrar |
| ❌ | `ReplaceWeaponWhenExitingVehicle` | `0x4A5443` | `0x59BBC4` | Restaura ao sair (144 bytes) |
| ❌ | `RemoveWeaponForScriptedCutscene` | `0x4A549F` | `0x59BC54` | Remove p/ cutscene (24 bytes) |
| ❌ | `ReplaceWeaponForScriptedCutscene` | `0x4A54B7` | `0x59BC6C` | Restaura após cutscene (68 bytes) |
| ❌ | `CreateDeadPedWeaponPickups` | `0x320F71` | `0x3E8E20` | Cria pickups ao morrer (44 bytes) |
| ❌ | `GetBonePosition` | `0x4A4B0D` | `0x59AEE4` | Posição do bone (228 bytes) |
| ❌ | `AttachPedToEntity` | `0x4A78BD` | `0x59E894` | Anexa ped |
| ❌ | `AttachPedToBike` | `0x4A7CB5` | `0x59EAEC` | Anexa a moto (68 bytes) |

---

## 🎯 CWeaponEffects (Efeitos de Mira)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `Init` | `0x5E3159` | `0x708BE0` | Inicializa (208 bytes) |
| ❌ | `Render` | `0x5E3391` | `0x708DF0` | Renderiza (3508 bytes) |
| ❌ | `Shutdown` | `0x5E3239` | `0x708CB0` | Finaliza (76 bytes) |
| ❌ | `MarkTarget` | `0x5E3295` | `0x708D18` | Marca alvo (68 bytes) |
| ❌ | `IsLockedOn` | `0x5E3275` | `0x708CFC` | Verifica lock-on (28 bytes) |
| ❌ | `ClearCrossHair` | `0x5E32F9` | `0x708D5C` | Limpa mira (40 bytes) |
| ❌ | `ClearCrossHairs` | `0x5E331D` | `0x708D84` | Limpa todas (20 bytes) |
| ❌ | `ClearCrossHairImmediately` | `0x5E3331` | `0x708D98` | Limpa imediato (44 bytes) |
| ❌ | `ClearAllCrosshairs` | `0x5E3369` | `0x708DC4` | Limpa tudo (44 bytes) |

---

## 💥 CExplosion (Explosões)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `Initialise` | `0x5D78C1` | `0x6FCE60` | Inicializa pool |
| ❌ | `AddExplosion` | `0x5D7A1D` | `0x6FCFAC` | Cria explosão (3292 bytes) |
| ❌ | `Update` | `0x5D89DD` | `0x6FDC88` | Atualiza (1708 bytes) |
| ❌ | `GetExplosionType` | `0x5D79F5` | - | Tipo da explosão |
| ❌ | `GetExplosionPosition` | `0x5D7A09` | - | Posição |
| ❌ | `GetExplosionActiveCounter` | `0x5D79AD` | - | Contador ativo |
| ❌ | `ResetExplosionActiveCounter` | `0x5D79C5` | - | Reseta contador |
| ❌ | `DoesExplosionMakeSound` | `0x5D79DD` | - | Faz som? |
| ❌ | `TestForExplosionInArea` | `0x5D90E5` | - | Testa área |
| ❌ | `RemoveAllExplosionsInArea` | `0x5D917D` | - | Remove em área |
| ❌ | `ClearAllExplosions` | `0x5D7935` | - | Limpa todas |
| ❌ | `Shutdown` | `0x5D79A9` | - | Finaliza |

---

## 💣 CProjectileInfo (Projéteis)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `Initialise` | `0x5D9205` | `0x6FF5C0` | Inicializa pool |
| ❌ | `AddProjectile` | `0x5D9305` | `0x6FF6B0` | Cria projétil |
| ❌ | `Update` | `0x5D9F39` | `0x700210` | Atualiza todos |
| ❌ | `RemoveProjectile` | `0x5D9D19` | `0x6FEFC8` | Remove (480 bytes) |
| ❌ | `RemoveAllProjectiles` | `0x5DAEAD` | `0x700068` | Remove todos (212 bytes) |
| ❌ | `RemoveDetonatorProjectiles` | `0x5D9C55` | - | Remove detonadores |
| ❌ | `RemoveNotAdd` | `0x5D9EB5` | - | Remove sem adicionar |
| ❌ | `IsProjectileInRange` | `0x5DAD49` | `0x6FFEF0` | Verifica alcance (376 bytes) |
| ❌ | `RemoveFXSystem` | `0x5D9275` | - | Remove efeitos |
| ❌ | `RemoveIfThisIsAProjectile` | `0x5DAF69` | `0x70013C` | Verifica e remove (208 bytes) |
| ❌ | `GetProjectileInfo` | `0x5D92F1` | - | Obtém info |
| ❌ | `Shutdown` | `0x5D92A5` | - | Finaliza |

---

## 💨 CBulletInfo / CBulletTraces (Balas)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CBulletInfo::Initialise` | `0x5D6F81` | `0x6FC4A0` | Inicializa |
| ❌ | `CBulletInfo::AddBullet` | `0x5D6FED` | `0x6FC55C` | Adiciona bala (220 bytes) |
| ❌ | `CBulletInfo::Update` | `0x5D7095` | `0x6FC630` | Atualiza |
| ❌ | `CBulletInfo::Shutdown` | `0x5D6FE9` | - | Finaliza |
| ❌ | `CBulletTraces::Init` | `0x5BFAD9` | `0x6E4154` | Inicializa rastros (76 bytes) |
| ❌ | `CBulletTraces::AddTrace` | `0x5C1DF9` | `0x6E5E80` | Adiciona rastro |
| ❌ | `CBulletTraces::Update` | `0x5C06B1` | `0x6E4820` | Atualiza rastros |
| ❌ | `CBulletTraces::Render` | `0x5C0C55` | `0x6E4D60` | Renderiza |
| ❌ | `CBulletTrace::Update` | `0x5C22FD` | - | Atualiza único |

---

## 🔥 CFireManager / CFire (Fogo)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CFireManager::Init` | `0x3F11AD` | `0x4D30CC` | Inicializa |
| ❌ | `CFireManager::Update` | `0x3F1629` | `0x4D361C` | Atualiza (1208 bytes) |
| ❌ | `CFireManager::Shutdown` | `0x3F1203` | `0x4D313C` | Finaliza |
| ❌ | `CFireManager::StartFire` | `0x3F2619` | `0x4D4550` | Inicia fogo |
| ❌ | `CFireManager::StartScriptFire` | `0x3F30F9` | `0x4D5268` | Fogo de script (896 bytes) |
| ❌ | `CFireManager::FindNearestFire` | `0x3F27D5` | `0x4D4780` | Fogo mais próximo |
| ❌ | `CFireManager::ExtinguishPoint` | `0x3F2CA5` | `0x4D4D8C` | Apaga ponto (244 bytes) |
| ❌ | `CFireManager::ExtinguishPointWithWater` | `0x3F2D75` | `0x4D4E80` | Apaga com água (1000 bytes) |
| ❌ | `CFireManager::GetNextFreeFire` | `0x3F12E3` | - | Próximo slot livre |
| ❌ | `CFireManager::GetNumFiresInArea` | `0x3F3599` | - | Conta em área |
| ❌ | `CFireManager::GetNumFiresInRange` | `0x3F354D` | `0x4D57E8` | Conta em raio (84 bytes) |
| ❌ | `CFireManager::PlentyFiresAvailable` | `0x3F2B8D` | - | Slots disponíveis? |
| ❌ | `CFireManager::RemoveScriptFire` | `0x3F3411` | `0x4D5604` | Remove de script (192 bytes) |
| ❌ | `CFireManager::RemoveAllScriptFires` | `0x3F348B` | - | Remove todos |
| ❌ | `CFireManager::IsScriptFireExtinguished` | `0x3F3401` | `0x4D55E8` | Script apagou? (28 bytes) |
| ❌ | `CFire::Initialise` | `0x3F10E1` | `0x4D2FE8` | Inicializa (64 bytes) |
| ❌ | `CFire::ProcessFire` | `0x3F1B09` | `0x4D3B50` | Processa |
| ❌ | `CFire::Extinguish` | `0x3F1285` | `0x4D30E0` | Apaga |
| ❌ | `CFire::CreateFxSysForStrength` | `0x3F2869` | - | Cria FX |

---

## 🔊 CAEWeaponAudioEntity (Áudio)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `Initialise` | `0x3B9EB1` | `0x497B00` | Inicializa (68 bytes) |
| ❌ | `WeaponFire` | `0x3B9F35` | `0x497BA4` | Som de tiro (756 bytes) |
| ❌ | `WeaponReload` | `0x3BAEED` | `0x498D90` | Som de recarga (424 bytes) |
| ❌ | `PlayGunSounds` | `0x3BA20D` | `0x497E98` | Sons diversos (1816 bytes) |
| ❌ | `PlayCameraSound` | `0x3BAB3D` | `0x498908` | Som câmera (292 bytes) |
| ❌ | `PlayGoggleSound` | `0x3BAD95` | `0x498BF0` | Som óculos (416 bytes) |
| ❌ | `PlayWeaponLoopSound` | `0x3BAA4D` | `0x4987C8` | Som contínuo (320 bytes) |
| ❌ | `PlayMiniGunStopSound` | `0x3B9CC1` | `0x4978B0` | Para minigun (352 bytes) |
| ❌ | `PlayMiniGunFireSounds` | `0x3BAC29` | `0x498A2C` | Tiro minigun (452 bytes) |
| ❌ | `PlayChainsawStopSound` | `0x3B9DE9` | `0x497A10` | Para motosserra (240 bytes) |
| ❌ | `PlayFlameThrowerSounds` | `0x3BA88D` | `0x4985B0` | Sons lança-chamas (536 bytes) |
| ❌ | `PlayFlameThrowerIdleGasLoop` | `0x3BB04D` | `0x498F38` | Idle lança-chamas (184 bytes) |
| ❌ | `StopFlameThrowerIdleGasLoop` | `0x3BB0ED` | `0x498FF0` | Para idle (44 bytes) |
| ❌ | `ReportStealthKill` | `0x3BB315` | `0x49928C` | Kill furtivo (532 bytes) |
| ❌ | `ReportChainsawEvent` | `0x3BB109` | `0x49901C` | Evento motosserra (624 bytes) |
| ❌ | `UpdateParameters` | `0x3B99BD` | `0x497534` | Atualiza params (892 bytes) |
| ❌ | `Reset` | `0x3B9EFD` | `0x497B60` | Reseta (68 bytes) |
| ❌ | `Terminate` | `0x3B9EE9` | `0x497B44` | Finaliza (28 bytes) |

---

## 🎮 CPlayerPed (Funções de Arma)

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `ProcessPlayerWeapon` | `0x4C682F` | `0x5C5890` | Processa arma |
| ❌ | `ProcessWeaponSwitch` | `0x4C5919` | `0x5C4990` | Troca de arma |
| ❌ | `ClearWeaponTarget` | `0x4C58E5` | `0x5C4960` | Limpa alvo |
| ❌ | `FindWeaponLockOnTarget` | `0x4C6D65` | `0x5C5D70` | Busca alvo |
| ❌ | `FindNextWeaponLockOnTarget` | `0x4C7D4D` | `0x5C6D80` | Próximo alvo |
| ❌ | `DoesPlayerWantNewWeapon` | `0x4C6779` | `0x5C5780` | Quer nova arma? |
| ❌ | `MakeChangesForNewWeapon(type)` | `0x4C68F1` | `0x5C5900` | Aplica mudança |
| ❌ | `MakeChangesForNewWeapon(slot)` | `0x4C6831` | `0x5C58A0` | Aplica por slot |
| ❌ | `UpdateCameraWeaponModes` | `0x4C6875` | `0x5C58E0` | Atualiza câmera |
| ❌ | `GetWeaponRadiusOnScreen` | `0x4C69E9` | `0x5C5A00` | Raio na tela |
| ❌ | `CanIKReachThisTarget` | `0x4C7321` | `0x5C4F90` | IK alcança? (144 bytes) |
| ❌ | `DoesTargetHaveToBeBroken` | `0x4C8EFD` | `0x5C6D9C` | Quebrar lock? (384 bytes) |
| ❌ | `PickWeaponAllowedFor2Player` | `0x4C6845` | - | Arma para 2P |

---

## 📊 Resumo de Cobertura

| Categoria | Total | ✅ | ⚠️ | ❌ |
|-----------|:-----:|:--:|:--:|:--:|
| CStreaming | 19 | 14 | 1 | 4 |
| CWeapon | 38 | 0 | 3 | 35 |
| CWeaponInfo | 9 | 0 | 1 | 8 |
| CPed | 26 | 2 | 5 | 19 |
| CWeaponEffects | 9 | 0 | 0 | 9 |
| CExplosion | 12 | 0 | 0 | 12 |
| CProjectileInfo | 12 | 0 | 0 | 12 |
| CBullet* | 9 | 0 | 0 | 9 |
| CFire* | 19 | 0 | 0 | 19 |
| CAEWeaponAudio | 18 | 0 | 0 | 18 |
| CPlayerPed | 13 | 0 | 0 | 13 |
| **TOTAL** | **184** | **16** | **10** | **158** |

---

## 💡 Como Usar os Offsets

### 32-bit (ARM Thumb)
```cpp
// Adicionar +1 para Thumb mode
auto func = (void(*)(...))(g_libGTASA + 0x49F588 + 1);
func(...);

// Ou usando CHook
CHook::CallFunction<void>(g_libGTASA + 0x49F588 + 1, args...);
```

### 64-bit (ARM64)
```cpp
// Sem modificação
auto func = (void(*)(...))(g_libGTASA + 0x59525C);
func(...);

// Ou usando CHook
CHook::CallFunction<void>(g_libGTASA + 0x59525C, args...);
```

### Macro de Versão
```cpp
#define WEAPON_FIRE_OFFSET (VER_x32 ? 0x5DB9F1 + 1 : 0x700C64)
```
