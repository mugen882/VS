# VS (Vampire Survivors-like) — 시스템 설계 문서

> UE 5.5.4 / C++ · 개인 포트폴리오 프로젝트
> 대량 적 처리를 위한 ISM 기반 뱀서류 게임. 아래는 핵심 시스템 7개의 설계 의도와 문제 해결 과정을 정리한 문서다.

이 프로젝트에서 일관되게 추구한 설계 원칙은 **데이터 주도 설계**와 **책임 분리**다. 무기·적·웨이브·성장 요소를 모두 데이터 애셋으로 정의해 코드 수정 없이 콘텐츠를 확장할 수 있게 했고, 각 시스템이 자기 책임만 갖도록 경계를 명확히 나눴다.

---

## 1. 무기 시스템 — Strategy 패턴 리팩터링

### 문제

초기 구현은 무기 종류(라이플·샷건·궤도·드론·방패)를 하나의 `UVSWeaponComponent`가 `switch`/`if-else`로 분기해 처리했다. 무기 타입 분기가 네 곳(`TickComponent`, `FireWeapon`, `AddWeapon`, `UpgradeWeaponByData`)에 흩어져 있었고, 무기를 하나 추가하려면 이 네 곳을 모두 수정해야 했다. 한 곳이라도 빠뜨리면 조용히 버그가 나는 구조였다.

### 해결: 무기 동작을 전략 객체로 분리

무기 타입별 동작을 `UVSWeaponBehavior`를 상속한 전략 객체로 분리했다. 각 무기는 생명주기 훅(`OnAdded` / `OnUpgraded` / `Tick`)을 자기 방식대로 구현한다.

```
UVSWeaponBehavior (추상)
├─ UVSNearestBehavior     (가장 가까운 적 조준 발사)
├─ UVSMultiShotBehavior   (부채꼴 다발 발사)
├─ UVSOrbitBehavior       (플레이어 주위 궤도 구슬)
├─ UVSDroneBehavior       (추종 드론 자동 발사)
└─ UVSShieldBehavior      (플레이어 주위 지속 데미지 영역)
```

무기 데이터 애셋(`UVSWeaponData`)이 자신의 `BehaviorClass`를 지정하고, `AddWeapon`에서 `NewObject`로 해당 behavior를 생성한다. 이로써 팩토리 `switch`조차 사라졌다.

컴포넌트는 무기 목록을 들고 각 behavior에 위임하는 얇은 관리자가 됐다.

```cpp
void UVSWeaponComponent::TickComponent(float Dt, ...)
{
    for (FVSWeaponInstance& W : Weapons)
        if (W.Behavior) W.Behavior->Tick(this, W, Dt);
}
```

### 결과

- 무기 추가가 **"behavior 클래스 하나 + 데이터 애셋에 지정"**으로 끝난다. 기존 코드를 건드리지 않는다.
- 컴포넌트는 behavior가 사용할 창구(`GetOwner` / `GetEnemyManager` / `ApplyContinuousDamage` / `GetFloorLocation`)만 노출하고, `EnemyManager` 같은 내부 상태는 캡슐화했다.

### 설계 판단 기록

- **behavior는 무상태로, 런타임 상태는 `FVSWeaponInstance`에 유지**했다. 궤도 각도·구슬 배열 등 인스턴스별 상태를 데이터 구조에 두어, behavior 객체를 무기 종류당 하나만 두면 되게 했다.
- **`GetWorld()`는 반드시 컴포넌트를 경유(`Comp->GetWorld()`)**하도록 했다. behavior는 `UObject`라 자체 `GetWorld()`가 null을 반환할 수 있어, 액터 컴포넌트의 것을 쓰는 게 안전하다.

---

## 2. 패시브 스킬 시스템 — 스탯 수정자 누적

### 문제

레벨업 시 무기뿐 아니라 플레이어·무기 공통 스탯(이동속도, 최대체력, 픽업범위, 전체 데미지 %, 쿨다운 감소 %)을 강화하는 패시브가 필요했다. 값을 직접 수정하면(예: `MoveSpeed *= 1.1`) 원본 base 값을 잃어 재계산·밸런싱이 불가능해진다.

### 해결: 수정자를 누적하고 항상 base에서 재계산

패시브를 획득하면 값을 직접 바꾸지 않고, `FVSStatModifiers`(TMap 기반)에 **누적 배율만 쌓는다.** 실제 스탯은 항상 `base × (1 + 누적)`으로 계산한다. 스탯 종류는 타입 안전을 위해 `EVSStatType` enum으로 관리한다.

```cpp
void AVSPlayerCharacter::RecalculateStats()
{
    // base는 보존, 누적 수정자만 반영 → 몇 번 획득해도 원본 유실 없음
    GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * (1.f + StatMods.Get(EVSStatType::MoveSpeed));
    MaxHealth = BaseMaxHealth * (1.f + StatMods.Get(EVSStatType::MaxHealth));
}
```

무기 스탯은 저장하지 않고 계산 시점에 반영한다. `GetDamage(Mods)`가 호출될 때마다 `base × (1 + GlobalDamage)`를 반환하므로, 패시브 획득 즉시 다음 발사부터 자동 적용된다.

### 설계 판단 기록

