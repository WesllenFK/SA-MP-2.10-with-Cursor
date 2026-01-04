# Lista de Problemas de Compatibilidade com Android 16 (API 36)

## Resumo

Este documento lista todos os problemas identificados no projeto que precisam ser corrigidos para garantir compatibilidade 100% com Android 16 (API 36).

**Total de Problemas Identificados: 30** (4 corrigidos)

---

## 🔴 Problemas Críticos (Podem Quebrar no Android 16)

### 1. ✅ SDK/Target SDK não atualizado para 36

**Status:** ✅ CORRIGIDO

- **Arquivo:** `app/build.gradle`
- **Linhas:** 21, 28
- **Problema:** `compileSdk 35` e `targetSdk 35` precisam ser atualizados para 36
- **Impacto:** App não será compatível com Android 16
- **Solução:** Atualizar para `compileSdk 36` e `targetSdk 36`

### 2. ✅ Caminhos hardcoded `/data/` não funcionam

**Status:** ✅ CORRIGIDO

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 254-317 (loadFile), 333-415 (loadTexture)
- **Problema:** Tentativa de acessar `/data/` diretamente, que não é permitido no Android moderno
- **Impacto:** Exceções desnecessárias, código não funciona como esperado
- **Solução Aplicada:**
  - Removido hardcoded `/data/`
  - Implementado uso de `getExternalFilesDir()` como primeira tentativa (igual aos outros sistemas)
  - Fallback para `getAssets()` se não encontrar
  - Corrigido problema do `available()` não confiável (agora lê em chunks)
  - Melhorado tratamento de erros com logging adequado
  - Adicionado `BitmapFactory.Options` para melhor gerenciamento de memória em texturas

### 3. ✅ Método `getOrientation()` ausente causando crash

**Status:** ✅ CORRIGIDO

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 1133-1144
- **Problema:** Código nativo (JNI) chama `getOrientation()` mas o método não existia no Java
- **Impacto:** **CRASH IMEDIATO** na inicialização do app (`NoSuchMethodError`)
- **Solução Aplicada:**
  - Adicionado método `getOrientation()` que retorna `display.getRotation()`
  - `Display.getOrientation()` foi deprecado em API 26, substituído por `getRotation()`
  - Ambos retornam os mesmos valores (0, 1, 2, 3)
  - Adicionada verificação de null para `display`

### 4. `onActivityResult` deprecado (Android 11+)

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/samp/mobile/game/GTASA.java`
- **Linhas:** 72-75
- **Problema:** `onActivityResult` está deprecado desde Android 11 (API 30)
- **Impacto:** Pode parar de funcionar no Android 16
- **Solução:** Migrar para `ActivityResultLauncher` e `registerForActivityResult()`

### 5. JNI_VERSION_1_6 muito antigo

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/cpp/samp/main.cpp`, `app/src/main/cpp/samp/java/jniutil.h`
- **Linhas:** 24, 487, 493, 559
- **Problema:** `JNI_VERSION_1_6` é de 2006, muito antigo
- **Impacto:** Pode causar problemas de compatibilidade
- **Solução:** Atualizar para `JNI_VERSION_1_8` ou `JNI_VERSION_1_9`

### 6. Meta-data FileProvider com namespace antigo

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/AndroidManifest.xml`
- **Linha:** 47
- **Problema:** Usa `android.support.FILE_PROVIDER_PATHS` em vez de `androidx.core.content.FileProvider`
- **Impacto:** Pode não funcionar corretamente
- **Solução:** Atualizar para `androidx.core.content.FileProvider`

### 7. Suporte a páginas de memória de 16KB não implementado

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/cpp/samp/vendor/armhook/patch.cpp`
- **Linhas:** 27-29
- **Problema:** Código usa `PAGE_SIZE` como constante, mas Android 16 requer suporte dinâmico para páginas de 16KB
- **Impacto:** Pode não funcionar em dispositivos com páginas de 16KB
- **Solução:** Implementar `GetPageSize()` usando `sysconf(_SC_PAGESIZE)`

---

## 🟡 Problemas Importantes (Podem Causar Problemas)

