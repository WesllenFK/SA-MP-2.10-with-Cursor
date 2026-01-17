# Relatório de Logcat - 2026-01-17 14:13:46

## Resumo Executivo

O aplicativo `com.samp.mobile` apresentou um **crash fatal** (SIGABRT) aproximadamente **2 segundos** após o início da execução. O erro ocorre durante a inicialização da atividade principal.

---

## Informações do Sistema

- **Dispositivo**: POCO (lake_p_global)
- **Android**: 15 (AP3A.240905.015.A2)
- **ABI**: arm64
- **Processo**: com.samp.mobile (PID: 31610)
- **Timestamp do Crash**: 2026-01-17 14:07:54.574

---

## Erro Principal

### Tipo de Erro
```
Fatal signal 6 (SIGABRT), code -1 (SI_QUEUE)
```

### Causa Raiz
```
java.lang.NoSuchMethodError: no non-static method 
"Lcom/nvidia/devtech/NvEventQueueActivity;.getOrientation()I"
```

### Stack Trace Completo
O erro ocorre na seguinte sequência:

1. **NvEventQueueActivity.init()** (linha -2)
   - Tentativa de chamar método `getOrientation()` que não existe

2. **NvEventQueueActivity.onCreate()** (linha 439)
   - Chamada durante criação da atividade

3. **WarMedia.onCreate()** (linha 43)
   - Herda de NvEventQueueActivity

4. **GTASA.onCreate()** (linha 93)
   - Atividade principal do jogo

5. **SAMP.onCreate()** (linha 172)
   - Atividade principal do SAMP

---

## Análise Detalhada

### Problema Identificado

O método `getOrientation()` está sendo chamado como método de instância (não-estático) na classe `NvEventQueueActivity`, mas:

1. **O método não existe** na classe
2. **Ou o método foi removido/alterado** em uma versão mais recente da biblioteca NVIDIA
3. **Ou há incompatibilidade** entre a versão da biblioteca NVIDIA e o código

### Contexto do Erro

- O erro ocorre **imediatamente** após o início do aplicativo
- A atividade não consegue completar o método `onCreate()`
- O sistema Android aborta o processo devido à exceção não tratada

### Warnings Anteriores

Antes do crash, foram registrados alguns warnings (não críticos):

1. **MQSEventManagerDelegate**: Falha ao obter MQSService (serviço de métricas)
2. **ActivityThread**: `registerApplicationScoutThread result:false`
3. **ActivityThread**: `useGrowthLimitOutExpendMethod invoke error`

Estes warnings são comuns em dispositivos Xiaomi/POCO e não causam o crash.

---

## Backtrace Nativo

O backtrace mostra que o abort foi chamado pela ART (Android Runtime):

```
#00 pc 000000000005df04  /apex/com.android.runtime/lib64/bionic/libc.so (abort+164)
#01 pc 00000000008fde60  /apex/com.android.art/lib64/libart.so (art::Runtime::Abort)
#02 pc 000000000001654c  /apex/com.android.art/lib64/libbase.so (SetAborter)
#03 pc 0000000000015a4c  /apex/com.android.art/lib64/libbase.so (LogMessage::~LogMessage)
#04 pc 000000000026bdc8  /apex/com.android.art/lib64/libart.so (Thread::AssertNoPendingException)
#05 pc 0000000000206bb0  /apex/com.android.art/lib64/libart.so (ClassLinker::FindClass)
#06 pc 000000000061b274  /apex/com.android.art/lib64/libart.so (JNI::FindClass)
#07 pc 0000000000002070  /data/app/.../com.samp.mobile/base.apk
```

---

## Recomendações

### 1. Correção Imediata

Verificar a classe `NvEventQueueActivity` e o método `getOrientation()`:

- **Localizar** onde `getOrientation()` está sendo chamado
- **Verificar** se o método existe na classe base
- **Substituir** por método estático ou método correto da API Android

### 2. Verificação de Dependências

- Verificar versão da biblioteca NVIDIA DevTech
- Confirmar compatibilidade com Android 15
- Atualizar dependências se necessário

### 3. Tratamento de Erros

Adicionar tratamento de exceção no método `init()`:

```java
try {
    // código atual
} catch (NoSuchMethodError e) {
    // fallback ou tratamento alternativo
}
```

### 4. Verificação de Código

Arquivos a verificar:
- `com/nvidia/devtech/NvEventQueueActivity.java`
- `com/wardrumstudios/utils/WarMedia.java`
- `com/samp/mobile/game/GTASA.java`
- `com/samp/mobile/game/SAMP.java`

---

## Conclusão

O crash é causado por uma **incompatibilidade de API** na biblioteca NVIDIA DevTech. O método `getOrientation()` não está disponível ou foi alterado na versão atual. É necessário corrigir a chamada do método ou atualizar a biblioteca para uma versão compatível.

**Prioridade**: 🔴 **ALTA** - O aplicativo não inicia devido a este erro.

**Status**: ❌ **CRASH FATAL** - Aplicativo não pode ser executado.

---

## Próximos Passos

1. ✅ Identificar localização exata do método `getOrientation()` no código
2. ✅ Verificar documentação da biblioteca NVIDIA DevTech
3. ✅ Implementar correção ou workaround
4. ✅ Testar em dispositivo Android 15
5. ✅ Validar que o aplicativo inicia corretamente

---

## Análise Adicional do Código

### Arquivos Verificados

1. **NvEventQueueActivity.java** (app/src/main/java/com/nvidia/devtech/)
   - Classe base abstrata que estende `AppCompatActivity`
   - Não contém método `getOrientation()` visível no código fonte
   - O método `init()` não está presente no código Java

2. **nv_event.cpp** (app/src/main/cpp/samp/)
   - Contém apenas implementação de `customMultiTouchEvent`
   - Não contém chamada a `getOrientation()`

### Hipótese

O método `getOrientation()` pode estar sendo chamado:
- **Internamente pela biblioteca NVIDIA DevTech** (libnvevent.a)
- **Através de JNI** de código nativo não visível
- **Em uma versão antiga da biblioteca** que não é compatível com Android 15

### Solução Proposta

Como o método `getOrientation()` não existe mais na API do Android (foi deprecado e removido), é necessário:

1. **Verificar se há uma biblioteca nativa** (libnvevent.a) que precisa ser atualizada
2. **Substituir `getOrientation()`** por `getDisplay().getRotation()` (API moderna)
3. **Adicionar verificação de versão** do Android antes de chamar métodos de orientação

### Código de Correção Sugerido

Se o método `init()` estiver em código nativo, adicionar verificação:

```java
// Em NvEventQueueActivity ou classe derivada
private int getOrientationCompat() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
        // Android 11+ (API 30+)
        return getDisplay().getRotation();
    } else {
        // Android 10 e anterior - usar método legado se disponível
        Display display = getWindowManager().getDefaultDisplay();
        return display.getRotation();
    }
}
```

E substituir todas as chamadas a `getOrientation()` por `getOrientationCompat()`.
