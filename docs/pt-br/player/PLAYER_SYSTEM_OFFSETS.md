# Sistema de Personagem - Tabela de Offsets

**Legenda:**
- ✅ = Implementado no APK
- ⚠️ = Parcialmente implementado
- ❌ = Não implementado

> **Nota:** Offsets 32-bit precisam de `+1` para ARM Thumb mode

---

## CPed - Funções Base

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ⚠️ | `CPed::CPed` | `0x49FA19` | `0x594D1C` | Construtor |
| ⚠️ | `CPed::~CPed` | - | `0x59541C` | Destrutor |
| ⚠️ | `CPed::Initialise` | `0x49FA29` | `0x595898` | Inicializa ped |
| ⚠️ | `CPed::SetModelIndex` | `0x49FAD5` | `0x595954` | Define modelo |
| ❌ | `CPed::DeleteRwObject` | `0x49FC87` | `0x595BB4` | Remove objeto RW |
| ⚠️ | `CPed::SetCharCreatedBy` | `0x49FA29` | `0x5958AC` | Define criador |
| ❌ | `CPed::SetPedDefaultDecisionMaker` | - | `0x595954` | IA padrão |

## CPed - Estado e Controle

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ⚠️ | `CPed::SetPedState` | `0x4A1EA5` | `0x597FF0` | Define estado |
| ❌ | `CPed::GetPedStateString` | `0x4A8569` | `0x59F690` | String do estado |
| ❌ | `CPed::IsPedInControl` | `0x4A18D1` | `0x597A0C` | Está em controle |
| ❌ | `CPed::CanSetPedState` | `0x4A4BED` | - | Pode mudar estado |
| ❌ | `CPed::CanBeDeleted` | `0x4A4C89` | `0x59B0D8` | Pode ser deletado |
| ❌ | `CPed::CanBeDeletedEvenInVehicle` | - | `0x59B150` | Pode deletar em veículo |
| ❌ | `CPed::CanBeArrested` | `0x4A4C0D` | `0x59B044` | Pode ser preso |
| ❌ | `CPed::IsPedShootable` | `0x4A4BBF` | - | Pode levar tiro |
| ❌ | `CPed::IsPointerValid` | `0x4A7371` | `0x59DF64` | Ponteiro válido |

## CPed - Movimento e Física

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::ProcessControl` | `0x4A2541` | `0x598730` | Update por frame |
| ❌ | `CPed::UpdatePosition` | `0x4A1999` | `0x597B04` | Atualiza posição |
| ❌ | `CPed::CalculateNewVelocity` | `0x4A1365` | `0x597518` | Calcula velocidade |
| ❌ | `CPed::CalculateNewOrientation` | `0x4A1DC5` | - | Calcula orientação |
| ❌ | `CPed::ProcessBuoyancy` | `0x4A1F01` | `0x598078` | Flutuação |
| ❌ | `CPed::ProcessEntityCollision` | `0x4A339D` | - | Colisão com entidade |
| ⚠️ | `CPed::Teleport` | - | `0x59DD90` | Teleporta |
| ❌ | `CPed::SetMoveState` | `0x4A0C31` | - | Estado de movimento |
| ❌ | `CPed::SetMoveAnim` | `0x4A0C39` | - | Animação de movimento |
| ❌ | `CPed::SetMoveAnimSpeed` | `0x4A0F6D` | - | Velocidade da anim |

## CPed - Olhar e Mira

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::SetLookFlag(CEntity*)` | `0x4A11C9` | - | Olha para entidade |
| ❌ | `CPed::SetLookFlag(float)` | `0x4A114D` | `0x597284` | Olha para ângulo |
| ❌ | `CPed::ClearLookFlag` | `0x4A1081` | `0x59718C` | Limpa look |
| ❌ | `CPed::SetLook(CEntity*)` | - | `0x59DB98` | Olha (simples) |
| ❌ | `CPed::SetLook(float)` | - | `0x59DAC8` | Olha (ângulo) |
| ❌ | `CPed::ClearLook` | - | `0x5973C0` | Limpa |
| ❌ | `CPed::Look` | - | `0x59DCA4` | Processa look |
| ❌ | `CPed::SetLookTimer` | `0x4A7135` | `0x59DC80` | Timer de look |
| ❌ | `CPed::SetAimFlag(CEntity*)` | `0x4A12F7` | `0x597474` | Mira entidade |
| ❌ | `CPed::SetAimFlag(float)` | `0x4A125B` | `0x5973C4` | Mira ângulo |
| ❌ | `CPed::ClearAimFlag` | `0x4A1031` | - | Limpa mira |
| ❌ | `CPed::SetWeaponLockOnTarget` | `0x4A8345` | `0x59F3EC` | Lock-on |
| ❌ | `CPed::UseFreeAimMagnetize` | `0x4A8341` | `0x59F3E4` | Mira livre |

