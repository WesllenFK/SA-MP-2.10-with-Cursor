# 🌳 Sistema de Armas GTA SA - Árvore Detalhada de Funções

**Legenda:**
- ✅ = Implementado no APK
- ⚠️ = Parcialmente implementado (via hook/offset)
- ❌ = Não implementado (apenas na libGTASA nativa)

---

## 📦 SISTEMA DE RECURSOS (Resource System)

```
CStreaming
│
├── ✅ RequestModel(modelId, flags)
│   └── Adiciona um modelo à fila de requisições do streaming
│       para ser carregado do arquivo IMG
│
├── ✅ LoadAllRequestedModels(bPriorityOnly)
│   └── Carrega todos os modelos pendentes na fila de requisições
│       Processa os canais de streaming até esvaziar a fila
│
├── ✅ TryLoadModel(modelId)
│   └── Tenta carregar um modelo de forma síncrona
│       Aguarda até 3 segundos pelo carregamento completo
│       Retorna true se carregou, false se timeout
│
├── ⚠️ LoadInitialWeapons()
│   └── Carrega os modelos de armas básicas na inicialização
│       Chamado durante o boot do jogo
│
├── ✅ RemoveModel(modelId)
│   └── Remove um modelo da memória
│       Libera recursos RW (RenderWare) associados
│       Atualiza contadores de memória usada
│
├── ✅ GetInfo(modelId)
│   └── Retorna CStreamingInfo do modelo
│       Contém estado de carregamento, flags, posição no CD
│
└── ✅ SetModelIsDeletable(modelId)
    └── Marca modelo como deletável pelo sistema de streaming
        Permite que seja removido quando memória estiver baixa
```

```
CWeaponModelInfo (herda de CClumpModelInfo)
│
├── ❌ Init()
│   └── Inicializa a estrutura de informações do modelo de arma
│       Configura valores padrão e aloca recursos
│
├── ❌ SetClump(RpClump*)
│   └── Define o modelo 3D (clump) para esta arma
│       Associa geometria RenderWare ao modelo
│
└── ❌ GetModelType()
    └── Retorna MODEL_INFO_WEAPON (tipo 8)
        Usado para identificar que é um modelo de arma
```

```
CModelInfo
│
├── ✅ GetModelInfo(index)
│   └── Retorna ponteiro CBaseModelInfo para o modelo
│       Acessa array global ms_modelInfoPtrs[20000]
│
├── ⚠️ AddWeaponModel(index)
│   └── Cria e registra um novo CWeaponModelInfo
│       Adiciona ao store de modelos de armas (51 slots)
│
└── ✅ ms_modelInfoPtrs[20000]
    └── Array global com ponteiros para todos os modelos
        Indexado pelo Model ID
```

---

## 🔫 SISTEMA DE ARMA CENTRAL (Core Weapon System)