- **플레이어 스탯 vs 무기 스탯의 반영 방식을 구분**했다. 플레이어 스탯(이동속도 등)은 실제 변수에 저장되므로 `RecalculateStats`에서 재계산·덮어쓰기가 필요하지만, 무기 데미지·쿨다운은 매번 계산되는 값이라 별도 재계산이 필요 없다 — 수정자만 쌓이면 다음 계산에 자동 반영된다.
- **쿨다운 감소에 하한(clamp)**을 뒀다. `base × (1 - 누적)` 방식은 누적이 1을 넘으면 0이나 음수가 되어 무한 발사 버그가 난다. `FMath::Max(0.1f, ...)`로 최소 쿨다운을 보장했다.
- **최대체력 증가 시 현재 체력도 증가분만큼 회복**시켜, "최대치는 늘었는데 체력이 안 찬" 위화감을 없앴다.
- **스탯 키를 `FName`에서 `EVSStatType` enum으로 전환**했다. 초기엔 `FName` 문자열 키를 썼으나 오타에 취약했고(예: `"MoveSpeed"` 오타 시 조용히 무시됨), 패시브 종류가 5종으로 확정된 뒤 enum으로 바꿔 컴파일 타임 검사·자동완성·추적 가능성을 확보했다. 런타임 유연성은 줄지만, 스탯 종류는 자주 바뀌지 않고 추가 시 `RecalculateStats` 처리도 함께 필요하므로 타입 안전이 더 가치 있다고 판단했다.

---

## 3. 적 다양화 + 웨이브 스케일링 — 데이터 주도 스폰

### 구조

대량의 적을 단일 `InstancedStaticMeshComponent`로 렌더링하면서, 시간에 따라 난이도가 상승하는 스폰 시스템을 구현했다. 역할을 셋으로 나눴다.

```
UVSDifficultySubsystem (WorldSubsystem + FTickableGameObject)
    │  경과 시간 추적, 현재 웨이브 판정, 스폰 타이밍 결정 (언제·무엇을)
    ▼  SpawnEnemy 호출
AVSEnemyManager (AActor, ISM 소유)
    │  실제 인스턴스 생성 + 타입별 스탯/외형 적용 (어떻게)
    ▼  참조
UVSEnemyTypeData / UVSWaveData (데이터 애셋)
```

**핵심 원칙**: 매니저는 "어떻게 스폰하는가(ISM 조작)"만 알고, "언제 무엇을 스폰하는가"는 서브시스템이 결정한다.

### 적 다양화 — 단일 ISM 유지

> **AnimToTexture (Bone 모드)** — 애니메이션의 프레임별 본 위치·회전을 텍스처에 미리 구워두고, 런타임에는 정점 셰이더가 그 텍스처를 읽어 스키닝한다. CPU에서 본 행렬을 계산하지 않으므로 개체 수가 늘어도 게임 스레드 비용이 거의 늘지 않고, 결과물이 스태틱 메시라 인스턴싱이 가능해진다. 대가는 애니메이션 블렌딩 불가와 정점당 셰이더 비용 증가. 카메라가 멀고 같은 적이 수백 마리 나오는 탑다운 뱀서라이크에 맞는 교환이다.


완전히 다른 모델을 쓰는 대신, 단일 ISM에서 **타입별 스탯·크기·색**으로 다양화했다. `UVSEnemyTypeData`가 체력·이동속도·접촉데미지·경험치·크기·색을 정의한다.

- **크기**: 스폰 시 트랜스폼 스케일 + 매 틱 `BatchUpdateInstancesTransforms`에서 스케일 유지 (틱마다 스케일이 1로 덮이던 버그를 인스턴스별 스케일 저장으로 해결)
- **색**: per-instance custom data(슬롯 4·5·6)로 RGB 전달 → 머티리얼의 `PerInstanceCustomData`가 읽어 Tint 적용. `NumCustomDataFloats`를 애니메이션 4개 + 색 3개 = 7로 확장

### 웨이브 스케일링

`UVSWaveData`가 시간대별 스폰 규칙(시작 시각·적 타입·스폰 간격·마리수·체력 배율)을 배열로 정의한다. 서브시스템이 경과 시간을 보고 현재 웨이브를 판정한다.

```cpp
// 경과 시간이 다음 웨이브 시작 시각을 넘으면 전환 (밀렸으면 한 번에)
while (CurrentWaveIndex + 1 < WaveData->Waves.Num()
    && ElapsedTime >= WaveData->Waves[CurrentWaveIndex + 1].StartTime)
{
    ++CurrentWaveIndex;
}
```

### 설계 판단 기록

- **ISM 소유자는 AActor, 순수 로직은 WorldSubsystem**으로 나눴다. ISM(씬 컴포넌트)은 월드에 존재해야 하므로 액터가 맞고, 시간·웨이브 관리는 씬 요소가 없으므로 레벨 수명과 함께하는 WorldSubsystem이 맞다. GameInstance는 레벨 전환 시에도 살아남아 적 데이터가 잔존하므로 부적합하다고 판단했다.
- **hot data는 `FEnemyData`에 복사**했다. 수백~천 마리를 매 틱 순회하므로, 자주 읽는 스탯(속도·데미지)을 데이터 애셋 포인터 역참조 대신 인접 메모리에서 읽도록 복사했다 (데이터 지향 접근).
- **swap-remove 시 ISM custom data도 함께 이동**시켜 배열 인덱스와 인스턴스 인덱스 정합성을 유지했다. custom data는 `NumCustomDataFloats`만큼 순회하며 옮기되, 렌더 상태 갱신(dirty)은 마지막 슬롯에서 한 번만 찍어 GPU 업로드를 최소화했다.
- **`FTickableGameObject` 다중 상속 주의점**: `Tick`은 `FTickableGameObject` 계통이라 `Super::Tick`을 호출하면 안 되고(컴파일 에러), `Initialize`는 `UWorldSubsystem` 계통이라 `Super` 호출이 맞다. `IsTickable`은 `!IsTemplate()`로 CDO 틱을 방지했다.
- **매니저는 레벨 배치가 아닌 런타임 스폰**으로 바꿨다. 레벨에 배치하면 PIE 중 추가된 ISM 인스턴스가 에디터 원본 액터에 잔존하는 문제가 있었다. 게임모드가 시작 시 스폰하도록 바꿔 이를 원천 차단하고, 매니저 수명을 게임 세션과 일치시켰다.