## CPed - Animação

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::PreRender` | - | `0x59C240` | Pré-render |
| ❌ | `CPed::PreRenderAfterTest` | `0x4A5981` | `0x59C254` | Pós-teste |
| ❌ | `CPed::StopNonPartialAnims` | `0x4A1105` | `0x59721C` | Para anims |
| ❌ | `CPed::RestartNonPartialAnims` | `0x4A1129` | - | Reinicia anims |
| ❌ | `CPed::DoFootLanded` | `0x4A4381` | `0x59A79C` | Pé no chão |
| ❌ | `CPed::PlayFootSteps` | `0x4A2DE5` | - | Sons de passo |
| ❌ | `CPed::RemoveWeaponAnims` | `0x4ADE25` | `0x5A6E7C` | Remove anims arma |
| ❌ | `CPed::ShoulderBoneRotation` | `0x4A5741` | - | Rotação ombro |
| ❌ | `CPed::TurnBody` | - | `0x59DCA8` | Gira corpo |
| ❌ | `CPed::SetIdle` | - | `0x59DA8C` | Define idle |
| ❌ | `CPed::IsPlayingHandSignal` | `0x4A829D` | `0x59F2F0` | Sinal com mão |
| ❌ | `CPed::StopPlayingHandSignal` | `0x4A82B7` | `0x59F318` | Para sinal |

## CPed - Bones

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ✅ | `CPed::GetBonePosition` | `0x4A4B0D` | `0x59AEE4` | Posição do bone |
| ❌ | `CPed::GetHeadAndFootPositions` | `0x4A83F1` | `0x59F4C4` | Cabeça e pés |
| ❌ | `CPed::IsPedHeadAbovePos` | `0x4AD0C5` | `0x5A60F0` | Cabeça acima |

## CPed - Veículos

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ⚠️ | `CPed::SetPedPositionInCar` | `0x4A73B1` | - | Posição no carro |
| ❌ | `CPed::UpdateStatEnteringVehicle` | - | `0x59ECFC` | Stats entrando |
| ❌ | `CPed::UpdateStatLeavingVehicle` | - | `0x5956AC` | Stats saindo |
| ❌ | `CPed::RemoveWeaponWhenEnteringVehicle` | - | - | Esconde arma |
| ❌ | `CPed::ReplaceWeaponWhenExitingVehicle` | - | `0x59BBC4` | Mostra arma |

## CPed - Anexos/Objetos

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::AttachPedToEntity` | `0x4A78BD` | `0x59E844` | Anexa a entidade |
| ❌ | `CPed::AttachPedToBike` | `0x4A7CB5` | `0x59EAEC` | Anexa a moto |
| ❌ | `CPed::DettachPedFromEntity` | `0x4A7CF5` | - | Desanexa |
| ❌ | `CPed::PositionAttachedPed` | `0x4A7A49` | `0x59E844` | Posição anexada |
| ❌ | `CPed::GiveObjectToPedToHold` | `0x4A7F43` | `0x59EE30` | Dá objeto |
| ❌ | `CPed::GetEntityThatThisPedIsHolding` | - | `0x59ED54` | Obtém entidade |
| ❌ | `CPed::DropEntityThatThisPedIsHolding` | - | `0x59B6BC` | Solta |
| ❌ | `CPed::PutOnGoggles` | `0x4A4F55` | - | Coloca óculos |
| ❌ | `CPed::TakeOffGoggles` | `0x4A503D` | - | Remove óculos |
| ❌ | `CPed::AddGogglesModel` | `0x4A4EAD` | `0x59B38C` | Modelo óculos |
| ❌ | `CPed::RemoveGogglesModel` | `0x4A4EF9` | `0x59B400` | Remove modelo |

