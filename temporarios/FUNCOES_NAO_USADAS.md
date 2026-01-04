# Análise de Funções Não Usadas Ativamente

## Resumo

Este documento lista todas as funções/métodos da lista de problemas que **NÃO são usadas ativamente** no código.

---

## ❌ Funções NÃO Usadas (Podem ser Removidas)

### 1. `onActivityResult()` - GTASA.java

**Status:** ❌ **NÃO USADO**

- **Arquivo:** `app/src/main/java/com/samp/mobile/game/GTASA.java`
- **Linhas:** 72-75
- **Análise:**
  - Método existe mas **não há nenhuma chamada de `startActivityForResult()`** no projeto
  - Apenas chama `super.onActivityResult()`
  - **Recomendação:** **REMOVER** o método completamente

**Código:**

```java
public void onActivityResult(int i, int i2, Intent intent)
{
    super.onActivityResult(i, i2, intent);
}
```

---

### 2. `loadFile()` - NvEventQueueActivity.java

**Status:** ❌ **NÃO USADO**

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 254-317
- **Análise:**
  - Método existe mas **não há nenhuma chamada** no código
  - Não é chamado via JNI
  - **Recomendação:** **MANTER** (já foi corrigido, pode ser útil no futuro)

**Nota:** Já foi corrigido para usar `getExternalFilesDir()` em vez de `/data/`

---

### 3. `loadTexture()` - NvEventQueueActivity.java

**Status:** ❌ **NÃO USADO**

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 333-415
- **Análise:**
  - Método existe mas **não há nenhuma chamada** no código
  - Não é chamado via JNI
  - **Recomendação:** **MANTER** (já foi corrigido, pode ser útil no futuro)

**Nota:** Já foi corrigido para usar `getExternalFilesDir()` em vez de `/data/`

---

### 4. `getClipboardText()` - NvEventQueueActivity.java

**Status:** ❌ **NÃO USADO**

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 1254-1274
- **Análise:**
  - Método existe em Java
  - Existe declaração `GetClipboardString()` em `jniutil.h` (C++)
  - **MAS:** **NÃO há implementação** de `GetClipboardString()` em `jniutil.cpp`
  - **NÃO há `GetMethodID`** para `getClipboardText` no construtor de `CJavaWrapper`
  - **NÃO há chamadas** de `GetClipboardString()` no código C++
  - **Padrão JNI do projeto:** Todas as funções usadas têm `GetMethodID` no construtor e implementação em `jniutil.cpp`
  - **Recomendação:** **REMOVER** (declaração em C++ também pode ser removida)

**Código Java:**

```java
public byte[] getClipboardText() {
    // ... implementação
}
```

**Código C++ (declaração apenas, SEM implementação):**

```cpp
// jniutil.h - linha 56
const char* GetClipboardString();  // Declarado mas NUNCA implementado

// jniutil.cpp - NÃO TEM implementação desta função
// CJavaWrapper::CJavaWrapper() - NÃO TEM GetMethodID para getClipboardText
```

---

## ✅ Funções USADAS (Precisam ser Corrigidas)

### 5. `setSystemUiVisibility()` - NvEventQueueActivity.java

**Status:** ✅ **USADO ATIVAMENTE**

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 176-182
- **Análise:**
  - **É USADO** no método `hideSystemUI()` (linha 176)
  - Usado como fallback para Android 10 e anterior
  - **MAS:** O listener `setOnSystemUiVisibilityChangeListener` (linha 545) está deprecado
  - **Recomendação:** **MANTER** para compatibilidade com Android 10-, mas **REMOVER** o listener deprecado

**Código:**

```java
// Linha 176 - USADO (fallback para Android 10-)
decorView.setSystemUiVisibility(
    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | ...);

// Linha 545 - DEPRECADO, deve ser removido
getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(i -> {
    // ...
});
```

---

## 📊 Resumo por Categoria

### Funções que PODEM ser REMOVIDAS

1. ✅ `onActivityResult()` - **REMOVER** (não usado, não há `startActivityForResult`)
2. ✅ `getClipboardText()` - **REMOVER** (declarado mas nunca implementado/usado via JNI)

### Funções que DEVEM ser MANTIDAS (mas corrigidas)

1. ✅ `loadFile()` - **MANTER** (já corrigido, pode ser útil)
2. ✅ `loadTexture()` - **MANTER** (já corrigido, pode ser útil)
3. ✅ `setSystemUiVisibility()` - **MANTER** (usado, mas remover listener deprecado)

---

## 🎯 Ações Recomendadas

### Prioridade ALTA (Remover código morto)

1. **Remover `onActivityResult()`** de `GTASA.java`
2. **Verificar e remover `getClipboardText()`** se não for usado via JNI

### Prioridade MÉDIA (Corrigir código usado)

1. **Remover `setOnSystemUiVisibilityChangeListener`** (linha 545) de `NvEventQueueActivity.java`
2. Manter `setSystemUiVisibility()` apenas como fallback para Android 10-

---

## 📝 Notas Importantes

### Como o Código Nativo Chama Funções Java

O projeto usa o padrão `CJavaWrapper` para chamar métodos Java via JNI:

1. **No construtor `CJavaWrapper::CJavaWrapper()`** (jniutil.cpp:5-42):
   - Obtém `GetMethodID` para cada método usado
   - Armazena em variáveis `s_*` (ex: `s_showHud`, `s_hideHud`)

2. **Nas funções wrapper** (ex: `CJavaWrapper::ShowHud()`):
   - Usa `env->CallVoidMethod(activity, s_showHud)`
   - Todas as funções **USADAS** seguem este padrão

3. **Conclusão:**
   - Se **NÃO há `GetMethodID`** no construtor → **NÃO é usado via JNI**
   - Se **NÃO há implementação** em `jniutil.cpp` → **NÃO é usado**
   - Se **NÃO há chamadas** no código C++ → **NÃO é usado**

### Funções Verificadas

- ✅ **Funções USADAS via JNI:** `showHud`, `hideHud`, `updateHudInfo`, `showKeyboard`, `hideKeyboard`, `showDialog`, `setPauseState`, etc.
- ❌ **Funções NÃO USADAS:** `getClipboardText`, `loadFile`, `loadTexture`, `onActivityResult`

### Recomendações

- **Funções corrigidas mas não usadas:** `loadFile()` e `loadTexture()` foram corrigidas mas não são chamadas. Podem ser úteis no futuro, então recomendo manter.

- **Código legado:** Algumas funções podem ser legado de versões anteriores do código. Se não são usadas, é melhor remover para manter o código limpo.

---

---

## 📚 Documento Completo

Para uma análise **COMPLETA** de todos os códigos não usados (66 itens encontrados), consulte:
**`temporarios/CODIGO_NAO_USADO_COMPLETO.md`**

Este documento inclui:

- Todos os métodos stub em WarMedia, WarBilling, WarGamepad
- Classe completa WarHttp não usada
- Métodos vazios chamados via JNI
- Código comentado que pode ser removido
- Análise detalhada de cada item

---

**Última atualização:** 2025-01-XX
**Versão do documento:** 1.0
