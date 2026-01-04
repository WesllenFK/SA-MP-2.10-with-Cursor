# Arquitetura Técnica do Sistema Launcher - Documentação Completa

## 1. Princípios de Design e Boas Práticas

### 1.1 Princípios SOLID Aplicados

**Single Responsibility Principle (SRP)**

- Cada classe possui uma única responsabilidade claramente definida
- DownloadManager gerencia apenas downloads, não UI ou arquivos
- FileBackupManager gerencia apenas backups, não downloads
- Separar concerns: lógica de negócio separada de apresentação

**Open/Closed Principle (OCP)**

- Sistema extensível através de interfaces e abstrações
- Novos tipos de verificadores podem ser adicionados sem modificar código existente
- Políticas de atualização configuráveis sem alterar código core

**Liskov Substitution Principle (LSP)**

- Implementações de interfaces são intercambiáveis
- Classes derivadas podem substituir classes base sem quebrar funcionalidade

**Interface Segregation Principle (ISP)**

- Interfaces específicas e focadas, não genéricas demais
- Clientes não dependem de métodos que não usam
- Interfaces pequenas e coesas

**Dependency Inversion Principle (DIP)**

- Módulos de alto nível não dependem de módulos de baixo nível
- Ambos dependem de abstrações
- Injeção de dependências para facilitar testes e manutenção

### 1.2 Design Patterns Utilizados

**Singleton Pattern**

- Launcher: instância única global do sistema
- Acesso controlado e thread-safe
- Inicialização lazy quando possível

**Factory Pattern**

- Criação de ações de download (DeleteAction, UpdateAction, DownloadAction)
- Criação de workers do thread pool
- Encapsulamento da lógica de criação

**Observer Pattern**

- Sistema de callbacks para progresso de download
- Notificações de mudança de estado
- Eventos de UI atualizados automaticamente

**Strategy Pattern**

- Diferentes estratégias de comparação de versões
- Políticas de atualização configuráveis
- Algoritmos de verificação intercambiáveis

**Command Pattern**

- Ações de download encapsuladas como objetos
- Fila de comandos para processamento
- Desfazer/refazer operações (através de backups)

**State Pattern**

- Máquina de estados para controlar fluxo do launcher
- Transições bem definidas entre estados
- Comportamento específico por estado

**Thread Pool Pattern**

- Pool fixo de threads para downloads
- Reutilização de threads
- Gerenciamento eficiente de recursos

### 1.3 Princípios Adicionais