## CPed - Dano/Morte

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::RemoveBodyPart` | `0x4AD00D` | `0x5A5FE4` | Remove parte |
| ❌ | `CPed::SpawnFlyingComponent` | `0x4AD03F` | - | Parte voando |
| ❌ | `CPed::DoesLOSBulletHitPed` | `0x4AD041` | - | Bala atingiu |
| ❌ | `CPed::KillPedWithCar` | `0x4AD139` | - | Mata com carro |
| ❌ | `CPed::DeadPedMakesTyresBloody` | `0x55DBD9` | `0x67E79C` | Pneus sangue |
| ❌ | `CPed::CreateDeadPedMoney` | `0x320DD1` | - | Dinheiro morte |
| ❌ | `CPed::CreateDeadPedPickupCoors` | - | `0x3E8E20` | Coords pickup |

## CPed - Áudio/Fala

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPed::Say` | - | `0x5A57C4` | Fala |
| ❌ | `CPed::SayScript` | - | `0x5A5798` | Fala script |
| ❌ | `CPed::GetPedTalking` | `0x4AC96D` | `0x5A5768` | Está falando |
| ❌ | `CPed::EnablePedSpeech` | `0x4AC97D` | `0x5A5778` | Habilita fala |
| ❌ | `CPed::DisablePedSpeech` | `0x4AC975` | `0x5A5770` | Desabilita |
| ❌ | `CPed::EnablePedSpeechForScriptSpeech` | - | `0x5A5788` | Para scripts |
| ❌ | `CPed::DisablePedSpeechForScriptSpeech` | - | `0x5A5780` | Desabilita scripts |
| ❌ | `CPed::PedIsInvolvedInConversation` | - | `0x3C7F24` | Em conversa |
| ❌ | `CPed::PedIsReadyForConversation` | - | `0x3C5E2C` | Pronto |

---

## CPlayerPed - Inicialização

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ⚠️ | `CPlayerPed::CPlayerPed` | `0x4C367D` | `0x5C0BAC` | Construtor |
| ⚠️ | `CPlayerPed::~CPlayerPed` | `0x4C3901` | `0x5C0EFC` | Destrutor |
| ⚠️ | `CPlayerPed::SetupPlayerPed` | `0x4C39A5` | `0x5C0FD4` | Setup |
| ⚠️ | `CPlayerPed::DeactivatePlayerPed` | `0x4C3AD5` | `0x5C1140` | Desativa |
| ⚠️ | `CPlayerPed::ReactivatePlayerPed` | `0x4C3AED` | `0x5C1158` | Reativa |
| ⚠️ | `CPlayerPed::RemovePlayerPed` | `0x4C3A61` | `0x5C10B4` | Remove |
| ❌ | `CPlayerPed::SetInitialState` | `0x4C37B5` | `0x5C0D50` | Estado inicial |
| ⚠️ | `CPlayerPed::Load` | `0x4851E9` | `0x5774B8` | Carrega save |
| ⚠️ | `CPlayerPed::Save` | `0x485163` | `0x577390` | Salva |