---

## 4. 게임 흐름 + MVVM HUD

### 게임오버 / 클리어 — 델리게이트 기반 느슨한 결합

플레이어 사망(게임오버)과 목표 시간 도달(클리어)을 게임 종료 조건으로 구현했다. 캐릭터·서브시스템은 "무슨 일이 일어났다"만 방송하고, UI 표시는 구독자가 담당한다.

```
캐릭터 사망 → OnPlayerDied.Broadcast()
    ├→ 컨트롤러: 결과 화면 표시 + 일시정지
    └→ 서브시스템: 스폰 정지 플래그
목표 시간 도달 → 서브시스템 OnRunCleared.Broadcast()
    └→ 게임모드 → 컨트롤러 ShowResult(승리)
```

결과 화면은 승/패 공용 위젯(`UVSResultWidget`)으로 재활용하고, 표시 로직은 컨트롤러의 `ShowResult(bIsVictory)` 한 곳으로 통합해 중복을 제거했다.

**스폰 정지**는 `FTickableGameObject`가 일시정지(`SetPause`)의 영향을 받지 않는 문제 때문에, 명시적 플래그(`bGameOver` / `bUpgradeSelecting` / `bGameClear`)로 제어하고 `CanSpawn()` 헬퍼 한 곳에서 판정하도록 했다. 각 플래그가 정지 사유를 구분해 디버깅이 용이하다.

### HUD — MVVM 아키텍처

체력·경험치·레벨·처치수·생존시간을 표시하는 HUD를 **Model-View-ViewModel** 패턴으로 구현했다. (실무에서 다룬 MVVM 위젯 아키텍처를 개인 프로젝트에서 재현)

```
Model (방송만)              ViewModel (구독·가공)          View (바인딩)
────────                   ──────────                    ────────
VSPlayerCharacter          UVSHUDViewModel                WBP_HUD
  OnHealthChanged  ──────→   HandleHealthChanged
  OnXPChanged      ──────→   HandleXPChanged        ──→   FieldNotify
  OnLevelChanged   ──────→   HandleLevelChanged            → 위젯 자동 갱신
VSDifficultySubsystem
  OnKillCountChanged ────→   HandleKillCountChanged
  OnTimeChanged      ────→   HandleTimeChanged
  OnTotalRunTimeChanged ─→   (진행바 목표치)
```

**단방향 의존**: Model은 ViewModel을 전혀 모르고 델리게이트로 방송만 한다. ViewModel이 Model을 구독하고, View는 ViewModel만 바인딩한다. 컨트롤러가 ViewModel을 생성해 Model과 연결(`BindModels`)하고 위젯에 주입(Manual 방식)한다.

### 설계 판단 기록

- **값 성격에 따라 push/pull을 구분**했다. 연속적으로 바뀌는 값(시간)은 매 프레임 방송하되, ViewModel Setter가 `UE_MVVM_SET_PROPERTY_VALUE`로 실제 변경 시에만 뷰 갱신을 발생시킨다. 덕분에 시간을 매 프레임 방송해도 텍스트(MM:SS)는 초 단위 변경 시에만 갱신되고, 진행바만 매 프레임 부드럽게 찬다.
- **초기값 pull이 낸 타이밍 버그를 push로 해결**했다. 목표 시간(`TotalRunTime`)을 `BindModels` 시점에 pull하니, 게임모드의 `SetWaveData`보다 먼저 실행되어 0을 받는 문제가 있었다. 이를 `OnTotalRunTimeChanged` 이벤트 구독으로 바꿔, 값이 세팅되는 시점에 받도록 해 해결했다. "한 번 세팅되지만 그 시점이 불확실한 값"은 push가 적합하다는 교훈.

---

## 5. 보스 시스템 — ISM 대량 처리와 개별 액터의 하이브리드

### 문제

일반 적은 단일 ISM으로 수백~천 마리를 렌더링한다(데이터 지향). 그러나 보스는 요구사항이 정반대다: 소수(1마리)지만 고유 메시·애니메이션·공격 패턴·개별 체력바가 필요하다. ISM 인스턴스는 개체를 지목해 위젯을 붙이거나 고유 행동을 주기 어렵다. 하나의 시스템으로 둘 다 처리하면 어느 한쪽이 어색해진다.

### 해결: 대량은 ISM, 특수는 별도 액터

적을 두 종류로 나눴다. 일반 적은 기존 ISM 배열(`FEnemyData[]`)로, 보스는 진짜 `AActor`(`AVSBossEnemy`)로 스폰한다. 보스는 개체 수가 적어 액터 하나하나의 비용이 문제되지 않으므로, 스켈레탈 메시·애님 블루프린트·고유 공격을 온전히 활용한다.