```
CWeapon (Estrutura de 28/32 bytes)
│
├── Campos:
│   ├── dwType (eWeaponType)     → Tipo da arma (0-46)
│   ├── dwState (eWeaponState)   → Estado: pronto, recarregando, etc
│   ├── dwAmmoInClip             → Munição no pente atual
│   ├── dwAmmo                   → Munição total disponível
│   ├── m_nTimer                 → Timer para cadência de tiro
│   └── m_pWeaponFxSys           → Sistema de efeitos visuais
│
├── ⚠️ Fire(pEntity, origin, muzzle, target, ...)
│   └── Função principal de disparo
│       Decide qual tipo de tiro executar:
│       - FireInstantHit para balas
│       - FireProjectile para foguetes/granadas
│       - FireAreaEffect para lança-chamas
│
├── ⚠️ FireInstantHit(pFiringEntity, vecOrigin, vecMuzzle, ...)
│   └── Dispara bala instantânea (hitscan)
│       Faz raycast do cano até alcance máximo
│       Calcula dano e aplica ao alvo atingido
│       Cria efeitos visuais (tracer, impacto)
│
├── ⚠️ FireSniper(pPed, pTargetEntity, vecOrigin)
│   └── Disparo especializado para rifles sniper
│       Usa mira telescópica e zoom
│       Maior precisão e dano à distância
│
├── ❌ FireProjectile(pEntity, vecOrigin, pTarget, vecTarget, speed)
│   └── Dispara projétil físico (foguete, granada)
│       Cria entidade CProjectile no mundo
│       Projétil tem física própria e pode ser rastreado
│
├── ❌ FireFromCar(pVehicle, bLeftHand, bRightHand)
│   └── Disparo de dentro de veículo
│       Ajusta ângulos para drive-by
│       Limita tipos de armas permitidas
│
├── ❌ FireAreaEffect(pEntity, vecOrigin, pTarget, vecTarget)
│   └── Disparo de área (lança-chamas, spray)
│       Cria cone de efeito à frente do jogador
│       Aplica dano/efeito contínuo na área
│
├── ❌ Reload(pPed)
│   └── Recarrega a arma
│       Move munição do estoque para o pente
│       Toca animação de recarga
│
├── ❌ Update(pPed)
│   └── Atualização por frame da arma
│       Gerencia cooldowns, efeitos contínuos
│       Atualiza estado da arma
│
├── ❌ Initialise(weaponType, ammo, pOwner)
│   └── Inicializa instância de arma
│       Define tipo, munição inicial, dono
│
├── ❌ Shutdown()
│   └── Finaliza e limpa recursos da arma
│       Para efeitos visuais, libera memória
│
├── ❌ AddGunshell(pEntity, vecPos, vecDir, size)
│   └── Cria cápsula ejetada do cartucho
│       Efeito visual de casquilho voando
│
├── ❌ DoBulletImpact(pShooter, pVictim, vecStart, vecEnd, colPoint, dmg)
│   └── Processa impacto de bala
│       Aplica dano, cria efeitos de sangue/faíscas
│       Gera evento de dano para a vítima
│
├── ❌ GenerateDamageEvent(pVictim, pAttacker, weaponType, damage, ...)
│   └── Gera evento de dano no sistema de eventos
│       Notifica IA, estatísticas, scripts
│
├── ❌ ProcessLineOfSight(start, end, colPoint, entity, ...)
│   └── Faz raycast para verificar linha de visão
│       Usado para determinar se tiro atinge alvo
│       Considera paredes, veículos, peds
│
├── ❌ DoDoomAiming(pTarget, vecStart, vecEnd)
│   └── Sistema de mira assistida (auto-aim)
│       Ajusta direção do tiro para o alvo mais próximo
│       "Doom aiming" = estilo clássico de FPS
│
├── ❌ DoTankDoomAiming(pShooter, pTarget, vecStart, vecEnd)
│   └── Mira assistida para canhão de tanque
│       Considera balística e distância
│
├── ❌ DoDriveByAutoAiming(pTarget, pVehicle, vecStart, vecEnd, bPassenger)
│   └── Mira assistida para drive-by
│       Compensa movimento do veículo
│       Ajusta para posição do passageiro
│
├── ❌ LaserScopeDot(vecOut, pRange)
│   └── Calcula posição do ponto laser na mira
│       Usado por rifles com mira laser
│
├── ❌ PickTargetForHeatSeekingMissile(pos, dir, range, pShooter, ...)
│   └── Seleciona alvo para míssil teleguiado
│       Prioriza veículos aéreos com motor ligado
│
├── ❌ FindNearestTargetEntityWithScreenCoors(x, y, z, pos, ...)
│   └── Encontra entidade mais próxima das coordenadas de tela
│       Usado para lock-on em terceira pessoa
│
├── ❌ IsTypeMelee()
│   └── Retorna true se arma é corpo-a-corpo
│       (faca, taco, soco, etc)
│
├── ❌ IsType2Handed()
│   └── Retorna true se arma usa duas mãos
│       (rifles, shotguns, etc)
│
├── ❌ IsTypeProjectile()
│   └── Retorna true se arma dispara projéteis
│       (foguetes, granadas, etc)
│
├── ❌ HasWeaponAmmoToBeUsed()
│   └── Verifica se tem munição disponível
│
├── ❌ CanBeUsedFor2Player()
│   └── Verifica se arma pode ser usada em co-op
│
├── ❌ DoWeaponEffect(vecStart, vecEnd)
│   └── Cria efeito visual do tiro (tracer, flash)
│
├── ❌ StopWeaponEffect()
│   └── Para efeitos contínuos (lança-chamas, minigun)
│
├── ❌ TakePhotograph(pTarget, vecPos)
│   └── Função especial da câmera fotográfica
│       Captura "foto" do alvo para missões
│
├── ❌ CheckForShootingVehicleOccupant(ppEntity, colPoint, ...)
│   └── Verifica se tiro atingiu ocupante de veículo
│       Permite matar motorista através do vidro
│
├── ❌ SetUpPelletCol(numPellets, pShooter, pVictim, ...)
│   └── Configura colisão para tiros de shotgun
│       Shotguns disparam múltiplos pellets
│
└── ❌ TargetWeaponRangeMultiplier(pShooter, pTarget)
    └── Calcula multiplicador de alcance
        Considera skill e tipo de arma
```

```
CWeaponInfo (Dados de weapon.dat)
│
├── Campos por arma:
│   ├── fireType          → Tipo de disparo (instant, projectile, area)
│   ├── targetRange       → Alcance máximo
│   ├── weaponRange       → Alcance efetivo
│   ├── damage            → Dano base
│   ├── accuracy          → Precisão base
│   ├── moveSpeed         → Velocidade de movimento com arma
│   ├── animGroup         → Grupo de animações
│   ├── ammoClip          → Tamanho do pente
│   ├── fireOffset        → Offset do cano da arma
│   └── skillLevel        → Requerimento de skill
│
├── ❌ Initialise()
│   └── Inicializa sistema de informações de armas
│       Aloca arrays e configura valores padrão
│
├── ❌ LoadWeaponData()
│   └── Carrega weapon.dat do disco
│       Parse de cada linha para preencher dados
│       Chamado durante inicialização do jogo
│
├── ⚠️ GetWeaponInfo(weaponType, skillLevel)
│   └── Retorna CWeaponInfo* para arma e skill
│       Cada arma tem 3 variantes (poor, std, pro)
│
├── ❌ FindWeaponType(name)
│   └── Busca tipo de arma pelo nome string
│       Ex: "PISTOL" → WEAPON_PISTOL (22)
│
├── ❌ FindWeaponFireType(name)
│   └── Busca tipo de disparo pelo nome
│       Ex: "INSTANT_HIT" → tipo 1
│
├── ❌ GetSkillStatIndex(weaponType)
│   └── Retorna índice da estatística de skill
│       Usado para ler/escrever skill do jogador
│
├── ❌ GetTargetHeadRange()
│   └── Retorna alcance para headshot
│       Varia conforme arma e skill
│
├── ❌ GetWeaponReloadTime()
│   └── Retorna tempo de recarga em ms
│
├── ❌ Shutdown()
│   └── Libera recursos do sistema
│
└── ms_aWeaponNames[47]
    └── Array com nomes das armas para debug
```