## CPlayerPed - Controle

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPlayerPed::ProcessControl` | `0x4C47E9` | `0x5C2088` | Update principal |
| ❌ | `CPlayerPed::ProcessPlayerWeapon` | `0x4C682F` | `0x5C4310` | Arma |
| ❌ | `CPlayerPed::ProcessWeaponSwitch` | `0x4C5919` | `0x5C3278` | Troca arma |
| ❌ | `CPlayerPed::ProcessAnimGroups` | `0x4C5E61` | `0x5C37E0` | Grupos anim |
| ❌ | `CPlayerPed::ProcessGroupBehaviour` | `0x4C6211` | `0x5C3C70` | Grupo |
| ❌ | `CPlayerPed::SetRealMoveAnim` | `0x4C3B05` | `0x5C1170` | Anim real |
| ❌ | `CPlayerPed::ReApplyMoveAnims` | `0x4C668D` | `0x5C40EC` | Reaplica anims |
| ❌ | `CPlayerPed::SetMoveAnim` | `0x4CA6D9` | `0x5C9C18` | Define anim |
| ❌ | `CPlayerPed::SetPlayerMoveBlendRatio` | `0x4C9A75` | `0x5C7AD4` | Blend ratio |
| ❌ | `CPlayerPed::MovementDisabledBecauseOfTargeting` | `0x4C7D47` | `0x5C5AA4` | Movimento bloqueado |

## CPlayerPed - Targeting

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPlayerPed::FindWeaponLockOnTarget` | `0x4C6D65` | `0x5C494C` | Busca alvo |
| ❌ | `CPlayerPed::FindNextWeaponLockOnTarget` | `0x4C7D4D` | `0x5C5AAC` | Próximo alvo |
| ❌ | `CPlayerPed::EvaluateTarget` | `0x4C739D` | `0x5C5020` | Avalia alvo |
| ❌ | `CPlayerPed::EvaluateNeighbouringTarget` | `0x4C826D` | `0x5C602C` | Avalia vizinhos |
| ❌ | `CPlayerPed::FindTargetPriority` | `0x4C7C49` | `0x5C597C` | Prioridade |
| ❌ | `CPlayerPed::HandleTapToTarget` | `0x4C849D` | `0x5C623C` | Tap to target |
| ❌ | `CPlayerPed::HandleMeleeTargeting` | `0x4C8D8D` | `0x5C6BE0` | Mira melee |
| ❌ | `CPlayerPed::Compute3rdPersonMouseTarget` | `0x4C901D` | `0x5C6F1C` | Mouse 3ª pessoa |
| ❌ | `CPlayerPed::Clear3rdPersonMouseTarget` | `0x4C57E5` | `0x5C310C` | Limpa mouse |
| ❌ | `CPlayerPed::ClearWeaponTarget` | `0x4C58E5` | `0x5C3228` | Limpa alvo |
| ❌ | `CPlayerPed::RotatePlayerToTrackTarget` | `0x4C7D45` | `0x5C5AA0` | Rotaciona |
| ❌ | `CPlayerPed::CanIKReachThisTarget` | `0x4C7321` | `0x5C4F90` | IK alcança |
| ❌ | `CPlayerPed::DoesTargetHaveToBeBroken` | `0x4C8EFD` | `0x5C6D9C` | Quebrar lock |
| ❌ | `CPlayerPed::PedCanBeTargettedVehicleWise` | `0x4C6A81` | `0x5C45CC` | Alvo em veículo |
| ❌ | `CPlayerPed::DisplayTargettingDebug` | `0x4C7511` | `0x5C51B0` | Debug |
| ❌ | `CPlayerPed::GetWeaponRadiusOnScreen` | `0x4C69E9` | `0x5C4528` | Raio na tela |
| ❌ | `CPlayerPed::UpdateCameraWeaponModes` | `0x4C6875` | `0x5C436C` | Câmera |