### 7. `Environment.getExternalStorageState().equals("mounted")` - usar constante

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/wardrumstudios/utils/WarMedia.java`
- **Linha:** 19
- **Problema:** Usa string literal em vez de constante `Environment.MEDIA_MOUNTED`
- **Impacto:** Código menos robusto
- **Solução:** Usar `Environment.MEDIA_MOUNTED`

### 8. `setSystemUiVisibility` deprecado

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 176-182, 467-475
- **Problema:** `setSystemUiVisibility` e `setOnSystemUiVisibilityChangeListener` estão deprecados
- **Impacto:** Pode parar de funcionar no Android 16
- **Solução:** Remover listener deprecado, usar apenas `WindowInsetsController` para Android 11+

### 9. Clipboard sem verificação de permissão (Android 13+)

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 1189-1216
- **Problema:** A partir do Android 13 (API 33), ler clipboard requer permissão ou contexto visível
- **Impacto:** Pode não funcionar no Android 13+
- **Solução:** Adicionar verificação de permissão ou usar `ClipboardManager.OnPrimaryClipChangedListener`

### 10. `Build.*` sem verificação de null

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/wardrumstudios/utils/WarMedia.java`
- **Linhas:** 111-118, 147-151
- **Problema:** Alguns campos de `Build` podem retornar `null` ou valores inesperados no Android 16
- **Impacto:** Possíveis `NullPointerException`
- **Solução:** Adicionar verificações de null e usar alternativas quando disponíveis

### 11. `gwpAsanMode="always"` pode causar problemas

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/AndroidManifest.xml`
- **Linha:** 27
- **Problema:** Pode causar problemas de performance ou compatibilidade no Android 16
- **Impacto:** Performance degradada
- **Solução:** Remover ou alterar para `"never"` ou condicional

### 12. `lintOptions` deprecado

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linhas:** 82-87
- **Problema:** `lintOptions` está deprecado no Android Gradle Plugin 8.0+
- **Impacto:** Pode parar de funcionar em versões futuras do AGP
- **Solução:** Migrar para `lint {}` block

### 13. `packagingOptions` deprecado

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linhas:** 41-48
- **Problema:** `packagingOptions` está deprecado no AGP 8.0+
- **Impacto:** Pode parar de funcionar em versões futuras do AGP
- **Solução:** Migrar para `packaging {}` block

### 14. `allprojects` no módulo app

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linha:** 22
- **Problema:** `allprojects` não deve estar dentro do módulo `app`
- **Impacto:** Configuração incorreta, pode causar problemas
- **Solução:** Remover ou mover para o `build.gradle` raiz

### 15. Dependência com versão dinâmica `+`

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linha:** 115
- **Problema:** `implementation 'com.hzy:un7zip:+'` usa versão dinâmica
- **Impacto:** Pode causar problemas de compatibilidade e builds não reproduzíveis
- **Solução:** Fixar a versão

### 16. Cleartext traffic habilitado

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/res/xml/network_security_config.xml`, `app/src/main/AndroidManifest.xml`
- **Linhas:** 3, 38
- **Problema:** Cleartext traffic pode ser restrito no Android 16
- **Impacto:** Conexões HTTP podem falhar
- **Solução:** Revisar se é necessário e, se for, documentar o motivo

### 17. `hardwareAccelerated="false"` pode causar problemas

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/AndroidManifest.xml`
- **Linha:** 29
- **Problema:** Desabilitar aceleração de hardware pode afetar performance
- **Impacto:** Performance degradada
- **Solução:** Avaliar se é necessário, remover ou definir como `true`

### 18. `largeHeap="true"` pode ser restrito

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/AndroidManifest.xml`
- **Linha:** 32
- **Problema:** `largeHeap` pode ser restrito ou ignorado no Android 16
- **Impacto:** Pode não funcionar como esperado
- **Solução:** Revisar se é necessário, otimizar uso de memória

### 19. `provider_paths.xml` muito permissivo

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/res/xml/provider_paths.xml`
- **Linhas:** 3-5
- **Problema:** `<external-path path="." />` expõe todo o armazenamento externo
- **Impacto:** Pode ser restrito no Android 16
- **Solução:** Restringir a caminhos específicos necessários

### 20. ProGuard rules podem precisar atualização

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/proguard-rules.pro`
- **Problema:** Regras podem não cobrir todas as classes necessárias para Android 16
- **Impacto:** Pode causar problemas em builds otimizados
- **Solução:** Adicionar regras para classes JNI e AndroidX

