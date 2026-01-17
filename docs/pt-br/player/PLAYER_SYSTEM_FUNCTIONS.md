# Sistema de Personagem - Árvore Detalhada de Funções

**Legenda:**
- ✅ = Implementado no APK
- ⚠️ = Parcialmente implementado (via hook)
- ❌ = Não implementado

---

## 1. CPed (Classe Base do Ped)

### 1.1 Inicialização e Ciclo de Vida
```
CPed
├── ✅ CPed(ePedType)
│   └── Construtor do ped, define tipo (COP, CIVMALE, GANG1, etc)
│
├── ✅ ~CPed()
│   └── Destrutor, libera recursos
│
├── ⚠️ Initialise()
│   └── Inicializa estruturas internas do ped
│
├── ⚠️ SetModelIndex(uint)
│   └── Define o modelo 3D do ped
│       Carrega skin via streaming se necessário
│
├── ❌ DeleteRwObject()
│   └── Remove objeto RenderWare
│
├── ⚠️ SetCharCreatedBy(uint8)
│   └── Define quem criou o ped (GAME, MISSION)
│
└── ❌ SetPedDefaultDecisionMaker()
    └── Define IA padrão para o ped
```

### 1.2 Controle de Estado
```
CPed
├── ⚠️ SetPedState(ePedState)
│   └── Define estado atual (IDLE, ATTACK, DRIVING, etc)
│       Estados controlam comportamento e animações
│
├── ❌ GetPedStateString()
│   └── Retorna string do estado para debug
│
├── ❌ IsPedInControl()
│   └── Verifica se ped tem controle de si mesmo
│       False se em ragdoll, cutscene, etc
│
├── ❌ CanSetPedState()
│   └── Verifica se pode mudar estado
│
├── ❌ CanBeDeleted()
│   └── Verifica se ped pode ser removido
│
├── ❌ CanBeDeletedEvenInVehicle()
│   └── Pode ser deletado mesmo em veículo
│
├── ❌ CanBeArrested()
│   └── Verifica se pode ser preso
│
├── ❌ IsPedShootable()
│   └── Verifica se pode levar tiro
│
└── ❌ IsPointerValid()
    └── Verifica se ponteiro do ped é válido
```

### 1.3 Movimento e Física
```
CPed
├── ❌ ProcessControl()
│   └── Função principal de update por frame
│       Processa física, colisão, IA, animação
│
├── ❌ UpdatePosition()
│   └── Atualiza posição baseado em velocidade
│       Aplica gravidade e constraints
│
├── ❌ CalculateNewVelocity()
│   └── Calcula nova velocidade baseada em input
│       Considera terreno e obstáculos
│
├── ❌ CalculateNewOrientation()
│   └── Calcula nova rotação do ped
│       Suaviza mudanças de direção
│
├── ❌ ProcessBuoyancy()
│   └── Processa flutuação na água
│       Afunda ou flutua conforme física
│
├── ❌ ProcessEntityCollision(CEntity*, CColPoint*)
│   └── Processa colisão com outra entidade
│       Responde a impactos
│
├── ⚠️ Teleport(CVector, uint8)
│   └── Teleporta ped para posição
│
├── ❌ SetMoveState(eMoveState)
│   └── Define estado de movimento (STILL, WALK, RUN, SPRINT)
│
├── ❌ SetMoveAnim()
│   └── Configura animação de movimento
│
└── ❌ SetMoveAnimSpeed(CAnimBlendAssociation*)
    └── Define velocidade da animação de movimento
```

### 1.4 Olhar e Mira
```
CPed
├── ❌ SetLookFlag(CEntity*, bool, bool)
│   └── Define entidade para olhar
│
├── ❌ SetLookFlag(float, bool, bool)
│   └── Define ângulo para olhar
│
├── ❌ ClearLookFlag()
│   └── Para de olhar para algo
│
├── ❌ SetLook(CEntity*)
│   └── Olha para entidade (versão simples)
│
├── ❌ SetLook(float)
│   └── Olha para ângulo
│
├── ❌ ClearLook()
│   └── Limpa estado de look
│
├── ❌ Look()
│   └── Processa look ativo
│
├── ❌ SetLookTimer(uint32)
│   └── Define timer para olhar
│
├── ❌ SetAimFlag(CEntity*)
│   └── Define entidade para mirar
│
├── ❌ SetAimFlag(float)
│   └── Define ângulo de mira
│
├── ❌ ClearAimFlag()
│   └── Para de mirar
│
├── ❌ SetWeaponLockOnTarget(CEntity*)
│   └── Define alvo de lock-on
│
└── ❌ UseFreeAimMagnetize()
    └── Usa sistema de mira livre com magnetismo
```