## CPlayerPed - Grupo

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPlayerPed::TellGroupToStartFollowingPlayer` | `0x4C4555` | `0x5C1D34` | Seguir |
| ❌ | `CPlayerPed::MakeThisPedJoinOurGroup` | `0x4C9D59` | `0x5C7E00` | Juntar |
| ❌ | `CPlayerPed::DisbandPlayerGroup` | `0x4C9CED` | `0x5C7D6C` | Dissolver |
| ❌ | `CPlayerPed::MakePlayerGroupDisappear` | `0x4CA2D9` | `0x5C8524` | Desaparecer |
| ❌ | `CPlayerPed::MakePlayerGroupReappear` | `0x4CA365` | `0x5C85D4` | Reaparecer |
| ❌ | `CPlayerPed::ForceGroupToAlwaysFollow` | `0x4CA111` | `0x5C82BC` | Sempre seguir |
| ❌ | `CPlayerPed::ForceGroupToNeverFollow` | `0x4CA135` | `0x5C82F0` | Nunca seguir |
| ❌ | `CPlayerPed::MakeGroupRespondToPlayerTakingDamage` | `0x4CA251` | `0x5C8464` | Responder dano |

## CPlayerPed - Stamina

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPlayerPed::HandlePlayerBreath` | `0x4C56F5` | `0x5C3010` | Fôlego |
| ❌ | `CPlayerPed::ResetPlayerBreath` | `0x4CA68D` | `0x5C895C` | Reseta fôlego |
| ❌ | `CPlayerPed::HandleSprintEnergy` | `0x4C5809` | `0x5C3140` | Energia sprint |
| ❌ | `CPlayerPed::ResetSprintEnergy` | `0x4CA401` | `0x5C868C` | Reseta energia |
| ❌ | `CPlayerPed::ControlButtonSprint` | `0x4CA415` | `0x5C86B8` | Botão sprint |
| ❌ | `CPlayerPed::GetButtonSprintResults` | `0x4C6701` | `0x5C41A0` | Resultado |

## CPlayerPed - Estado

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPlayerPed::SetWantedLevel` | `0x4C9719` | `0x5C76D0` | Wanted level |
| ❌ | `CPlayerPed::SetWantedLevelNoDrop` | `0x4C9729` | `0x5C76E8` | Sem diminuir |
| ❌ | `CPlayerPed::CheatWantedLevel` | `0x4C9739` | `0x5C7700` | Via cheat |
| ❌ | `CPlayerPed::Busted` | `0x4C970D` | `0x5C76C0` | Preso |
| ❌ | `CPlayerPed::AnnoyPlayerPed` | `0x4C9A15` | `0x5C7A74` | Irritar |
| ❌ | `CPlayerPed::PlayerWantsToAttack` | `0x4CA189` | `0x5C8358` | Quer atacar |
| ❌ | `CPlayerPed::PlayerHasJustAttackedSomeone` | `0x4CA185` | `0x5C8354` | Atacou alguém |
| ❌ | `CPlayerPed::ClearAdrenaline` | `0x4C9A49` | `0x5C7AA8` | Limpa adrenalina |
| ❌ | `CPlayerPed::DoStuffToGoOnFire` | `0x4C99FD` | `0x5C7A58` | Pega fogo |
| ❌ | `CPlayerPed::KeepAreaAroundPlayerClear` | `0x4C9749` | `0x5C6F1C` | Área limpa |
| ❌ | `CPlayerPed::IsHidden` | `0x4C665D` | - | Escondido |

---

## CPedModelInfo

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CPedModelInfo::SetClump` | `0x3866C5` | `0x45D514` | Define clump |
| ❌ | `CPedModelInfo::DeleteRwObject` | `0x38687D` | `0x45D744` | Remove RW |
| ❌ | `CPedModelInfo::GetModelType` | `0x386CD1` | `0x45DD2C` | Tipo do modelo |
| ❌ | `CPedModelInfo::SetFaceTexture` | `0x386937` | `0x45D890` | Textura face |
| ❌ | `CPedModelInfo::IncrementVoice` | `0x386C49` | `0x45DC50` | Próxima voz |
| ❌ | `CPedModelInfo::CreateHitColModelSkinned` | `0x386711` | `0x45D578` | Colisão skinned |
| ❌ | `CPedModelInfo::AnimatePedColModelSkinned` | `0x386959` | `0x45D8D0` | Anima colisão |
| ❌ | `CPedModelInfo::AnimatePedColModelSkinnedWorld` | `0x386B25` | `0x45DAD8` | World space |
| ❌ | `CPedModelInfo::AddXtraAtomics` | `0x386935` | `0x45D88C` | Extras |

---