---

## 👤 SISTEMA DO PED (Ped Weapon System)

```
CPedGTA (Estrutura do Ped - ~0x7A4/0x988 bytes)
│
├── Campos de Armas:
│   ├── m_aWeapons[13]       → Array com 13 slots de CWeapon
│   ├── m_nActiveWeaponSlot  → Índice do slot ativo (0-12)
│   ├── m_nSavedWeapon       → Arma salva temporariamente
│   ├── m_nDelayedWeapon     → Arma de reserva
│   ├── m_nDelayedWeaponAmmo → Munição da arma de reserva
│   ├── m_nWeaponModelId     → ID do modelo 3D atual (-1 se nenhum)
│   ├── m_nWeaponSkill       → Nível de habilidade atual
│   ├── m_nWeaponShootingRate→ Taxa de disparo
│   ├── m_nWeaponAccuracy    → Precisão (0-100)
│   ├── m_pWeaponObject      → RpClump* do modelo da arma
│   ├── m_pGunflashObject    → RpClump* do flash do tiro
│   ├── m_pEntLockOnTarget   → Entidade com lock-on
│   └── m_vecWeaponPrevPos   → Posição anterior da arma
│
├── ✅ GiveWeapon(weaponType, ammo)
│   └── Dá uma arma ao ped com munição
│       1. Carrega modelo via CStreaming
│       2. Encontra slot correto para o tipo
│       3. Inicializa CWeapon no slot
│       4. Define como arma atual
│
├── ❌ ClearWeapon(weaponType)
│   └── Remove arma específica do inventário
│       Libera slot e remove modelo visual
│
├── ⚠️ ClearWeapons()
│   └── Remove todas as armas do ped
│       Limpa todos os 13 slots
│       Remove modelo visual da mão
│
├── ⚠️ AddWeaponModel(modelId)
│   └── Anexa modelo 3D da arma à mão do ped
│       1. Remove modelo anterior se houver
│       2. Clona RpClump do modelo
│       3. Anexa ao bone da mão direita (24)
│       4. Atualiza m_pWeaponObject
│
├── ⚠️ RemoveWeaponModel(modelId)
│   └── Remove modelo 3D da arma da mão
│       Desanexa do bone e destrói clone
│       Seta m_pWeaponObject = nullptr
│
├── ⚠️ SetCurrentWeapon(weaponType)
│   └── Troca para arma especificada
│       1. Encontra slot da arma
│       2. Atualiza m_nActiveWeaponSlot
│       3. Troca modelo visual
│       4. Atualiza animações
│
├── ❌ GetWeaponSlot(weaponType)
│   └── Retorna índice do slot para o tipo
│       Cada tipo tem slot fixo (pistolas=2, etc)
│
├── ❌ GetWeaponSkill(weaponType)
│   └── Retorna nível de skill para arma
│       0=poor, 1=std, 2=pro
│
├── ⚠️ SetWeaponSkill(weaponType, skillLevel)
│   └── Define skill para tipo de arma
│       Afeta precisão, dano, animações
│
├── ❌ SetWeaponLockOnTarget(pEntity)
│   └── Define entidade como alvo de lock-on
│       Armazena em m_pEntLockOnTarget
│
├── ❌ DoWeHaveWeaponAvailable(weaponType)
│   └── Verifica se ped possui a arma
│       Retorna true se slot tem munição > 0
│
├── ❌ GiveDelayedWeapon(weaponType, ammo)
│   └── Configura arma de reserva
│       Ex: policial tem cassetete + pistola
│
├── ❌ RequestDelayedWeapon()
│   └── Saca arma de reserva
│       Troca da arma atual para a reserva
│
├── ❌ RemoveWeaponAnims(slot, blendDelta)
│   └── Remove animações de arma
│       Usado ao guardar arma ou morrer
│
├── ❌ CanWeRunAndFireWithWeapon()
│   └── Verifica se pode correr e atirar
│       Depende do tipo (SMGs sim, rifles não)
│
├── ❌ RemoveWeaponWhenEnteringVehicle(flags)
│   └── Guarda arma ao entrar em veículo
│       Algumas armas são escondidas automaticamente
│
├── ❌ ReplaceWeaponWhenExitingVehicle()
│   └── Restaura arma ao sair do veículo
│       Reexibe arma que estava escondida
│
├── ✅ SetAmmo(weaponType, ammo)
│   └── Define munição para arma específica
│       Encontra slot e atualiza dwAmmo
│
├── ❌ GrantAmmo(weaponType, ammo)
│   └── Adiciona munição (não substitui)
│       ammo_atual += ammo
│
├── ❌ CreateDeadPedWeaponPickups()
│   └── Cria pickups de armas ao morrer
│       Dropa armas que o ped carregava
│
├── ❌ RemoveWeaponForScriptedCutscene()
│   └── Remove arma para cutscene
│       Esconde modelo mas mantém no inventário
│
└── ❌ ReplaceWeaponForScriptedCutscene()
    └── Restaura arma após cutscene
```

