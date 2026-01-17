# Anexo: Erros Atuais da Compilação

**Data de Coleta:** 2024-12-19

---

## Status Atual

A compilação ainda está falhando com:

```
fatal error: too many errors emitted, stopping now [-ferror-limit=]
```

O compilador está limitando a saída de erros porque há muitos erros.

---

## Tipos de Erros Identificados

### 1. Includes de Arquivos Não Encontrados

Arquivos sendo procurados mas não existem no caminho especificado:

- `CompatibilityMacros.h` - Arquivo foi removido mas ainda referenciado
- Arquivos em `core/bootstrap/` - Diretório não existe mais
- Arquivos em `multiplayer/connection/` - Ainda estão em `net/`

### 2. Caminhos Relativos Incorretos

Arquivos em subdiretórios usando caminhos relativos errados:

- Arquivos em `game/entities/` tentando acessar `game/`
- Arquivos em `game/engine/` tentando acessar `game/`
- Arquivos em `game/Animation/` tentando acessar `game/RW/`

### 3. Mistura de Estruturas Antigas e Novas

- Alguns arquivos referenciam `game/RW/` (estrutura antiga)
- Outros referenciam `game/rendering/RW/` (estrutura nova planejada)
- RW/ ainda está em `game/RW/`, não foi movido para `rendering/`

### 4. Arquivos Duplicados

- Diretórios `net/` e `gui/` ainda existem
- Conflitos entre arquivos antigos e nova estrutura modular
- Ambos sendo compilados pelo CMakeLists.txt

---

## Estratégia de Correção Recomendada

### Fase 1: Corrigir Estrutura Base
1. Decidir se RW/ fica em `game/RW/` ou move para `game/rendering/RW/`
2. Atualizar CMakeLists.txt com a estrutura correta
3. Corrigir includes baseado na estrutura escolhida

### Fase 2: Remover Duplicatas
1. Verificar se `net/` e `gui/` podem ser removidos
2. Ou migrar arquivos restantes para nova estrutura
3. Atualizar CMakeLists.txt para excluir duplicatas

### Fase 3: Padronizar Includes
1. Escolher padrão: relativos ao arquivo ou relativos a `samp/`
2. Aplicar padrão consistentemente
3. Validar todos os includes

---

**Nota:** Este anexo será atualizado conforme os erros são corrigidos e novos são descobertos.