## CAnimManager

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CAnimManager::Initialise` | `0x471F95` | - | Inicializa |
| ❌ | `CAnimManager::Shutdown` | `0x38DCA1` | `0x466668` | Finaliza |
| ⚠️ | `CAnimManager::AddAnimation(AssocGroupId)` | `0x38E069` | `0x466B64` | Adiciona anim |
| ⚠️ | `CAnimManager::BlendAnimation` | `0x38E341` | - | Blend |
| ❌ | `CAnimManager::AddAnimationAndSync` | `0x38E169` | - | Adiciona sync |
| ❌ | `CAnimManager::GetAnimation(char*)` | `0x38DF85` | - | Por nome |
| ❌ | `CAnimManager::GetAnimation(uint)` | `0x38DFC1` | - | Por índice |
| ❌ | `CAnimManager::GetAnimAssociation` | `0x38E039` | `0x466B34` | Associação |
| ⚠️ | `CAnimManager::LoadAnimFile(char*)` | `0x38E629` | - | Carrega IFP |
| ❌ | `CAnimManager::LoadAnimFile(RwStream*)` | `0x38EA3D` | - | De stream |
| ❌ | `CAnimManager::LoadAnimFiles` | `0x38E561` | - | Todos arquivos |
| ❌ | `CAnimManager::RegisterAnimBlock` | `0x38E971` | - | Registra bloco |
| ❌ | `CAnimManager::AddAnimBlockRef` | `0x38F905` | `0x4688B4` | Ref de bloco |
| ❌ | `CAnimManager::RemoveAnimBlockRef` | `0x38F91D` | `0x4688D4` | Remove ref |
| ❌ | `CAnimManager::RemoveAnimBlock` | `0x38F841` | `0x4687D4` | Remove bloco |
| ❌ | `CAnimManager::CreateAnimAssocGroups` | `0x38E689` | `0x467328` | Cria grupos |
| ❌ | `CAnimManager::UncompressAnimation` | `0x38DD55` | - | Descomprime |
| ❌ | `CAnimManager::GetFirstAssocGroup` | `0x38DF45` | `0x4669DC` | Primeiro grupo |
| ❌ | `CAnimManager::GetAnimBlockName` | `0x38E009` | - | Nome do bloco |
| ❌ | `CAnimManager::GetAnimationBlockIndex` | `0x38DEE9` | `0x46695C` | Índice |

---

## CAEPedAudioEntity

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CAEPedAudioEntity::Initialise` | `0x399A51` | - | Inicializa |
| ❌ | `CAEPedAudioEntity::Terminate` | `0x399A81` | - | Finaliza |
| ❌ | `CAEPedAudioEntity::Service` | `0x399B25` | - | Update |
| ❌ | `CAEPedAudioEntity::HandleFootstepEvent` | `0x39852D` | - | Passos |
| ❌ | `CAEPedAudioEntity::HandleLandingEvent` | `0x398BB1` | - | Aterrissagem |
| ❌ | `CAEPedAudioEntity::HandlePedHit` | `0x398ED9` | - | Impacto |
| ❌ | `CAEPedAudioEntity::HandlePedSwing` | `0x398D01` | - | Balanço |
| ❌ | `CAEPedAudioEntity::HandleSkateEvent` | `0x398A99` | - | Skate |
| ❌ | `CAEPedAudioEntity::TurnOnJetPack` | `0x399EB1` | - | Liga jetpack |
| ❌ | `CAEPedAudioEntity::TurnOffJetPack` | `0x39A005` | - | Desliga |
| ❌ | `CAEPedAudioEntity::UpdateJetPack` | `0x39A045` | - | Atualiza |
| ❌ | `CAEPedAudioEntity::StopJetPackSound` | `0x399AE9` | - | Para som |
| ❌ | `CAEPedAudioEntity::PlayWindRush` | `0x39A221` | - | Vento |
| ❌ | `CAEPedAudioEntity::PlayShirtFlap` | `0x399E05` | - | Roupa |
| ❌ | `CAEPedAudioEntity::HandleSwimSplash` | `0x399711` | - | Splash |
| ❌ | `CAEPedAudioEntity::HandleSwimWake` | `0x3997ED` | - | Rastro água |

---