**보스는 상속으로 확장**했다. 무기는 컴포지션(behavior)으로 갔지만, 보스는 개체마다 메시·전체가 다르고 "이 보스는 이 종류"로 고정이므로 상속이 자연스럽다.

베이스가 공통 기능(이동, 접촉 데미지, 체력, 사망, 체력바)을 담당하고, 파생은 `MoveTowardPlayer`와 `UpdateAttack`만 구현한다.

**무기·적과 마찬가지로 데이터 주도로 정의**했다. `UVSBossData`가 스탯뿐 아니라 **스켈레탈 메시·애님 클래스까지** 담고, `InitBoss`에서 `SetSkeletalMesh`/`SetAnimInstanceClass`로 적용한다. 보스 하나가 데이터 애셋 하나로 완전히 정의된다.

### 아키타입 확장 — 액터와 데이터를 나란히 상속

보스가 1종에서 3종으로 늘면서 확장 축을 정리했다.

```
AVSBossEnemy (Abstract)            UVSBossData
├─ AVSBossCharger  (돌진)          ├─ UVSBossChargerData
├─ AVSBossRanger   (전방위 탄막)    ├─ UVSBossRangerData
└─ AVSBossSummoner (잡몹 소환)      └─ UVSBossSummonerData
```

처음에는 `EBossType` enum과 `EditCondition`으로 한 데이터 애셋 안에서 패턴별 필드를 감췄다. 그러나 이 방식은 **같은 정보를 두 곳에 두는 문제**가 있다. "이 보스는 돌진형"이라는 사실을 액터 클래스와 데이터의 enum이 각각 표현하니, Charger 클래스인데 데이터의 `BossType`은 Summoner인 애셋이 만들어질 수 있었다.

데이터 애셋을 액터와 같은 모양으로 상속시켜 그 가능성 자체를 없앴다. 파생 보스는 `Cast<UVSBossChargerData>`로 자기 데이터를 꺼내고, 타입이 어긋나면 초기화 단계에서 바로 드러난다. enum과 `EditCondition` 표현식이 통째로 사라진 것은 부수 효과다.

### 공유 행동 블록 — 상속의 실익 확보

보스가 3종이 되자 파생 클래스에 같은 코드가 복제되기 시작했다. 카이팅(거리 유지)은 Ranger와 Summoner가 글자 단위로 동일했고, 회전·접촉 데미지·플레이어 조회는 네 곳에 흩어졌다.

베이스에 행동 블록 다섯 개를 올렸다.

| 블록 | 역할 |
|---|---|
| `QueryPlayer()` | 플레이어 방향·거리를 한 번에 조회해 `FVSBossPlayerInfo`로 반환 |
| `MoveInDirection()` | 지정 방향으로 이동 |
| `ApplyKiting()` | `FleeRange`보다 가까우면 후퇴, `KeepDistance`보다 멀면 접근 |
| `FaceDirection()` | 지정 방향으로 등속 선회 |
| `ApplyContactDamage()` | 접촉 사거리 내 초당 데미지 |

결과적으로 Ranger의 이동 함수는 세 줄이 됐다.

```cpp
void AVSBossRanger::MoveTowardPlayer(float DeltaTime)
{
    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    ApplyKiting(Info, DeltaTime);
    FaceDirection(Info.Dir, GetRotateSpeedDeg(), DeltaTime);
    ApplyContactDamage(DeltaTime);
}
```

Charger만 상태 머신 때문에 길게 남는데, 그 길이 차이가 곧 "이 보스는 무엇이 다른가"를 드러낸다.

### 예고 가능한 공격 — 텔레그래프

돌진 보스에서 가장 오래 붙든 문제는 구현이 아니라 **플레이어가 읽을 수 있는가**였다.

상태 머신(Chase → Aim → Charge → Recover)으로 예비 동작과 방향 고정을 만들었는데도 여전히 피할 수 없었다. 원인은 회전 함수였다.

```cpp
const float BOSSCHARGE_ROTATE_SPEED = 30.f;   // 이름은 "초당 30도"
FMath::RInterpTo(Cur, Target, DeltaTime, BOSSCHARGE_ROTATE_SPEED);
```

`RInterpTo`의 `InterpSpeed`는 각속도가 아니라 **지수 감쇠 계수**다. 60fps에서 `DeltaTime * 30 = 0.5`, 즉 매 프레임 남은 각도의 절반을 좁힌다 — 사실상 즉시 스냅이었다. 조준 중에도 보스가 플레이어를 완벽히 추적했으니 예비 동작이 아무 정보도 주지 못했다.

`RInterpConstantTo`(등속, 도/초)로 바꾸자 `AimRotateSpeedDeg`가 비로소 의도한 대로 동작했다.

여기서 처음에는 이 값을 **회피 난이도**를 정하는 노브로 생각했다. 조준 중 보스의 선회가 플레이어의 방위각 변화율(`이동속도 / 거리`)보다 빠르면 각도가 고정되어 피할 수 없다는 계산이었다. 실제로 플레이해 보니 틀린 모델이었다.

`ChargeDir`은 조준이 끝나는 시점에 고정되고, 돌진 중에는 추적하지 않는다. 즉 **회피는 조준 중이 아니라 돌진이 날아오는 동안 일어난다.**