### 1.5 Animação
```
CPed
├── ❌ PreRender()
│   └── Preparação antes do render
│
├── ❌ PreRenderAfterTest()
│   └── Preparação após testes de visibilidade
│       Atualiza bones, IK, roupas
│
├── ❌ StopNonPartialAnims()
│   └── Para animações que não são parciais
│
├── ❌ RestartNonPartialAnims()
│   └── Reinicia animações não-parciais
│
├── ❌ DoFootLanded(bool, uint8)
│   └── Processa pé tocando o chão
│       Gera som de passo
│
├── ❌ PlayFootSteps()
│   └── Toca sons de passos
│
├── ❌ RemoveWeaponAnims(int, float)
│   └── Remove animações de arma
│
├── ❌ ShoulderBoneRotation(RpClump*)
│   └── Rotação do bone do ombro
│
├── ❌ TurnBody()
│   └── Gira o corpo do ped
│
├── ❌ SetIdle()
│   └── Define animação idle
│
├── ❌ IsPlayingHandSignal()
│   └── Verifica se está fazendo sinal com mão
│
└── ❌ StopPlayingHandSignal()
    └── Para sinal com mão
```

### 1.6 Posição de Bones
```
CPed
├── ✅ GetBonePosition(RwV3d&, uint, bool)
│   └── Obtém posição de um bone específico
│       Usado para posicionar armas, efeitos
│
├── ❌ GetHeadAndFootPositions(CVector&, CVector&, bool)
│   └── Obtém posições da cabeça e pés
│
└── ❌ IsPedHeadAbovePos(float)
    └── Verifica se cabeça está acima de altura
```

### 1.7 Veículos
```
CPed
├── ✅ IsInVehicle()
│   └── Verifica se está em veículo
│
├── ✅ IsAPassenger()
│   └── Verifica se é passageiro
│
├── ⚠️ SetPedPositionInCar()
│   └── Define posição dentro do carro
│
├── ❌ UpdateStatEnteringVehicle()
│   └── Atualiza stats ao entrar em veículo
│
├── ❌ UpdateStatLeavingVehicle()
│   └── Atualiza stats ao sair de veículo
│
├── ❌ RemoveWeaponWhenEnteringVehicle(flags)
│   └── Esconde arma ao entrar
│
└── ❌ ReplaceWeaponWhenExitingVehicle()
    └── Mostra arma ao sair
```

### 1.8 Objetos e Anexos
```
CPed
├── ❌ AttachPedToEntity(CEntity*, CVector, ushort, float, eWeaponType)
│   └── Anexa ped a uma entidade
│
├── ❌ AttachPedToBike(CEntity*, CVector, ushort, float, float, eWeaponType)
│   └── Anexa ped a uma moto
│
├── ❌ DettachPedFromEntity()
│   └── Desanexa ped
│
├── ❌ PositionAttachedPed()
│   └── Atualiza posição do ped anexado
│
├── ❌ GiveObjectToPedToHold(int, uint8)
│   └── Dá objeto para ped segurar
│
├── ❌ GetEntityThatThisPedIsHolding()
│   └── Obtém entidade que ped está segurando
│
├── ❌ CanThrowEntityThatThisPedIsHolding()
│   └── Pode jogar entidade que segura
│
├── ❌ DropEntityThatThisPedIsHolding(uint8)
│   └── Solta entidade
│
├── ❌ PutOnGoggles()
│   └── Coloca óculos (visão noturna, etc)
│
├── ❌ TakeOffGoggles()
│   └── Remove óculos
│
├── ❌ AddGogglesModel(int, bool*)
│   └── Adiciona modelo de óculos
│
└── ❌ RemoveGogglesModel()
    └── Remove modelo de óculos
```

### 1.9 Dano e Morte
```
CPed
├── ❌ RemoveBodyPart(int, int8)
│   └── Remove parte do corpo (desmembramento)
│
├── ❌ SpawnFlyingComponent(int, int8)
│   └── Spawna parte do corpo voando
│
├── ❌ DoesLOSBulletHitPed(CColPoint&)
│   └── Verifica se bala atingiu ped
│
├── ❌ KillPedWithCar(CVehicle*, float, bool)
│   └── Mata ped com carro
│
├── ❌ DeadPedMakesTyresBloody()
│   └── Ped morto suja pneus de sangue
│
├── ❌ CreateDeadPedMoney()
│   └── Cria dinheiro do ped morto
│
└── ❌ CreateDeadPedPickupCoors(float*, float*, float*)
    └── Calcula posição para pickup de morte
```

