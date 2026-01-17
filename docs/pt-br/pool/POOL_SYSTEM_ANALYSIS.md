# Sistema de Sinuca (Pool) - GTA San Andreas

## Visão Geral

O sistema de sinuca do GTA San Andreas é um minigame completo que simula uma partida de sinuca (pool/bilhar) com física realista de colisão entre esferas, fricção de superfície e controles touch específicos para dispositivos móveis.

---

## Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                    SISTEMA DE SINUCA                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   OBJETOS    │    │    FÍSICA    │    │   COLISÃO    │      │
│  │              │    │              │    │              │      │
│  │ • Mesa       │    │ • CPhysical  │    │ • CColSphere │      │
│  │ • Bolas      │    │ • Fricção    │    │ • SphereSphere│     │
│  │ • Taco       │    │ • Velocidade │    │ • LineSphere │      │
│  │ • Triângulo  │    │ • Força      │    │ • Raycasting │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│          │                  │                   │               │
│          └──────────────────┼───────────────────┘               │
│                             ▼                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   WIDGETS    │    │   SCRIPTS    │    │    ÁUDIO     │      │
│  │              │    │              │    │              │      │
│  │ • English    │    │ • MiniGame   │    │ • Colisões   │      │
│  │ • BallInHand │    │ • InProgress │    │ • Tacada     │      │
│  │ • Pool       │    │ • Regras     │    │ • Encaçapa   │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Componentes Principais

### 1. Objetos do Jogo

#### Modelos de Sinuca (eModelID.h)

| Modelo | ID | Descrição |
|--------|-----|-----------|
| `MODEL_POOLCUE` | 338 | Taco de sinuca (arma) |
| `MODEL_K_POOLTABLESM` | 2964 | Mesa de sinuca pequena |
| `MODEL_K_POOLTRIANGLE01` | 2965 | Triângulo organizador |
| `MODEL_K_POOLBALLSTP01-07` | 2995-3001 | Bolas listradas (1-7) |
| `MODEL_K_POOLBALLSPT01-07` | 3002, 3100-3105 | Bolas sólidas |
| `MODEL_K_POOLBALLCUE` | 3003 | Bola branca (cue ball) |
| `MODEL_K_POOLBALL8` | 3106 | Bola 8 (preta) |
| `MODEL_K_POOLTABLEB` | 3094 | Mesa de sinuca grande |
| `MODEL_K_POOLQ` | 3122 | Taco alternativo |
| `MODEL_K_POOLQ2` | 3004 | Taco decorativo |

#### Variável Global

```cpp
// Dump 32-bit: 0x006aad08
// Dump 64-bit: 0x00876314 (offset em memória: 0x00886314)
MI_POOL_CUE_BALL  // Model ID da bola branca
```

---

### 2. Sistema de Física

A física da sinuca utiliza o sistema `CPhysical` do GTA SA, com funções específicas para simulação de colisão entre esferas.

#### Física de Movimento

```cpp
// Aplicação de força na tacada
CPhysical::ApplyMoveForce(CVector force);

// Força com ponto de aplicação (para efeito "english")
CPhysical::ApplyForce(CVector force, CVector point, bool);

// Velocidade e direção
CPhysical::ApplySpeed();
CPhysical::ApplyMoveSpeed();
CPhysical::ApplyTurnSpeed();
CPhysical::ApplyTurnForce(CVector force, CVector point);
```

#### Fricção de Mesa

```cpp
// Fricção geral
CPhysical::ApplyFriction();
CPhysical::ApplyFriction(float friction, CColPoint& colPoint);
CPhysical::ApplyFriction(CPhysical* other, float friction, CColPoint& colPoint);

// Força de fricção
CPhysical::ApplyFrictionMoveForce(CVector force);
CPhysical::ApplyFrictionTurnForce(CVector force, CVector point);
CPhysical::ApplyFrictionForce(CVector force, CVector point);

// Resistência do ar (mínima para bolas de sinuca)
CPhysical::ApplyAirResistance();
```

#### Gravidade e Controle

```cpp
CPhysical::ApplyGravity();
CPhysical::ProcessControl();
CPhysical::ProcessCollision();
CPhysical::SkipPhysics();
```

---

### 3. Sistema de Colisão

A sinuca depende fortemente do sistema de colisão esfera-esfera e esfera-linha.

#### CColSphere