```
CPlayerPed (Wrapper de alto nível)
│
├── ✅ GiveWeapon(weaponId, ammo)
│   └── Wrapper que chama CPedGTA::GiveWeapon
│       Verifica se m_pPed é válido primeiro
│
├── ✅ GetCurrentWeapon()
│   └── Retorna tipo da arma atual (0-46)
│       Lê do slot ativo do ped
│
├── ✅ GetCurrentWeaponSlot()
│   └── Retorna ponteiro CWeapon* do slot ativo
│       &m_pPed->m_aWeapons[m_nActiveWeaponSlot]
│
├── ✅ FindWeaponSlot(weaponType)
│   └── Busca slot que contém a arma
│       Itera pelos 13 slots procurando
│
├── ✅ SetAmmo(weaponType, ammo)
│   └── Define munição via FindWeaponSlot
│
├── ✅ SetArmedWeapon(weaponType, unk)
│   └── Troca arma atual (não implementado)
│
├── ✅ ClearWeapons()
│   └── Remove todas armas via hook nativo
│
├── ✅ FireInstant()
│   └── Dispara arma atual
│       1. Salva estado da câmera
│       2. Configura aim do player remoto
│       3. Chama CWeapon::FireInstantHit
│       4. Restaura estado da câmera
│
├── ✅ GetWeaponInfoForFire(bLeftWrist, vecBonePos, vecOut)
│   └── Obtém posição e offset para disparo
│       Calcula posição do cano baseado no bone
│
├── ✅ GetCurrentWeaponFireOffset()
│   └── Retorna offset de disparo do weapon.dat
│       Posição relativa do cano da arma
│
├── ✅ ProcessBulletData(btData)
│   └── Processa dados de bala para sync
│       Prepara BULLET_SYNC_DATA para envio
│       Identifica tipo de hit (player/vehicle/object)
│
├── ✅ SetWeaponSkill(weaponType, skill)
│   └── Define skill via sistema de stats
│       Atualiza local ou remote conforme player
│
├── ❌ ProcessPlayerWeapon(pPad)
│   └── Processa input de arma do jogador
│       Verifica botões de tiro, mira, troca
│
├── ❌ ProcessWeaponSwitch(pPad)
│   └── Processa troca de armas via input
│       Scroll ou teclas numéricas
│
├── ❌ FindWeaponLockOnTarget()
│   └── Busca alvo válido para lock-on
│       Prioriza ameaças e proximidade
│
├── ❌ FindNextWeaponLockOnTarget(pCurrent, bForward)
│   └── Cicla para próximo/anterior alvo
│
├── ❌ DoesPlayerWantNewWeapon(weaponType, bForce)
│   └── Verifica se jogador quer trocar
│
├── ❌ MakeChangesForNewWeapon(weaponType)
│   └── Aplica mudanças visuais/animação
│       ao trocar de arma
│
├── ❌ ClearWeaponTarget()
│   └── Remove lock-on atual
│
├── ❌ GetWeaponRadiusOnScreen()
│   └── Retorna raio da mira na tela
│       Usado para desenhar crosshair
│
├── ❌ UpdateCameraWeaponModes(pPad)
│   └── Atualiza modo de câmera conforme arma
│       Alterna entre modos de mira
│
├── ❌ CanIKReachThisTarget(vecTarget, pWeapon, bCheck)
│   └── Verifica se IK pode mirar no alvo
│       Limites de rotação do torso/braços
│
└── ❌ DoesTargetHaveToBeBroken(pEntity, pWeapon)
    └── Verifica se deve quebrar lock-on
        Alvo morreu, saiu de alcance, etc
```

---

## 🎯 SISTEMA DE MIRA/TARGETING (Aiming System)

```
CWeaponEffects
│
├── ❌ Init()
│   └── Inicializa sistema de efeitos de mira
│       Aloca estruturas para crosshairs
│
├── ❌ Render()
│   └── Renderiza crosshairs e marcadores
│       Desenha sprites 2D na posição do alvo
│
├── ❌ MarkTarget(index, vecPos, r, g, b, a, size, type)
│   └── Marca posição com crosshair
│       Cria indicador visual no alvo
│
├── ❌ IsLockedOn(index)
│   └── Retorna true se slot tem lock-on
│
├── ❌ ClearCrossHair(index)
│   └── Remove crosshair específico
│
├── ❌ ClearCrossHairs()
│   └── Remove todos crosshairs
│
├── ❌ ClearCrossHairImmediately(index)
│   └── Remove crosshair sem fade
│
├── ❌ ClearAllCrosshairs()
│   └── Limpa todos os marcadores
│
└── ❌ Shutdown()
    └── Finaliza sistema de efeitos
```