### 1.10 Fala/Áudio
```
CPed
├── ❌ Say(short, uint32, float, uint8, uint8, uint8)
│   └── Ped fala uma frase
│
├── ❌ SayScript(int, uint8, uint8, uint8)
│   └── Fala via script
│
├── ❌ GetPedTalking()
│   └── Verifica se está falando
│
├── ❌ EnablePedSpeech()
│   └── Habilita fala
│
├── ❌ DisablePedSpeech(short)
│   └── Desabilita fala
│
├── ❌ EnablePedSpeechForScriptSpeech()
│   └── Habilita fala para scripts
│
├── ❌ DisablePedSpeechForScriptSpeech(short)
│   └── Desabilita fala de scripts
│
├── ❌ PedIsInvolvedInConversation()
│   └── Ped está em conversa
│
└── ❌ PedIsReadyForConversation(bool)
    └── Ped pronto para conversar
```

---

## 2. CPlayerPed (Extensão para Jogador)

### 2.1 Inicialização
```
CPlayerPed
├── ✅ CPlayerPed(int, bool)
│   └── Construtor do player
│
├── ✅ ~CPlayerPed()
│   └── Destrutor
│
├── ⚠️ SetupPlayerPed(int)
│   └── Configura estruturas do player
│       Chamado durante criação
│
├── ⚠️ DeactivatePlayerPed(int)
│   └── Desativa player temporariamente
│
├── ⚠️ ReactivatePlayerPed(int)
│   └── Reativa player
│
├── ⚠️ RemovePlayerPed(int)
│   └── Remove player do jogo
│
├── ❌ SetInitialState(bool)
│   └── Define estado inicial após spawn
│
├── ⚠️ Load()
│   └── Carrega dados salvos
│
└── ⚠️ Save()
    └── Salva dados do player
```

### 2.2 Controle de Movimento
```
CPlayerPed
├── ❌ ProcessControl()
│   └── Update principal por frame
│       Processa input, física, IA
│
├── ❌ ProcessPlayerWeapon(CPad*)
│   └── Processa input de arma
│
├── ❌ ProcessWeaponSwitch(CPad*)
│   └── Processa troca de arma
│
├── ❌ ProcessAnimGroups()
│   └── Processa grupos de animação
│       Atualiza conforme arma equipada
│
├── ❌ ProcessGroupBehaviour(CPad*)
│   └── Processa comportamento do grupo
│
├── ❌ SetRealMoveAnim()
│   └── Define animação real de movimento
│
├── ❌ ReApplyMoveAnims()
│   └── Reaplicar animações de movimento
│
├── ❌ SetMoveAnim()
│   └── Define animação de movimento (override)
│
├── ❌ SetPlayerMoveBlendRatio(CVector*)
│   └── Define blend ratio do movimento
│
└── ❌ MovementDisabledBecauseOfTargeting()
    └── Movimento desabilitado por estar mirando
```

### 2.3 Sistema de Mira/Targeting
```
CPlayerPed
├── ❌ FindWeaponLockOnTarget()
│   └── Busca alvo para lock-on
│       Prioriza ameaças próximas
│
├── ❌ FindNextWeaponLockOnTarget(CEntity*, bool)
│   └── Próximo alvo de lock-on
│
├── ❌ EvaluateTarget(CEntity*, CEntity**, float*, float, float, bool)
│   └── Avalia qualidade do alvo
│
├── ❌ EvaluateNeighbouringTarget(CEntity*, CEntity**, float*, float, float, bool)
│   └── Avalia alvos vizinhos
│
├── ❌ FindTargetPriority(CEntity*)
│   └── Calcula prioridade do alvo
│
├── ❌ HandleTapToTarget(float, float, bool)
│   └── Sistema de mira com toque
│
├── ❌ HandleMeleeTargeting()
│   └── Mira para combate corpo-a-corpo
│
├── ❌ Compute3rdPersonMouseTarget(bool)
│   └── Calcula alvo via mouse em 3ª pessoa
│
├── ❌ Clear3rdPersonMouseTarget()
│   └── Limpa alvo de mouse
│
├── ❌ ClearWeaponTarget()
│   └── Limpa alvo de arma
│
├── ❌ RotatePlayerToTrackTarget()
│   └── Rotaciona player para acompanhar alvo
│
├── ❌ CanIKReachThisTarget(CVector, CWeapon*, bool)
│   └── Verifica se IK pode alcançar alvo
│
├── ❌ DoesTargetHaveToBeBroken(CEntity*, CWeapon*)
│   └── Verifica se deve quebrar lock-on
│
├── ❌ PedCanBeTargettedVehicleWise(CPed*)
│   └── Pode ser alvo considerando veículos
│
├── ❌ DisplayTargettingDebug()
│   └── Exibe debug de targeting
│
├── ❌ GetWeaponRadiusOnScreen()
│   └── Raio da mira na tela
│
└── ❌ UpdateCameraWeaponModes(CPad*)
    └── Atualiza modos de câmera de arma
```