```cpp
// Estrutura básica
class CColSphere {
    CVector m_vecCenter;    // Centro da esfera
    float   m_fRadius;      // Raio
    uint8   m_nMaterial;    // Material da superfície
    uint8   m_nFlags;       // Flags de colisão
    uint8   m_nLighting;    // Iluminação
};

// Métodos principais
void Set(float radius, CVector const& center, uint8 material, uint8 flags, uint8 lighting);
bool IntersectRay(CVector const& origin, CVector const& dir, CVector& hit1, CVector& hit2) const;
bool IntersectEdge(CVector const& p1, CVector const& p2, CVector& hit1, CVector& hit2) const;
bool IntersectPoint(CVector const& point) const;
bool IntersectSphere(CColSphere const& other) const;
```

#### Funções de Colisão (CCollision)

| Função | Descrição |
|--------|-----------|
| `TestSphereSphere` | Testa se duas esferas se tocam |
| `ProcessSphereSphere` | Processa colisão entre esferas com resposta |
| `TestLineSphere` | Testa se linha intercepta esfera |
| `ProcessLineSphere` | Processa interceptação linha-esfera |
| `SphereCastVsSphere` | Raycast de esfera contra esfera |
| `SphereCastVsEntity` | Raycast de esfera contra entidade |

---

### 4. Widgets de Interface (Mobile)

#### Posições de Widget (TouchInterface.h)

| Constante | Valor | Uso |
|-----------|-------|-----|
| `WIDGET_POSITION_POOL_ENGLISH` | 0x16 | Controle de efeito na bola |
| `WIDGET_POSITION_POOL` | 0x81 | Widget principal de sinuca |

#### IDs de Widget

| Constante | Valor | Uso |
|-----------|-------|-----|
| `WIDGET_POOL_ENGLISH` | 0x23 | Efeito "english" na tacada |
| `WIDGET_POOL_BALL_IN_HAND` | 0x24 | Posicionar bola branca |
| `WIDGET_POOL` | 0xB5 | Interface de sinuca |

---

### 5. Classes de Widget

#### CWidgetPool

Widget principal que gerencia a interface do minigame.

```cpp
class CWidgetPool {
public:
    CWidgetPool(char const* name, WidgetPosition const& pos);
    ~CWidgetPool();
    
    void Update();
    void Draw();
    void SetWidgetInfo2(int, int, int, int, char*, char*);
    char* GetName();
};
```

| Função | Offset 32-bit | Offset 64-bit |
|--------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetPool::SetWidgetInfo2` | 0x002be9d5 | 0x0037ecd4 |
| `CWidgetPool::GetName` | 0x002bea6d | 0x0037edb0 |
| `CWidgetPool::~CWidgetPool` | 0x002bea5d | 0x0037ed8c |

#### CWidgetRegionPoolEnglish

Controla o efeito de "english" (rotação aplicada na bola branca).

```cpp
class CWidgetRegionPoolEnglish {
public:
    CWidgetRegionPoolEnglish(char const* name, WidgetPosition const& pos);
    ~CWidgetRegionPoolEnglish();
    
    void GetWidgetValue2(float* x, float* y);  // Obtém offset do toque
    bool IsTouched(CVector2D* touchPos);
    char* GetName();
};
```

| Função | Offset 32-bit | Offset 64-bit |
|--------|---------------|---------------|
| `CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish` | 0x002c1e29 | 0x00382814 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolEnglish::IsTouched` | 0x002c1e7d | 0x00382890 |
| `CWidgetRegionPoolEnglish::GetName` | 0x002c1ea9 | 0x003828d0 |
| `CWidgetRegionPoolEnglish::~CWidgetRegionPoolEnglish` | 0x002c1e99 | 0x003828ac |

#### CWidgetRegionPoolBallInHand

Permite ao jogador posicionar a bola branca após falta.

```cpp
class CWidgetRegionPoolBallInHand {
public:
    CWidgetRegionPoolBallInHand(char const* name, WidgetPosition const& pos);
    ~CWidgetRegionPoolBallInHand();
    
    void GetWidgetValue2(float* x, float* y);
    void SetWidgetValue3(float x, float y, float z);  // Define posição da bola
    bool IsTouched(CVector2D* touchPos);
    char* GetName();
};
```

| Função | Offset 32-bit | Offset 64-bit |
|--------|---------------|---------------|
| `CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand` | 0x002c1c91 | 0x00382650 |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |
| `CWidgetRegionPoolBallInHand::IsTouched` | 0x002c1d65 | 0x0038272c |
| `CWidgetRegionPoolBallInHand::GetName` | 0x002c1e19 | 0x00382808 |
| `CWidgetRegionPoolBallInHand::~CWidgetRegionPoolBallInHand` | 0x002c1e07 | 0x003827e4 |

