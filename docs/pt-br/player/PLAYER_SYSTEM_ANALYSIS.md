# Sistema de Personagem/Player - GTA San Andreas

## Índice
1. [Visão Geral](#1-visão-geral)
2. [Hierarquia de Classes](#2-hierarquia-de-classes)
3. [Estrutura CPedGTA](#3-estrutura-cpedgta)
4. [Sistema de Modelos](#4-sistema-de-modelos)
5. [Sistema de Animação](#5-sistema-de-animação)
6. [Sistema de Áudio](#6-sistema-de-áudio)
7. [Fluxo de Criação](#7-fluxo-de-criação)
8. [Implementação no Projeto](#8-implementação-no-projeto)

---

## 1. Visão Geral

O sistema de personagem no GTA San Andreas é composto por múltiplas camadas que gerenciam:
- **Modelo 3D** (Skinned mesh com esqueleto)
- **Animações** (Blend de animações via hierarquia HAnim)
- **Física** (Colisão, gravidade, movimento)
- **IA/Tasks** (Sistema de tarefas para comportamento)
- **Áudio** (Voz, passos, efeitos)
- **Roupas** (Sistema dinâmico do CJ)

### Componentes Principais:
- `CEntity` → `CPhysical` → `CPed` → `CPlayerPed`
- `CPedModelInfo` - Informações do modelo de ped
- `CAnimManager` / `CAnimBlendAssociation` - Sistema de animação
- `CAEPedAudioEntity` / `CAEPedSpeechAudioEntity` - Áudio
- `CClothes` / `CClothesBuilder` - Sistema de roupas
- `CTaskManager` - Gerenciador de tarefas/ações
- `CPedIntelligence` - IA e tomada de decisões

---

## 2. Hierarquia de Classes

```
CPlaceable
    └── CEntity
            └── CPhysical
                    └── CPed (CPedGTA)
                            └── CPlayerPed
```

### Tamanhos das Estruturas:
| Classe | 32-bit | 64-bit |
|--------|--------|--------|
| CPlaceable | 0x18 | 0x20 |
| CEntity | 0x38 | 0x48 |
| CPhysical | 0x138 | 0x190 |
| CPedGTA | 0x7A4 | 0x988 |

---

## 3. Estrutura CPedGTA

### 3.1 Campos de Áudio
```cpp
uint8_t m_PedAudioEntity[0x15C/0x1A8];       // Áudio geral do ped
uint8_t m_PedSpeechAudioEntity[0x100/0x130]; // Fala/voz
uint8_t m_PedWeaponAudioEntity[0xA8/0xC8];   // Sons de arma
```

### 3.2 Campos de IA e Estado
```cpp
CPedIntelligence*  m_pIntelligence;   // Sistema de IA
CPlayerPedData*    m_pPlayerData;     // Dados específicos do player
ePedCreatedBy      m_nCreatedBy;      // Quem criou (GAME, MISSION, etc)
ePedState          m_nPedState;       // Estado atual (IDLE, ATTACK, etc)
eMoveState         m_nMoveState;      // Estado de movimento
```

### 3.3 Campos de Física/Colisão
```cpp
CStoredCollPoly    m_storedCollPoly;           // Polígono de colisão
CPhysical*         m_pGroundPhysical;          // Objeto no chão
CVector            m_vecGroundOffset;          // Offset do chão
CVector            m_vecGroundNormal;          // Normal do chão
CEntityGTA*        m_pEntityStandingOn;        // Entidade em pé sobre
float              m_fHitHeadHeight;           // Altura da cabeça
```

### 3.4 Campos de Saúde
```cpp
float m_fHealth;     // Vida atual
float m_fMaxHealth;  // Vida máxima
float m_fArmour;     // Colete
```

### 3.5 Campos de Rotação/Movimento
```cpp
float m_fCurrentRotation;   // Rotação atual
float m_fAimingRotation;    // Rotação de mira
float m_fHeadingChangeRate; // Taxa de mudança de direção
float m_fMoveAnim;          // Velocidade da animação de movimento
```

### 3.6 Campos de Veículo
```cpp
CVehicleGTA* pVehicle;          // Veículo atual
CVehicleGTA* m_VehDeadInFrontOf; // Veículo onde morreu na frente
```

### 3.7 Campos de Animação
```cpp
std::array<AnimBlendFrameData*, TOTAL_PED_NODES> m_apBones; // Array de bones
AssocGroupId m_nAnimGroup;           // Grupo de animação atual
CVector2D    m_vecAnimMovingShiftLocal; // Shift local da animação
CVector2D    m_vecAnimMovingShift;   // Shift global
CPedIK       m_pedIK;                // Sistema de Inverse Kinematics
```

### 3.8 Flags do Ped (16 bytes de flags)
```cpp
// 1º byte
bool bIsStanding : 1;          // Está em pé
bool bWasStanding : 1;         // Estava em pé
bool bIsLooking : 1;           // Está olhando
bool bIsRestoringLook : 1;     // Restaurando olhar
bool bIsAimingGun : 1;         // Mirando arma
bool bIsRestoringGun : 1;      // Restaurando posição da arma
bool bCanPointGunAtTarget : 1; // Pode apontar arma
bool bIsTalking : 1;           // Está falando

// 2º byte
bool bInVehicle : 1;           // Está em veículo
bool bIsInTheAir : 1;          // Está no ar
bool bIsLanding : 1;           // Está aterrissando
bool bHitSomethingLastFrame : 1; // Colidiu no último frame
bool bRenderPedInCar : 1;      // Renderizar dentro do carro
bool bUpdateAnimHeading : 1;   // Atualizar heading por animação
bool bRemoveHead : 1;          // Aguardando remover cabeça

// 3º byte
bool bFiringWeapon : 1;        // Disparando arma
bool bPedIsBleeding : 1;       // Ped está sangrando
bool bStopAndShoot : 1;        // Parar e atirar
bool bIsPedDieAnimPlaying : 1; // Animação de morte tocando
bool bStayInSamePlace : 1;     // Ficar no mesmo lugar
bool bBeingChasedByPolice : 1; // Perseguido pela polícia

// ... (mais flags para duck, drown, fight, etc)
```

---

## 4. Sistema de Modelos

### 4.1 CPedModelInfo
Armazena informações do modelo de ped (herda de CClumpModelInfo).

```cpp
class CPedModelInfo : public CClumpModelInfo {
    void Init();                              // Inicializa
    void SetClump(RpClump* clump);           // Define modelo 3D
    void DeleteRwObject();                    // Remove objeto RW
    void SetFaceTexture(RwTexture*);         // Textura do rosto
    void IncrementVoice();                    // Próxima voz
    void CreateHitColModelSkinned(RpClump*); // Cria colisão skinned
    void AnimatePedColModelSkinned(RpClump*);// Anima colisão
    ModelInfoType GetModelType();            // Retorna tipo (PED)
};
```

### 4.2 Bones do Ped (ePedNode)
```cpp
enum ePedNode : int32 {
    PED_NODE_UPPER_TORSO     = 1,   // Torso superior
    PED_NODE_HEAD            = 2,   // Cabeça
    PED_NODE_LEFT_ARM        = 3,   // Braço esquerdo
    PED_NODE_RIGHT_ARM       = 4,   // Braço direito
    PED_NODE_LEFT_HAND       = 5,   // Mão esquerda
    PED_NODE_RIGHT_HAND      = 6,   // Mão direita (arma)
    PED_NODE_LEFT_LEG        = 7,   // Perna esquerda
    PED_NODE_RIGHT_LEG       = 8,   // Perna direita
    PED_NODE_LEFT_FOOT       = 9,   // Pé esquerdo
    PED_NODE_RIGHT_FOOT      = 10,  // Pé direito
    PED_NODE_RIGHT_LOWER_LEG = 11,  // Canela direita
    PED_NODE_LEFT_LOWER_LEG  = 12,  // Canela esquerda
    PED_NODE_LEFT_LOWER_ARM  = 13,  // Antebraço esquerdo
    PED_NODE_RIGHT_LOWER_ARM = 14,  // Antebraço direito
    PED_NODE_LEFT_CLAVICLE   = 15,  // Clavícula esquerda
    PED_NODE_RIGHT_CLAVICLE  = 16,  // Clavícula direita
    PED_NODE_NECK            = 17,  // Pescoço
    PED_NODE_JAW             = 18,  // Mandíbula
    TOTAL_PED_NODES          = 19
};
```

### 4.3 Partes do Corpo (ePedPieceTypes)
```cpp
enum ePedPieceTypes {
    PED_PIECE_UNKNOWN   = 0,
    PED_PIECE_TORSO     = 3,  // Tronco
    PED_PIECE_ASS       = 4,  // Quadril
    PED_PIECE_LEFT_ARM  = 5,  // Braço esquerdo
    PED_PIECE_RIGHT_ARM = 6,  // Braço direito
    PED_PIECE_LEFT_LEG  = 7,  // Perna esquerda
    PED_PIECE_RIGHT_LEG = 8,  // Perna direita
    PED_PIECE_HEAD      = 9   // Cabeça
};
```

---

## 5. Sistema de Animação

### 5.1 CAnimManager
Gerencia todas as animações do jogo.

```cpp
class CAnimManager {
    // Estruturas estáticas
    static CAnimBlendHierarchy ms_aAnimations[]; // Todas animações
    static CAnimBlendAssocGroup ms_aAnimAssocGroups[]; // Grupos
    static CAnimBlock ms_aAnimBlocks[]; // Blocos de animação
    
    // Adicionar animação
    static CAnimBlendAssociation* AddAnimation(RpClump*, AssocGroupId, AnimationId);
    static CAnimBlendAssociation* AddAnimation(RpClump*, CAnimBlendHierarchy*, int);
    
    // Blend de animações
    static CAnimBlendAssociation* BlendAnimation(RpClump*, AssocGroupId, AnimationId, float);
    static CAnimBlendAssociation* BlendAnimation(RpClump*, CAnimBlendHierarchy*, int, float);
    
    // Obter animação
    static CAnimBlendHierarchy* GetAnimation(char const*, CAnimBlock const*);
    static CAnimBlendAssociation* GetAnimAssociation(AssocGroupId, AnimationId);
    
    // Gerenciamento de blocos
    static void LoadAnimFile(char const*);
    static void LoadAnimFile(RwStream*, bool, char const (*)[32]);
    static void RegisterAnimBlock(char const*);
    static void RemoveAnimBlock(int);
    static void AddAnimBlockRef(int);
    static void RemoveAnimBlockRef(int);
    
    // Inicialização
    static void Initialise();
    static void Shutdown();
    static void CreateAnimAssocGroups();
    static void UncompressAnimation(CAnimBlendHierarchy*);
};
```

### 5.2 CAnimBlendAssociation
Representa uma instância de animação em um ped.

### 5.3 RpHAnimHierarchy
Sistema de hierarquia de bones do RenderWare.

```cpp
// Funções principais
RpHAnimHierarchy* RpHAnimHierarchyCreate(int, uint*, int*, RpHAnimHierarchyFlag, int);
void RpHAnimHierarchyDestroy(RpHAnimHierarchy*);
void RpHAnimHierarchyAttach(RpHAnimHierarchy*);
void RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy*);
RwMatrix* RpHAnimHierarchyGetMatrixArray(RpHAnimHierarchy*);
int RpHAnimIDGetIndex(RpHAnimHierarchy*, int);
```

### 5.4 CPedIK (Inverse Kinematics)
Sistema que ajusta os bones em tempo real para mira, olhar, etc.

```cpp
class CPedIK {
    uint32 m_flags;          // Flags de estado
    float  m_torsoOrien;     // Orientação do torso
    float  m_headOrien;      // Orientação da cabeça
    
    void RotateTorso(RwFrame*, float, bool); // Rotaciona torso
    void PitchForSlope();                     // Ajusta para terreno
    void GetWorldMatrix(RwFrame*, RwMatrix*); // Matriz do bone
};
```

---

## 6. Sistema de Áudio

### 6.1 CAEPedAudioEntity
Gerencia sons gerais do ped (passos, impactos, etc).

```cpp
class CAEPedAudioEntity {
    void Initialise(CEntity*);          // Inicializa
    void Terminate();                    // Finaliza
    void Service();                      // Atualização por frame
    
    // Eventos de som
    void AddAudioEvent(int, float, float, CPhysical*, uint8, int, uint32);
    void HandleFootstepEvent(int, float, float, uint8); // Passos
    void HandleLandingEvent(int);        // Aterrissagem
    void HandlePedHit(int, CPhysical*, uint8, float, uint32); // Impacto
    void HandlePedSwing(int, int, uint32); // Balanço de ataque
    void HandleSkateEvent(int, float, float); // Skate
    
    // JetPack
    void TurnOnJetPack();
    void TurnOffJetPack();
    void UpdateJetPack(float, float);
    void StopJetPackSound();
    
    // Outros
    void PlayWindRush(float, float);     // Vento (queda)
    void PlayShirtFlap(float, float);    // Roupa balançando
    void HandleSwimSplash(int);          // Splash na água
    void HandleSwimWake(int);            // Rastro na água
};
```

### 6.2 CAEPedSpeechAudioEntity
Gerencia a fala/voz do ped.

```cpp
class CAEPedSpeechAudioEntity {
    void Initialise(CEntity*);
    void Terminate();
    
    // Fala
    void AddSayEvent(int, short, uint32, float, uint8, uint8, uint8);
    void AddScriptSayEvent(int, int, uint8, uint8, uint8);
    void PlayLoadedSound();
    void StopCurrentSpeech();
    void LoadAndPlaySpeech(uint32);
    
    // Controle
    void EnablePedSpeech();
    void DisablePedSpeech(short);
    bool GetPedTalking();
    
    // Tipos e mood
    int GetPedType();
    int GetAudioPedType(char*);
    int GetSexFromModel(int);
    int GetCurrentCJMood();
    int GetNextMoodToUse(short);
    int GetVoiceForMood(short);
    void SetUpConversation();
    
    // Variáveis estáticas
    static int s_nCJBasicMood;
    static int s_nCJGangBanging;
    static int s_nCJWellDressed;
};
```

---

## 7. Fluxo de Criação

### 7.1 Criação de Player Local
```
CPlayerPed::CPlayerPed(playerNum, skin, x, y, z, rotation)
    │
    ├──▶ CPlayerPed::SetupPlayerPed(playerNum)
    │         └── Configura estruturas internas do player
    │
    ├──▶ CPlayerPed::DeactivatePlayerPed(playerNum)
    │         └── Desativa temporariamente
    │
    ├──▶ FindPlayerPed(playerNum)
    │         └── Obtém ponteiro do ped criado
    │
    ├──▶ CTheScripts::ClearSpaceForMissionEntity(pos, ped)
    │         └── Limpa espaço para o ped
    │
    ├──▶ CPlayerPed::ReactivatePlayerPed(playerNum)
    │         └── Reativa o player
    │
    ├──▶ CWorld::Add(ped)
    │         └── Adiciona ao mundo
    │
    ├──▶ SetModelIndex(skin)
    │         │
    │         ├──▶ CStreaming::RequestModel(skinId, flags)
    │         ├──▶ CStreaming::LoadAllRequestedModels(false)
    │         └──▶ CPed::SetModelIndex(modelId) [nativo]
    │
    └──▶ SetMatrix(pos, rotation)
             └── Define posição e rotação inicial
```

### 7.2 Criação de Player Remoto (NPC/Outros Jogadores)
```
CPlayerPed::CPlayerPed(playerNum, skin, x, y, z, rotation)
    │
    ├──▶ SetupPlayerPed → DeactivatePlayerPed → FindPlayerPed
    │
    ├──▶ ClearSpaceForMissionEntity → ReactivatePlayerPed
    │
    ├──▶ CWorld::Add(ped)
    │
    ├──▶ SetModelIndex(skin)
    │
    ├──▶ Configurações de imunidade
    │         └── ScriptCommand(&set_actor_immunities, ...)
    │
    ├──▶ bNeverEverTargetThisPed = true
    │         └── Não pode ser alvo de auto-aim
    │
    └──▶ Configurações de dinheiro, rotação, etc.
```

---

## 8. Implementação no Projeto

### 8.1 Funções Implementadas no APK

| Status | Função | Descrição |
|:------:|--------|-----------|
| ✅ | `CPlayerPed::CPlayerPed()` | Construtor padrão (player local) |
| ✅ | `CPlayerPed::CPlayerPed(int,int,float,float,float,float)` | Construtor completo |
| ✅ | `CPlayerPed::~CPlayerPed()` | Destrutor |
| ✅ | `CPlayerPed::IsInVehicle()` | Verifica se está em veículo |
| ✅ | `CPlayerPed::IsAPassenger()` | Verifica se é passageiro |
| ✅ | `CPlayerPed::SetModelIndex()` | Define modelo/skin |
| ✅ | `CPlayerPed::SetHealth()` / `GetHealth()` | Vida |
| ✅ | `CPlayerPed::SetArmour()` / `GetArmour()` | Colete |
| ✅ | `CPlayerPed::GiveWeapon()` | Dá arma ao ped |
| ✅ | `CPlayerPed::ClearWeapons()` | Remove todas as armas |
| ✅ | `CPlayerPed::SetAmmo()` | Define munição |
| ✅ | `CPlayerPed::GetCurrentWeapon()` | Obtém arma atual |
| ✅ | `CPlayerPed::SetRotation()` | Define rotação |
| ✅ | `CPlayerPed::SetTargetRotation()` | Define rotação alvo |
| ✅ | `CPlayerPed::ApplyAnimation()` | Aplica animação |
| ✅ | `CPlayerPed::GetBonePosition()` | Posição de bone |
| ✅ | `CPlayerPed::PutDirectlyInVehicle()` | Coloca em veículo |
| ✅ | `CPlayerPed::EnterVehicle()` | Entra em veículo |
| ✅ | `CPlayerPed::ExitCurrentVehicle()` | Sai do veículo |
| ✅ | `CPlayerPed::RemoveFromVehicleAndPutAt()` | Remove do veículo |
| ✅ | `CPlayerPed::FireInstant()` | Dispara arma |
| ✅ | `CPlayerPed::TogglePlayerControllable()` | Controle do jogador |
| ✅ | `CPlayerPed::SetInterior()` | Define interior |
| ✅ | `CPlayerPed::StartJetpack()` / `StopJetpack()` | Jetpack |
| ✅ | `CPlayerPed::StartDancing()` / `StopDancing()` | Dança |
| ✅ | `CPlayerPed::HandsUp()` / `HasHandsUp()` | Mãos ao alto |
| ✅ | `CPlayerPed::AttachObject()` / `DeattachObject()` | Objetos anexados |
| ✅ | `CPlayerPed::SetWeaponSkill()` | Define skill de arma |
| ✅ | `CPlayerPed::ToggleCellphone()` | Celular |
| ✅ | `CPlayerPed::StartPissing()` / `StopPissing()` | Ação especial |
| ✅ | `CPlayerPed::SetDrunkLevel()` / `ProcessDrunk()` | Sistema de embriaguez |

### 8.2 Offsets Nativos Importantes

| Função | 32-bit | 64-bit |
|--------|--------|--------|
| `CPed::Initialise` | `0x49FA19` | `0x595898` |
| `CPed::SetModelIndex` | `0x49FAD5` | `0x595954` |
| `CPed::ProcessControl` | `0x4A2541` | `0x598730` |
| `CPed::PreRender` | - | `0x59C240` |
| `CPed::PreRenderAfterTest` | `0x4A5981` | `0x59C254` |
| `CPed::UpdatePosition` | `0x4A1999` | `0x597B04` |
| `CPed::CalculateNewVelocity` | `0x4A1365` | `0x597518` |
| `CPed::CalculateNewOrientation` | `0x4A1DC5` | - |
| `CPed::SetPedState` | `0x4A1EA5` | `0x597FF0` |
| `CPed::IsPedInControl` | `0x4A18D1` | `0x597A0C` |
| `CPed::GetBonePosition` | `0x4A4B0D` | `0x59AEE4` |
| `CPed::Teleport` | - | `0x59DD90` |
| `CPlayerPed::SetupPlayerPed` | `0x4C39A5` | `0x5C0FD4` |
| `CPlayerPed::DeactivatePlayerPed` | `0x4C3AD5` | `0x5C1140` |
| `CPlayerPed::ReactivatePlayerPed` | `0x4C3AED` | `0x5C1158` |
| `CPlayerPed::RemovePlayerPed` | `0x4C3A61` | `0x5C10B4` |
| `CPlayerPed::ProcessControl` | `0x4C47E9` | `0x5C2088` |
| `CPlayerPed::SetRealMoveAnim` | `0x4C3B05` | `0x5C1170` |
| `CPlayerPed::ProcessAnimGroups` | `0x4C5E61` | `0x5C37E0` |
| `CPlayerPed::SetInitialState` | `0x4C37B5` | `0x5C0D50` |
| `CPlayerPed::FindWeaponLockOnTarget` | `0x4C6D65` | `0x5C494C` |
| `CPlayerPed::ProcessPlayerWeapon` | `0x4C682F` | `0x5C4310` |
| `CPlayerPed::ProcessWeaponSwitch` | `0x4C5919` | `0x5C3278` |
| `CPlayerPed::HandlePlayerBreath` | `0x4C56F5` | `0x5C3010` |
| `CPlayerPed::HandleSprintEnergy` | `0x4C5809` | `0x5C3140` |
| `CPlayerPed::Load` | `0x4851E9` | `0x5774B8` |
| `CPlayerPed::Save` | `0x485163` | `0x577390` |

---

## Diagrama: Ciclo de Vida do Ped

```
┌─────────────────────────────────────────────────────────────┐
│                    CICLO DE VIDA - CPed                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐               │
│  │  Create  │───▶│ Process  │───▶│  Delete  │               │
│  │  (Init)  │    │ (Update) │    │(Cleanup) │               │
│  └──────────┘    └────┬─────┘    └──────────┘               │
│                       │                                      │
│                       ▼                                      │
│  ┌────────────────────────────────────────────────────┐     │
│  │               ProcessControl() - Por Frame          │     │
│  ├────────────────────────────────────────────────────┤     │
│  │  1. UpdatePosition()    → Física, gravidade         │     │
│  │  2. ProcessBuoyancy()   → Flutuação na água        │     │
│  │  3. CalculateNewVelocity() → Velocidade            │     │
│  │  4. CalculateNewOrientation() → Rotação            │     │
│  │  5. ProcessEntityCollision() → Colisões            │     │
│  │  6. CTaskManager::ManageTasks() → IA/Comportamento │     │
│  │  7. UpdateMovement() → Animação de movimento       │     │
│  │  8. PreRenderAfterTest() → Preparar render         │     │
│  └────────────────────────────────────────────────────┘     │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

**Documento criado para análise do sistema de personagem/player do GTA San Andreas.**
**Baseado nos dumps da libGTASA versão 2.1 (32-bit e 64-bit).**