```
CCamera (Funções de arma)
│
├── ✅ SetBehindPlayer()
│   └── Posiciona câmera atrás do jogador
│       Usado ao sair de veículo, morrer, etc
│
├── ✅ GetPosition()
│   └── Retorna posição atual da câmera
│
├── ❌ IsTargetingActive()
│   └── Verifica se está em modo de mira
│       True quando segurando botão de mirar
│
├── ❌ UpdateAimingCoors(vecTarget)
│   └── Atualiza coordenadas de mira
│       Move câmera suavemente para alvo
│
├── ❌ UpdateTargetEntity()
│   └── Atualiza entidade alvo da câmera
│
├── ❌ SetColVarsAimWeapon(mode)
│   └── Configura colisão para modo de mira
│       Ajusta clipping da câmera
│
├── ❌ ClearPlayerWeaponMode()
│   └── Limpa modo de arma do jogador
│       Volta para câmera normal
│
├── ❌ SetNewPlayerWeaponMode(primary, secondary, mode)
│   └── Define novo modo de câmera de arma
│       Ex: mira por cima do ombro, primeira pessoa
│
├── ❌ Using1stPersonWeaponMode()
│   └── Retorna true se em primeira pessoa
│       Sniper, lança-foguetes usam isso
│
├── ❌ Find3rdPersonQuickAimPitch()
│   └── Calcula pitch para mira rápida
│       Usado quando mirando sem lock-on
│
├── ❌ CameraPedAimModeSpecialCases(pPed)
│   └── Trata casos especiais de mira
│       Ajustes para jetpack, bicicleta, etc
│
├── ❌ Find3rdPersonCamTargetVector(angle, source, dir, pos)
│   └── Calcula vetor de mira em 3ª pessoa
│       Projeta ray da câmera para o mundo
│
├── ❌ Enable1rstPersonWeaponsCamera()
│   └── Ativa câmera de primeira pessoa
│       Para sniper e miras telescópicas
│
└── ❌ HandleCameraMotionForDuckingDuringAim(pPed, ...)
    └── Ajusta câmera ao agachar mirando
        Compensação de altura e ângulo
```

---

## 💥 SISTEMA DE DANO (Damage System)

```
CPedDamageResponseCalculator
│
├── ❌ CPedDamageResponseCalculator(pAttacker, damage, weaponType, pieceType, bSpeak)
│   └── Construtor que calcula resposta ao dano
│       Considera armadura, parte do corpo, tipo de arma
│       Calcula dano final e reação do ped
│
└── ❌ IsBleedingWeapon(pPed)
    └── Verifica se arma causa sangramento
        Facas, machados causam sangramento
        Afeta visualmente e pode causar morte lenta
```

```
CDamageManager (Dano em veículos)
│
├── ❌ ApplyDamage(pAutomobile, component, intensity, damageType)
│   └── Aplica dano a componente do veículo
│       Calcula deformação e pode destruir parte
│
├── ❌ SetDoorStatus(door, status)
│   └── Define estado da porta
│       0=ok, 1=amassada, 2=solta, 3=removida
│
├── ❌ SetPanelStatus(panel, status)
│   └── Define estado do painel
│       Capô, porta-malas, para-lamas
│
├── ❌ SetWheelStatus(wheel, status)
│   └── Define estado da roda
│       0=ok, 1=furada, 2=removida
│
├── ❌ SetLightStatus(light, status)
│   └── Define estado do farol
│       0=funcionando, 1=quebrado
│
├── ❌ SetEngineStatus(status)
│   └── Define estado do motor
│       Afeta performance e pode pegar fogo
│
├── ❌ FuckCarCompletely(bExplode)
│   └── Destrói completamente o veículo
│       Todas as partes danificadas
│
├── ❌ ProgressDoorDamage(door, pAutomobile)
│   └── Progride dano da porta
│       Amassada → Solta → Removida
│
└── ❌ GetCarNodeIndexFromDoor(door)
    └── Converte enum de porta para índice de node
        Usado para encontrar geometria correta
```

---

## 💣 SISTEMA DE PROJÉTEIS (Projectile System)

```
CProjectileInfo
│
├── ❌ Initialise()
│   └── Inicializa pool de projéteis
│       Aloca array para 32 projéteis simultâneos
│
├── ❌ AddProjectile(pCreator, weaponType, vecPos, speed, vecDir, pTarget)
│   └── Cria novo projétil no mundo
│       1. Encontra slot livre no pool
│       2. Cria entidade CProjectile
│       3. Define física (velocidade, gravidade)
│       4. Configura tracking se míssil guiado
│
├── ❌ Update()
│   └── Atualiza todos projéteis ativos
│       Move projéteis, verifica colisões
│       Explode se atingiu algo ou timeout
│
├── ❌ RemoveProjectile(pInfo, pProjectile)
│   └── Remove projétil específico
│       Destrói entidade e libera slot
│
├── ❌ RemoveAllProjectiles()
│   └── Remove todos os projéteis
│       Chamado ao carregar save
│
├── ❌ RemoveDetonatorProjectiles()
│   └── Remove bombas de controle remoto
│       Chamado ao usar detonador
│
├── ❌ IsProjectileInRange(x1, y1, z1, x2, y2, z2, bExplode)
│   └── Verifica se há projétil na área
│       Opcionalmente explode projéteis encontrados
│
├── ❌ RemoveFXSystem(index)
│   └── Remove sistema de efeitos do projétil
│       Fumaça, fogo, etc
│
├── ❌ RemoveIfThisIsAProjectile(pObject)
│   └── Remove se objeto for projétil
│       Verificação de segurança
│
├── ❌ GetProjectileInfo(index)
│   └── Retorna info do projétil por índice
│
└── ms_apProjectile[32]
    └── Array de ponteiros para projéteis ativos
```