---

### 6. Limites da Mesa

```cpp
// Variáveis globais que definem os limites físicos da mesa
// Usadas para detectar quando bolas saem do limite

// Dump 32-bit
CWorld::SnookerTableMin  // 0x0096b9e8 - Canto mínimo (X, Y, Z)
CWorld::SnookerTableMax  // 0x0096b9dc - Canto máximo (X, Y, Z)

// Dump 64-bit
CWorld::SnookerTableMin  // 0x00bdcb14
CWorld::SnookerTableMax  // 0x00bdcb08
```

---

### 7. Controle de Minigame

```cpp
// Flag global que indica se um minigame está ativo
CTheScripts::bMiniGameInProgress  // 32-bit: 0x00819d80 | 64-bit: 0x009ff3a0

// Flag para exibir mensagens de ajuda fora do minigame
CTheScripts::bDisplayNonMiniGameHelpMessages  // 32-bit: 0x00819d81 | 64-bit: 0x009ff3a1
```

---

### 8. Arma - Taco de Sinuca

O taco também funciona como arma corpo-a-corpo.

```cpp
// eWeaponType.h
WEAPON_POOL_CUE = 7  // Slot do taco como arma

// common.h
#define WEAPON_MODEL_POOLSTICK 338  // Modelo 3D do taco
```

---

## Fluxo de Gameplay

```
┌─────────────────────────────────────────────────────────────────┐
│                    FLUXO DO MINIGAME                            │
└─────────────────────────────────────────────────────────────────┘

[1. INÍCIO]
    │
    ▼
[Jogador interage com mesa de sinuca]
    │
    ▼
[CTheScripts::bMiniGameInProgress = true]
    │
    ▼
[2. POSICIONAMENTO] ◄─────────────────────────────────────┐
    │                                                      │
    ▼                                                      │
[Widgets de sinuca ativados]                               │
    │                                                      │
    ▼                                                      │
[3. MIRA E FORÇA]                                          │
    │                                                      │
    ├── CWidgetPool::Update()                              │
    ├── CWidgetRegionPoolEnglish::GetWidgetValue2()        │
    │   (Obtém efeito english)                             │
    │                                                      │
    ▼                                                      │
[4. TACADA]                                                │
    │                                                      │
    ├── CPhysical::ApplyForce() na bola branca             │
    ├── Direção + Força + English                          │
    │                                                      │
    ▼                                                      │
[5. SIMULAÇÃO FÍSICA]                                      │
    │                                                      │
    ├── Loop de física:                                    │
    │   ├── CPhysical::ApplySpeed()                        │
    │   ├── CPhysical::ApplyFriction()                     │
    │   ├── CCollision::TestSphereSphere()                 │
    │   ├── CCollision::ProcessSphereSphere()              │
    │   └── Verificar limites (SnookerTableMin/Max)        │
    │                                                      │
    ▼                                                      │
[6. VERIFICAR RESULTADO]                                   │
    │                                                      │
    ├── Bola encaçapada? ───► Atualizar pontuação          │
    ├── Bola branca encaçapada? ───► Falta                 │
    │   └── CWidgetRegionPoolBallInHand ativo              │
    ├── Bola 8 encaçapada prematuramente? ───► Derrota     │
    │                                                      │
    ▼                                                      │
[7. PRÓXIMO TURNO]                                         │
    │                                                      │
    ├── Alternar jogador (se multiplayer)                  │
    └──────────────────────────────────────────────────────┘

[8. FIM DO JOGO]
    │
    ├── Bola 8 encaçapada corretamente ───► Vitória
    │
    ▼
[CTheScripts::bMiniGameInProgress = false]
```

---

## Física da Tacada com English

O sistema de "english" permite aplicar rotação lateral na bola branca, afetando sua trajetória após colisões.

```
┌─────────────────────────────────────────────────────────────────┐
│                    EFEITO ENGLISH                               │
└─────────────────────────────────────────────────────────────────┘

        Vista superior da bola branca:
        
              [TOP]
                │
     [LEFT] ────┼──── [RIGHT]
                │
            [BOTTOM]

    Toque no centro:  Tacada reta, sem efeito
    Toque à esquerda: Bola curva para esquerda após colisão
    Toque à direita:  Bola curva para direita após colisão
    Toque acima:      "Follow" - bola continua após colisão
    Toque abaixo:     "Draw" - bola retorna após colisão

    Implementação:
    CWidgetRegionPoolEnglish::GetWidgetValue2(&offsetX, &offsetY);
    
    // Aplicar força com offset do centro
    CVector forcePoint = ballCenter + CVector(offsetX, offsetY, 0);
    CPhysical::ApplyForce(force, forcePoint, true);
```

