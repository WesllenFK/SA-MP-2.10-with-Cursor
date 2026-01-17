# Sistema de Basquete - GTA San Andreas

## Índice
1. [Visão Geral](#1-visão-geral)
2. [Arquitetura do Sistema](#2-arquitetura-do-sistema)
3. [Sistema de Objetos](#3-sistema-de-objetos)
4. [Sistema de Física](#4-sistema-de-física)
5. [Sistema de Animação/Tasks](#5-sistema-de-animaçãotasks)
6. [Sistema de UI/Widgets](#6-sistema-de-uiwidgets)
7. [Sistema de Scripts](#7-sistema-de-scripts)
8. [Fluxo de Gameplay](#8-fluxo-de-gameplay)
9. [Implementação no Projeto](#9-implementação-no-projeto)

---

## 1. Visão Geral

O sistema de basquete no GTA San Andreas é um **minigame** que utiliza múltiplos sistemas do jogo:

- **CObject** - A bola de basquete como objeto físico
- **CPhysical** - Física da bola (gravidade, colisão, bounce)
- **CTaskSimpleHoldEntity** - Segurar a bola
- **CTaskSimpleThrowControl/Projectile** - Arremessar a bola
- **Widgets** - Controles de UI para mobile
- **CTheScripts** - Flag de minigame ativo

### Componentes Principais:
- `CObject` / `CObjectGta` - Objeto da bola
- `CPhysical` - Sistema de física
- `CTaskSimpleHoldEntity` - Task para segurar objetos
- `CTaskSimpleThrowControl` - Task de controle de arremesso
- `CTaskSimpleThrowProjectile` - Task de arremesso
- `CWidgetButtonBasketballShoot` - Botão de arremesso (mobile)
- `CWidgetRegionBasketBallJump` - Região de pulo (mobile)

---

## 2. Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SISTEMA DE BASQUETE - GTA SA                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐           │
│  │   CObject    │───▶│  CPhysical   │───▶│  Collision   │           │
│  │ (Basketball) │    │   (Física)   │    │   (Cesta)    │           │
│  └──────────────┘    └──────────────┘    └──────────────┘           │
│         │                   │                   │                    │
│         ▼                   ▼                   ▼                    │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐           │
│  │ Hold Entity  │    │    Apply     │    │   Script     │           │
│  │    Task      │    │Force/Gravity │    │   (Score)    │           │
│  └──────────────┘    └──────────────┘    └──────────────┘           │
│         │                                                            │
│         ▼                                                            │
│  ┌──────────────┐    ┌──────────────┐                               │
│  │    Throw     │    │   Widgets    │                               │
│  │    Task      │◀───│  (UI Touch)  │                               │
│  └──────────────┘    └──────────────┘                               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Sistema de Objetos

### 3.1 Modelo da Bola
```cpp
// ID do modelo da bola de basquete
enum eModelID {
    MODEL_BASKETBALL = 2114,  // Modelo 3D da bola
};

// Variável global com ID do modelo
MI_BASKETBALL // 0x6AAD06 (32-bit) / 0x876312 (64-bit)
```

### 3.2 CObjectGta (Estrutura do Objeto)

```cpp
class CObjectGta : public CPhysical {
    uintptr*    m_pControlCodeList;     // Lista de código de controle
    uint8       m_nObjectType;          // Tipo (OBJECT_GAME, OBJECT_TEMPORARY, etc)
    uint8       m_nBonusValue;          // Valor de bônus
    uint16      m_wCostValue;           // Custo
    
    // Flags do objeto (32 bits)
    union {
        struct {
            uint32 bIsPickup : 1;               // É pickup
            uint32 bHasBrokenGlass : 1;         // Vidro quebrado
            uint32 bIsExploded : 1;             // Explodido
            uint32 bIsTargetable : 1;           // Pode ser alvo
            uint32 bIsBroken : 1;               // Quebrado
            uint32 bIsLiftable : 1;             // Pode ser levantado
            uint32 bCanBeAttachedToMagnet : 1;  // Pode anexar a imã
            // ...
        } objectFlags;
        uint32 m_nObjectFlags;
    };
    
    uint8       m_nColDamageEffect;     // Efeito de dano de colisão
    float       m_fHealth;              // Vida do objeto
    float       m_fScale;               // Escala
    uintptr     *m_pObjectInfo;         // Info do objeto
    uintptr     *m_pFire;               // Ponteiro de fogo
    // ...
};
// Tamanho: 0x184 (32-bit) / 0x1F8 (64-bit)
```

### 3.3 eObjectType
```cpp
enum eObjectType {
    OBJECT_UNKNOWN         = 0,
    OBJECT_GAME            = 1,  // Objeto do jogo
    OBJECT_MISSION         = 2,  // Objeto de missão
    OBJECT_TEMPORARY       = 3,  // Temporário (partes voando)
    OBJECT_TYPE_CUTSCENE   = 4,  // Cutscene
    OBJECT_TYPE_DECORATION = 5,  // Decoração (mãos, projéteis)
    OBJECT_MISSION2        = 6
};
```

### 3.4 Funções Principais do CObject

| Função | Descrição |
|--------|-----------|
| `CObject::Create(int, bool)` | Cria objeto por ID do modelo |
| `CObject::Create(CDummyObject*)` | Cria de dummy object |
| `CObject::Init()` | Inicializa estruturas |
| `CObject::ProcessControl()` | Update por frame |
| `CObject::ProcessControlLogic()` | Lógica de controle |
| `CObject::ObjectDamage(...)` | Processa dano |
| `CObject::Teleport(CVector, uint8)` | Teleporta |
| `CObject::SetIsStatic(bool)` | Define estático/dinâmico |
| `CObject::CanBeDeleted()` | Pode ser removido |
| `CObject::CanBeTargetted()` | Pode ser alvo |
| `CObject::Explode()` | Explode objeto |
| `CObject::TryToExplode()` | Tenta explodir |
| `CObject::Render()` | Renderiza |
| `CObject::PreRender()` | Pré-renderiza |

---

## 4. Sistema de Física

### 4.1 CPhysical - Funções de Física

A bola de basquete usa o sistema de física do `CPhysical` para movimento realista.

```cpp
class CPhysical : public CEntity {
    // Aplicar forças
    void ApplyForce(CVector force, CVector point, bool);
    void ApplyMoveForce(CVector force);
    void ApplyTurnForce(CVector force, CVector point);
    
    // Velocidade
    void ApplySpeed();
    void ApplyMoveSpeed();
    void ApplyTurnSpeed();
    
    // Gravidade e resistência
    void ApplyGravity();
    void ApplyAirResistance();
    void ApplyFriction();
    void ApplyFriction(float, CColPoint&);
    void ApplyFriction(CPhysical*, float, CColPoint&);
    void ApplyFrictionForce(CVector, CVector);
    void ApplyFrictionMoveForce(CVector);
    void ApplyFrictionTurnForce(CVector, CVector);
    
    // Colisão
    void ApplyCollision(CEntity*, CColPoint&, float&);
    void ApplyCollision(CPhysical*, CColPoint&, float&, float&);
    void ApplyCollisionAlt(CEntity*, CColPoint&, float&, CVector&, CVector&);
    void ApplySoftCollision(CEntity*, CColPoint&, float&);
    void ApplySoftCollision(CPhysical*, CColPoint&, float&, float&);
    
    // Molas (para bouncing)
    void ApplySpringCollision(float, CVector&, CVector&, float, float, float&);
    void ApplySpringCollisionAlt(float, CVector&, CVector&, float, float, CVector&, float&);
    void ApplySpringDampening(float, float, CVector&, CVector&, CVector&);
    void ApplySpringDampeningOld(float, float, CVector&, CVector&, CVector&);
    
    // Script
    void ApplyScriptCollision(CVector, float, float, CVector*);
};
```

### 4.2 Física da Bola

Quando a bola é arremessada:

1. **ApplyMoveForce** - Aplica força inicial do arremesso
2. **ApplyGravity** - Gravidade puxa a bola para baixo
3. **ApplyAirResistance** - Resistência do ar desacelera
4. **ApplyCollision** - Colisão com cesta, chão, etc
5. **ApplySpringCollision** - Efeito de bounce

---

## 5. Sistema de Animação/Tasks

### 5.1 Tasks Relacionadas

O basquete usa o sistema de Tasks para gerenciar ações do ped.

#### CTaskSimpleHoldEntity
Gerencia segurar a bola.

```cpp
class CTaskSimpleHoldEntity : public CTaskSimple {
    // Verifica se pode arremessar
    bool CanThrowEntity();
};
```

#### CTaskSimpleThrowControl
Controla o arremesso.

```cpp
class CTaskSimpleThrowControl : public CTaskSimple {
    CEntity* m_pTargetEntity;   // Alvo (cesta)
    CVector  m_vecTarget;       // Posição alvo
    
    bool ProcessPed(CPed*);
    bool MakeAbortable(CPed*, int, CEvent const*);
};
```

#### CTaskSimpleThrowProjectile
Executa o arremesso físico.

```cpp
class CTaskSimpleThrowProjectile : public CTaskSimple {
    CEntity* m_pProjectile;     // Objeto sendo arremessado
    CVector  m_vecTarget;       // Posição alvo
    
    bool ProcessPed(CPed*);
    bool MakeAbortable(CPed*, int, CEvent const*);
    void ControlThrow(bool, CEntity*, CVector*);
    void StartAnim(CPed*);
    
    // Callback de animação
    static void FinishAnimThrowProjectileCB(CAnimBlendAssociation*, void*);
};
```

### 5.2 CPedIntelligence

```cpp
class CPedIntelligence {
    // Obtém task de arremesso ativa
    CTask* GetTaskThrow() const;
};
```

---

## 6. Sistema de UI/Widgets

### 6.1 Widgets de Basquete (Mobile)

Para controle touch no mobile, o jogo tem widgets específicos.

#### CWidgetButtonBasketballShoot
Botão de arremesso.

```cpp
class CWidgetButtonBasketballShoot : public CWidget {
    CWidgetButtonBasketballShoot(char const*, WidgetPosition const&);
    ~CWidgetButtonBasketballShoot();
    
    void Update();      // Atualiza estado do botão
    char* GetName();    // Retorna nome do widget
};
```

| Função | 32-bit | 64-bit |
|--------|--------|--------|
| Constructor | `0x2B51F1` | `0x37461C` |
| Update | `0x2B5219` | `0x374658` |
| GetName | `0x2B53B9` | `0x374814` |

#### CWidgetRegionBasketBallJump
Região de pulo no basquete.

```cpp
class CWidgetRegionBasketBallJump : public CWidget {
    CWidgetRegionBasketBallJump(char const*, WidgetPosition const&);
    ~CWidgetRegionBasketBallJump();
    
    void Update();
    void OnInitialTouch();
    bool IsTouched(CVector2D*);
    char* GetName();
};
```

| Função | 32-bit | 64-bit |
|--------|--------|--------|
| Constructor | `0x2BFF19` | `0x380588` |
| Update | `0x2BFF4D` | `0x3805D0` |
| OnInitialTouch | `0x2BFF69` | `0x380600` |
| IsTouched | `0x2BFF7B` | `0x380628` |
| GetName | `0x2BFF91` | `0x380654` |

### 6.2 Posições de Widget

```cpp
enum WidgetPosition {
    WIDGET_POSITION_BASKETBALL_SHOOT = 0x34,
    WIDGET_POSITION_BASKETBALL_TRICK = 0x35,
    WIDGET_POSITION_SPRINT_AND_BASKETBALL_JUMP = 0x77,
};

enum WidgetId {
    WIDGET_BASKETBALL_SHOOT = 0x57,
    WIDGET_BASKETBALL_TRICK = 0x58,
    WIDGET_BASKETBALL_JUMP = 0xA9,
};
```

### 6.3 HID Mapping

```cpp
enum HIDMapping {
    HID_MAPPING_BASKETBALL_SHOOT = 0x5E,  // Mapeamento de input
};
```

---

## 7. Sistema de Scripts

### 7.1 Flag de Minigame

O jogo usa flags para indicar que um minigame está ativo.

```cpp
class CTheScripts {
    static bool bMiniGameInProgress;              // Minigame ativo
    static bool bDisplayNonMiniGameHelpMessages;  // Mostrar ajuda
};
```

| Variável | 32-bit | 64-bit |
|----------|--------|--------|
| `bMiniGameInProgress` | `0x819D80` | `0x9FF3A0` |
| `bDisplayNonMiniGameHelpMessages` | `0x819D81` | `0x9FF3A1` |

### 7.2 CScriptedBrainTaskStore

Armazena tasks scriptadas para peds.

```cpp
class CScriptedBrainTaskStore {
    static Entry ms_entries[]; // Array de entries
    
    static void SetTask(CPed*, CTask*);  // Define task
    static CTask* GetTask(CPed const*);  // Obtém task
    static void Clear(CPed*);            // Limpa task
    static void Clear(CTask*);           // Limpa por task
};
```

| Função | 32-bit | 64-bit |
|--------|--------|--------|
| `SetTask` | `0x4F197D` | `0x5F94A0` |
| `GetTask` | `0x4F19F9` | `0x5F955C` |
| `Clear(CPed*)` | `0x4EE425` | `0x5F4870` |
| `Clear(CTask*)` | `0x4F1A25` | `0x5F9598` |

---

## 8. Fluxo de Gameplay

### 8.1 Iniciando Minigame

```
Jogador entra na área de basquete
    │
    ├──▶ CTheScripts::bMiniGameInProgress = true
    │
    ├──▶ CObject::Create(MODEL_BASKETBALL, ...)
    │         └── Cria a bola de basquete
    │
    ├──▶ Mostra widgets de basquete
    │         ├── WIDGET_BASKETBALL_SHOOT
    │         ├── WIDGET_BASKETBALL_TRICK
    │         └── WIDGET_BASKETBALL_JUMP
    │
    └──▶ Ativa scripts do minigame
```

### 8.2 Fluxo de Arremesso

```
Jogador pressiona botão de arremesso
    │
    ├──▶ CWidgetButtonBasketballShoot::Update()
    │         └── Detecta input
    │
    ├──▶ CTaskSimpleHoldEntity::CanThrowEntity()
    │         └── Verifica se pode arremessar
    │
    ├──▶ CTaskSimpleThrowControl criada
    │         └── Inicia controle do arremesso
    │
    ├──▶ CTaskSimpleThrowProjectile::StartAnim()
    │         └── Inicia animação de arremesso
    │
    ├──▶ CPhysical::ApplyMoveForce()
    │         └── Aplica força inicial na bola
    │
    ├──▶ CPhysical::ApplyGravity() [por frame]
    │         └── Gravidade atua na bola
    │
    ├──▶ Colisão detectada com cesta
    │         │
    │         ├── Acertou: Script incrementa pontuação
    │         │
    │         └── Errou: Bola cai e pode ser pegada novamente
    │
    └──▶ CTaskSimpleThrowProjectile::FinishAnimThrowProjectileCB()
             └── Callback finaliza animação
```

### 8.3 Física da Bola Durante Voo

```
Por frame (CObject::ProcessControl):
    │
    ├──▶ CPhysical::ApplyGravity()
    │         └── F = m * g (gravidade para baixo)
    │
    ├──▶ CPhysical::ApplyAirResistance()
    │         └── Desaceleração por resistência do ar
    │
    ├──▶ CPhysical::ApplySpeed()
    │         └── Atualiza posição baseado em velocidade
    │
    └──▶ Se colisão:
             │
             ├──▶ CPhysical::ApplyCollision()
             │         └── Processa colisão
             │
             └──▶ CPhysical::ApplySpringCollision()
                      └── Bounce (quique)
```

---

## 9. Implementação no Projeto

### 9.1 Status de Implementação

| Componente | Status | Descrição |
|------------|:------:|-----------|
| CObject wrapper | ✅ | Wrapper básico implementado |
| CObjectGta struct | ✅ | Estrutura definida |
| Physics hooks | ⚠️ | Parcial (via native calls) |
| Widget classes | ❌ | Não implementado |
| Task classes | ❌ | Não implementado |
| Script flags | ❌ | Não implementado |

### 9.2 CObject no Projeto

O projeto tem uma classe `CObject` wrapper:

```cpp
class CObject {
    CPhysical*  m_pEntity;      // Entidade física
    uint32_t    m_dwGTAId;      // ID no pool do GTA
    
    CObject(int iModel, CVector vecPos, CVector vecRot, float fDrawDistance, uint8_t unk);
    ~CObject();
    
    void Process(float fElapsedTime);
    void SetRotation(CVector* vecRotation);
    void GetRotation(float* pfX, float* pfY, float* pfZ);
    void RotateMatrix(CVector vecRot);
    void ApplyMoveSpeed();
    float DistanceRemaining(RwMatrix* matPos);
    void MoveTo(float fX, float fY, float fZ, float fSpeed, ...);
    void StopMoving();
    void SetPos(float x, float y, float z);
    
    // Anexos
    void SetAttachedObject(uint16_t ObjectID, CVector* vecPos, CVector* vecRot, bool bSyncRotation);
    void SetAttachedVehicle(uint16_t VehicleID, CVector* vecPos, CVector* vecRot);
    void AttachToVehicle(CVehicle* pVehicle);
    void AttachToObject(CObject* pObject);
    bool AttachedToMovingEntity();
    
    // Material/Textura
    void SetMaterial(int iModel, int iIndex, char* txdname, char* texturename, uint32_t dwColor);
    void SetMaterialText(...);
    void ProcessMaterialText();
};
```

---

## Tabela de Offsets

### CObject

| Função | 32-bit | 64-bit | Descrição |
|--------|--------|--------|-----------|
| `CObject::CObject()` | `0x452C6D` | `0x53B828` | Construtor vazio |
| `CObject::CObject(int, bool)` | `0x453011` | `0x53BC64` | Por model ID |
| `CObject::CObject(CDummyObject*)` | `0x453059` | `0x53BCC4` | De dummy |
| `CObject::~CObject` | `0x4530FD` | `0x53BDA0` | Destrutor |
| `CObject::Init` | `0x452C99` | `0x53B868` | Inicializa |
| `CObject::Create(int, bool)` | `0x45343D` | `0x53C19C` | Cria por ID |
| `CObject::Create(CDummyObject*)` | `0x453661` | `0x53C4E4` | Cria de dummy |
| `CObject::ProcessControl` | `0x453825` | `0x53C790` | Update |
| `CObject::ProcessControlLogic` | `0x4543D5` | `0x53D434` | Lógica |
| `CObject::ObjectDamage` | `0x453F71` | `0x53CF40` | Dano |
| `CObject::Render` | `0x454F55` | `0x53E0F0` | Renderiza |
| `CObject::PreRender` | `0x454D0D` | `0x53DE48` | Pré-render |
| `CObject::Teleport` | `0x455BB9` | `0x53EEDC` | Teleporta |
| `CObject::SetIsStatic` | `0x455681` | `0x53E8A8` | Estático |
| `CObject::CanBeDeleted` | `0x4537F9` | `0x53C750` | Pode deletar |
| `CObject::Explode` | `0x4557A1` | `0x53EA24` | Explode |
| `CObject::TryToExplode` | `0x455775` | `0x53E9DC` | Tenta explodir |
| `CObject::SetObjectTargettable` | - | `0x53EE90` | Pode ser alvo |
| `CObject::Load` | `0x484FAD` | `0x577274` | Carrega |
| `CObject::Save` | `0x484F41` | `0x5771E4` | Salva |

### CPhysical (Relevante para bola)

| Função | 32-bit | 64-bit | Descrição |
|--------|--------|--------|-----------|
| `ApplyForce` | `0x3FD619` | - | Aplica força |
| `ApplyMoveForce` | `0x3FD4D9` | `0x4E0DE0` | Força de movimento |
| `ApplyTurnForce` | `0x3FD541` | `0x4E0E28` | Força de rotação |
| `ApplySpeed` | `0x3FDA95` | `0x4E1300` | Aplica velocidade |
| `ApplyMoveSpeed` | `0x3FD86D` | `0x4E1104` | Velocidade linear |
| `ApplyTurnSpeed` | `0x3FD8D5` | - | Velocidade angular |
| `ApplyGravity` | `0x3FE785` | `0x4E1E74` | Gravidade |
| `ApplyAirResistance` | `0x3FE9DD` | `0x4E2104` | Resistência do ar |
| `ApplyFriction` | `0x3FE819` | `0x4E1F34` | Fricção |
| `ApplyCollision` | `0x401B75` | `0x4E5598` | Colisão |
| `ApplySpringCollision` | `0x405A11` | `0x4E8DF4` | Bounce |

### Tasks de Throw

| Função | 32-bit | 64-bit | Descrição |
|--------|--------|--------|-----------|
| `CTaskSimpleHoldEntity::CanThrowEntity` | `0x5421C9` | - | Pode arremessar |
| `CTaskSimpleThrowControl::ProcessPed` | `0x4DEF91` | - | Processa |
| `CTaskSimpleThrowControl::MakeAbortable` | `0x4DEF4D` | - | Abortar |
| `CTaskSimpleThrowProjectile::ProcessPed` | `0x4DEAF9` | - | Processa |
| `CTaskSimpleThrowProjectile::ControlThrow` | `0x4DEE05` | - | Controla |
| `CTaskSimpleThrowProjectile::StartAnim` | `0x4DECB1` | - | Inicia anim |
| `CTaskSimpleThrowProjectile::FinishAnimCB` | `0x4DEE81` | - | Callback |

---

## Diagrama: Ciclo de Vida da Bola

```
┌─────────────────────────────────────────────────────────────┐
│                 CICLO DE VIDA - BOLA DE BASQUETE            │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐               │
│  │  Criar   │───▶│ Segurar  │───▶│ Arremesso│               │
│  │  Bola    │    │  (Hold)  │    │ (Throw)  │               │
│  └──────────┘    └────┬─────┘    └────┬─────┘               │
│                       │               │                      │
│                       ▼               ▼                      │
│               ┌──────────────────────────┐                  │
│               │     VOO DA BOLA          │                  │
│               │  (Physics cada frame)    │                  │
│               │  - Gravidade             │                  │
│               │  - Resistência do ar     │                  │
│               │  - Velocidade            │                  │
│               └───────────┬──────────────┘                  │
│                           │                                  │
│         ┌─────────────────┼─────────────────┐               │
│         ▼                 ▼                 ▼               │
│  ┌──────────┐      ┌──────────┐      ┌──────────┐          │
│  │  Acertou │      │   Chão   │      │  Saiu    │          │
│  │  Cesta   │      │ (Bounce) │      │  Área    │          │
│  └──────────┘      └──────────┘      └──────────┘          │
│         │                 │                 │               │
│         ▼                 ▼                 ▼               │
│  ┌──────────┐      ┌──────────┐      ┌──────────┐          │
│  │  Pontos  │      │  Pegar   │      │  Reset   │          │
│  │  +2/+3   │      │  Novamente│     │  Bola    │          │
│  └──────────┘      └──────────┘      └──────────┘          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

**Documento criado para análise do sistema de basquete do GTA San Andreas.**
**Baseado nos dumps da libGTASA versão 2.1 (32-bit e 64-bit).**
