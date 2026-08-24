<!-- TODO: 저장소명/표기명 확인. 폴더명은 UnrealShowdows_LJW이지만 표기는 아래처럼 정리하는 걸 권장 -->

# Unreal Showdown

**Unreal Engine 5.4 · C++ · 서버 권위(Server-Authoritative) 협동 멀티플레이 던전 크롤러**

세션으로 모인 플레이어가 로비에서 준비를 맞추고, 던전에 들어가 미니언을 뚫고 갇힌 NPC를 구출한 뒤 탈출 포탈로 빠져나오는 협동 게임입니다.
전투·AI·퀘스트·대화까지 **모든 게임 로직을 서버가 판정**하고 클라이언트는 결과를 복제받습니다.

<!-- TODO: GIF 3개 삽입. 각 5초 내외, 800px 폭 권장.
     녹화한 영상에서 잘라 쓰시면 됩니다. 맨 위에 움직이는 게 없으면 리뷰어가 스크롤을 안 내립니다. -->
| 2인 협동 전투 | 미니언 추격 AI | NPC 구출 → 탈출 포탈 |
|:---:|:---:|:---:|
| ![전투](Docs/media/combat.gif) | ![AI](Docs/media/chase.gif) | ![퀘스트](Docs/media/rescue.gif) |

<!-- TODO: 링크 채우기 -->
**▶ [데모 영상 (3분)](영상URL)**  ·  **⬇ [실행 파일 다운로드](릴리스URL)**  ·  📄 [리팩토링 기록](Docs/REFACTOR_TODO.md)

---

## 목차