---

## Tabela de Offsets Completa

### Física (CPhysical)

| Função | Offset 32-bit | Offset 64-bit | Descrição |
|--------|---------------|---------------|-----------|
| `ApplyForce` | 0x003fd619 | 0x004e0f00 | Aplica força com ponto |
| `ApplyMoveForce` | 0x003fd4d9 | 0x004e0de0 | Aplica força de movimento |
| `ApplyTurnForce` | 0x003fd541 | 0x004e0e28 | Aplica força de rotação |
| `ApplySpeed` | 0x003fda95 | 0x004e1300 | Aplica velocidade |
| `ApplyMoveSpeed` | 0x003fd86d | 0x004e1104 | Aplica velocidade linear |
| `ApplyTurnSpeed` | 0x003fd8d5 | 0x004e116c | Aplica velocidade angular |
| `ApplyGravity` | 0x003fe785 | 0x004e1e74 | Aplica gravidade |
| `ApplyFriction` | 0x003fe819 | 0x004e1f34 | Aplica fricção geral |
| `ApplyFriction (colPoint)` | 0x003fe2a9 | 0x004e1a50 | Fricção com ponto |
| `ApplyFriction (physical)` | 0x00405f5d | 0x004e9248 | Fricção entre físicos |
| `ApplyFrictionMoveForce` | 0x003feb0d | 0x004e2218 | Força de fricção linear |
| `ApplyFrictionTurnForce` | 0x003feb75 | 0x004e2260 | Força de fricção angular |
| `ApplyFrictionForce` | 0x003fec71 | 0x004e2358 | Força de fricção geral |
| `ApplyAirResistance` | 0x003fe9dd | 0x004e2104 | Resistência do ar |
| `ApplyCollision` | 0x00401b75 | 0x004e5598 | Colisão com entidade |
| `ApplyCollision (physical)` | 0x00401e81 | 0x004e583c | Colisão entre físicos |
| `ApplyCollisionAlt` | 0x00403a6d | 0x004e7108 | Colisão alternativa |
| `ApplySoftCollision` | 0x004040ed | 0x004e76fc | Colisão suave |
| `ProcessControl` | 0x003fedcd | 0x004e2668 (nearby) | Processa controle |
| `ProcessCollision` | 0x003fef49 | 0x004e2668 | Processa colisão |
| `CheckCollision` | 0x003ff655 | 0x004e2e30 | Verifica colisão |
| `TestCollision` | 0x004019bd | 0x004e532c | Testa colisão |
| `GetSpeed` | 0x003fd775 | 0x004e1038 | Obtém velocidade |

### Colisão de Esferas (CCollision / CColSphere)

| Função | Offset 32-bit | Offset 64-bit | Descrição |
|--------|---------------|---------------|-----------|
| `CColSphere::Set` | 0x002e1cc5 | 0x003a4d0c | Define esfera |
| `CColSphere::IntersectRay` | 0x002e1ce1 | 0x003a4d30 | Intercepta raio |
| `CColSphere::IntersectEdge` | 0x002e1e03 | 0x003a4e1c | Intercepta aresta |
| `CColSphere::IntersectPoint` | 0x002e1ff5 | 0x003a5010 | Intercepta ponto |
| `CColSphere::IntersectSphere` | 0x002e2035 | 0x003a504c | Intercepta esfera |
| `TestSphereSphere` | 0x002d9a19 (nearby) | 0x0039c648 | Testa esfera-esfera |
| `ProcessSphereSphere` | 0x002d9a19 | 0x0039c68c | Processa esfera-esfera |
| `TestLineSphere` | 0x002dabbd | 0x0039d5c4 | Testa linha-esfera |
| `ProcessLineSphere` | 0x002db031 | 0x0039d8e8 | Processa linha-esfera |
| `SphereCastVsSphere` | 0x002e0135 | 0x003a2e40 | Raycast esfera-esfera |
| `SphereCastVsEntity` | 0x002deb51 | 0x003a1660 | Raycast esfera-entidade |

### Widgets de Sinuca

