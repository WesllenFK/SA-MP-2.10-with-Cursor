# Análise do Sistema de Carregamento de Modelos de Armas - GTA San Andreas

## Índice
1. [Visão Geral](#visão-geral)
2. [Arquitetura do Sistema](#arquitetura-do-sistema)
3. [Classes Principais](#classes-principais)
4. [Fluxo de Carregamento](#fluxo-de-carregamento)
5. [Estruturas de Dados](#estruturas-de-dados)
6. [Funções Nativas do Dump](#funções-nativas-do-dump)
7. [Implementação no Projeto](#implementação-no-projeto)
8. [Considerações Técnicas](#considerações-técnicas)

---

## 1. Visão Geral

O sistema de carregamento de modelos de armas no GTA San Andreas é baseado no **sistema de streaming** do jogo. Os modelos de armas são tratados como recursos DFF (modelos 3D) que são carregados sob demanda quando um ped precisa equipar uma arma.

### Componentes Principais:
- **CStreaming** - Sistema de streaming principal
- **CWeaponModelInfo** - Informações específicas de modelos de armas
- **CWeaponInfo** - Dados de gameplay das armas (dano, alcance, etc.)
- **CWeapon** - Instância de arma em uso
- **CPed::AddWeaponModel/RemoveWeaponModel** - Gerenciamento de modelo visual no ped

---

## 2. Arquitetura do Sistema

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SISTEMA DE ARMAS - GTA SA                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │  CStreaming  │───▶│CWeaponModel- │───▶│  RpClump     │          │
│  │  RequestModel│    │    Info      │    │ (Modelo 3D)  │          │
│  └──────────────┘    └──────────────┘    └──────────────┘          │
│         │                   │                   │                   │
│         ▼                   ▼                   ▼                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │   IMG File   │    │  CWeaponInfo │    │CPed::m_pWeap-│          │
│  │  (GTA3.IMG)  │    │(weapon.dat)  │    │  onObject    │          │
│  └──────────────┘    └──────────────┘    └──────────────┘          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Classes Principais

### 3.1 CWeapon (Estrutura)
```cpp
// Localização: app/src/main/cpp/samp/game/common.h
struct CWeapon
{
    eWeaponType dwType;        // Tipo da arma (enum)
    eWeaponState dwState;      // Estado atual da arma
    union {
        uint32_t dwAmmoInClip;
        uint32_t m_nAmmoInClip;
    };
    uint32_t dwAmmo;           // Munição total
    uint32_t m_nTimer;         // Timer interno
    bool m_bFirstPersonWeaponModeSelected;
    bool m_bDontPlaceInHand;
    uint8_t pad[2];
    uintptr_t *m_pWeaponFxSys; // Sistema de efeitos visuais
};
// Tamanho: 28 bytes (32-bit) / 32 bytes (64-bit)
```

### 3.2 CPedGTA - Campos Relacionados a Armas
```cpp
// Localização: app/src/main/cpp/samp/game/Entity/CPedGTA.h

// Ponteiros para objetos RenderWare
RpClump* *m_pWeaponObject;      // Objeto 3D da arma atual
RpClump* *m_pGunflashObject;    // Objeto do flash do tiro
RpClump* *m_pGogglesObject;     // Objeto dos óculos (visão noturna, etc.)

// Array de armas (13 slots)
CWeapon m_aWeapons[13];

// Informações de estado
eWeaponType m_nSavedWeapon;     // Arma salva temporariamente
eWeaponType m_nDelayedWeapon;   // Arma de reserva (ex: pistola do policial)
uint32 m_nDelayedWeaponAmmo;
uint8_t m_nActiveWeaponSlot;    // Slot ativo (0-12)
uint8 m_nWeaponShootingRate;
uint8 m_nWeaponAccuracy;
eWeaponSkill m_nWeaponSkill;    // Nível de habilidade
int32 m_nWeaponModelId;         // ID do modelo carregado (-1 se nenhum)
```

### 3.3 eWeaponType (Enum)
```cpp
// Localização: app/src/main/cpp/samp/game/Enums/eWeaponType.h
enum eWeaponType : uint32_t {
    WEAPON_UNARMED = 0,
    WEAPON_BRASSKNUCKLE,       // 1
    WEAPON_GOLFCLUB,           // 2
    WEAPON_NIGHTSTICK,         // 3
    WEAPON_KNIFE,              // 4
    WEAPON_BASEBALLBAT,        // 5
    // ... (melee weapons)
    
    WEAPON_PISTOL = 22,        // Primeira arma com skill
    WEAPON_PISTOL_SILENCED,    // 23
    WEAPON_DESERT_EAGLE,       // 24
    WEAPON_SHOTGUN,            // 25
    WEAPON_SAWNOFF_SHOTGUN,    // 26
    WEAPON_SPAS12_SHOTGUN,     // 27
    WEAPON_MICRO_UZI,          // 28
    WEAPON_MP5,                // 29
    WEAPON_AK47,               // 30
    WEAPON_M4,                 // 31
    WEAPON_TEC9,               // 32
    WEAPON_COUNTRYRIFLE,       // 33
    WEAPON_SNIPERRIFLE,        // 34
    WEAPON_RLAUNCHER,          // 35
    WEAPON_RLAUNCHER_HS,       // 36
    WEAPON_FLAMETHROWER,       // 37
    WEAPON_MINIGUN,            // 38
    // ...
    WEAPON_PARACHUTE = 46,
    WEAPON_LAST_WEAPON = 47,
};
```

---

## 4. Fluxo de Carregamento

### 4.1 Fluxo Completo: GiveWeapon
```
CPed::GiveWeapon(weaponType, ammo)
    │
    ├──▶ GameGetWeaponModelIDFromWeaponID(weaponType)
    │         └──▶ Mapeia weaponType → modelId (330-372)
    │
    ├──▶ CStreaming::TryLoadModel(modelId)
    │         │
    │         ├──▶ CStreaming::RequestModel(modelId, flags)
    │         │         └──▶ Adiciona à lista de requisições
    │         │
    │         ├──▶ CStreaming::LoadAllRequestedModels(false)
    │         │         │
    │         │         ├──▶ CStreaming::RequestModelStream(chIdx)
    │         │         │         └──▶ CdStreamRead() - Lê do IMG
    │         │         │
    │         │         └──▶ CStreaming::ProcessLoadingChannel(chIdx)
    │         │                   └──▶ ConvertBufferToObject()
    │         │                             └──▶ CWeaponModelInfo::SetClump()
    │         │
    │         └──▶ Aguarda até modelo estar carregado
    │
    └──▶ CPed::GiveWeapon (nativo)
              │
              ├──▶ Encontra slot apropriado para a arma
              │
              ├──▶ Inicializa CWeapon no slot
              │
              └──▶ CPed::SetCurrentWeapon(weaponType)
                        │
                        └──▶ CPed::AddWeaponModel(modelId)
                                  │
                                  ├──▶ Remove modelo anterior se houver
                                  │
                                  └──▶ Anexa RpClump à mão do ped
```

### 4.2 Funções Nativas (do Dump 64-bit)

| Função | Endereço | Tamanho | Descrição |
|--------|----------|---------|-----------|
| `CPed::GiveWeapon` | 0x59525C | 420 bytes | Dá arma ao ped |
| `CPed::AddWeaponModel` | 0x59B174 | 320 bytes | Adiciona modelo visual |
| `CPed::RemoveWeaponModel` | 0x59B2B4 | 216 bytes | Remove modelo visual |
| `CPed::SetWeaponSkill` | 0x59BF24 | 20 bytes | Define skill |
| `CStreaming::LoadInitialWeapons` | 0x399654 | 48 bytes | Carrega armas iniciais |
| `CWeaponModelInfo::Init` | 0x460B7C | 36 bytes | Inicializa info do modelo |
| `CWeaponModelInfo::SetClump` | 0x460BA0 | 124 bytes | Define clump do modelo |
| `CWeaponInfo::LoadWeaponData` | 0x560EA8 | 1812 bytes | Carrega weapon.dat |
| `CWeaponInfo::GetWeaponInfo` | 0x709BA8 | 84 bytes | Obtém info da arma |
| `CModelInfo::AddWeaponModel` | 0x45CB04 | 92 bytes | Adiciona modelo ao store |

### 4.3 Funções Nativas (do Dump 32-bit)

| Função | Endereço |
|--------|----------|
| `CPed::GiveWeapon` | 0x49F589 |
| `CPed::AddWeaponModel` | 0x4A4CE9 |
| `CPed::RemoveWeaponModel` | 0x4A4DF5 |
| `CStreaming::LoadInitialWeapons` | 0x2D6BEF |
| `CWeaponModelInfo::Init` | 0x389135 |
| `CWeaponModelInfo::SetClump` | 0x389145 |
| `CWeaponInfo::LoadWeaponData` | 0x474751 |
| `CWeaponInfo::GetWeaponInfo` | 0x5E42E9 |
| `CModelInfo::AddWeaponModel` | 0x386071 |

---

## 5. Estruturas de Dados

### 5.1 Mapeamento WeaponType → ModelID
```cpp
// Mapeamento interno do jogo
// WeaponType 0-46 → ModelID 330-372 (aproximadamente)

// Exemplos:
WEAPON_PISTOL (22)        → Model ID 346
WEAPON_SHOTGUN (25)       → Model ID 349
WEAPON_AK47 (30)          → Model ID 355
WEAPON_M4 (31)            → Model ID 356
WEAPON_SNIPERRIFLE (34)   → Model ID 358

// Função: GameGetWeaponModelIDFromWeaponID
// Retorna -1 para armas sem modelo (UNARMED, etc.)
// Retorna 350 ou 365 para armas especiais
```

### 5.2 CWeaponModelInfo (Derivada de CClumpModelInfo)
```cpp
// Store de modelos de armas: 51 slots
// Variável: ms_weaponModelStore (0x00B01360 no dump 64-bit)

class CWeaponModelInfo : public CClumpModelInfo {
    // Herda todos os campos de CClumpModelInfo
    // Adiciona informações específicas de armas
    
    // vtable em 0x82F440 (64-bit)
    virtual ModelInfoType GetModelType() override; // Retorna MODEL_INFO_WEAPON
    virtual void Init() override;
    virtual void SetClump(RpClump* clump) override;
};
```

### 5.3 Slots de Armas no Ped
```
Slot 0: Mãos / Soco americano
Slot 1: Armas de ataque corpo a corpo (taco, bastão, etc.)
Slot 2: Pistolas
Slot 3: Shotguns
Slot 4: Submetralhadoras
Slot 5: Rifles de assalto
Slot 6: Rifles de precisão
Slot 7: Armas pesadas (lança-foguetes, minigun)
Slot 8: Arremessáveis (granadas, molotov)
Slot 9: Equipamentos especiais (spray, extintor)
Slot 10: Presentes (flores, dildo)
Slot 11: Acessórios (paraquedas, óculos)
Slot 12: Detonador
```

---

## 6. Sistema de Streaming

### 6.1 CStreaming - Visão Geral
```cpp
// Localização: app/src/main/cpp/samp/game/Streaming.h

class CStreaming {
public:
    static CStreamingInfo ms_aInfoForModel[26316]; // Info para todos os recursos
    static tStreamingChannel ms_channel[2];        // 2 canais de streaming
    static tStreamingFileDesc ms_files[8];         // Arquivos IMG abertos
    
    // Funções principais
    static void RequestModel(int32 modelId, int32 flags);
    static void LoadAllRequestedModels(bool bPriorityOnly);
    static bool TryLoadModel(int modelId);
    static void RemoveModel(int32 modelId);
};
```

### 6.2 Flags de Streaming
```cpp
#define STREAMING_GAME_REQUIRED     0x02  // Necessário pelo jogo
#define STREAMING_MISSION_REQUIRED  0x04  // Necessário por missão
#define STREAMING_KEEP_IN_MEMORY    0x08  // Manter na memória
#define STREAMING_PRIORITY_REQUEST  0x10  // Requisição prioritária
```

### 6.3 Estados de Carregamento
```cpp
enum eStreamingLoadState : uint8 {
    LOADSTATE_NOT_LOADED = 0,  // Não carregado
    LOADSTATE_LOADED = 1,      // Carregado na memória
    LOADSTATE_REQUESTED = 2,   // Requisitado, aguardando
    LOADSTATE_READING = 3,     // Sendo lido do disco
    LOADSTATE_FINISHING = 4,   // Finalizando (modelos grandes)
};
```

---

## 7. Implementação no Projeto

### 7.1 CPedGTA::GiveWeapon
```cpp
// Localização: app/src/main/cpp/samp/game/Entity/CPedGTA.cpp

void CPedGTA::GiveWeapon(int iWeaponID, int iAmmo)
{
    int iModelID = 0;
    iModelID = GameGetWeaponModelIDFromWeaponID(iWeaponID);
    
    // Validação
    if (iModelID == -1 || iModelID == 350 || iModelID == 365) 
        return;

    // Carrega o modelo se necessário
    if (!CStreaming::TryLoadModel(iModelID))
        return;

    // Chama função nativa
    CHook::CallFunction<void>(
        g_libGTASA + (VER_x32 ? 0x0049F588 + 1 : 0x59525C), 
        this, iWeaponID, iAmmo
    );
    
    // Define como arma atual
    CHook::CallFunction<void>(
        g_libGTASA + (VER_x32 ? 0x004A521C + 1 : 0x59B86C), 
        this, iWeaponID
    );
}
```

### 7.2 CStreaming::TryLoadModel
```cpp
// Localização: app/src/main/cpp/samp/game/Streaming.cpp

bool CStreaming::TryLoadModel(int modelId) {
    if(!CStreaming::GetInfo(modelId).IsLoaded()) {
        // Requisita o modelo
        CStreaming::RequestModel(modelId, 
            STREAMING_GAME_REQUIRED | STREAMING_KEEP_IN_MEMORY);
        
        // Carrega todos os modelos requisitados
        CStreaming::LoadAllRequestedModels(false);
        
        // Aguarda carregamento com timeout
        uint32 count = 0;
        while (!CStreaming::GetInfo(modelId).IsLoaded()) {
            count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (count > 30) { // 3 segundos de timeout
                return false;
            }
        }
    }
    return true;
}
```

---

## 8. Considerações Técnicas

### 8.1 Problemas Comuns

1. **Modelo não carrega**
   - Verificar se modelId está no range válido (330-372)
   - Verificar se arquivo IMG está acessível
   - Verificar memória disponível para streaming

2. **Arma invisível no ped**
   - Verificar se `AddWeaponModel` foi chamado
   - Verificar se bone da mão está correto
   - Verificar se clump foi anexado corretamente

3. **Crash ao trocar arma**
   - Verificar se modelo anterior foi removido
   - Verificar referências pendentes ao modelo
   - Verificar se streaming não está sobrecarregado

### 8.2 Boas Práticas

1. **Sempre verificar carregamento**
   ```cpp
   if (!CStreaming::TryLoadModel(modelId))
       return; // Não prosseguir se falhar
   ```

2. **Usar flags apropriadas**
   ```cpp
   // Para armas de missão
   RequestModel(id, STREAMING_MISSION_REQUIRED | STREAMING_KEEP_IN_MEMORY);
   
   // Para armas temporárias
   RequestModel(id, STREAMING_GAME_REQUIRED);
   ```

3. **Liberar modelos não utilizados**
   ```cpp
   CStreaming::SetModelIsDeletable(modelId);
   // ou
   CStreaming::RemoveModelIfNoRefs(modelId);
   ```

### 8.3 Offsets Importantes (64-bit)

| Campo/Função | Offset |
|-------------|--------|
| g_libGTASA + CPed::GiveWeapon | +0x59525C |
| g_libGTASA + CPed::AddWeaponModel | +0x59B174 |
| g_libGTASA + CPed::RemoveWeaponModel | +0x59B2B4 |
| g_libGTASA + CPed::SetCurrentWeapon | +0x59B86C |
| g_libGTASA + CWeaponInfo::GetWeaponInfo | +0x709BA8 |
| CPedGTA::m_aWeapons | offset varia conforme versão |
| CPedGTA::m_nActiveWeaponSlot | offset varia conforme versão |
| CPedGTA::m_nWeaponModelId | offset varia conforme versão |

### 8.4 Offsets Importantes (32-bit)

| Campo/Função | Offset |
|-------------|--------|
| g_libGTASA + CPed::GiveWeapon | +0x49F588+1 |
| g_libGTASA + CPed::AddWeaponModel | +0x4A4CE9+1 |
| g_libGTASA + CPed::RemoveWeaponModel | +0x4A4DF5+1 |
| g_libGTASA + CPed::SetCurrentWeapon | +0x4A521C+1 |
| g_libGTASA + CWeaponInfo::GetWeaponInfo | +0x5E42E8+1 |

---

## Diagrama de Memória - Array de Armas no Ped

```
CPedGTA (Offset: ~0x700 em 32-bit)
    │
    ├── m_aWeapons[0]  ─────┬── dwType (4 bytes)
    │   (28 bytes)          ├── dwState (4 bytes)
    │                       ├── dwAmmoInClip (4 bytes)
    │                       ├── dwAmmo (4 bytes)
    │                       ├── m_nTimer (4 bytes)
    │                       ├── flags (2 bytes)
    │                       ├── padding (2 bytes)
    │                       └── m_pWeaponFxSys (4/8 bytes)
    │
    ├── m_aWeapons[1]  (28 bytes)
    ├── m_aWeapons[2]  (28 bytes)
    │   ...
    └── m_aWeapons[12] (28 bytes)
    
    Total: 13 slots × 28 bytes = 364 bytes
```

---

**Documento criado para análise do sistema de carregamento de armas do GTA San Andreas.**
**Baseado nos dumps da libGTASA versão 2.1 (32-bit e 64-bit).**