```
CProjectile (Entidade física)
│
├── ❌ CProjectile(modelId)
│   └── Construtor da entidade projétil
│       Herda de CObject
│       Configura colisão especial
│
└── ❌ ~CProjectile()
    └── Destrutor
        Libera recursos e efeitos
```

---

## 💨 SISTEMA DE BALAS (Bullet System)

```
CBulletInfo
│
├── ❌ Initialise()
│   └── Inicializa sistema de balas
│       Aloca array para tracking de balas
│       Usado para balas físicas (não hitscan)
│
├── ❌ AddBullet(pShooter, weaponType, vecPos, vecVelocity)
│   └── Adiciona bala ao sistema
│       Para armas com balas visíveis
│       (minigun, etc)
│
├── ❌ Update()
│   └── Atualiza todas as balas
│       Move, verifica colisões, remove antigas
│
└── ❌ Shutdown()
    └── Finaliza sistema de balas
```

```
CBulletTraces (Rastros visuais)
│
├── ❌ Init()
│   └── Inicializa sistema de tracers
│       Aloca estruturas para rastros
│
├── ❌ AddTrace(vecStart, vecEnd, width, color, type)
│   └── Adiciona rastro de bala
│       Linha luminosa do cano ao impacto
│       Versão 1: posição inicial e final
│
├── ❌ AddTrace(vecStart, vecEnd, time, pEntity)
│   └── Adiciona rastro com duração
│       Versão 2: com tempo de vida
│
├── ❌ Update()
│   └── Atualiza rastros ativos
│       Fade out gradual, remove antigos
│
├── ❌ Render()
│   └── Renderiza todos os rastros
│       Desenha linhas/sprites
│
└── aTraces[16]
    └── Array de rastros ativos
```

---

## 💥 SISTEMA DE EXPLOSÕES (Explosion System)

```
CExplosion
│
├── ❌ Initialise()
│   └── Inicializa pool de explosões
│       Aloca array para 48 explosões
│
├── ❌ AddExplosion(pCreator, pVictim, type, vecPos, time, bMakeSound, camShake, bVisible)
│   └── Cria nova explosão
│       1. Encontra slot livre
│       2. Configura tipo (granada, carro, barril, etc)
│       3. Aplica dano em raio
│       4. Cria efeitos visuais e sonoros
│       5. Empurra objetos/peds próximos
│
├── ❌ Update()
│   └── Atualiza explosões ativas
│       Expande raio, cria fogo secundário
│       Remove explosões terminadas
│
├── ❌ GetExplosionType(index)
│   └── Retorna tipo da explosão
│       EXPLOSION_GRENADE, EXPLOSION_CAR, etc
│
├── ❌ GetExplosionPosition(index)
│   └── Retorna posição da explosão
│
├── ❌ GetExplosionActiveCounter(index)
│   └── Retorna contador de frames ativo
│
├── ❌ ResetExplosionActiveCounter(index)
│   └── Reseta contador para zero
│
├── ❌ DoesExplosionMakeSound(index)
│   └── Verifica se explosão faz som
│       Algumas são silenciosas
│
├── ❌ TestForExplosionInArea(type, x1, y1, z1, x2, y2, z2)
│   └── Testa se há explosão do tipo na área
│       Usado por scripts
│
├── ❌ RemoveAllExplosionsInArea(vecCenter, radius)
│   └── Remove explosões em raio
│
├── ❌ ClearAllExplosions()
│   └── Limpa todas as explosões
│
└── ❌ Shutdown()
    └── Finaliza sistema de explosões
```

---

## 🔥 SISTEMA DE FOGO (Fire System)

```
CFire
│
├── ❌ Initialise()
│   └── Inicializa instância de fogo
│       Configura partículas, tamanho, dano
│
├── ❌ ProcessFire()
│   └── Processa fogo por frame
│       Espalha para objetos próximos
│       Aplica dano a peds/veículos
│       Consome combustível e pode apagar
│
├── ❌ Extinguish()
│   └── Apaga o fogo
│       Remove partículas, para dano
│
└── ❌ CreateFxSysForStrength(vecPos, matrix)
    └── Cria sistema de partículas conforme força
        Fogo maior = mais partículas
```