| Função | Offset 32-bit | Offset 64-bit |
|--------|---------------|---------------|
| `CWidgetPool::CWidgetPool` | 0x002be525 | 0x0037e784 |
| `CWidgetPool::Update` | 0x002be56d | 0x0037e7e0 |
| `CWidgetPool::Draw` | 0x002be581 | 0x0037e804 |
| `CWidgetPool::SetWidgetInfo2` | 0x002be9d5 | 0x0037ecd4 |
| `CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish` | 0x002c1e29 | 0x00382814 |
| `CWidgetRegionPoolEnglish::GetWidgetValue2` | 0x002c1e4d | 0x0038284c |
| `CWidgetRegionPoolEnglish::IsTouched` | 0x002c1e7d | 0x00382890 |
| `CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand` | 0x002c1c91 | 0x00382650 |
| `CWidgetRegionPoolBallInHand::GetWidgetValue2` | 0x002c1cb9 | 0x00382688 |
| `CWidgetRegionPoolBallInHand::SetWidgetValue3` | 0x002c1d81 | 0x00382748 |
| `CWidgetRegionPoolBallInHand::IsTouched` | 0x002c1d65 | 0x0038272c |

### Variáveis Globais

| Variável | Offset 32-bit | Offset 64-bit |
|----------|---------------|---------------|
| `MI_POOL_CUE_BALL` | 0x006aad08 | 0x00876314 |
| `CWorld::SnookerTableMin` | 0x0096b9e8 | 0x00bdcb14 |
| `CWorld::SnookerTableMax` | 0x0096b9dc | 0x00bdcb08 |
| `CTheScripts::bMiniGameInProgress` | 0x00819d80 | 0x009ff3a0 |
| `CTheScripts::bDisplayNonMiniGameHelpMessages` | 0x00819d81 | 0x009ff3a1 |

---

## Implementação no Projeto

### Status de Implementação

| Componente | Status | Notas |
|------------|--------|-------|
| Modelos de bolas | ✅ | Definidos em eModelID.h |
| Taco como arma | ✅ | WEAPON_POOL_CUE em eWeaponType.h |
| Widget IDs | ✅ | Definidos em TouchInterface.h |
| CPhysical | ⚠️ | Parcialmente via hooks |
| CColSphere | ⚠️ | Parcialmente via hooks |
| CWidgetPool | ❌ | Não implementado |
| CWidgetRegionPoolEnglish | ❌ | Não implementado |
| CWidgetRegionPoolBallInHand | ❌ | Não implementado |
| Limites da mesa | ❌ | Não implementado |

### Arquivos Relevantes do Projeto

```
app/src/main/cpp/samp/game/
├── Enums/
│   ├── eModelID.h          # IDs dos modelos de sinuca
│   ├── eWeaponType.h       # WEAPON_POOL_CUE
│   ├── eSurfaceType.h      # SURFACE_P_POOLSIDE
│   └── HIDMapping.h        # Mapeamentos de input
├── Widgets/
│   └── TouchInterface.h    # Widgets de sinuca
├── Entity/
│   └── Object.h            # CObjectGta (base das bolas)
├── object.h                # CObject wrapper
└── common.h                # WEAPON_MODEL_POOLSTICK
```

---

## Considerações para SA-MP

### Desafios de Implementação Multiplayer

1. **Sincronização de Física**
   - Múltiplas bolas com física independente
   - Colisões em cadeia (efeito dominó)
   - Fricção e velocidade precisam ser consistentes

2. **Estrutura de Rede Sugerida**

```cpp
struct PoolGameState {
    uint16_t player1ID;
    uint16_t player2ID;
    uint8_t  currentPlayer;     // 0 ou 1
    uint8_t  gameType;          // 8-ball, 9-ball, etc.
    
    struct BallState {
        uint8_t  ballID;        // 0-15 (0 = cue ball)
        CVector  position;
        CVector  velocity;
        bool     pocketed;
    } balls[16];
    
    uint8_t  player1Balls;      // Bitmask de bolas do P1
    uint8_t  player2Balls;      // Bitmask de bolas do P2
    bool     ball8Assignable;   // Se bola 8 pode ser atribuída
};
```

3. **Validação Server-Side**
   - Verificar tacadas válidas
   - Calcular física no servidor
   - Broadcast de estado para clientes

---

## Referências

- Dumps: `dumps_libGTASA_32and64/`
- Código fonte: `app/src/main/cpp/samp/game/`
- Arquivos de widget: `FrontEnd/WidgetPool.cpp`, `FrontEnd/WidgetRegionPoolBallInHand.cpp`, `FrontEnd/WidgetRegionPoolEnglish.cpp`