### 2.4 Sistema de Grupo
```
CPlayerPed
├── ❌ TellGroupToStartFollowingPlayer(bool, bool, bool)
│   └── Ordena grupo a seguir
│
├── ❌ MakeThisPedJoinOurGroup(CPed*)
│   └── Adiciona ped ao grupo
│
├── ❌ DisbandPlayerGroup()
│   └── Dissolve grupo
│
├── ❌ MakePlayerGroupDisappear()
│   └── Faz grupo desaparecer
│
├── ❌ MakePlayerGroupReappear()
│   └── Faz grupo reaparecer
│
├── ❌ ForceGroupToAlwaysFollow(bool)
│   └── Força grupo a sempre seguir
│
├── ❌ ForceGroupToNeverFollow(bool)
│   └── Força grupo a nunca seguir
│
└── ❌ MakeGroupRespondToPlayerTakingDamage(CEventDamage const&)
    └── Grupo responde a dano no player
```

### 2.5 Stamina e Energia
```
CPlayerPed
├── ❌ HandlePlayerBreath(bool, float)
│   └── Gerencia fôlego (mergulho)
│
├── ❌ ResetPlayerBreath()
│   └── Reseta fôlego
│
├── ❌ HandleSprintEnergy(bool, float)
│   └── Gerencia energia de sprint
│
├── ❌ ResetSprintEnergy()
│   └── Reseta energia de sprint
│
├── ❌ ControlButtonSprint(eSprintType)
│   └── Controla botão de sprint
│
└── ❌ GetButtonSprintResults(eSprintType)
    └── Resultado do botão de sprint
```

### 2.6 Estado e Wanted Level
```
CPlayerPed
├── ❌ SetWantedLevel(int)
│   └── Define nível de procurado
│
├── ❌ SetWantedLevelNoDrop(int)
│   └── Define procurado sem diminuir
│
├── ❌ CheatWantedLevel(int)
│   └── Define via cheat
│
├── ❌ Busted()
│   └── Player foi preso
│
├── ❌ AnnoyPlayerPed(bool)
│   └── Irrita o player (NPCs)
│
├── ❌ PlayerWantsToAttack()
│   └── Player quer atacar
│
├── ❌ PlayerHasJustAttackedSomeone()
│   └── Player acabou de atacar alguém
│
├── ❌ ClearAdrenaline()
│   └── Limpa efeito de adrenalina
│
├── ❌ DoStuffToGoOnFire()
│   └── Processa pegar fogo
│
├── ❌ KeepAreaAroundPlayerClear()
│   └── Mantém área ao redor limpa
│
└── ❌ IsHidden()
    └── Verifica se está escondido
```

### 2.7 Armas (Específico do Player)
```
CPlayerPed
├── ❌ DoesPlayerWantNewWeapon(eWeaponType, bool)
│   └── Player quer trocar de arma
│
├── ❌ MakeChangesForNewWeapon(eWeaponType)
│   └── Aplica mudanças ao trocar arma
│
├── ❌ MakeChangesForNewWeapon(int)
│   └── Versão com slot
│
├── ❌ PickWeaponAllowedFor2Player()
│   └── Escolhe arma permitida para 2 jogadores
│
└── ❌ FindPedToAttack()
    └── Encontra ped para atacar
```

### 2.8 Utilidades
```
CPlayerPed
├── ❌ GetPadFromPlayer()
│   └── Obtém CPad do jogador
│
├── ❌ GetPlayerInfoForThisPlayerPed()
│   └── Obtém CPlayerInfo deste ped
│
├── ❌ CanPlayerStartMission()
│   └── Pode iniciar missão
│
├── ❌ FindSprayableBuilding()
│   └── Encontra prédio para grafitar
│
└── ❌ GetWalkAnimSpeed()
    └── Velocidade da animação de caminhada
```

