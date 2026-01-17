# Reformulação: Monólito Modular

**Branch:** `cursor/arquitetura-monolito-modular`  
**Status:** 🟡 Em Progresso (~30%)  
**Última Atualização:** 2024-12-19

---

## 🎯 Objetivo

Transformar o código monolítico em arquitetura modular mantendo funcionalidade, seguindo os princípios descritos em `temporarios/ARCHITECTURE_PLAN.md`.

---

## 📊 Progresso Atual

### ✅ O Que Foi Concluído

- **Fase 2: Game Module (~67%)**
  - ✅ Criado `game/engine/` com `game.cpp`
  - ✅ Criado `game/entities/` com actor, object, playerped, vehicle
  - ✅ Criado `game/hooks/` com `patches.cpp`
  - ✅ Criado `game/input/` com `pad.cpp`
  - ⚠️ `game/RW/` ainda não movido para `game/rendering/RW/`
  - ⚠️ `game/Collision/` ainda não movido para `game/physics/Collision/`

- **Fase 6: Platform Module (~33%)**
  - ✅ Estrutura criada (`platform/android/`, `platform/debug/`, `platform/lifecycle/`, `platform/public/`)
  - ⚠️ Arquivos ainda não movidos

### ❌ O Que Falta

- **Fase 1: Infraestrutura Core (0%)**
  - ❌ Service Locator (`core/services/ServiceLocator.h/cpp`)
  - ❌ Event Bus (`core/events/EventBus.h/cpp`)
  - ❌ Interfaces (`core/interfaces/`)
  - ❌ Bootstrap (`core/bootstrap/Bootstrap.h/cpp`)

- **Fase 3: Multiplayer Module (5%)**
  - ⚠️ Estrutura básica criada
  - ❌ `net/` ainda existe (18+ arquivos não migrados)
  - ❌ Pools não organizados em `multiplayer/pools/`
  - ❌ RPCs não organizados em `multiplayer/rpc/`

- **Fase 4: UI Module (4%)**
  - ⚠️ Estrutura básica criada
  - ❌ `gui/` ainda existe (25+ arquivos não migrados)
  - ❌ Widgets não organizados em `ui/screens/`

- **Fase 5: Audio Module (3%)**
  - ⚠️ Estrutura básica criada
  - ❌ `voice_new/` ainda existe (53 arquivos não migrados)

- **Fase 7: Refatoração Final (0%)**
  - ❌ Globals ainda em uso (41 arquivos)
  - ❌ Dependências circulares não resolvidas
  - ❌ CMakeLists.txt não modularizado

**Progresso Total Estimado: ~30%**

---

## 📁 Estrutura Atual

```
samp/
├── core/
│   └── memory/         ✅ Criado
│
├── game/
│   ├── engine/         ✅ Criado (game.cpp)
│   ├── entities/       ✅ Criado (actor, object, playerped, vehicle)
│   ├── hooks/          ✅ Criado (patches.cpp)
│   ├── input/          ✅ Criado (pad.cpp)
│   ├── RW/             ⚠️ Ainda não movido para rendering/
│   └── Collision/      ⚠️ Ainda não movido para physics/
│
├── multiplayer/
│   ├── player/         ⚠️ Estrutura básica
│   └── public/         ⚠️ Estrutura básica
│
├── net/                ❌ AINDA EXISTE (deveria estar em multiplayer/)
├── gui/                ❌ AINDA EXISTE (deveria estar em ui/)
├── voice_new/          ❌ AINDA EXISTE (deveria estar em audio/voice/)
│
├── platform/
│   ├── android/        ✅ Estrutura criada
│   ├── debug/          ✅ Estrutura criada
│   ├── lifecycle/      ✅ Estrutura criada
│   └── public/         ✅ Estrutura criada
│
├── ui/
│   ├── components/     ⚠️ Estrutura básica
│   └── public/         ⚠️ Estrutura básica
│
└── audio/
    └── public/         ⚠️ Estrutura básica
```

---

## 🔄 Próximos Passos (Prioridade)

### 🔴 Prioridade ALTA - Bloqueadores

1. **Implementar Service Locator** (`core/services/ServiceLocator.h/cpp`)
   - Migrar globals para Service Locator
   - Registrar serviços em `main.cpp`

2. **Implementar Event Bus** (`core/events/EventBus.h/cpp`)
   - Sistema pub/sub básico
   - Quebrar dependências diretas entre módulos

3. **Criar Interfaces** (`core/interfaces/`)
   - `IGameService`, `INetworkService`, `IChatOutput`, `IAudioService`

4. **Criar Bootstrap** (`core/bootstrap/Bootstrap.h/cpp`)
   - Extrair inicialização de `main.cpp`
   - Ordem controlada de init/shutdown

### 🟡 Prioridade MÉDIA - Reorganização

5. **Migrar `net/` → `multiplayer/`**
6. **Migrar `gui/` → `ui/`**
7. **Migrar `voice_new/` → `audio/voice/`**
8. **Finalizar organização de `game/`**

### 🟢 Prioridade BAIXA - Refatoração

9. Remover globals gradualmente
10. Migrar chamadas diretas para eventos
11. Modularizar CMakeLists.txt

---

## 📚 Documentação

- **Plano Completo:** `temporarios/ARCHITECTURE_PLAN.md`
- **Análise de Dependências:** `temporarios/MIGRATION_ANALYSIS.md`
- **Status Detalhado:** `temporarios/STATUS_PROGRESSO.md`

---

## ⚠️ IMPORTANTE

- **Este branch é experimental** - não fazer merge para `main` até estar completo
- Build pode estar quebrado durante refatoração
- Commits incrementais por fase
- Progresso documentado em cada commit

---

## 🏷️ Tags de Versão

- `v0.1-fase-1-core` - Quando Fase 1 completar
- `v0.2-fase-2-game` - Quando Fase 2 completar
- `v0.3-fase-3-multiplayer` - Quando Fase 3 completar
- `v1.0-modular-monolith-complete` - Quando tudo estiver 100%

---

**Status:** 🟡 Em Progresso (30% completo)