```
거리 600 기준
  돌진 도달 시간 = 600 / Speed 900     ≈ 0.67초
  그 사이 플레이어 횡이동 = 600 × 0.67 ≈ 400
  필요한 횡 오프셋 = ContactRange 150 + 캡슐 42 = 192
```

400 > 192이므로 조준이 완벽하게 따라붙었더라도 빠져나간다. 회피 여유는 `Speed × Duration`과 플레이어 이동 속도의 관계에서 나오며, 선회 속도와는 무관했다.

그러면 이 값이 정하는 것은 **예고의 가독성**이다. 선회가 느리면 예고선이 플레이어를 뒤늦게 따라오므로 "저쪽으로 온다"를 미리 읽을 수 있고, 빠르면 발사 직전까지 정확히 겨눠 방향 정보를 얻는 시점이 늦어진다. 30·45·120·240을 비교해 **120°/s**로 정했다 — 방향은 미리 읽히지만 굼뜨게 보이지도 않는 지점이다.

그다음은 방향을 보여주는 일이다. `UDecalComponent`를 액터에 붙여 돌진 경로를 바닥에 깔고, 조준 진행도(`StateTimer / AimTime`)를 머티리얼 스칼라 파라미터로 흘려 예고선이 차오르게 했다. 데칼이 액터에 붙어 있으므로 보스가 선회하면 예고선도 따라 돌고, 별도 동기화 코드가 없다.

**여기서 지킨 원칙은 연출이 판정에서 값을 읽게 하는 것이다.**

```cpp
const float ChargeDist = CfgData->Speed * CfgData->Duration;   // 실제 도달 거리
TelegraphDecal->DecalSize = FVector(
    CfgData->TelegraphDepth,
    CfgData->ContactRange,     // 반폭 = 접촉 판정 반경
    DrawLen * 0.5f);
```

길이는 눈대중 상수가 아니라 `Speed × Duration`이고, 폭은 별도 프로퍼티가 아니라 `ContactRange`다. 데이터에서 속도나 판정 반경을 바꿔도 예고선이 자동으로 따라온다 — 어긋날 수 있는 경로 자체를 없앴다.

마지막 한 칸은 판정 쪽에서 맞췄다. 접촉 판정이 중심점 거리만 재고 있어서, 캐릭터 몸이 예고선에 걸쳐도 중심이 밖이면 맞지 않았다. 플레이어가 억울해하는 전형적인 형태다.

```cpp
const float HitRange = Data->ContactRange + Info.Pawn->GetSimpleCollisionRadius();
```

플레이어 캡슐 반경을 더하니 "예고선에 몸이 닿는 순간"이 정확히 판정 경계가 됐다. 보이는 것과 맞는 것이 일치한다.

### 성능 측정 — 두 표현 방식의 실제 비용

이 절의 전제인 "대량은 ISM, 특수는 액터"가 실제로 성립하는지 측정했다. 콘솔에서 두 경로에 같은 수를 스폰하고 프레임 타임을 수집하는 벤치마크 액터를 만들었다.

비교가 성립하려면 두 경로가 **같은 일만** 해야 한다. 조건을 맞춘 부분은 다음과 같다.

- 측정 중 정규 웨이브 스폰을 정지 — 그러지 않으면 5초 동안 개체 수가 계속 늘어 "N마리일 때 몇 ms"가 성립하지 않는다
- 플레이어 무기 Tick 비활성화 — 무기는 ISM 배열만 조준하므로, 켜두면 ISM 경로만 사격·피격·사망·젬 드롭 비용을 추가로 떠안는다
- 액터 더미도 `AlwaysTickPoseAndRefreshBones` 활성화 — AnimToTexture 경로는 화면 밖에서도 정점 셰이더가 돌지만 스켈레탈 메시는 기본적으로 포즈 갱신을 건너뛴다
- 스폰 링 반경 동일, 2초 워밍업 후 5초 수집(첫 프레임의 셰이더 컴파일 힛치 제외)

#### 결과 (적 500마리)

| 항목 | ISM + AnimToTexture | 액터 + 스켈레탈 |
|---|---|---|
| Frame avg | 28.52 ms | **14.72 ms** |
| **게임 스레드** | **5.90 ms** | 14.52 ms |
| 렌더 스레드 | 28.38 ms | 10.24 ms |
| GPU | 27.71 ms | 13.37 ms |
| **드로우콜** | **약 28** | 약 2,765 |

CPU 축에서는 의도한 결과가 나왔다. **게임 스레드 2.5배, 드로우콜 100배.** 액터 경로는 Frame(14.72)과 게임 스레드(14.52)가 소수점까지 일치해 CPU에 완전히 갇혀 있다 — 액터 방식의 교과서적인 한계다.

그런데 총 프레임은 ISM이 두 배 느리다. GPU에서 역전됐다.

#### 원인 추적

렌더 스레드가 28ms인데 Exclusive는 0.03ms였다. 그 스코프에서 CPU가 계산하는 게 없다는 뜻이고, 남는 해석은 하나 — **GPU를 기다리는 시간**이다. `stat gpu`로 패스를 쪼갰다.

| GPU 패스 | ISM + AnimToTexture | 액터 | 배율 |
|---|---|---|---|
| Shadow Depths | 14.16 ms | 2.09 ms | 6.8× |
| Basepass | 6.50 ms | 0.96 ms | 6.8× |
| Render Velocities | 4.37 ms | 0.57 ms | 7.7× |
| TemporalSuperResolution | 2.94 ms | 2.66 ms | 1.1× |
| LumenReflections | 1.86 ms | 2.17 ms | — |