---

## 3. CPedModelInfo (Modelo do Ped)

```
CPedModelInfo
├── ❌ Init()
│   └── Inicializa info do modelo
│
├── ❌ SetClump(RpClump*)
│   └── Define clump do modelo
│       Associa geometria RW
│
├── ❌ DeleteRwObject()
│   └── Remove objeto RW
│
├── ❌ GetModelType()
│   └── Retorna tipo (MODEL_INFO_PED)
│
├── ❌ SetFaceTexture(RwTexture*)
│   └── Define textura do rosto
│
├── ❌ IncrementVoice()
│   └── Próxima voz disponível
│
├── ❌ CreateHitColModelSkinned(RpClump*)
│   └── Cria modelo de colisão skinned
│       Para detecção de tiros
│
├── ❌ AnimatePedColModelSkinned(RpClump*)
│   └── Anima colisão skinned
│       Atualiza por frame
│
├── ❌ AnimatePedColModelSkinnedWorld(RpClump*)
│   └── Versão world-space
│
└── ❌ AddXtraAtomics(RpClump*)
    └── Adiciona atomics extras
```

---

## 4. CAnimManager (Gerenciador de Animação)

```
CAnimManager
├── ❌ Initialise()
│   └── Inicializa sistema de animação
│       Aloca estruturas, carrega anims básicas
│
├── ❌ Shutdown()
│   └── Finaliza sistema
│
├── ⚠️ AddAnimation(RpClump*, AssocGroupId, AnimationId)
│   └── Adiciona animação a um clump
│       Retorna CAnimBlendAssociation*
│
├── ⚠️ BlendAnimation(RpClump*, AssocGroupId, AnimationId, float)
│   └── Faz blend de animação
│       float = blend amount
│
├── ❌ AddAnimationAndSync(RpClump*, CAnimBlendAssociation*, AssocGroupId, AnimationId)
│   └── Adiciona e sincroniza com outra
│
├── ❌ GetAnimation(char const*, CAnimBlock const*)
│   └── Obtém animação por nome
│
├── ❌ GetAnimation(uint, CAnimBlock const*)
│   └── Obtém por índice
│
├── ❌ GetAnimAssociation(AssocGroupId, AnimationId)
│   └── Obtém associação de animação
│
├── ❌ GetAnimAssociation(AssocGroupId, char const*)
│   └── Por nome
│
├── ⚠️ LoadAnimFile(char const*)
│   └── Carrega arquivo IFP
│
├── ❌ LoadAnimFiles()
│   └── Carrega todos arquivos de animação
│
├── ❌ RegisterAnimBlock(char const*)
│   └── Registra bloco de animação
│
├── ❌ AddAnimBlockRef(int)
│   └── Adiciona referência a bloco
│
├── ❌ RemoveAnimBlockRef(int)
│   └── Remove referência
│
├── ❌ RemoveAnimBlock(int)
│   └── Remove bloco completamente
│
├── ❌ CreateAnimAssocGroups()
│   └── Cria grupos de associação
│
├── ❌ UncompressAnimation(CAnimBlendHierarchy*)
│   └── Descomprime animação
│
├── ❌ RemoveFromUncompressedCache(CAnimBlendHierarchy*)
│   └── Remove do cache descomprimido
│
├── ❌ GetFirstAssocGroup(char const*)
│   └── Primeiro grupo para anim
│
├── ❌ GetAnimBlockName(AssocGroupId)
│   └── Nome do bloco
│
├── ❌ GetAnimGroupName(AssocGroupId)
│   └── Nome do grupo
│
└── ❌ GetAnimationBlockIndex(char const*)
    └── Índice do bloco por nome
```

---

## 5. CTaskManager (Gerenciador de Tarefas)

