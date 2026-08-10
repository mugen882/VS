# VS — Vampire Survivors-like

> **UE 5.5.4 / C++ · 개인 포트폴리오 프로젝트**
> 수백 마리의 적을 단일 ISM으로 처리하는 탑다운 서바이버 게임

<!-- TODO: 대표 GIF (수백 마리 몰려오는 장면). 폭 800px 내외, 5MB 이하 권장 -->
![대량 적 렌더링](Docs/gif/crowd_v2.gif)

**[▶ 플레이 빌드 다운로드](https://github.com/mugen882/VS/releases/tag/v1.0.0)** · **[📄 시스템 설계 문서](Docs/VS_Portfolio_Systems.md)**

---

## 이 프로젝트에서 다룬 것

무기·적·웨이브·성장 요소를 전부 데이터 애셋으로 정의해 **코드 수정 없이 콘텐츠를 확장**할 수 있게 만들고, 각 시스템이 자기 책임만 갖도록 경계를 나눴습니다.
대량 처리가 필요한 잡몹과 개별 연출이 필요한 보스를 **서로 다른 방식으로 나눠 구현**한 뒤, 그 선택이 옳았는지 직접 측정해 검증했습니다.

---

## 핵심 시스템

| 시스템 | 요약 |
|---|---|
| **[무기](Docs/VS_Portfolio_Systems.md#1-무기-시스템--strategy-패턴-리팩터링)** | Strategy 패턴으로 발사 로직 분리. 새 무기 = 클래스 하나 + 데이터 애셋 하나 |
| **[패시브](Docs/VS_Portfolio_Systems.md#2-패시브-스킬-시스템--스탯-수정자-누적)** | 수정자를 누적하고 항상 base에서 재계산해 중첩 오차 제거 |
| **[적 · 웨이브](Docs/VS_Portfolio_Systems.md#3-적-다양화--웨이브-스케일링--데이터-주도-스폰)** | 단일 ISM 유지하며 타입 다양화. AnimToTexture로 애니메이션을 GPU로 이관 |
| **[게임 흐름 · HUD](Docs/VS_Portfolio_Systems.md#4-게임-흐름--mvvm-hud)** | 델리게이트 기반 느슨한 결합 + MVVM 바인딩 |
| **[보스](Docs/VS_Portfolio_Systems.md#5-보스-시스템--ism-대량-처리와-개별-액터의-하이브리드)** | 액터/데이터 이중 상속 계층, 공유 행동 블록, 예고 가능한 돌진 |
| **[타격 통합](Docs/VS_Portfolio_Systems.md#6-타격-통합--두-적-표현의-통일)** | ISM 인덱스와 액터 포인터를 센티넬 값으로 통일 |
| **[엘리트](Docs/VS_Portfolio_Systems.md#7-적-다양성-완성--엘리트-데이터-주도)** | 데이터만으로 정의되는 강화 개체 |

---

## 성능 측정

적 500마리를 두 방식으로 스폰해 같은 조건에서 비교했습니다. 콘솔 명령으로 실행하는 벤치마크 액터를 만들어 스레드별 프레임 타임을 수집했습니다.

| 항목 | ISM + AnimToTexture | 액터 + 스켈레탈 |
|---|---|---|
| **게임 스레드** | **5.90 ms** | 14.52 ms |
| **드로우콜** | **약 28** | 약 2,765 |
| GPU | 27.71 ms | 13.37 ms |
| Frame avg | 28.52 ms | 14.72 ms |

CPU 축에서는 의도한 결과가 나왔지만 **총 프레임은 오히려 느렸습니다.** 가상 그림자 맵 · 레이트레이싱 · 드로우콜 · 렌더 스테이트 재생성을 순서대로 배제한 끝에, 원인이 LOD 부재임을 확인했습니다.
액터 쪽 메시는 LOD가 여러 단계라 거리에 따라 저해상도로 내려가지만, ISM에 쓰는 메시는 LOD 0 하나(41,052 트라이앵글)뿐이라 500마리가 거리와 무관하게 전부 풀 해상도로 그려집니다.

> 측정 조건, 가설 소거 과정, 대응 방안은 [설계 문서의 성능 측정 절](Docs/VS_Portfolio_Systems.md#성능-측정--두-표현-방식의-실제-비용)에 정리했습니다.

---

## 보스 — 예고 가능한 공격

<!-- TODO: 돌진 GIF (예고선 차오르고 옆으로 피하는 순간) -->
![보스 돌진 텔레그래프](Docs/gif/boss_charge.gif)

돌진 보스에서 가장 오래 붙든 문제는 구현이 아니라 **플레이어가 읽을 수 있는가**였습니다.

상태 머신(Chase → Aim → Charge → Recover)을 넣고도 피할 수 없었는데, 원인은 회전 함수였습니다. `RInterpTo`의 `InterpSpeed`는 각속도가 아니라 지수 감쇠 계수라 사실상 즉시 스냅이었고, 조준 중에도 보스가 플레이어를 완벽히 추적했습니다. `RInterpConstantTo`(등속, 도/초)로 바꾼 뒤 바닥 데칼로 돌진 경로를 표시했습니다.

**예고선이 판정에서 값을 읽게 한 것**이 핵심입니다. 길이는 `Speed × Duration`, 반폭은 `ContactRange`에서 파생시켜 연출이 판정과 어긋날 경로 자체를 없앴습니다.

---

## 기술 스택

| 영역 | 사용 기술 |
|---|---|
| 대량 적 렌더링 | InstancedStaticMeshComponent + per-instance custom data, AnimToTexture(Bone 모드) |
| 보스 | 별도 AActor + 액터/데이터 이중 상속 계층, 데이터 주도 메시·애니 |
| 보스 예고 연출 | UDecalComponent + UMaterialInstanceDynamic 스칼라 파라미터 |
| 성능 측정 | 콘솔 exec 벤치마크 액터, 스레드별 프레임 타임 수집, stat gpu 패스 분석 |
| 무기 시스템 | Strategy 패턴 (UObject 기반 behavior) |
| 스폰/난이도 | WorldSubsystem + FTickableGameObject, 데이터 주도 웨이브 |
| UI | MVVM (UMVVMViewModelBase, FieldNotify), 델리게이트 기반 이벤트 |
| 데이터 정의 | UPrimaryDataAsset (무기·적·웨이브·업그레이드·보스) |

---

## 프로젝트 구조

```
Source/VS/
├─ Character/     플레이어 캐릭터·컨트롤러, 패시브 스탯 수정자
├─ Component/     무기 컴포넌트, 업그레이드 컴포넌트
├─ Weapon/        투사체·드론·오라 + Behavior/ (Strategy 구현체)
├─ Enemy/         보스 베이스 + 아키타입 3종
├─ Manager/       적 매니저(ISM), 젬 매니저(ISM)
├─ Subsystem/     난이도·웨이브 서브시스템
├─ Data/          데이터 애셋 정의 (무기·적·웨이브·업그레이드·보스)
├─ ViewModel/     MVVM 뷰모델
├─ UI/            위젯
└─ Debug/         치트 매니저, 벤치마크
```

---

## 실행

패키징된 빌드는 위 링크에서 받을 수 있습니다. 소스에서 빌드하려면 UE 5.5.4가 필요하며, `VS.uproject` 우클릭 → *Generate Visual Studio project files* 후 빌드하면 됩니다.

디버그·측정용 콘솔 명령(`~` 키):

```
VSGiveAllWeapons 5    모든 무기 지급&강화
VSGiveAllPassive 5    모든 패시브 강화
VSAddXP 100           XP 추가
VSSkipLevelUp 1       레벨업/업그레이드 봉인
VSStopSpawn 1         웨이브 자동 스폰 전체 봉인
VSBenchISM 300        벤치시작, 적소환(ISM + AnimToTexture)
VSBenchActors 300     벤치시작, 적소환(액터 + 스켈레탈)
VSEnemyClear          모든 적 즉시 제거
VSObjectClear         VSEnemyClear + 바닥의 XP 젬까지 전부 제거
VSSpawnBoss 0 600     지정 웨이브 보스를 지정 거리에 소환
```