지오메트리를 실제로 그리는 세 패스가 나란히 7배, 화면 기반 패스(TSR·Lumen)는 동일. **정점 수에 비례하는 항목만 커졌다.**

구운 스태틱 메시를 열어보니 **LOD 0 하나, 41,052 트라이앵글**이었다. 500 인스턴스가 거리와 무관하게 풀 해상도로 그려지고, 그림자 패스에서 한 번 더 그려진다. 반면 `SK_Mannequin`은 LOD가 여러 단계라 먼 개체가 자동으로 저해상도로 내려간다.

LOD가 없는 것은 이 기법의 원리적 한계가 아니라 **아직 만들지 않은 것**이다. Bone 모드는 본 트랜스폼만 텍스처에 담으므로 메시 정점 수와 무관하고, LOD별 메시가 같은 스켈레톤을 공유하면 그대로 재생된다. 다만 엔진의 자동 LOD 감축은 UV를 보간하기 때문에 정점에 실린 본 인덱스가 섞여 깨진다(3번 본과 7번 본의 중간값은 엉뚱한 본을 가리킨다). LOD별로 따로 베이크하거나 감축 설정에서 해당 채널을 보존해야 한다.

그림자를 끈 대조군으로 이 비용을 분리 확인했다.

| | 그림자 ON | 그림자 OFF |
|---|---|---|
| GPU TOTAL | 36.48 ms | 22.30 ms |
| Shadow Depths | 14.42 ms | 0.19 ms |

**이 실험은 채택할 해결책이 아니라 원인을 분리하기 위한 측정이다.** 그림자는 켜고 쓰는 게 정상이며, 실제 대응은 섀도 프록시 메시 지정이다 — 화면에는 41K 메시를 쓰고 그림자 패스에만 저폴리 메시를 넘기면 실루엣만 맞으면 되므로 품질 손실이 거의 없다.

#### 배제한 가설

순서대로 소거했다. 각각 한 줄 토글로 확인 가능한 것부터 접근했다.

| 가설 | 검증 | 결과 |
|---|---|---|
| 가상 그림자 맵 | `r.Shadow.Virtual.Enable 0` | 변화 없음 |
| 레이트레이싱 / Lumen HWRT | `r.RayTracing.Enable 0` | 변화 없음 |
| 드로우콜 | 드로우콜 카운터 비교 | 액터가 4,565개인데도 빠르다. 병목 아님 |
| 매 프레임 렌더 스테이트 재생성 | `bMarkRenderStateDirty` | 주원인은 아니었으나 아래 참조 |

마지막 항목은 원인이 아니었지만 **CPU 개선을 얻었다.** `BatchUpdateInstancesTransforms`를 매 프레임 `bMarkRenderStateDirty = true`로 호출하면 컴포넌트의 렌더 스테이트가 통째로 재생성된다. `false`로 바꿔 게임 스레드 7.28 → 5.90ms, 렌더 스레드 31.41 → 28.38ms를 회수했다.

#### 결론

**ISM + AnimToTexture는 CPU 비용을 절반 이하로 줄이는 대신, LOD가 없는 현 상태에서는 GPU 지오메트리 비용을 그대로 떠안는다.**

현재 게임의 적 상한은 500이고, 이 구간에서는 총 프레임만 보면 액터 경로가 앞선다. 그럼에도 이 구조를 유지하는 근거는 두 가지다.

첫째, 게임 스레드에서 아낀 8.6ms가 다른 시스템(무기 5종, 투사체, 젬 자석 흡수, 웨이브 스케일링, 보스)의 예산이 된다. 액터 경로는 이미 CPU에 갇혀 있어 여기서 시스템을 더 얹을 여유가 없다.

둘째, GPU 쪽 손실은 **에셋 작업으로 해결 가능한 영역**이고 아키텍처 변경을 요구하지 않는다. 섀도 프록시 메시와 저폴리 LOD 베이크가 남은 작업이며, 반대로 액터 경로의 CPU 한계는 구조를 바꾸지 않으면 넘을 수 없다.

### 설계 판단 기록