- [핵심 기능](#핵심-기능)
- [기술 구조](#기술-구조)
- [트러블슈팅 · 리팩토링](#트러블슈팅--리팩토링)
- [학습 기반과 직접 구현 범위](#학습-기반과-직접-구현-범위)
- [프로젝트 구조](#프로젝트-구조)
- [빌드 및 실행](#빌드-및-실행)
- [크레딧](#크레딧)

---

## 핵심 기능

### 세션 → 로비 → 던전 → 탈출

```
메인 메뉴          세션 생성 / 검색 (OnlineSubsystem Null, LAN)
   ↓
로비              PlayerState.bIsReady 복제 → 전원 준비 확인
   ↓              → 전 클라이언트 페이드 아웃(Client RPC) → ServerTravel
던전 (Level_01)    미니언 전투 · 열쇠/문 · NPC 구출
   ↓
탈출              구출 완료 시 포탈 개방 (GameState 복제 + Multicast 알림)
```

### 전투

- **투사체 기반 원거리 전투** — 서버에서만 스폰·판정, 쿨다운도 서버 권위
- **넉백 / 스턴 / 피격 플래시** — 피격 시 `LaunchCharacter`로 밀려나고, 동적 머티리얼로 전 클라이언트에서 붉게 점멸 (`NetMulticast`)
- **체력 · 사망 처리** — `TakeDamage` 오버라이드, `CurrentHealth` 복제, 사망 몽타주 / 래그돌 분기

### 미니언 AI

- `PawnSensingComponent` 기반 **시야 + 청각** 감지
- **순찰 → 추격 → 공격 → 타겟 상실 복귀** 상태 전이
- **경보 전파** — 플레이어를 발견한 미니언이 반경 내 동료를 호출 (GameMode가 미니언 레지스트리로 중계, 쿨다운 적용)
- **스텔스** — 달릴 때만 소음 발생, 캐릭터 스탯의 `StealthMultiplier`로 감쇠

### 퀘스트 · 상호작용

- **NPC 구출 카운터** — `GameState`에 복제, 전원 구출 시 탈출 포탈 개방
- **파티 공용 열쇠** — 누가 먹든 파티 전체가 공유 (`GameState.bPartyHasPrisonKey`)
- **GameplayTag 조건부 대화 시스템** — 노드/선택지 그래프를 데이터로 정의하고, 선택지마다 `FGameplayTag` 조건과 반전 플래그를 걸어 상황별로 다른 분기를 노출
- **인터페이스 기반 상호작용** — `IUS_Interactable`을 구현한 액터를 스피어 트레이스로 탐색

---

## 기술 구조

### 서버 권위 모델

모든 게임 상태 변경은 서버에서만 일어나고, 클라이언트는 **입력을 보내고 결과를 복제받습니다.**

| 계층 | 역할 |
|---|---|
| `AUS_GameMode` | 서버 전용. 스킨 배정, 미니언 경보 중계, 준비 확인 후 `ServerTravel` |
| `AUS_GameState` | 전원 복제. 게임 페이즈, 구출 카운트, 파티 공용 열쇠 |
| `AUS_PlayerState` | 플레이어별 영속 데이터. XP·레벨(소유자 한정 복제), 준비 상태·스킨(전원 복제) |
| `AUS_PlayerController` | 로컬 UI 생명주기, 페이즈별 위젯 전환 |
| `AUS_Character` | 입력 수집 → 서버 RPC. 체력·사망은 서버 권위 |

### RPC 사용 기준

3종을 용도에 따라 구분해 사용했습니다.

| 종류 | 개수 | 사용처 | 선택 이유 |
|---|:---:|---|---|
| `Server` | 5 | 스프린트, 상호작용, 투척, 스킨 지정, 준비 토글 | 클라이언트 입력을 서버 권위로 넘김 |
| `Client` | 3 | 상호작용 프롬프트 표시/숨김, 페이드 | **소유 클라이언트 1명**에게만 필요한 UI |
| `NetMulticast` | 5 | 이동 속도 변경, 피격 플래시, 구출 완료 알림 | **모두가 봐야 하는** 연출·상태 |

### 복제 프로퍼티 (9종)

```cpp
// 소유자에게만 — 다른 플레이어가 알 필요 없는 개인 데이터
DOREPLIFETIME_CONDITION(AUS_PlayerState, Xp,             COND_OwnerOnly);
DOREPLIFETIME_CONDITION(AUS_PlayerState, CharacterLevel, COND_OwnerOnly);

// 전원에게 — 공개 정보
DOREPLIFETIME(AUS_PlayerState, bIsReady);      // 로비에서 서로의 준비 상태를 봐야 함
DOREPLIFETIME(AUS_PlayerState, SkinIndex);     // 서로의 외형
DOREPLIFETIME(AUS_GameState,   CurrentPhase);  // 페이즈 전환 → 각자 UI 갱신
DOREPLIFETIME(AUS_GameState,   RescuedNPCCount);
DOREPLIFETIME(AUS_GameState,   bPartyHasPrisonKey);
DOREPLIFETIME(AUS_Character,   CurrentHealth);
DOREPLIFETIME(AUS_Character,   bIsDead);
```

`COND_OwnerOnly`로 대역폭을 줄이면서, **로비 UI에 필요한 값은 의도적으로 전원 복제**로 분리했습니다.

### 데이터 주도 설계

- `FUS_CharacterStats` (DataTable) — 레벨별 이동속도 / 스프린트 속도 / 데미지 배율 / 스텔스 계수 / 최대 체력 / 다음 레벨 경험치
- `FUS_CharacterSkins` (DataTable) — 캐릭터 머티리얼 세트. 접속 순서대로 배정
- `UDialogueDataAsset` — 대화 노드 그래프

---

## 트러블슈팅 · 리팩토링

프로토타입이 동작한 뒤, **네트워크 코드 전반을 감사해 실제 버그 8종과 구조 문제 7종을 수정**했습니다.
전체 기록은 [`Docs/REFACTOR_TODO.md`](Docs/REFACTOR_TODO.md)(작업 지시서)와 [`Docs/BP_VERIFICATION.md`](Docs/BP_VERIFICATION.md)(검증 목록)에 있습니다.

<details>
<summary><b>① 매 프레임 Reliable RPC — 멀티플레이 중 클라이언트가 무작위로 끊김</b></summary>

**증상** — 2인 플레이 중 클라이언트가 예고 없이 접속 종료.

**원인** — 서버 `Tick`이 상호작용 대상을 매 프레임 탐색하고, 그 결과를 `Client, Reliable` RPC로 **초당 60회** 전송하고 있었습니다. Reliable RPC는 순서 보장 큐에 적재되며, 큐가 넘치면 UE는 해당 연결을 강제로 끊습니다(`Reliable buffer overflow`). 상태가 바뀌지 않아도(계속 같은 NPC를 보고 있어도) 같은 명령을 계속 재전송하고 있었습니다.

**해결**
- 탐색 주기를 `InteractionCheckInterval`(0.15초)로 스로틀
- 프롬프트 표시 여부를 `bInteractPromptVisible`로 캐시해 **상태가 전환될 때만** RPC 1회 전송

**결과** — RPC 호출이 초당 60회 → 상태 전환당 1회. 부수적으로, 트레이스가 비대상 액터에 맞았을 때 직전 대상이 남아 있던 버그(벽을 보면서 이전 NPC와 상호작용 가능)도 함께 해소됐습니다.
</details>

<details>
<summary><b>② 스프린트가 서버에 전달되지 않아 스텔스 시스템 전체가 미작동</b></summary>

**증상** — 달려도 미니언이 소리를 듣지 못함.

**원인** — 입력 핸들러가 `MaxWalkSpeed`를 **로컬에서만** 변경하고 서버 RPC를 호출하지 않았습니다. 서버는 계속 걷기 속도로 인식 → 소음 발생 조건인 `MaxWalkSpeed == SprintSpeed`가 서버에서 **영원히 false** → `NoiseEmitter`·`OnHearNoise`·`StealthMultiplier`가 **구현은 되어 있으나 한 번도 실행된 적 없는 상태**였습니다.

**해결** — 입력 시점에 `SprintStart_Server()` / `SprintEnd_Server()` 호출 추가.

**배운 점** — "코드가 있다"와 "실행된다"는 다르다는 것. 이 버그는 컴파일 에러도, 크래시도, 로그 경고도 남기지 않고 **기능 하나를 통째로 죽이고 있었습니다.** 이후 기능 검증 시 "해당 코드 경로에 실제로 도달하는가"를 로그로 확인하는 절차를 추가했습니다.
</details>

<details>
<summary><b>③ 사망 처리에서 AIController 누수 — 엔진 소스로 근거 확인</b></summary>

**증상** — 미니언 사망 시 정리 코드를 넣었는데도 월드의 액터가 계속 증가.

**원인** — 사망 시 `GetController()->UnPossess()`를 먼저 호출하고 있었습니다. 엔진 코드를 확인한 결과:

```cpp
// Engine/Private/Controller.cpp — AController::PawnPendingDestroy()
UnPossess();
ChangeState(NAME_Inactive);
if (PlayerState == NULL) { Destroy(); }   // AIController는 여기서 정리된다
```

이 경로는 `APawn::DetachFromControllerPendingDestroy()`가 **`Controller != nullptr`일 때만** 호출합니다. 즉 폰이 **possess된 상태로 파괴되어야** 컨트롤러가 함께 정리되는데, 미리 분리해 버려서 AIController가 월드에 영구히 남고 있었습니다.

**해결** — `UnPossess()` 제거. 대신 사망 후 재행동을 막기 위해 감지 콜백 6곳(`OnHearNoise` / `OnPawnDetected` / `GoToLocation` / `SetNextPatrolLocation` / `Chase` / `AttemptAttack`)에 사망 가드를 추가했습니다. `Tick`에는 가드가 있었지만 **감지 콜백에는 없어서** 시체가 소리를 듣고 움직일 수 있었습니다.
</details>

<details>
<summary><b>④ 검증 절차 — "C++ 빌드 통과 ≠ 블루프린트 정상"</b></summary>

미사용 코드를 정리하며 C++ 프로퍼티를 삭제했을 때, **C++ 빌드는 통과하지만 그 프로퍼티를 참조하던 블루프린트가 조용히 깨지는** 문제를 겪었습니다. 에디터를 열기 전까지 드러나지 않는 종류의 사고입니다.

커맨드렛으로 회귀 검사를 자동화했습니다:

```bash
UnrealEditor-Cmd <프로젝트>.uproject -run=CompileAllBlueprints -unattended -nopause -nosplash
```

```
Compiling Completed with 2 errors and 2 warnings
LogBlueprint: Error: The property associated with "Total Players" could not be found
  from Source: /Game/BP/BP_Potal.BP_Potal
```

이 검사로 `BP_Potal`이 삭제된 프로퍼티를 참조하고 있음을 즉시 특정했고, 이후 **BP에 노출된 심볼(`UPROPERTY`/`UFUNCTION`/`UENUM`)을 건드릴 때는 반드시 이 커맨드렛을 통과시키는 것**을 절차로 만들었습니다.

덤으로, 해당 프로퍼티가 `int32 TotalPlayers = PlayerArray.Num();`로 선언되어 있어 **멤버 초기화식이 생성 시점에 평가되는 탓에 항상 0이었다**는 사실도 발견했습니다.
</details>

### 그 밖의 수정 요약

| 분류 | 내용 |
|---|---|
| 권한 검사 | 데미지·픽업·경험치 획득 경로 4곳에 `HasAuthority()` 누락 → 클라이언트 상태 불일치 |
| 리소스 | 투사체 수명 미설정으로 무한 비행 · 입력 트리거가 `Triggered`라 프레임마다 스폰 |
| 안정성 | 타이머 람다의 `[&]` 캡처 → 0.4초 내 소유자 파괴 시 크래시 가능 |
| 성능 | 미니언 경보가 `GetAllActorsOfClass`를 매 프레임 호출 (O(n²)) → 레지스트리 + 쿨다운 |
| 구조 | DataTable 행을 raw 포인터로 보유 → 값 복사 + 유효성 플래그 |
| 구조 | 델리게이트 중복 바인딩 (`bUseSeamlessTravel` 환경에서 콜백 2회 실행) |

---

## 학습 기반과 직접 구현 범위

이 프로젝트는 **마르코 세치, 『언리얼 엔진 5로 개발하는 멀티플레이어 게임』**의 예제를 기반으로 시작해, 그 위에 기능을 확장하고 구조를 개선한 결과물입니다.

| 구분 | 내용 |
|---|---|
| **책 기반** | Character + Enhanced Input · DataTable 스탯 시스템 · `PawnSensing` 미니언 · 스포너 · 픽업/투사체 · XP·레벨 · 스킨 DataTable · 세션 위젯 |
| **직접 구현** | 체력·사망·피격 몽타주 · 넉백/스턴/피격 플래시 · 확률 기반 드랍 테이블 · **게임 페이즈 + 준비 시스템 + 페이드 `ServerTravel`** · 파티 공용 열쇠 · **NPC 구출 퀘스트 + 탈출 포탈** · **GameplayTag 조건부 대화 시스템** · 보스 맵 · 던전 절차 생성 |
| **리팩토링** | 네트워크 버그 8종 · 구조 개선 7종 (위 [트러블슈팅](#트러블슈팅--리팩토링) 참고) |

---

## 프로젝트 구조

```
Source/UnrealShowdows_LJW/          C++ 3,032줄 / 32개 파일 / 19개 클래스·구조체
├── Public/ · Private/
│   ├── US_GameMode / US_GameState / US_PlayerState / US_PlayerController
│   │                               게임 프레임워크 전 계층 커스텀
│   ├── US_Character                플레이어 (입력·전투·상호작용·스킨)
│   ├── US_Minion / US_MinionSpawner
│   │                               감지 기반 AI · 스폰 상한 관리
│   ├── US_BaseWeaponProjectile / US_WeaponprojectileComponent
│   │                               투사체 전투
│   ├── US_BasePickup / US_Interactable
│   │                               픽업 · 상호작용 인터페이스
│   ├── US_CharacterStats / US_CharacterSkins
│   │                               DataTable 행 구조체
│   └── DialogueData / DialogueDataAsset / US_NPCDialogueActor
│                                   GameplayTag 조건부 대화
Content/
├── BP/                             블루프린트 (캐릭터·미니언 3종·NPC·픽업·위젯)
├── Maps/                           Level_MainMenu · Level_01 · Level_Boss · Gym
└── Blueprints/Dungeon/             던전 절차 생성
Docs/
├── REFACTOR_TODO.md                리팩토링 작업 지시서 (원인 분석 + 수정안)
└── BP_VERIFICATION.md              에디터/PIE 검증 목록
```

---

## 빌드 및 실행

### 실행만 해보기

<!-- TODO: 릴리스 URL -->
[Releases](릴리스URL)에서 패키징된 빌드를 받아 실행하세요. 별도 설치가 필요 없습니다.

> macOS는 서명되지 않은 앱이라 첫 실행 시 **우클릭 → 열기**로 실행해야 합니다.

**멀티플레이 테스트** — 같은 네트워크에서 두 개의 인스턴스를 실행하고, 한쪽에서 방을 만든 뒤 다른 쪽에서 검색·참여하면 됩니다. (OnlineSubsystem Null 기반 LAN)

### 소스에서 빌드

```bash
git clone <저장소URL>
```

**⚠️ 애셋 안내** — 이 저장소에는 **유료 마켓플레이스 애셋이 포함되어 있지 않습니다**(재배포 불가). 클론 직후에는 메시·사운드가 비어 있는 상태로 열립니다. 코드 리뷰 목적이라면 그대로 보셔도 되고, 실행까지 원하시면 [크레딧](#크레딧)의 애셋을 `Content/`에 배치하세요.

```bash
# 에디터 빌드
<UE_5.4>/Engine/Build/BatchFiles/Mac/Build.sh \
  UnrealShowdows_LJWEditor Mac Development -project="$(pwd)/UnrealShowdows_LJW.uproject"

# 블루프린트 회귀 검사 (에디터 종료 상태에서)
<UE_5.4>/Engine/Binaries/Mac/UnrealEditor-Cmd \
  "$(pwd)/UnrealShowdows_LJW.uproject" -run=CompileAllBlueprints -unattended -nopause -nosplash

# 패키징 (Shipping)
<UE_5.4>/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
  -project="$(pwd)/UnrealShowdows_LJW.uproject" -noP4 -platform=Mac \
  -clientconfig=Shipping -cook -build -stage -pak -archive -archivedirectory="$(pwd)/Builds"
```

---

## 크레딧

| 애셋 | 제작 | 라이선스 |
|---|---|---|
| [KayKit Dungeon Pack](https://kaylousberg.itch.io/) | Kay Lousberg | CC0 |
<!-- TODO: 아래는 실제 구매처/이름 확인해서 채우기. 링크는 Fab 페이지로 -->
| Low Poly Viking World | (제작사) | Fab 표준 라이선스 |
| Caves and Dungeons (SFX) | (제작사) | Fab 표준 라이선스 |
| Music For Your RPG | (제작사) | Fab 표준 라이선스 |
| Deadly Creatures Pack | (제작사) | Fab 표준 라이선스 |

유료 애셋은 라이선스상 재배포가 불가하여 저장소에서 제외했습니다.

<!-- TODO: LICENSE 파일 추가 후 아래 활성화
## 라이선스
이 저장소의 **코드**는 MIT 라이선스입니다. 애셋은 각 제작자의 라이선스를 따릅니다.
-->
