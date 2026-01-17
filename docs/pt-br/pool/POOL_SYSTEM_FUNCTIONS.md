# Sistema de Sinuca - Árvore de Funções Detalhada

## Legenda de Status

| Símbolo | Significado |
|---------|-------------|
| ✅ | Implementado no projeto |
| ⚠️ | Parcialmente implementado (via hooks) |
| ❌ | Não implementado |

---

## 1. Objetos de Sinuca (CObject)

```
CObject ⚠️
├── Ciclo de Vida
│   ├── CObject::CObject() ⚠️
│   │   └── Construtor padrão
│   ├── CObject::CObject(int modelId, bool) ⚠️
│   │   └── Cria objeto com modelo específico
│   ├── CObject::CObject(CDummyObject*) ⚠️
│   │   └── Cria objeto a partir de dummy
│   ├── CObject::~CObject() ⚠️
│   │   └── Destrutor
│   ├── CObject::Init() ⚠️
│   │   └── Inicializa objeto
│   └── CObject::Create(int modelId, bool) ⚠️
│       └── Cria e retorna novo objeto
│
├── Renderização
│   ├── CObject::PreRender() ⚠️
│   │   └── Preparação antes de renderizar
│   ├── CObject::Render() ⚠️
│   │   └── Renderiza objeto
│   ├── CObject::SetupLighting() ⚠️
│   │   └── Configura iluminação
│   └── CObject::RemoveLighting(bool) ⚠️
│       └── Remove iluminação
│
├── Física e Controle
│   ├── CObject::ProcessControl() ⚠️
│   │   └── Processa lógica de controle
│   ├── CObject::ProcessControlLogic() ⚠️
│   │   └── Lógica específica de controle
│   ├── CObject::SetIsStatic(bool) ⚠️
│   │   └── Define se objeto é estático
│   └── CObject::SpecialEntityPreCollisionStuff(...) ⚠️
│       └── Preparação especial pré-colisão
│
├── Gerenciamento
│   ├── CObject::CanBeDeleted() ⚠️
│   │   └── Verifica se pode ser deletado
│   ├── CObject::CanBeTargetted() ⚠️
│   │   └── Verifica se pode ser mirado
│   ├── CObject::SetObjectTargettable(bool) ⚠️
│   │   └── Define se pode ser mirado
│   ├── CObject::AddToControlCodeList() ⚠️
│   │   └── Adiciona à lista de controle
│   └── CObject::RemoveFromControlCodeList() ⚠️
│       └── Remove da lista de controle
│
└── Persistência
    ├── CObject::Save() ⚠️
    │   └── Salva estado do objeto
    └── CObject::Load() ⚠️
        └── Carrega estado do objeto
```

---

## 2. Sistema de Física (CPhysical)