**DRY (Don't Repeat Yourself)**

- Funções utilitárias centralizadas
- Código compartilhado em helpers
- Evitar duplicação de lógica

**KISS (Keep It Simple, Stupid)**

- Soluções simples quando possível
- Evitar over-engineering
- Código legível e direto

**YAGNI (You Aren't Gonna Need It)**

- Implementar apenas funcionalidades necessárias
- Evitar features não solicitadas
- Priorizar MVP funcional

**Separation of Concerns**

- Lógica de negócio separada de apresentação
- Camada de dados isolada
- Network layer independente

**Fail-Fast Principle**

- Validações antecipadas
- Erros detectados o mais cedo possível
- Feedback imediato ao usuário

**Defensive Programming**

- Validação de todos os inputs
- Verificação de null pointers
- Tratamento robusto de erros

**Resource Management**

- RAII (Resource Acquisition Is Initialization)
- Destrutores garantem limpeza
- Gerenciamento explícito de recursos

---

## 2. Arquitetura de Módulos Detalhada

### 2.1 Core - Launcher

#### 2.1.1 Classe: Launcher

**Responsabilidades:**

- Coordenar todos os componentes do sistema
- Gerenciar ciclo de vida completo
- Controlar máquina de estados
- Interface principal para comunicação externa

**Propriedades Privadas:**

- `storagePath`: Caminho absoluto do diretório de armazenamento
- `manifestURL`: URL base do manifest no servidor
- `currentState`: Estado atual do launcher (enum)
- `components`: Container de ponteiros para componentes principais
- `stateMachine`: Instância da máquina de estados
- `isInitialized`: Flag booleana de inicialização
- `isShuttingDown`: Flag booleana de finalização

**Métodos Públicos:**

**`Initialize(activity, storagePath, manifestURL)`**

- **Responsabilidade**: Inicializar todos os componentes do launcher
- **Parâmetros**:
  - `activity`: Jobject Java Activity (para JNI)
  - `storagePath`: String com caminho de armazenamento
  - `manifestURL`: String com URL do manifest
- **Retorno**: Boolean indicando sucesso
- **Pré-condições**:
  - Activity válida e não nula
  - Storage path válido e acessível
  - Manifest URL válido e acessível
- **Pós-condições**:
  - Todos os componentes inicializados
  - Estado transitado para INITIALIZING
  - Sistema pronto para operação
- **Comportamento**:
  1. Validar todos os parâmetros
  2. Criar diretórios necessários
  3. Inicializar componentes na ordem correta (DownloadManager, UI, etc.)
  4. Configurar callbacks entre componentes
  5. Carregar metadados locais existentes
  6. Transicionar para estado INITIALIZING
- **Tratamento de Erros**:
  - Retornar false e logar erro específico
  - Garantir cleanup de recursos parcialmente alocados
  - Não deixar sistema em estado inconsistente

**`Process()`**

- **Responsabilidade**: Processar um ciclo de atualização do launcher
- **Parâmetros**: Nenhum
- **Retorno**: Void
- **Frequência**: Chamado a cada frame/tick
- **Comportamento**:
  1. Verificar estado atual
  2. Executar lógica específica do estado
  3. Verificar condições de transição
  4. Executar transições de estado se necessário
  5. Processar filas e operações pendentes
- **Thread Safety**: Thread-safe, pode ser chamado de qualquer thread
- **Bloqueio**: Não bloqueante, execução rápida

**`Render()`**

- **Responsabilidade**: Renderizar UI do launcher
- **Parâmetros**: Nenhum
- **Retorno**: Void
- **Frequência**: Chamado a cada frame de renderização
- **Pré-condições**:
  - ImGui inicializado
  - Contexto de renderização ativo
- **Comportamento**:
  1. Determinar qual tela mostrar baseado no estado
  2. Chamar método Render() da tela apropriada
  3. Atualizar elementos visuais com dados atuais
  4. Renderizar através do sistema ImGui
- **Performance**: Otimizado para 60 FPS

**`CheckAndUpdate()`**

- **Responsabilidade**: Verificar e atualizar arquivos necessários
- **Parâmetros**: Nenhum
- **Retorno**: Boolean indicando sucesso
- **Comportamento**:
  1. Baixar manifest remoto
  2. Comparar com manifest local
  3. Identificar arquivos necessários
  4. Planejar ações (DELETE, UPDATE, DOWNLOAD)
  5. Executar ações na ordem correta
  6. Validar resultados
  7. Atualizar metadados locais
- **Assíncrono**: Execução assíncrona em background
- **Progresso**: Notifica progresso através de callbacks

**`Shutdown()`**

- **Responsabilidade**: Finalizar sistema de forma segura
- **Parâmetros**: Nenhum
- **Retorno**: Void
- **Comportamento**:
  1. Parar todas as operações em andamento
  2. Aguardar conclusão de threads
  3. Salvar estado final (metadados, etc.)
  4. Liberar recursos alocados
  5. Destruir componentes na ordem inversa de criação
- **Cleanup**: Garantir limpeza completa mesmo em caso de erro

**Métodos Privados:**

**`InitializeComponents()`**

- Inicializar cada componente na ordem correta
- Configurar dependências entre componentes
- Validar inicialização de cada componente

**`ValidateStorage()`**

- Verificar se storage path existe e é acessível
- Criar diretórios necessários se não existirem
- Verificar permissões de escrita

**`LoadLocalMetadata()`**

- Carregar metadados de arquivos locais
- Validar integridade dos metadados
- Tratar metadados corrompidos ou ausentes

#### 2.1.2 Classe: StateMachine

**Responsabilidades:**

- Gerenciar transições de estado
- Validar transições permitidas
- Executar lógica específica de cada estado
- Notificar mudanças de estado

**Propriedades:**

- `currentState`: Estado atual (enum LauncherState)
- `previousState`: Estado anterior (para histórico)
- `stateHistory`: Histórico de estados (para debug)
- `transitionCallbacks`: Map de callbacks por transição
- `stateCallbacks`: Map de callbacks por estado

**Métodos:**

**`TransitionTo(newState)`**

- **Responsabilidade**: Transicionar para novo estado
- **Validação**: Verificar se transição é permitida
- **Comportamento**:
  1. Validar transição
  2. Executar callback de saída do estado atual
  3. Atualizar estado
  4. Executar callback de entrada do novo estado
  5. Notificar observadores
- **Thread Safety**: Protegido por mutex

**`CanTransition(from, to)`**

- **Responsabilidade**: Verificar se transição é permitida
- **Lógica**: Matriz de transições permitidas
- **Retorno**: Boolean

**`ExecuteStateLogic(state)`**

- **Responsabilidade**: Executar lógica específica do estado
- **Switch**: Switch-case com lógica por estado
- **Retorno**: Próximo estado sugerido (opcional)

**`RegisterTransitionCallback(from, to, callback)`**

- **Responsabilidade**: Registrar callback para transição específica
- **Uso**: Permitir código customizado em transições

**`RegisterStateCallback(state, callback)`**

- **Responsabilidade**: Registrar callback para estado específico
- **Uso**: Permitir código customizado ao entrar/sair de estado

---

### 2.2 Download - Sistema de Download

#### 2.2.1 Classe: DownloadManager

**Responsabilidades:**

- Coordenar todas as operações de download
- Gerenciar pool de threads
- Manter fila de ações
- Rastrear progresso global
- Implementar retry e recuperação de erros

**Propriedades:**

**Privadas:**

- `threadPool`: Instância do pool de 5 threads
- `actionQueue`: Fila thread-safe de ações pendentes
- `activeDownloads`: Map de downloads ativos (threadId -> DownloadInfo)
- `progressTracker`: Instância do rastreador de progresso
- `fileBackupManager`: Instância do gerenciador de backups
- `httpClient`: Instância do cliente HTTP
- `mutex`: Mutex para sincronização
- `shutdownFlag`: Flag atômica para shutdown
- `maxRetries`: Número máximo de tentativas (3)
- `retryDelay`: Delay entre tentativas em segundos (1)

**Métodos Públicos:**

**`AddAction(action)`**

- **Responsabilidade**: Adicionar ação à fila de processamento
- **Parâmetros**: `action` do tipo FileAction (struct com type, path, remoteFile, etc.)
- **Retorno**: Boolean indicando se ação foi adicionada
- **Thread Safety**: Thread-safe, usa mutex
- **Comportamento**:
  1. Validar ação
  2. Calcular prioridade
  3. Inserir na fila ordenada por prioridade
  4. Notificar thread pool se necessário
- **Validação**:
  - Verificar se path é válido
  - Verificar se remoteFile não é nulo para UPDATE/DOWNLOAD
  - Verificar se storage path é acessível

**`ProcessActionQueue()`**

- **Responsabilidade**: Processar fila de ações
- **Retorno**: Void
- **Frequência**: Chamado continuamente até fila vazia
- **Comportamento**:
  1. Verificar se há ações na fila
  2. Para cada ação, verificar tipo:
     - DELETE: Executar DeleteFile()
     - UPDATE: Executar UpdateFileWithBackup()
     - DOWNLOAD: Executar DownloadNewFile()
  3. Atualizar progresso após cada ação
  4. Tratar erros e fazer retry se necessário
- **Thread Safety**: Executado por múltiplas threads do pool
- **Bloqueio**: Pode bloquear durante download/operações I/O

**`DeleteFile(path)`**

- **Responsabilidade**: Deletar arquivo do sistema
- **Parâmetros**: `path` relativo ao storage
- **Retorno**: Boolean indicando sucesso
- **Comportamento**:
  1. Construir caminho absoluto
  2. Verificar se arquivo existe
  3. Verificar se não é arquivo crítico (whitelist)
  4. Deletar arquivo
  5. Remover metadados do arquivo
  6. Notificar progresso
- **Segurança**:
  - Verificar whitelist de arquivos que não devem ser deletados
  - Validar path para prevenir directory traversal
  - Logar todas as deleções

**`UpdateFileWithBackup(path, remoteFile)`**

- **Responsabilidade**: Atualizar arquivo existente com backup automático
- **Parâmetros**:
  - `path`: Caminho do arquivo
  - `remoteFile`: Estrutura com informações do arquivo remoto
- **Retorno**: Boolean indicando sucesso
- **Pré-condições**: Arquivo local existe
- **Comportamento**:
  1. Criar backup do arquivo atual através de FileBackupManager
  2. Se backup falhar, abortar operação (fail-fast)
  3. Deletar arquivo antigo
  4. Baixar novo arquivo através de DownloadThread
  5. Validar checksum do arquivo baixado
  6. Se validação falhar:
     - Deletar arquivo baixado
     - Restaurar backup
     - Retornar false
  7. Se sucesso:
     - Limpar backup (deletar arquivo de backup)
     - Atualizar metadados
     - Retornar true
- **Transações**: Operação atômica (ou tudo ou nada)
- **Recuperação**: Sempre restaura backup em caso de erro

**`DownloadNewFile(path, remoteFile)`**

- **Responsabilidade**: Baixar arquivo que não existe localmente
- **Parâmetros**: Similar a UpdateFileWithBackup
- **Retorno**: Boolean indicando sucesso
- **Pré-condições**: Arquivo local não existe
- **Comportamento**:
  1. Verificar espaço em disco disponível
  2. Criar diretórios necessários
  3. Baixar arquivo através de DownloadThread
  4. Validar checksum
  5. Se validação falhar, deletar arquivo e retornar false
  6. Atualizar metadados
- **Não usa backup**: Arquivo não existe, não há o que fazer backup

**`GetProgress()`**

- **Responsabilidade**: Obter progresso global de download
- **Retorno**: Estrutura ProgressInfo com:
  - `totalFiles`: Total de arquivos
  - `completedFiles`: Arquivos completos
  - `failedFiles`: Arquivos com falha
  - `currentFile`: Arquivo sendo processado atualmente
  - `percentComplete`: Percentual (0-100)
  - `bytesDownloaded`: Bytes baixados
  - `totalBytes`: Total de bytes
- **Thread Safety**: Thread-safe, usa locks apropriados
- **Performance**: Leitura otimizada, sem bloqueio longo

**`CancelAll()`**

- **Responsabilidade**: Cancelar todas as operações pendentes
- **Comportamento**:
  1. Definir shutdown flag
  2. Parar thread pool
  3. Limpar fila de ações
  4. Aguardar conclusão de downloads ativos (com timeout)
  5. Limpar recursos temporários
- **Graceful Shutdown**: Tentar finalizar graciosamente

**Métodos Privados:**

**`ExecuteActionWithRetry(action)`**

- Executar ação com sistema de retry
- Implementar backoff exponencial
- Máximo de 3 tentativas

**`ValidateAction(action)`**

- Validar integridade da ação
- Verificar tipos, paths, etc.

#### 2.2.2 Classe: DownloadThread (Worker)

**Responsabilidades:**

- Executar download de um único arquivo
- Implementar retry específico do arquivo
- Reportar progresso granular
- Validar integridade após download

**Propriedades:**

- `fileAction`: Ação a executar
- `httpClient`: Cliente HTTP compartilhado
- `progressCallback`: Função de callback para progresso
- `state`: Estado do worker (IDLE, DOWNLOADING, VALIDATING, SUCCESS, FAILED)
- `bytesDownloaded`: Bytes baixados atualmente
- `totalBytes`: Total esperado
- `retryCount`: Contador de tentativas

**Métodos:**

**`Execute()`**

- **Responsabilidade**: Executar download completo
- **Comportamento**:
  1. Validar URL e path
  2. Verificar espaço em disco
  3. Loop de retry (máximo 3):
     a. Tentar baixar arquivo
     b. Se sucesso, sair do loop
     c. Se falha, aguardar retryDelay e tentar novamente
  4. Validar checksum
  5. Atualizar estado
- **Retorno**: Boolean indicando sucesso

**`DownloadFile(url, destinationPath)`**

- **Responsabilidade**: Baixar arquivo HTTP para disco
- **Parâmetros**: URL e caminho de destino
- **Comportamento**:
  1. Abrir conexão HTTP
  2. Configurar timeouts e headers
  3. Criar arquivo temporário
  4. Stream de dados do servidor para arquivo
  5. Chamar callback de progresso periodicamente
  6. Renomear arquivo temporário para final após conclusão
- **Resumir downloads**: Suportar resumo de downloads interrompidos (Range header)

**`ValidateChecksum(filePath, expectedChecksum)`**

- **Responsabilidade**: Validar integridade do arquivo
- **Algoritmo**: SHA256
- **Comportamento**:
  1. Ler arquivo em chunks
  2. Calcular hash SHA256
  3. Comparar com hash esperado
  4. Retornar resultado da comparação
- **Performance**: Leitura eficiente, não carregar arquivo inteiro na memória

#### 2.2.3 Classe: ThreadPool

**Responsabilidades:**

- Gerenciar pool fixo de threads
- Distribuir tarefas entre threads
- Controlar concorrência
- Gerenciar ciclo de vida das threads

**Propriedades:**

- `workers`: Array de 5 threads
- `taskQueue`: Fila thread-safe de tarefas
- `mutex`: Mutex para sincronização da fila
- `conditionVariable`: Condition variable para notificar threads
- `shutdownFlag`: Flag atômica para shutdown
- `activeTaskCount`: Contador atômico de tarefas ativas

**Métodos:**

**`AddTask(task)`**

- Adicionar tarefa à fila
- Notificar uma thread disponível
- Thread-safe

**`WorkerFunction(workerId)`**

- Loop principal de cada worker thread
- Aguardar tarefas na fila
- Executar tarefa quando disponível
- Retornar ao pool após conclusão

**`WaitForCompletion()`**

- Aguardar todas as tarefas concluírem
- Timeout configurável
- Thread-safe

**`Shutdown()`**

- Parar todas as threads graciosamente
- Aguardar conclusão de tarefas ativas
- Limpar recursos

#### 2.2.4 Classe: ManifestParser

**Responsabilidades:**

- Baixar manifest do servidor
- Parsear estrutura JSON
- Validar integridade do manifest
- Comparar versões de manifest

**Métodos:**

**`DownloadManifest(url)`**

- Baixar manifest via HTTP
- Cachear resposta em memória
- Retornar dados brutos

**`ParseJSON(jsonData)`**

- Parsear string JSON
- Validar estrutura obrigatória
- Criar objeto Manifest estruturado
- Validar tipos de dados

**`ValidateStructure(manifest)`**

- Verificar campos obrigatórios
- Validar tipos de dados
- Verificar ranges válidos
- Retornar lista de erros encontrados

**`CompareManifests(local, remote)`**

- Comparar versões de manifest
- Identificar diferenças
- Retornar resultado estruturado

**`SaveManifest(manifest, path)`**

- Serializar manifest para JSON
- Salvar em arquivo local
- Criar backup do manifest anterior

#### 2.2.5 Classe: SemanticVersion

**Responsabilidades:**

- Representar versão semântica
- Comparar versões
- Parsear strings de versão

**Propriedades:**

- `major`: Versão major (int)
- `minor`: Versão minor (int)
- `patch`: Versão patch (int)
- `suffix`: Sufixo opcional (string: "-beta", "-alpha", etc.)

**Métodos:**

**`Parse(versionString)`**

- Parsear string "1.2.3" ou "1.2.3-beta"
- Extrair major, minor, patch
- Extrair sufixo se presente
- Validar formato
- Lançar exceção se formato inválido

**`operator<(other)`**

- Comparar versões
- Lógica:
  1. Comparar major
  2. Se igual, comparar minor
  3. Se igual, comparar patch
  4. Se igual, comparar sufixo:
     - Versão sem sufixo > versão com sufixo
     - Comparar sufixos lexicograficamente

**`operator==(other)`**

- Comparar igualdade
- Todos os componentes devem ser iguais

**`ToString()`**

- Serializar para string
- Formato: "major.minor.patch-suffix"

#### 2.2.6 Classe: FileComparator

**Responsabilidades:**

- Comparar arquivo local com remoto
- Decidir se atualização é necessária
- Considerar políticas de atualização

**Métodos:**

**`Compare(localPath, remoteFile, localMetadata)`**

- **Comportamento**:
  1. Verificar se arquivo local existe
  2. Se não existe: retornar NOT_FOUND
  3. Comparar tamanho
  4. Comparar versão semântica
  5. Comparar checksum
  6. Determinar resultado baseado em diferenças
- **Retorno**: Enum CompareResult

**`NeedsUpdate(compareResult, updatePolicy)`**

- Decidir se precisa atualizar baseado em política:
  - "always": Sempre atualizar
  - "ifNewer": Atualizar se versão mais nova
  - "manual": Nunca atualizar automaticamente

#### 2.2.7 Classe: OrphanFileDetector

**Responsabilidades:**

- Encontrar arquivos locais não presentes no manifest
- Verificar se arquivo deve ser preservado

**Métodos:**

**`FindOrphans(localFiles, manifest)`**

- Percorrer todos os arquivos locais
- Verificar presença no manifest
- Retornar lista de paths órfãos
- Excluir arquivos em whitelist (preservar)

**`ShouldPreserve(path)`**

- Verificar whitelist de arquivos
- Arquivos críticos nunca deletados
- Arquivos de configuração do usuário preservados

#### 2.2.8 Classe: UpdateActionPlanner

**Responsabilidades:**

- Planejar todas as ações necessárias
- Ordenar por prioridade
- Agrupar ações similares

**Métodos:**

**`PlanActions(remoteManifest, localMetadata, localFiles)`**

- Para cada arquivo no manifest remoto:
  - Comparar com local
  - Criar ação apropriada (UPDATE/DOWNLOAD)
- Para cada arquivo local:
  - Verificar se está no manifest
  - Se não, criar ação DELETE
- Ordenar ações por prioridade
- Retornar lista ordenada

**`SortByPriority(actions)`**

- DELETE: Prioridade 0 (mais alta)
- UPDATE: Prioridade 1
- DOWNLOAD: Prioridade 2 (mais baixa)
- Ordenar dentro de cada prioridade alfabeticamente

#### 2.2.9 Classe: FileBackupManager

**Responsabilidades:**

- Criar backups antes de atualizações
- Restaurar backups em caso de erro
- Limpar backups após sucesso
- Manutenção de backups antigos

**Propriedades:**

- `backupBasePath`: Caminho base dos backups
- `activeBackups`: Map de backups ativos (path -> backupPath)

**Métodos:**

**`CreateBackup(filePath)`**

- **Comportamento**:
  1. Verificar se arquivo existe
  2. Gerar timestamp único
  3. Criar caminho de backup: `backups/{relativePath}.backup_{timestamp}`
  4. Criar diretórios necessários
  5. Copiar arquivo para backup
  6. Registrar backup ativo
  7. Retornar caminho do backup
- **Validação**: Verificar espaço antes de criar backup

**`RestoreFromBackup(filePath, backupPath)`**

- Restaurar arquivo do backup
- Validar backup existe
- Copiar backup para localização original
- Validar integridade do arquivo restaurado

**`CleanupBackup(backupPath)`**

- Deletar arquivo de backup
- Remover do registro de backups ativos
- Não falhar se backup já não existe

**`CleanupOldBackups(maxAgeDays)`**

- Encontrar backups mais antigos que maxAgeDays
- Deletar backups antigos
- Manutenção periódica (não bloqueante)

---

### 2.3 UI - Interface ImGui

#### 2.3.1 Classe: LauncherUI

**Responsabilidades:**

- Coordenar todas as telas
- Gerenciar transições entre telas
- Integrar com sistema ImGui
- Gerenciar recursos gráficos

**Propriedades:**

- `downloadScreen`: Instância da tela de download
- `loadingScreen`: Instância da tela de loading
- `currentScreen`: Tela atualmente visível
- `screenSize`: Tamanho da tela (ImVec2)
- `backgroundTexture`: Textura de fundo (opcional)

**Métodos:**

**`Initialize(displaySize, fontPath)`**

- Inicializar contexto ImGui
- Criar instâncias das telas
- Carregar fontes
- Configurar estilo ImGui
- Carregar texturas de fundo

**`Render()`**

- Determinar tela a renderizar
- Chamar Render() da tela apropriada
- Renderizar elementos comuns (background)
- Finalizar frame ImGui

**`ShowDownloadScreen()`**

- Mostrar tela de download
- Esconder outras telas
- Resetar estado da tela

**`ShowLoadingScreen()`**

- Mostrar tela de loading
- Esconder outras telas
- Resetar progresso

**`UpdateProgress(percent, currentFile)`**

- Atualizar progresso na tela ativa
- Thread-safe
- Debounce para evitar updates excessivos

#### 2.3.2 Classe: DownloadScreen

**Responsabilidades:**

- Renderizar tela de download
- Mostrar progresso e status
- Exibir arquivo atual sendo baixado

**Propriedades:**

- `statusText`: Texto de status ("Baixando...", "Verificando...")
- `progressPercent`: Percentual global (0-100)
- `currentFileName`: Nome do arquivo atual
- `currentFileProgress`: Progresso do arquivo atual
- `isVisible`: Flag de visibilidade

**Métodos:**

**`Render()`**

- **Layout**:
  - Window fullscreen (NoTitleBar, NoResize, NoMove)
  - Background (imagem ou cor sólida)
  - Status text no topo (fonte grande)
  - Progress bar no meio
  - File text abaixo (fonte pequena)
- **Posicionamento**: Elementos centralizados horizontalmente, alinhados na parte inferior
- **Cores**: Definidas por tema/configuração

**`UpdateStatus(text)`**

- Atualizar texto de status
- Thread-safe
- Redesenhar UI se necessário

**`UpdateProgress(percent)`**

- Atualizar percentual global
- Atualizar barra de progresso visual

**`UpdateCurrentFile(fileName, fileProgress)`**

- Atualizar nome do arquivo atual
- Atualizar progresso do arquivo
- Formatar string: "arquivo.ext (1.2 MB / 2.0 MB)"

#### 2.3.3 Classe: LoadingScreen

**Responsabilidades:**

- Renderizar tela de loading
- Mostrar progresso de carregamento
- Exibir ação atual sendo executada

**Propriedades:**

- `statusText`: Texto de status
- `progressPercent`: Percentual de progresso
- `actionText`: Texto da ação atual
- `isVisible`: Flag de visibilidade

**Métodos:**

**`Render()`**

- Similar ao DownloadScreen
- Layout específico para loading

**`UpdateStatus(text)`**

- Atualizar status do carregamento

**`UpdateProgress(percent)`**

- Atualizar progresso visual

**`UpdateAction(actionText)`**

- Atualizar texto da ação atual
- Exemplos: "Carregando texturas...", "Carregando modelos..."

**`Show()` / `Hide()`**

- Controlar visibilidade
- Resetar estado ao mostrar

---

### 2.4 Network - Rede e HTTP

#### 2.4.1 Classe: HTTPClient

**Responsabilidades:**

- Encapsular funcionalidades libcurl
- Prover interface simples para downloads
- Gerenciar conexões HTTP
- Implementar retry e timeout

**Propriedades:**

- `curlHandle`: Handle do libcurl (CURL*)
- `timeout`: Timeout em segundos (30)
- `userAgent`: User-Agent string
- `progressCallback`: Callback para progresso

**Métodos:**

**`Initialize()`**

- Inicializar libcurl
- Configurar opções globais
- Criar handle
- Configurar SSL (sem pinning por enquanto)

**`DownloadFile(url, destinationPath, progressCallback)`**

- **Comportamento**:
  1. Abrir conexão HTTP
  2. Configurar timeouts
  3. Configurar callback de progresso
  4. Stream de dados para arquivo
  5. Validar código HTTP (200 OK)
  6. Fechar conexão
- **Retorno**: Boolean indicando sucesso
- **Suporte a Range**: Suportar resumo de downloads (Range header)

**`DownloadToMemory(url)`**

- Baixar para buffer em memória
- Retornar dados como vector<unsigned char>
- Útil para manifests JSON

**`SetProgressCallback(callback)`**

- Configurar callback de progresso
- Chamado periodicamente durante download
- Parâmetros: bytesDownloaded, totalBytes

**`SetTimeout(seconds)`**

- Configurar timeout de conexão
- Timeout de leitura
- Timeout de escrita

#### 2.4.2 Classe: NetworkMonitor

**Responsabilidades:**

- Monitorar conectividade
- Verificar acessibilidade do servidor
- Avaliar qualidade da conexão

**Métodos:**

**`IsConnected()`**

- Verificar conectividade básica
- Usar sistema Android ou ping simples
- Retornar boolean

**`CanReachServer(url)`**

- Verificar se servidor está acessível
- Tentar conexão HTTP HEAD
- Timeout curto (5 segundos)
- Retornar boolean

**`GetConnectionQuality()`**

- Avaliar qualidade da conexão
- Baseado em latência e taxa de erro
- Retornar enum: EXCELLENT, GOOD, POOR, NONE

**`MonitorConnection(callback)`**

- Monitorar continuamente
- Notificar mudanças através de callback
- Executar em thread separada

---

### 2.5 Storage - Sistema de Arquivos

#### 2.5.1 Classe: FileSystem

**Responsabilidades:**

- Operações básicas de arquivo
- Abstração de sistema de arquivos
- Cross-platform compatibility

**Métodos:**

**`FileExists(path)`**

- Verificar existência de arquivo
- Usar stat() ou equivalente
- Retornar boolean

**`CreateDirectory(path)`**

- Criar diretório e pais se necessário
- Similar a mkdir -p
- Retornar boolean

**`DeleteFile(path)`**

- Deletar arquivo
- Validar path
- Retornar boolean

**`CopyFile(source, destination)`**

- Copiar arquivo
- Criar diretório destino se necessário
- Validar espaço disponível
- Retornar boolean

**`MoveFile(source, destination)`**

- Mover arquivo (rename)
- Mais eficiente que copy+delete
- Retornar boolean

**`GetFileSize(path)`**

- Obter tamanho do arquivo
- Retornar size_t
- Lançar exceção se arquivo não existe

**`GetFileChecksum(path, algorithm)`**

- Calcular checksum do arquivo
- Algoritmo: SHA256
- Retornar string hex
- Leitura eficiente (chunks)

**`EnsureDirectoryExists(path)`**

- Garantir que diretório existe
- Criar se não existe
- Não falhar se já existe

#### 2.5.2 Classe: DiskSpaceMonitor

**Responsabilidades:**

- Monitorar espaço em disco
- Alertar espaço insuficiente
- Estimar espaço necessário

**Métodos:**

**`GetAvailableSpace(path)`**

- Obter espaço disponível no filesystem
- Usar statvfs() ou equivalente
- Retornar bytes disponíveis

**`GetUsedSpace(path)`**

- Obter espaço usado
- Calcular recursivamente
- Retornar bytes usados

**`CheckSpace(requiredBytes, path)`**

- Verificar se há espaço suficiente
- Margem de segurança (10% extra)
- Retornar boolean

**`EstimateRequiredSpace(actions)`**

- Estimar espaço necessário para ações
- Somar tamanhos de arquivos a baixar
- Considerar backups temporários
- Retornar bytes estimados

---

### 2.6 JNI - Ponte Java

#### 2.6.1 Classe: JNIBridge

**Responsabilidades:**

- Interface entre código nativo e Java
- Obter informações do Android
- Notificar eventos para Java

**Propriedades:**

- `javaVM`: Ponteiro para JavaVM
- `activity`: Referência global para Activity
- `env`: JNIEnv (obtido por thread)

**Métodos:**

**`Initialize(javaVM, activity)`**

- Armazenar referências
- Obter métodos JNI necessários
- Validar acesso

**`GetStoragePath()`**

- Chamar getExternalFilesDir() via JNI
- Converter jstring para std::string
- Retornar caminho absoluto

**`NotifyComplete()`**

- Chamar método Java para notificar conclusão
- Thread-safe
- Não bloquear thread nativa

**`RequestStoragePermission()`**

- Solicitar permissão de storage se necessário
- Verificar permissão atual
- Retornar boolean

---

## 3. Estruturas de Dados

### 3.1 FileAction

```cpp
struct FileAction {
    ActionType type;              // DELETE, UPDATE, DOWNLOAD, KEEP
    std::string path;             // Caminho relativo do arquivo
    ManifestFile* remoteFile;     // Dados do arquivo remoto (NULL se DELETE)
    std::string reason;           // Razão da ação (para logging)
    int priority;                 // Prioridade (0 = maior)
    size_t estimatedSize;         // Tamanho estimado em bytes
};
```

### 3.2 ManifestFile

```cpp
struct ManifestFile {
    std::string path;             // Caminho relativo
    std::string url;              // URL completa para download
    size_t size;                  // Tamanho em bytes
    std::string checksum;         // SHA256 checksum
    std::string version;          // Versão semântica
    bool required;                // Arquivo obrigatório
    std::string updatePolicy;     // "always", "ifNewer", "manual"
};
```

### 3.3 FileMetadata

```cpp
struct FileMetadata {
    std::string path;             // Caminho relativo
    std::string version;          // Versão semântica
    std::string checksum;         // SHA256 checksum
    size_t size;                  // Tamanho em bytes
    time_t lastModified;          // Timestamp de modificação
    time_t downloadedAt;          // Timestamp de download
    bool hasBackup;               // Flag de backup existente
    std::string backupPath;       // Caminho do backup (se existir)
};
```

### 3.4 ProgressInfo

```cpp
struct ProgressInfo {
    size_t totalFiles;            // Total de arquivos
    size_t completedFiles;        // Arquivos completos
    size_t failedFiles;           // Arquivos com falha
    std::string currentFile;      // Arquivo atual
    float percentComplete;        // Percentual (0-100)
    size_t bytesDownloaded;       // Bytes baixados
    size_t totalBytes;            // Total de bytes
    std::string status;           // Status textual
};
```

---

## 4. Fluxos de Execução Detalhados

### 4.1 Fluxo de Inicialização Completo

1. **App Android inicia**
   - GTASA.onCreate() chamado
   - System.loadLibrary("launcher") executado

2. **JNI_OnLoad da liblauncher.so**
   - Registrar funções JNI
   - Inicializar variáveis globais
   - Retornar JNI_VERSION_1_6

3. **SAMP.onCreate() continua**
   - Chamar setStoragePath() (JNI)
   - Chamar initializeLauncher(this) (JNI)

4. **Launcher::Initialize()**
   - Obter storage path via JNI
   - Validar storage path
   - Criar diretórios necessários
   - Inicializar componentes na ordem:
     a. FileSystem
     b. DiskSpaceMonitor
     c. NetworkMonitor
     d. HTTPClient
     e. FileBackupManager
     f. DownloadManager
     g. ManifestParser
     h. LauncherUI
   - Carregar metadados locais
   - Transicionar para estado INITIALIZING

5. **Validação inicial**
   - Verificar conectividade
   - Verificar espaço em disco
   - Verificar permissões

6. **Transição para CHECKING**
   - EstadoMachine::TransitionTo(CHECKING)

### 4.2 Fluxo de Verificação e Atualização

1. **Estado: CHECKING**
   - Baixar manifest remoto
   - Carregar manifest local (se existir)
   - Comparar versões de manifest

2. **Estado: COMPARING**
   - Para cada arquivo no manifest remoto:
     - Buscar metadados locais
     - Comparar versão, checksum, tamanho
     - Determinar ação necessária
   - Criar lista de ações

3. **Estado: DETECTING_ORPHANS**
   - Listar todos os arquivos locais
   - Verificar presença no manifest
   - Identificar arquivos órfãos
   - Criar ações DELETE para órfãos

4. **Estado: PLANNING**
   - Consolidar todas as ações
   - Ordenar por prioridade
   - Validar plano
   - Verificar espaço em disco necessário

5. **Estado: DELETING**
   - Processar ações DELETE sequencialmente
   - Para cada DELETE:
     - Validar arquivo não é crítico
     - Deletar arquivo
     - Remover metadados
     - Atualizar progresso

6. **Estado: DOWNLOADING**
   - Processar ações UPDATE e DOWNLOAD em paralelo (5 threads)
   - Para cada ação:
     - UPDATE: UpdateFileWithBackup()
     - DOWNLOAD: DownloadNewFile()
   - Atualizar progresso continuamente

7. **Estado: VALIDATING**
   - Validar checksums de todos os arquivos baixados
   - Se algum falhar:
     - Re-baixar arquivo específico
     - Se falhar novamente, reportar erro

8. **Estado: UPDATING_METADATA**
   - Atualizar metadados de arquivos modificados
   - Salvar manifest local
   - Salvar files_metadata.json
   - Limpar backups bem-sucedidos

9. **Estado: COMPLETE**
   - Notificar Java que launcher está completo
   - Sistema pronto para carregar jogo

### 4.3 Fluxo de Download Individual

1. **DownloadThread::Execute() inicia**
   - Validar URL e path
   - Verificar espaço em disco

2. **Loop de retry (máximo 3 tentativas)**
   a. **Tentativa de download**
      - HTTPClient::DownloadFile()
      - Stream de dados para arquivo temporário
      - Callback de progresso chamado periodicamente

   b. **Se sucesso**
      - Sair do loop
      - Continuar para validação

   c. **Se falha**
      - Incrementar contador de retry
      - Se retry < 3:
        - Aguardar retryDelay segundos
        - Tentar novamente
      - Se retry == 3:
        - Reportar falha
        - Retornar false

3. **Validação de checksum**
   - FileVerifier::ValidateChecksum()
   - Comparar com checksum esperado
   - Se inválido:
     - Deletar arquivo baixado
     - Retornar false (trigger retry)

4. **Finalização**
   - Renomear arquivo temporário para final
   - Atualizar metadados
   - Reportar sucesso

### 4.4 Fluxo de Atualização com Backup

1. **UpdateFileWithBackup() inicia**
   - Verificar arquivo local existe

2. **Criar backup**
   - FileBackupManager::CreateBackup()
   - Se falhar: abortar operação (fail-fast)

3. **Deletar arquivo antigo**
   - FileSystem::DeleteFile()
   - Se falhar: restaurar backup e abortar

4. **Baixar novo arquivo**
   - DownloadThread::Execute()
   - Se falhar: restaurar backup e abortar

5. **Validar arquivo baixado**
   - FileVerifier::ValidateChecksum()
   - Se inválido:
     - Deletar arquivo baixado
     - Restaurar backup
     - Abortar

6. **Sucesso**
   - FileBackupManager::CleanupBackup()
   - Atualizar metadados
   - Retornar true

### 4.5 Fluxo de Loading Screen

1. **Jogo inicia carregamento**
   - CNetGame::CNetGame() ou similar
   - CJavaWrapper::ShowLoadingScreen() chamado

2. **Launcher intercepta**
   - Verificar se launcher está disponível
   - Se sim: Launcher::ShowLoadingScreen()
   - Se não: fallback para Java

3. **LoadingScreen::Show()**
   - SetVisible(true)
   - Resetar progresso para 0
   - Resetar textos

4. **Durante carregamento**
   - Callbacks do jogo chamam UpdateProgress()
   - LoadingScreen::Render() a cada frame
   - Atualizar elementos visuais

5. **Carregamento completo**
   - CJavaWrapper::HideLoadingScreen() chamado
   - LoadingScreen::Hide()
   - SetVisible(false)

---

## 5. Tratamento de Erros e Recuperação

### 5.1 Estratégias de Tratamento

**Fail-Fast**

- Erros críticos param operação imediatamente
- Não continuar em estado inconsistente
- Exemplo: falha ao criar backup aborta atualização

**Retry com Backoff**

- Tentativas automáticas com delay crescente
- Máximo de 3 tentativas
- Delay fixo de 1 segundo entre tentativas

**Rollback Automático**

- Backup sempre restaurado em caso de erro
- Sistema sempre volta ao estado anterior conhecido
- Nunca deixar sistema em estado corrompido

**Graceful Degradation**

- Se funcionalidade opcional falha, continuar com outras
- Exemplo: falha ao limpar backup não impede sucesso da operação

**Logging Detalhado**

- Todos os erros logados com contexto
- Stack traces quando possível
- Informações para debugging

### 5.2 Cenários de Erro Específicos

**Manifest não acessível**

- Tentar usar manifest local (se existir)
- Alertar usuário sobre problema de conectividade
- Oferecer opção de retry manual

**Espaço em disco insuficiente**

- Verificar antes de iniciar downloads
- Alertar usuário imediatamente
- Não iniciar downloads até espaço disponível

**Arquivo corrompido após download**

- Detectar através de checksum
- Re-baixar arquivo automaticamente
- Se falhar novamente, reportar erro específico

**Thread pool esgotado**

- Aguardar thread disponível
- Não criar novas threads
- Timeout se espera muito longa

**Arquivo crítico marcado para delete**

- Detectar através de whitelist
- Não deletar
- Alertar e logar

---

## 6. Performance e Otimizações

### 6.1 Otimizações de Download

**Download paralelo**

- 5 threads simultâneas
- Máximo throughput sem sobrecarregar

**Resumo de downloads**

- Suporte a Range header
- Continuar downloads interrompidos
- Evitar re-baixar dados já transferidos

**Validação incremental**

- Validar arquivos apenas modificados
- Cache de checksums

**Compressão HTTP**

- Suporte a gzip/deflate
- Reduzir bandwidth

### 6.2 Otimizações de UI

**Renderização eficiente**

- Atualizar UI apenas quando necessário
- Debounce de updates de progresso
- Limitar FPS de atualização (30 FPS suficiente)

**Caching de texturas**

- Carregar texturas uma vez
- Reutilizar entre frames

### 6.3 Otimizações de Memória

**Streaming de arquivos**

- Não carregar arquivos inteiros na memória
- Processar em chunks

**Pool de buffers**

- Reutilizar buffers de download
- Reduzir alocações

**Limpeza proativa**

- Limpar backups imediatamente após sucesso
- Limpar arquivos temporários regularmente

### 6.4 Otimizações de I/O

**Operações assíncronas**

- Não bloquear thread principal
- I/O em threads separadas

**Batching de operações**

- Agrupar operações de I/O quando possível
- Reduzir syscalls

**Cache de metadados**

- Manter metadados em memória
- Salvar apenas quando necessário

---

## 7. Thread Safety e Concorrência

### 7.1 Princípios

**Imutabilidade quando possível**

- Estruturas de dados imutáveis preferidas
- Reduz necessidade de locks

**Lock-free quando possível**

- Atomic operations para contadores
- Read-copy-update patterns

**Locks granulares**

- Locks específicos, não globais
- Reduz contenção

**Deadlock prevention**

- Ordem consistente de aquisição de locks
- Timeout em locks quando possível

### 7.2 Áreas Críticas

**Fila de ações**

- Mutex para acesso
- Condition variable para notificação

**Progresso global**

- Atomic counters quando possível
- Mutex para estruturas complexas

**Metadados**

- Read-write locks
- Múltiplos leitores, escritor exclusivo

**Estado do launcher**

- Mutex para transições
- Validação de estado

---

## 8. Testabilidade

### 8.1 Abstrações para Testes

**Interfaces para dependências**

- HTTPClient como interface
- Mock implementations para testes

**Injeção de dependências**

- Componentes recebem dependências via construtor
- Facilita substituição em testes

**Separação de lógica**

- Lógica de negócio isolada de I/O
- Testável sem filesystem/network

### 8.2 Testes Recomendados

**Unit tests**

- Cada classe testada isoladamente
- Mocks para dependências
- Cobertura de casos de erro

**Integration tests**

- Testar interação entre componentes
- Testar fluxos completos

**Performance tests**

- Medir throughput de download
- Medir uso de memória
- Identificar bottlenecks

---

## 9. Manutenibilidade

### 9.1 Documentação

**Código auto-documentado**

- Nomes descritivos
- Comentários apenas quando necessário
- Documentação de APIs públicas

**Documentação de decisões**

- ADRs (Architecture Decision Records)
- Explicar escolhas de design

### 9.2 Modularidade

**Baixo acoplamento**

- Componentes independentes
- Interfaces bem definidas

**Alta coesão**

- Responsabilidades claras
- Funcionalidades relacionadas juntas

### 9.3 Versionamento

**Semantic versioning**

- Versionamento claro
- Changelog mantido

**Compatibilidade**

- Backward compatibility quando possível
- Breaking changes documentados

---

## 10. Segurança

### 10.1 Validações

**Input validation**

- Validar todos os inputs
- Sanitizar paths
- Prevenir directory traversal

**Checksum verification**

- Sempre validar checksums
- Prevenir arquivos corrompidos ou maliciosos

### 10.2 Segurança de Dados

**Permissões mínimas**

- Apenas permissões necessárias
- Não acessar dados desnecessários

**Armazenamento seguro**

- Dados sensíveis não em logs
- Limpeza de dados temporários

### 10.3 Network Security

**HTTPS obrigatório**

- Sempre usar HTTPS para downloads
- Validar certificados SSL
- (Certificate pinning futuro)

---

## 11. Logging e Debugging

### 11.1 Sistema de Logging

**Níveis de log**

- DEBUG: Informações detalhadas
- INFO: Informações gerais
- WARN: Avisos
- ERROR: Erros
- FATAL: Erros críticos

**Contexto**

- Todos os logs incluem contexto
- Stack traces em erros
- Timestamps precisos

**Destinos**

- Arquivo de log
- Console (debug builds)
- Sistema de crash reporting (Firebase)

### 11.2 Debugging

**Flags de debug**

- Modo verbose configurável
- Logs adicionais em debug builds

**Informações úteis**

- Estado atual do sistema
- Progresso detalhado
- Estatísticas de performance

---

Esta documentação técnica detalha toda a arquitetura do sistema launcher, seguindo boas práticas de programação e princípios de engenharia de software. Cada componente é descrito em termos de responsabilidades, propriedades, métodos, comportamentos e considerações de design.