```
CFireManager
│
├── ❌ Init()
│   └── Inicializa gerenciador de fogos
│       Pool de 60 fogos simultâneos
│
├── ❌ Update()
│   └── Atualiza todos os fogos
│       Processa cada CFire ativo
│
├── ❌ StartFire(vecPos, size, bAllowSpreading, pCreator, burnTime, strength, bSmall)
│   └── Inicia fogo em posição
│       Versão 1: fogo no chão/objeto
│
├── ❌ StartFire(pEntity, pAttacker, size, bAllowSpreading, burnTime, strength)
│   └── Inicia fogo em entidade
│       Versão 2: ped/veículo pegando fogo
│
├── ❌ StartScriptFire(vecPos, pTarget, size, strength, bPropagate, duration)
│   └── Inicia fogo controlado por script
│       Persistente, não apaga sozinho
│
├── ❌ FindNearestFire(vecPos, bCheckOnlyScript, bCheckOnlyNoScript)
│   └── Encontra fogo mais próximo
│       Usado por bombeiros
│
├── ❌ ExtinguishPoint(vecPos, radius)
│   └── Apaga fogos em raio
│       Versão básica
│
├── ❌ ExtinguishPointWithWater(vecPos, radius, waterAmount)
│   └── Apaga fogos com quantidade de água
│       Afeta progressivamente
│
├── ❌ GetNextFreeFire(bScriptFire)
│   └── Retorna próximo slot livre
│
├── ❌ GetNumFiresInArea(x1, y1, z1, x2, y2, z2)
│   └── Conta fogos na área
│
├── ❌ GetNumFiresInRange(vecPos, radius)
│   └── Conta fogos em raio
│
├── ❌ PlentyFiresAvailable()
│   └── Verifica se há slots livres
│
├── ❌ RemoveScriptFire(scriptFireIndex)
│   └── Remove fogo de script específico
│
├── ❌ RemoveAllScriptFires()
│   └── Remove todos fogos de script
│
├── ❌ IsScriptFireExtinguished(index)
│   └── Verifica se fogo de script apagou
│
└── ❌ Shutdown()
    └── Finaliza gerenciador
```

---

## 🎭 SISTEMA DE ANIMAÇÃO (Animation System)

```
CTaskSimpleUseGun
│
├── ❌ ProcessPed(pPed)
│   └── Processa ped usando arma
│       Atualiza IK, animações, estado
│
├── ❌ ControlGun(pPed, pTarget, mode)
│   └── Controla arma do ped
│       Gerencia mira e disparo
│
├── ❌ ControlGunMove(vecMove)
│   └── Controla movimento com arma
│       Ajusta velocidade e animações
│
├── ❌ AimGun(pPed)
│   └── Executa mira da arma
│       Rotaciona ped e braços para alvo
│
├── ❌ FireGun(pPed, bLeftHand)
│   └── Executa disparo
│       Chama CWeapon::Fire e toca anim
│
├── ❌ StartAnim(pPed)
│   └── Inicia animação de arma
│       Seleciona anim baseado no tipo
│
├── ❌ ClearAnim(pPed)
│   └── Limpa animação de arma
│
├── ❌ SetMoveAnim(pPed)
│   └── Define animação de movimento
│       Andar/correr com arma
│
├── ❌ SetPedPosition(pPed)
│   └── Ajusta posição do ped
│       Compensação para animações
│
├── ❌ RemoveStanceAnims(pPed, blendDelta)
│   └── Remove anims de postura de arma
│
├── ❌ RequirePistolWhip(pPed, pTarget)
│   └── Verifica se deve fazer melee
│       Pistol whip quando muito perto
│
├── ❌ StartCountDown(time, bAutoFire)
│   └── Inicia contagem para disparo
│       Usado para burst fire
│
├── ❌ FinishGunAnimCB(pAssoc, pData)
│   └── Callback ao terminar anim de arma
│
├── ❌ PlayerPassiveControlGun()
│   └── Controle passivo da arma
│       Quando jogador não está mirando
│
├── ❌ AbortIK(pPed)
│   └── Aborta sistema IK
│       Reseta rotações de ossos
│
├── ❌ Reset(pPed, pTarget, vecTarget, mode, time)
│   └── Reseta task com novos parâmetros
│
└── ❌ MakeAbortable(pPed, priority, pEvent)
    └── Permite abortar task
        Verifica se pode ser interrompida
```

```
CTaskSimpleGunControl
│
├── ❌ ProcessPed(pPed)
│   └── Processa controle de arma
│       Task de nível mais alto que UseGun
│
├── ❌ MakeAbortable(pPed, priority, pEvent)
│   └── Permite abortar task
│
└── ❌ ChangeFiringTask(mode, pTarget, vecPos, vecDir)
    └── Muda task de disparo
        Troca entre modos de tiro
```

```
CTaskManager
│
├── ✅ GetActiveTask()
│   └── Retorna task ativa principal
│       Do array de tasks primárias
│
├── ✅ GetSimplestActiveTask()
│   └── Retorna task mais simples ativa
│       Desce hierarquia de subtasks
│
└── ✅ FlushImmediately()
    └── Remove todas tasks imediatamente
        Usado ao teleportar/respawnar
```

```
CPedIK (Inverse Kinematics)
│
├── Campos:
│   ├── m_flags        → Flags de estado IK
│   ├── m_torsoOrien   → Orientação do torso
│   ├── m_headOrien    → Orientação da cabeça
│   └── ms_*Info       → Infos estáticas de limites
│
├── ❌ RotateTorso(pFrame, orientation, bUpdatePos)
│   └── Rotaciona torso para mirar
│       Aplica rotação ao bone do torso
│       Respeita limites de rotação
│
├── ❌ PitchForSlope()
│   └── Ajusta pitch para terreno
│       Inclinação do corpo em morros
│
└── ❌ GetWorldMatrix(pFrame, pMatrix)
    └── Obtém matriz mundial do bone
        Usado para cálculos de posição
```