```
CPhysical ⚠️
├── Ciclo de Vida
│   ├── CPhysical::CPhysical() ⚠️
│   │   └── Construtor
│   ├── CPhysical::~CPhysical() ⚠️
│   │   └── Destrutor
│   ├── CPhysical::Add() ⚠️
│   │   └── Adiciona ao mundo
│   └── CPhysical::Remove() ⚠️
│       └── Remove do mundo
│
├── Aplicação de Forças
│   ├── CPhysical::ApplyForce(CVector, CVector, bool) ⚠️
│   │   └── Aplica força com ponto de aplicação
│   │       Usado para efeito "english" na tacada
│   ├── CPhysical::ApplyMoveForce(CVector) ⚠️
│   │   └── Aplica força de movimento linear
│   │       Principal função para tacadas
│   ├── CPhysical::ApplyTurnForce(CVector, CVector) ⚠️
│   │   └── Aplica força de rotação
│   │       Gera spin na bola
│   ├── CPhysical::ApplyFrictionMoveForce(CVector) ⚠️
│   │   └── Aplica força de fricção linear
│   ├── CPhysical::ApplyFrictionTurnForce(CVector, CVector) ⚠️
│   │   └── Aplica força de fricção rotacional
│   └── CPhysical::ApplyFrictionForce(CVector, CVector) ⚠️
│       └── Aplica força de fricção geral
│
├── Velocidade
│   ├── CPhysical::ApplySpeed() ⚠️
│   │   └── Aplica velocidade acumulada
│   ├── CPhysical::ApplyMoveSpeed() ⚠️
│   │   └── Aplica velocidade linear
│   ├── CPhysical::ApplyTurnSpeed() ⚠️
│   │   └── Aplica velocidade angular
│   └── CPhysical::GetSpeed(CVector) ⚠️
│       └── Obtém velocidade em ponto
│
├── Fricção
│   ├── CPhysical::ApplyFriction() ⚠️
│   │   └── Aplica fricção geral
│   │       Desacelera bolas na mesa
│   ├── CPhysical::ApplyFriction(float, CColPoint&) ⚠️
│   │   └── Aplica fricção com ponto de colisão
│   └── CPhysical::ApplyFriction(CPhysical*, float, CColPoint&) ⚠️
│       └── Aplica fricção entre dois físicos
│
├── Gravidade e Resistência
│   ├── CPhysical::ApplyGravity() ⚠️
│   │   └── Aplica gravidade
│   │       Mantém bolas na mesa
│   └── CPhysical::ApplyAirResistance() ⚠️
│       └── Aplica resistência do ar
│       Mínima para bolas de sinuca
│
├── Colisão
│   ├── CPhysical::CheckCollision() ⚠️
│   │   └── Verifica colisões
│   ├── CPhysical::CheckCollision_SimpleCar() ⚠️
│   │   └── Verificação simplificada
│   ├── CPhysical::TestCollision(bool) ⚠️
│   │   └── Testa colisão
│   ├── CPhysical::ApplyCollision(CEntity*, CColPoint&, float&) ⚠️
│   │   └── Aplica colisão com entidade
│   ├── CPhysical::ApplyCollision(CPhysical*, CColPoint&, float&, float&) ⚠️
│   │   └── Aplica colisão entre físicos
│   │       Usado para colisão bola-bola
│   ├── CPhysical::ApplyCollisionAlt(...) ⚠️
│   │   └── Colisão alternativa
│   ├── CPhysical::ApplySoftCollision(CEntity*, ...) ⚠️
│   │   └── Colisão suave com entidade
│   ├── CPhysical::ApplySoftCollision(CPhysical*, ...) ⚠️
│   │   └── Colisão suave entre físicos
│   ├── CPhysical::ApplySpringCollision(...) ⚠️
│   │   └── Colisão com mola (bordas da mesa)
│   ├── CPhysical::ApplySpringCollisionAlt(...) ⚠️
│   │   └── Colisão com mola alternativa
│   ├── CPhysical::ApplySpringDampening(...) ⚠️
│   │   └── Amortecimento de mola
│   ├── CPhysical::ApplySpringDampeningOld(...) ⚠️
│   │   └── Amortecimento legado
│   └── CPhysical::ApplyScriptCollision(...) ⚠️
│       └── Colisão via script
│
├── Processamento
│   ├── CPhysical::ProcessControl() ⚠️
│   │   └── Processa controle físico
│   ├── CPhysical::ProcessCollision() ⚠️
│   │   └── Processa colisões
│   ├── CPhysical::ProcessShift() ⚠️
│   │   └── Processa deslocamento
│   ├── CPhysical::ProcessEntityCollision(CEntity*, CColPoint*) ⚠️
│   │   └── Processa colisão com entidade
│   ├── CPhysical::ProcessCollisionSectorList(int, int) ⚠️
│   │   └── Processa lista de setores
│   ├── CPhysical::ProcessCollisionSectorList_SimpleCar(...) ⚠️
│   │   └── Versão simplificada
│   └── CPhysical::SkipPhysics() ⚠️
│       └── Pula processamento físico
│
├── Registros
│   ├── CPhysical::AddCollisionRecord(CEntity*) ⚠️
│   │   └── Registra colisão
│   ├── CPhysical::GetHasCollidedWith(CEntity*) ⚠️
│   │   └── Verifica se colidiu com entidade
│   ├── CPhysical::GetHasCollidedWithAnyObject() ⚠️
│   │   └── Verifica colisão com qualquer objeto
│   └── CPhysical::SetDamagedPieceRecord(...) ⚠️
│       └── Registra peça danificada
│
├── Anexação
│   ├── CPhysical::AttachEntityToEntity(...) ⚠️
│   │   └── Anexa entidade a outra
│   ├── CPhysical::PositionAttachedEntity() ⚠️
│   │   └── Posiciona entidade anexada
│   ├── CPhysical::DettachEntityFromEntity(...) ⚠️
│   │   └── Desanexa entidade
│   └── CPhysical::DettachAutoAttachedEntity() ⚠️
│       └── Desanexa automaticamente
│
└── Utilitários
    ├── CPhysical::GetBoundRect() ⚠️
    │   └── Obtém retângulo de limite
    ├── CPhysical::GetLightingTotal() ⚠️
    │   └── Obtém iluminação total
    ├── CPhysical::GetLightingFromCol(bool) ⚠️
    │   └── Obtém iluminação de colisão
    ├── CPhysical::CanPhysicalBeDamaged(...) ⚠️
    │   └── Verifica se pode ser danificado
    ├── CPhysical::AddToMovingList() ⚠️
    │   └── Adiciona à lista de móveis
    ├── CPhysical::RemoveFromMovingList() ⚠️
    │   └── Remove da lista de móveis
    ├── CPhysical::RemoveAndAdd() ⚠️
    │   └── Remove e adiciona novamente
    ├── CPhysical::RemoveRefsToEntity(CEntity*) ⚠️
    │   └── Remove referências
    ├── CPhysical::UnsetIsInSafePosition() ⚠️
    │   └── Desmarca posição segura
    └── CPhysical::PlacePhysicalRelativeToOtherPhysical(...) ⚠️
        └── Posiciona relativo a outro físico
```