- **무기는 컴포지션, 보스는 상속** — 문제 성격에 맞는 확장 방식을 골랐다. 무기는 한 캐릭터가 여러 개를 동시에 들어 컴포지션이 맞고, 보스는 한 종류로 고정이며 전체(메시·크기)가 달라 상속이 맞다.
- **enum 분기 대신 데이터 애셋 상속** — 액터 계층이 이미 아키타입을 표현하는데 데이터가 enum으로 한 번 더 표현하면 두 표현이 어긋날 수 있다. 데이터를 같은 모양으로 상속시켜 불일치가 성립하지 않게 했다.
- **회전 주체를 메시에서 액터로** — 소환수가 항상 화면 오른쪽에 나오는 버그를 추적하니 원인은 소환 코드가 아니라 회전이었다. 메시 컴포넌트만 돌리고 액터는 스폰 시 회전 그대로여서 `GetActorForwardVector()`가 항상 월드 +X를 반환했다. 액터를 돌리고 메시 축 보정(`MeshYawOffset`)을 초기화 시 한 번만 적용하도록 바꿔, 이동·조준·소환이 같은 전방 정의를 공유하게 했다.
- **엔진 API의 의미를 이름으로 짐작하지 않는다** — `RInterpTo`의 `InterpSpeed`를 각속도로 오해해 상수 이름까지 "초당 30도"로 붙였고, 그 결과 조준 연출 전체가 무의미했다. 보간은 등속(`ConstantTo`)과 지수 감쇠(`InterpTo`) 중 무엇이 필요한지부터 정하는 게 맞다.
- **계산은 출발점이고 답은 플레이가 정한다** — 회피 가능성을 각속도 비교로 모델링했지만, 실제 회피는 방향이 고정된 뒤 돌진이 날아오는 동안 일어났다. 계산이 국면 자체를 잘못 잡은 것이다. 값을 손으로 돌려보고 나서야 이 파라미터가 회피 난이도가 아니라 예고 가독성을 정한다는 걸 알았다.
- **연출이 판정에서 값을 읽게 한다** — 예고선의 길이·폭을 별도 상수로 두지 않고 `Speed × Duration`, `ContactRange`에서 파생시켰다. 연출과 판정이 어긋나면 텔레그래프는 없느니만 못하므로, 어긋날 수 있는 경로를 구조적으로 제거했다.
- **디퍼드 데칼은 GBuffer에 덧칠된다** — 바닥 데칼이 투영 박스 안의 모든 불투명 표면에 그려져 플레이어가 배경에 묻혀 사라졌다. 캐릭터·잡몹 ISM·젬 ISM에 `SetReceivesDecals(false)`를 적용해 해결했다. 데칼을 "바닥에 붙는 그림"으로만 이해하면 놓치는 함정이다.
- **대조군의 조건을 맞추는 것이 측정의 절반** — 첫 측정에서는 ISM 경로만 전투 로직(사격·피격·젬 드롭)을 떠안아 결과가 뒤집혔다. "무엇을 재는가"보다 "두 쪽이 같은 일을 하는가"를 먼저 점검해야 한다.
- **스레드별로 나눠 기록한다** — 총 프레임 타임만 재면 "느리다"까지만 알 수 있다. 게임/렌더/GPU를 분리했기에 "CPU는 이겼고 GPU에서 졌다"는 정확한 문장이 나왔고, 개선 방향도 갈렸다.
- **Exclusive 시간이 0인 스코프는 대기 시간이다** — 렌더 스레드 28ms 중 Exclusive가 0.03ms인 것을 보고 CPU 최적화 방향을 접고 GPU 패스로 넘어갔다. 이 판단이 없었으면 CPU 쪽을 계속 팠을 것이다.
- **투사체 재활용 + 타격 대상 플래그** — 무기 투사체(`AVSProjectile`)에 `bHitsPlayer` 플래그를 더해, 같은 클래스가 적(무기용)과 플레이어(보스용)를 모두 타격하도록 분기했다. 클래스를 새로 만들지 않고 로직이 거의 같은 부분을 재사용했다.
- **애님 BP 의존성 함정** — 언리얼 기본 캐릭터 애님 BP는 `CharacterMovementComponent`에 의존하는데, 보스는 `AActor`라 그 컴포넌트가 없어 애님이 구동되지 않았다. 보스 전용 경량 애님 BP를 만들어 해결했다. "대량은 최적화, 특수는 품질"이라는 하이브리드가 렌더링 전략(잡몹=AnimToTexture 스태틱, 보스=스켈레탈)에도 나타난다.

## 6. 타격 통합 — 두 적 표현의 통일

### 문제

플레이어 공격은 ISM 적과 보스를 **모두** 타격해야 한다. 그러나 둘은 저장 방식이 다르다. 일반 적은 `Enemies[]` 배열의 인덱스로 접근하고, 보스는 액터 포인터다. 특히 `FindNearestEnemy`가 int32 인덱스를 반환하는데, 보스는 인덱스 체계 밖이라 이 시그니처에 담기지 않는다.

### 해결: 조준·범위·명중을 각각 확장

세 타격 경로를 성격에 맞게 확장했다.

- **조준(`FindNearestEnemy`)** — 보스도 후보에 넣어 더 가까우면 그 위치를 반환한다. 반환 타입은 그대로 두되, 보스가 최근접일 때 `BOSS_TARGET_INDEX`(-2)라는 센티넬 값으로 표시한다. ISM 인덱스(0..N)·`INDEX_NONE`(-1)과 겹치지 않아, 기존 호출부와 호환되면서 "보스가 타겟"임을 알린다.
- **범위 데미지(`ApplyDamageInRadius`)** — 범위 내 ISM 적과 보스를 모두 순회해 타격한다. 방패 같은 지속 범위 무기가 자동으로 보스를 포함한다.
- **투사체 명중** — 센티넬을 감지하면 `FindNearestBoss`로 보스를 직접 찾아 `ReceiveDamage`를 호출한다.

보스는 `BeginPlay`에서 EnemyManager에 자신을 등록(`RegisterBoss`)하고 `EndPlay`에서 해제해, 등록/해제가 생명주기에 묶인다.

### 설계 판단 기록