---

## 🔊 SISTEMA DE ÁUDIO (Audio System)

```
CAEWeaponAudioEntity
│
├── ❌ Initialise()
│   └── Inicializa entidade de áudio de arma
│       Configura slots de som
│
├── ❌ WeaponFire(weaponType, pEntity, slot)
│   └── Toca som de disparo
│       Seleciona som baseado na arma
│       Considera distância e obstáculos
│
├── ❌ WeaponReload(weaponType, pEntity, slot)
│   └── Toca som de recarga
│
├── ❌ PlayGunSounds(pEntity, sfx1, sfx2, sfx3, sfx4, sfx5, slot, vol, freq, rolloff)
│   └── Toca múltiplos sons de arma
│       Disparo, eco, casquilho, etc
│
├── ❌ PlayCameraSound(pEntity, slot, volume)
│   └── Som da câmera fotográfica
│
├── ❌ PlayGoggleSound(sfx, slot)
│   └── Som de óculos (visão noturna/térmica)
│
├── ❌ PlayWeaponLoopSound(pEntity, sfx, slot, vol, freq, delay)
│   └── Som contínuo de arma
│       Minigun, lança-chamas
│
├── ❌ PlayMiniGunStopSound(pEntity)
│   └── Som ao parar minigun
│       Desaceleração do motor
│
├── ❌ PlayMiniGunFireSounds(pEntity, slot)
│   └── Sons de tiro da minigun
│
├── ❌ PlayChainsawStopSound(pEntity)
│   └── Som ao parar motosserra
│
├── ❌ PlayFlameThrowerSounds(pEntity, sfx1, sfx2, slot, vol, freq)
│   └── Sons do lança-chamas
│
├── ❌ PlayFlameThrowerIdleGasLoop(pEntity)
│   └── Som de gás do lança-chamas em idle
│
├── ❌ StopFlameThrowerIdleGasLoop()
│   └── Para som de idle do lança-chamas
│
├── ❌ ReportStealthKill(weaponType, pEntity, slot)
│   └── Reporta kill furtivo para áudio
│       Som especial de assassinato
│
├── ❌ ReportChainsawEvent(pEntity, slot)
│   └── Reporta evento de motosserra
│       Sons de corte
│
├── ❌ UpdateParameters(pSound, slot)
│   └── Atualiza parâmetros de som
│       Volume, posição, etc
│
├── ❌ Reset()
│   └── Reseta estado do áudio
│
└── ❌ Terminate()
    └── Finaliza entidade de áudio
```

---

## 🔄 SISTEMA DE COLISÃO (Collision System)

```
CCollision
│
├── ✅ ProcessLineOfSight(line, matrix, model, colPoint, range, ...)
│   └── Processa linha de visão contra modelo
│       Raycast completo com resultado de colisão
│       Usado por armas para verificar hits
│
├── ⚠️ ProcessLineSphere(line, sphere, colPoint, range)
│   └── Testa linha contra esfera
│       Colisão simplificada
│
├── ⚠️ ProcessLineBox(line, box, colPoint, range)
│   └── Testa linha contra caixa
│       Para objetos retangulares
│
├── ⚠️ ProcessLineTriangle(line, verts, triangle, plane, colPoint, range, ...)
│   └── Testa linha contra triângulo
│       Nível mais baixo de colisão
│
├── ⚠️ ProcessColModels(mat1, model1, mat2, model2, spheres, colPoints, depths, ...)
│   └── Processa colisão entre dois modelos
│       Retorna pontos de contato
│
└── ⚠️ ProcessVerticalLine(line, matrix, model, colPoint, range, ...)
    └── Linha vertical otimizada
        Para verificar chão
```

```
CColPoint (Estrutura de resultado)
│
├── Campos:
│   ├── m_vecPoint      → Ponto de colisão
│   ├── m_vecNormal     → Normal da superfície
│   ├── m_nSurfaceTypeA → Tipo de superfície A
│   ├── m_nSurfaceTypeB → Tipo de superfície B
│   ├── m_fDepth        → Profundidade de penetração
│   └── m_nPieceTypeA/B → Tipo de peça atingida
│
└── Usado para:
    - Determinar onde bala atingiu
    - Calcular ângulo de ricochete
    - Identificar material para efeitos
    - Aplicar dano à parte correta
```

---

## 📊 Resumo Final

```
IMPLEMENTAÇÃO NO APK:
├── ✅ Total Implementado:        ~32 funções/classes
├── ⚠️ Total Parcial (hooks):     ~27 funções/classes  
├── ❌ Total Não Implementado:    ~104 funções/classes
│
└── Cobertura: ~20% do sistema completo
    
    O projeto foca em:
    1. Streaming de modelos (para carregar armas)
    2. Estruturas de dados (CWeapon, slots)
    3. Interface de alto nível (CPlayerPed)
    4. Sistema de sincronização (BULLET_SYNC)
    
    O resto usa chamadas diretas à libGTASA via hooks.
```