---

## 3. Colisão de Esferas (CColSphere / CCollision)

```
CColSphere ⚠️
├── Configuração
│   └── CColSphere::Set(float, CVector const&, uint8, uint8, uint8) ⚠️
│       └── Define raio, centro, material, flags, iluminação
│
└── Interseção
    ├── CColSphere::IntersectRay(CVector const&, CVector const&, CVector&, CVector&) ⚠️
    │   └── Testa interseção com raio
    │       Usado para mirar tacada
    ├── CColSphere::IntersectEdge(CVector const&, CVector const&, CVector&, CVector&) ⚠️
    │   └── Testa interseção com aresta
    │       Colisão com bordas da mesa
    ├── CColSphere::IntersectPoint(CVector const&) ⚠️
    │   └── Testa se ponto está dentro
    │       Detecção de encaçapamento
    └── CColSphere::IntersectSphere(CColSphere const&) ⚠️
        └── Testa interseção com outra esfera
        Colisão bola-bola

CCollision ⚠️
├── Testes Esfera-Esfera
│   ├── CCollision::TestSphereSphere(CColSphere const&, CColSphere const&) ⚠️
│   │   └── Testa se duas esferas se tocam
│   │       Retorna bool
│   └── CCollision::ProcessSphereSphere(CColSphere const&, CColSphere const&, CColPoint&, float&) ⚠️
│       └── Processa colisão entre esferas
│       Calcula ponto de contato e profundidade
│
├── Testes Linha-Esfera
│   ├── CCollision::TestLineSphere(CColLine const&, CColSphere const&) ⚠️
│   │   └── Testa se linha intercepta esfera
│   │       Usado para raycast de mira
│   └── CCollision::ProcessLineSphere(CColLine const&, CColSphere const&, CColPoint&, float&) ⚠️
│       └── Processa interseção linha-esfera
│       Calcula ponto de impacto
│
├── SphereCast
│   ├── CCollision::SphereCastVsSphere(CColSphere*, CColSphere*, CColSphere*) ⚠️
│   │   └── Raycast de esfera contra esfera
│   │       Previsão de trajetória
│   ├── CCollision::SphereCastVsEntity(CColSphere*, CColSphere*, CEntity*) ⚠️
│   │   └── Raycast de esfera contra entidade
│   ├── CCollision::SphereCastVsBBox(CColSphere*, CColSphere*, CColBox*) ⚠️
│   │   └── Raycast de esfera contra caixa
│   │       Colisão com bordas
│   ├── CCollision::SphereCastVsCaches(...) ⚠️
│   │   └── Raycast usando cache
│   └── CCollision::SphereCastVersusVsPoly(...) ⚠️
│       └── Raycast contra polígono
│
├── Esfera-Box/Triângulo
│   ├── CCollision::ProcessSphereBox(CColSphere const&, CColBox const&, CColPoint&, float&) ⚠️
│   │   └── Processa colisão esfera-caixa
│   │       Colisão com bordas da mesa
│   ├── CCollision::TestSphereTriangle(...) ⚠️
│   │   └── Testa esfera contra triângulo
│   └── CCollision::ProcessSphereTriangle(...) ⚠️
│       └── Processa colisão com triângulo
│
├── Entidade
│   └── CCollision::SphereVsEntity(CColSphere*, CEntity*) ⚠️
│       └── Testa esfera contra entidade
│
└── Câmera
    ├── CCollision::BuildCacheOfCameraCollision(...) ⚠️
    │   └── Constrói cache para câmera
    ├── CCollision::CheckCameraCollisionBuildings(...) ⚠️
    │   └── Verifica colisão com prédios
    ├── CCollision::CheckCameraCollisionVehicles(...) ⚠️
    │   └── Verifica colisão com veículos
    ├── CCollision::CheckCameraCollisionObjects(...) ⚠️
    │   └── Verifica colisão com objetos
    ├── CCollision::CameraConeCastVsWorldCollision(...) ⚠️
    │   └── Cone cast da câmera
    └── CCollision::GetBoundingBoxFromTwoSpheres(...) ⚠️
        └── Obtém bounding box de duas esferas
```

---

## 4. Widgets de Sinuca