```
CTaskManager
├── ✅ GetActiveTask()
│   └── Retorna tarefa ativa principal
│
├── ✅ GetSimplestActiveTask()
│   └── Retorna tarefa mais simples
│
├── ✅ FlushImmediately()
│   └── Remove todas tarefas imediatamente
│
├── ❌ StopTimers(CEvent const*)
│   └── Para timers de tarefas
│
├── ❌ AddSubTasks(CTask*)
│   └── Adiciona sub-tarefas
│
├── ❌ ManageTasks()
│   └── Gerencia todas tarefas
│       Chamado por frame
│
├── ❌ SetNextSubTask(CTask*)
│   └── Define próxima sub-tarefa
│
├── ❌ GetSimplestTask(CTask*)
│   └── Obtém tarefa mais simples
│
├── ❌ SetTask(CTask*, int, bool)
│   └── Define tarefa primária
│
├── ❌ SetTaskSecondary(CTask*, int)
│   └── Define tarefa secundária
│
├── ❌ ClearTaskEventResponse()
│   └── Limpa resposta a eventos
│
├── ❌ ParentsControlChildren(CTask*)
│   └── Pais controlam filhos
│
└── ❌ Flush()
    └── Limpa todas tarefas
```

---

## 6. Sistema de Roupas (CClothes / CClothesBuilder)

```
CClothes
├── ❌ Init()
│   └── Inicializa sistema de roupas
│
├── ❌ LoadClothesFile()
│   └── Carrega arquivo de roupas
│
├── ❌ RebuildPlayer(CPlayerPed*, bool)
│   └── Reconstrói modelo do player
│       Aplica roupas selecionadas
│
├── ❌ RebuildPlayerIfNeeded(CPlayerPed*)
│   └── Reconstrói se necessário
│
├── ❌ RebuildCutscenePlayer(CPlayerPed*, int)
│   └── Reconstrói para cutscene
│
├── ❌ ConstructPedModel(uint, CPedClothesDesc&, CPedClothesDesc const*, bool)
│   └── Constrói modelo completo
│
├── ❌ GetTextureDependency(int)
│   └── Obtém dependência de textura
│
├── ❌ GetDependentTexture(int)
│   └── Obtém textura dependente
│
├── ❌ RequestMotionGroupAnims()
│   └── Requisita animações do grupo
│
├── ❌ GetPlayerMotionGroupToLoad()
│   └── Grupo de movimento a carregar
│
└── ❌ GetDefaultPlayerMotionGroup()
    └── Grupo padrão
```

```
CClothesBuilder
├── ❌ CreateSkinnedClump(RpClump*, RwTexDictionary*, CPedClothesDesc&, CPedClothesDesc const*, bool)
│   └── Cria clump skinned com roupas
│
├── ❌ PreprocessClothesDesc(CPedClothesDesc&, bool)
│   └── Pré-processa descrição de roupas
│
├── ❌ ConstructTextures(RwTexDictionary*, uint*, float, float, float)
│   └── Constrói texturas
│
├── ❌ ConstructGeometryArray(RpGeometry**, uint*, float, float, float)
│   └── Constrói array de geometrias
│
├── ❌ ConstructGeometryAndSkinArrays(...)
│   └── Constrói geometria e skin
│
├── ❌ BlendGeometry(RpClump*, char const*, char const*, float, float)
│   └── Faz blend de geometria
│
├── ❌ BlendTextures(RwTexture*, RwTexture*, float, float, int)
│   └── Faz blend de texturas
│
├── ❌ CopyGeometry(RpClump*, char const*, char const*)
│   └── Copia geometria
│
├── ❌ CopyTexture(RwTexture*)
│   └── Copia textura
│
├── ❌ PutOnClothes(RwTexture*, RwTexture*, bool)
│   └── Aplica roupa
│
├── ❌ GetClothesTexture(uint, char const*)
│   └── Obtém textura de roupa
│
├── ❌ ReleaseGeometry(int)
│   └── Libera geometria
│
├── ❌ RequestGeometry(int, uint)
│   └── Requisita geometria
│
├── ❌ StoreBoneArray(RpClump*, int)
│   └── Armazena array de bones
│
├── ❌ DestroySkinArrays(RwMatrixWeights*, uint*)
│   └── Destroi arrays de skin
│
└── ❌ LoadCdDirectory()
    └── Carrega diretório do CD
```

---

## Resumo de Cobertura

| Categoria | Total | ✅ | ⚠️ | ❌ |
|-----------|:-----:|:--:|:--:|:--:|
| CPed Básico | 65 | 5 | 8 | 52 |
| CPlayerPed | 55 | 2 | 6 | 47 |
| CPedModelInfo | 10 | 0 | 0 | 10 |
| CAnimManager | 22 | 0 | 3 | 19 |
| CTaskManager | 13 | 3 | 0 | 10 |
| CClothes | 21 | 0 | 0 | 21 |
| **TOTAL** | **186** | **10** | **17** | **159** |

---

**Cobertura: ~15% do sistema completo de personagem**