## CAEPedSpeechAudioEntity

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ❌ | `CAEPedSpeechAudioEntity::Initialise` | `0x39CEB9` | - | Inicializa |
| ❌ | `CAEPedSpeechAudioEntity::AddSayEvent` | `0x39A271` | - | Fala |
| ❌ | `CAEPedSpeechAudioEntity::AddScriptSayEvent` | `0x39B49D` | - | Fala script |
| ❌ | `CAEPedSpeechAudioEntity::PlayLoadedSound` | `0x39D545` | - | Toca som |
| ❌ | `CAEPedSpeechAudioEntity::LoadAndPlaySpeech` | `0x39B3B1` | - | Carrega e toca |
| ❌ | `CAEPedSpeechAudioEntity::StopCurrentSpeech` | `0x39A875` | - | Para fala |
| ❌ | `CAEPedSpeechAudioEntity::EnablePedSpeech` | `0x39D141` | - | Habilita |
| ❌ | `CAEPedSpeechAudioEntity::DisablePedSpeech` | `0x39D12B` | - | Desabilita |
| ❌ | `CAEPedSpeechAudioEntity::GetPedTalking` | `0x39D11B` | - | Está falando |
| ❌ | `CAEPedSpeechAudioEntity::GetPedType` | `0x39D3BB` | - | Tipo do ped |
| ❌ | `CAEPedSpeechAudioEntity::GetCurrentCJMood` | `0x39CA39` | - | Mood do CJ |
| ❌ | `CAEPedSpeechAudioEntity::GetVoiceForMood` | `0x39D46D` | - | Voz por mood |
| ❌ | `CAEPedSpeechAudioEntity::SetUpConversation` | `0x39C14D` | - | Setup conversa |

---

## CTaskManager

| Status | Função | 32-bit | 64-bit | Descrição |
|:------:|--------|:------:|:------:|-----------|
| ✅ | `CTaskManager::CTaskManager` | `0x5338B1` | - | Construtor |
| ✅ | `CTaskManager::~CTaskManager` | `0x5338CB` | - | Destrutor |
| ❌ | `CTaskManager::SetTask` | `0x53397B` | - | Define tarefa |
| ❌ | `CTaskManager::AddSubTasks` | `0x533A33` | - | Sub-tarefas |
| ❌ | `CTaskManager::SetNextSubTask` | `0x533DAB` | - | Próxima sub |
| ❌ | `CTaskManager::GetSimplestTask` | `0x533A85` | - | Mais simples |
| ❌ | `CTaskManager::SetTaskSecondary` | `0x533CAB` | - | Secundária |
| ❌ | `CTaskManager::ClearTaskEventResponse` | `0x533D7D` | - | Limpa evento |
| ❌ | `CTaskManager::ManageTasks` | `0x533E23` | - | Gerencia |
| ❌ | `CTaskManager::ParentsControlChildren` | `0x533FC9` | - | Pais/filhos |
| ❌ | `CTaskManager::Flush` | `0x534071` | - | Limpa todas |
| ✅ | `CTaskManager::FlushImmediately` | `0x53411F` | - | Limpa imediato |
| ❌ | `CTaskManager::StopTimers` | `0x5342CD` | - | Para timers |

---

## Resumo

| Categoria | Total | ✅ | ⚠️ | ❌ |
|-----------|:-----:|:--:|:--:|:--:|
| CPed Base | 52 | 1 | 7 | 44 |
| CPlayerPed | 45 | 0 | 8 | 37 |
| CPedModelInfo | 9 | 0 | 0 | 9 |
| CAnimManager | 20 | 0 | 3 | 17 |
| CAEPed*AudioEntity | 26 | 0 | 0 | 26 |
| CTaskManager | 13 | 3 | 0 | 10 |
| **TOTAL** | **165** | **4** | **18** | **143** |

---

## Como Usar

### 32-bit (ARM Thumb)
```cpp
// +1 para Thumb mode
auto SetupPlayerPed = (void(*)(int))(g_libGTASA + 0x4C39A5 + 1);
SetupPlayerPed(playerNum);
```

### 64-bit (ARM64)
```cpp
auto SetupPlayerPed = (void(*)(int))(g_libGTASA + 0x5C0FD4);
SetupPlayerPed(playerNum);
```

### Macro Multi-versão
```cpp
#define SETUP_PLAYER_PED_OFFSET (VER_x32 ? 0x4C39A5 + 1 : 0x5C0FD4)
```