- **센티넬 인덱스로 반환 타입 유지** — `FindNearestEnemy`의 int32 반환을 구조체로 바꾸면 모든 호출부(behavior들)를 고쳐야 한다. 대신 겹치지 않는 특수값(-2)으로 "보스"를 표현해, 기존 인터페이스를 깨지 않고 확장했다.
- **데이터 지향 vs 객체 지향의 공존** — 일반 적은 매니저가 배열을 직접 깎고(`ApplyDamageToEnemy`), 보스는 자기가 데미지를 처리한다(`ReceiveDamage`). 두 적의 저장 방식 차이가 타격 처리 방식의 차이로 그대로 이어진다.
- **`AActor::TakeDamage`와의 이름 충돌 회피** — 보스 피격 함수를 `ReceiveDamage`로 명명해, 언리얼 기본 가상 함수를 숨기는 경고(C4263/C4264)를 피했다.

## 7. 적 다양성 완성 — 엘리트 (데이터 주도)

보스가 "특수 이벤트 개체"라면, 엘리트는 "일반 적 사이에 섞이는 강한 잡몹"이다. 별도 액터 없이 기존 ISM 적 시스템을 그대로 쓰고, `UVSEnemyTypeData`의 값(체력·크기·색·보상)을 극단적으로 준 데이터 애셋 하나로 정의한다.

웨이브 엔트리(`FVSWaveEntry`)에 `EliteType`·`EliteInterval`을 추가해, 한 웨이브 안에서 일반 적(다중, `while`로 스폰)과 엘리트(소수, `if`로 틱당 최대 1마리)를 **동시에** 스폰한다. 엘리트를 `if`로 제한한 것은 프레임 드랍이나 긴 간격에서도 엘리트가 한꺼번에 쏟아지지 않고 띄엄띄엄 나오도록 하기 위함이다.

이로써 적이 3단계 강약 리듬을 갖는다: **일반 잡몹**(대량, ISM) → **엘리트**(강한 잡몹, 데이터) → **보스**(특수 액터). 각 단계가 서로 다른 구현 전략을 쓰되, 데이터 주도 정의라는 일관된 원칙 위에 있다.

---

## 반복적으로 적용한 설계 습관

프로젝트 전반에서 일관되게 적용한 것들:

- **참조 주입(push)으로 결합도 관리**: 소유자가 참조를 찾아 사용처에 주입한다. 매 틱 `GetActorOfClass` 검색 대신 초기화 시 한 번 찾아 전달하고, 소유권에 따라 `TObjectPtr`(소유) / `TWeakObjectPtr`(사용)를 구분했다.
- **죽은 코드 정리**: 리팩터링 후 미사용 함수·필드를 지속적으로 제거해 코드베이스를 얇게 유지했다.
- **정지·상태 플래그의 판정 일원화**: 여러 사유를 개별 플래그로 두되 판정은 헬퍼 함수 한 곳(`CanSpawn`)에 모아, 사유 구분과 유지보수성을 동시에 확보했다.
- **초기화 시 상태 리셋**: 서브시스템 `Initialize`에서 모든 런타임 상태를 리셋해, 레벨 재시작(`OpenLevel`) 시 깨끗한 상태를 보장했다.
- **문제 성격에 맞는 확장 방식 선택**: 같은 "확장 가능한 시스템"이라도 무기는 컴포지션(strategy), 보스는 상속으로 나눴다. 동시 다중 보유는 컴포지션, 개체별 고정·전체 상이는 상속이라는 기준을 적용했다.
- **인터페이스를 깨지 않는 확장**: 새 요구(보스 타격)를 기존 반환 타입을 바꾸지 않고 센티넬 값으로 수용해, 모든 호출부를 수정하지 않고 확장했다.
- **중복된 표현 제거**: 같은 사실을 두 곳에서 표현하면(액터 클래스 + enum, 예고선 폭 + 판정 반경) 언젠가 어긋난다. 한쪽이 다른 쪽에서 파생되도록 바꿔 불일치가 성립할 수 없게 만들었다.
- **플레이어가 읽을 수 있는가를 완료 기준으로**: 돌진 보스는 동작한 뒤에도 "피할 수 있는가"를 만족할 때까지 선회 방식·예고 연출·판정 반경을 차례로 고쳤다. 기능 완성과 체감 완성을 분리해 봤다.

---

## 기술 스택 요약

| 영역 | 사용 기술 |
|---|---|
| 대량 적 렌더링 | InstancedStaticMeshComponent + per-instance custom data, AnimToTexture(Bone 모드) |
| 보스 (특수 개체) | 별도 AActor + 액터/데이터 이중 상속 계층, 스켈레탈 메시, 데이터 주도 메시·애니 |
| 보스 예고 연출 | UDecalComponent + UMaterialInstanceDynamic 스칼라 파라미터, 등속 보간(RInterpConstantTo) |
| 성능 측정 | 콘솔 exec 벤치마크 액터, 스레드별 프레임 타임 수집(RenderCore/RHI 전역 타이머), stat gpu 패스 분석 |
| 적 타격 통합 | ISM 인덱스 + 액터 포인터를 센티넬 값(BOSS_TARGET_INDEX)으로 통일 |
| 무기 시스템 | Strategy 패턴 (UObject 기반 behavior) |
| 성장 시스템 | 스탯 수정자 누적 (base × 수정자 재계산) |
| 스폰/난이도 | WorldSubsystem + FTickableGameObject, 데이터 주도 웨이브 |
| UI | MVVM (UMVVMViewModelBase, FieldNotify), 델리게이트 기반 이벤트 |
| 데이터 정의 | UPrimaryDataAsset (무기·적·웨이브·업그레이드) |