```
CWidgetPool ❌
├── Ciclo de Vida
│   ├── CWidgetPool::CWidgetPool(char const*, WidgetPosition const&) ❌
│   │   └── Construtor com nome e posição
│   └── CWidgetPool::~CWidgetPool() ❌
│       └── Destrutor
│
├── Atualização
│   ├── CWidgetPool::Update() ❌
│   │   └── Atualiza estado do widget
│   │       Chamado a cada frame
│   └── CWidgetPool::Draw() ❌
│       └── Renderiza widget
│
└── Configuração
    ├── CWidgetPool::SetWidgetInfo2(int, int, int, int, char*, char*) ❌
    │   └── Define informações do widget
    └── CWidgetPool::GetName() ❌
        └── Retorna nome do widget

CWidgetRegionPoolEnglish ❌
├── Ciclo de Vida
│   ├── CWidgetRegionPoolEnglish::CWidgetRegionPoolEnglish(char const*, WidgetPosition const&) ❌
│   │   └── Construtor
│   └── CWidgetRegionPoolEnglish::~CWidgetRegionPoolEnglish() ❌
│       └── Destrutor
│
├── Interação
│   ├── CWidgetRegionPoolEnglish::GetWidgetValue2(float*, float*) ❌
│   │   └── Obtém offset X/Y do toque
│   │       Determina efeito english na bola
│   │       Valores: -1.0 a 1.0 para cada eixo
│   └── CWidgetRegionPoolEnglish::IsTouched(CVector2D*) ❌
│       └── Verifica se está sendo tocado
│
└── Utilitário
    └── CWidgetRegionPoolEnglish::GetName() ❌
        └── Retorna nome do widget

CWidgetRegionPoolBallInHand ❌
├── Ciclo de Vida
│   ├── CWidgetRegionPoolBallInHand::CWidgetRegionPoolBallInHand(char const*, WidgetPosition const&) ❌
│   │   └── Construtor
│   └── CWidgetRegionPoolBallInHand::~CWidgetRegionPoolBallInHand() ❌
│       └── Destrutor
│
├── Interação
│   ├── CWidgetRegionPoolBallInHand::GetWidgetValue2(float*, float*) ❌
│   │   └── Obtém posição do toque
│   │       Para posicionar bola branca
│   ├── CWidgetRegionPoolBallInHand::SetWidgetValue3(float, float, float) ❌
│   │   └── Define posição da bola
│   │       Usado após falta
│   └── CWidgetRegionPoolBallInHand::IsTouched(CVector2D*) ❌
│       └── Verifica se está sendo tocado
│
└── Utilitário
    └── CWidgetRegionPoolBallInHand::GetName() ❌
        └── Retorna nome do widget
```

---

## 5. Controle de Minigame

```
CTheScripts ⚠️
├── bMiniGameInProgress ⚠️
│   └── Flag: minigame está ativo
│       true durante partida de sinuca
│
└── bDisplayNonMiniGameHelpMessages ⚠️
    └── Flag: exibir mensagens externas
    false durante minigame

CWorld ❌
├── SnookerTableMin ❌
│   └── CVector: canto mínimo da mesa
│       Limite inferior X, Y, Z
│
└── SnookerTableMax ❌
    └── CVector: canto máximo da mesa
    Limite superior X, Y, Z
```

---

## 6. Superfícies e Materiais

```
eSurfaceType ✅
└── SURFACE_P_POOLSIDE = 127 ✅
    └── Tipo de superfície: borda da mesa de sinuca
    Define fricção e som de colisão

SurfaceInfos_c ⚠️
└── SurfaceInfos_c::GetFrictionEffect(unsigned int) ⚠️
    └── Obtém efeito de fricção para superfície
    Usado para desacelerar bolas
```

---

## 7. Arma - Taco de Sinuca

```
eWeaponType ✅
└── WEAPON_POOL_CUE = 7 ✅
    └── Slot do taco de sinuca como arma melee

eModelID ✅
├── MODEL_POOLCUE = 338 ✅
│   └── Modelo 3D do taco
│
└── MODEL_K_POOLQ = 3122 ✅
    └── Modelo alternativo do taco

common.h ✅
└── WEAPON_MODEL_POOLSTICK = 338 ✅
    └── Define modelo do taco para sistema de armas
```

---

## Resumo de Implementação

| Categoria | Total | ✅ | ⚠️ | ❌ |
|-----------|-------|-----|-----|-----|
| CObject | 20 | 0 | 20 | 0 |
| CPhysical | 45 | 0 | 45 | 0 |
| CColSphere | 4 | 0 | 4 | 0 |
| CCollision | 15 | 0 | 15 | 0 |
| CWidgetPool | 6 | 0 | 0 | 6 |
| CWidgetRegionPoolEnglish | 5 | 0 | 0 | 5 |
| CWidgetRegionPoolBallInHand | 6 | 0 | 0 | 6 |
| CTheScripts | 2 | 0 | 2 | 0 |
| CWorld | 2 | 0 | 0 | 2 |
| Enums/Defines | 6 | 6 | 0 | 0 |
| **Total** | **111** | **6** | **86** | **19** |