---

## 🟢 Problemas Menores (Melhorias)

### 21. Dependências desatualizadas

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linhas:** 95-120
- **Problema:** Muitas dependências estão muito antigas
- **Impacto:** Falta de recursos, correções de bugs e compatibilidade
- **Solução:** Atualizar dependências AndroidX e Firebase

### 22. Repositórios `jcenter()` deprecados

**Status:** ⏳ PENDENTE

- **Arquivo:** `build.gradle` (raiz)
- **Linhas:** 5, 26
- **Problema:** `jcenter()` foi descontinuado
- **Impacto:** Pode parar de funcionar
- **Solução:** Remover `jcenter()` dos repositórios

### 23. Java 8 em vez de 11/17

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/build.gradle`
- **Linhas:** 79-80
- **Problema:** Usa Java 8, recomendado Java 11 ou 17
- **Impacto:** Pode não ser suportado em versões futuras
- **Solução:** Atualizar para `JavaVersion.VERSION_11` ou `VERSION_17`

### 24. `substring()` sem verificação de bounds

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/wardrumstudios/utils/WarMedia.java`
- **Linha:** 25
- **Problema:** Pode causar `StringIndexOutOfBoundsException` se `/Android` não existir
- **Impacto:** Possível crash
- **Solução:** Adicionar verificação antes de usar `substring()`

### 25. `available()` não confiável

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linha:** 274
- **Problema:** `InputStream.available()` pode não retornar o tamanho real do arquivo
- **Impacto:** Pode causar problemas ao ler arquivos
- **Solução:** Ler o arquivo completamente ou usar outro método

### 26. `BitmapFactory.decodeStream()` sem otimizações

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linha:** 328
- **Problema:** Pode causar problemas de memória com imagens grandes
- **Impacto:** Possível `OutOfMemoryError`
- **Solução:** Usar `BitmapFactory.Options` com `inSampleSize`

### 27. Tratamento de erros insuficiente

**Status:** ⏳ PENDENTE

- **Arquivo:** `app/src/main/java/com/nvidia/devtech/NvEventQueueActivity.java`
- **Linhas:** 264, 270, 317, 323
- **Problema:** Blocos `catch` vazios ignoram erros silenciosamente
- **Impacto:** Dificulta debugging
- **Solução:** Adicionar logging adequado

### 28. Falta de logging adequado

**Status:** ⏳ PENDENTE

- **Arquivo:** Vários
- **Problema:** Falta de logging para debugging
- **Impacto:** Dificulta identificar problemas
- **Solução:** Adicionar logging estratégico

---

## Estatísticas

- **Total de Problemas:** 28
- **Críticos:** 6 (1 corrigido, 5 pendentes)
- **Importantes:** 14 (todos pendentes)
- **Menores:** 8 (todos pendentes)
- **Corrigidos:** 1
- **Pendentes:** 27

---

## Priorização Sugerida

### Alta Prioridade (Fazer Primeiro)

1. ✅ Problema #1 - SDK/Target SDK (CORRIGIDO)
2. Problema #2 - Caminhos `/data/`
3. Problema #6 - Páginas de 16KB
4. Problema #4 - JNI_VERSION
5. Problema #5 - FileProvider namespace

### Média Prioridade

1. Problema #3 - onActivityResult
2. Problema #8 - setSystemUiVisibility
3. Problema #12 - lintOptions
4. Problema #13 - packagingOptions
5. Problema #22 - jcenter()

### Baixa Prioridade (Melhorias)

1. Problema #21 - Dependências
2. Problema #23 - Java version
3. Problema #27 - Tratamento de erros
4. Problema #28 - Logging

---

## Notas Importantes

- **Android 16 (API 36) ainda não foi lançado oficialmente.** Essas mudanças preparam o projeto para quando estiver disponível.
- **Teste em dispositivos reais** após aplicar as correções.
- **Faça backup** antes de aplicar mudanças críticas.
- **Atualize dependências gradualmente** para identificar problemas de compatibilidade.

---

**Última atualização:** 2025-01-XX
**Versão do documento:** 1.0
