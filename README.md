# UnrealShowdows_LJW

Unreal Engine 5.4 기반 협동 멀티플레이 던전 크롤러.

## 개발 환경

- Unreal Engine 5.4
- C++ / Blueprint
- 네트워킹: OnlineSubsystem Null (LAN)

## 구현 내용

**네트워크**
- 서버 권위 모델 — 전투·AI·퀘스트 판정은 서버에서만 수행
- RPC 13종 (Server 5 / Client 3 / NetMulticast 5)
- 복제 프로퍼티 9종, 개인 데이터는 `COND_OwnerOnly`로 대역폭 제한
- 로비 준비 확인 후 `ServerTravel`로 던전 이동

**게임플레이**
- 투사체 전투, 체력·사망 처리, 넉백/스턴/피격 연출
- 감지 기반 미니언 AI (시야 + 청각, 순찰 → 추격 → 공격), 경보 전파
- 스텔스 — 스프린트 시 소음 발생, 캐릭터 스탯으로 감쇠
- NPC 구출 퀘스트, 파티 공용 열쇠, 확률 기반 아이템 드랍
- GameplayTag 조건부 대화 시스템

**데이터 주도**
- `FUS_CharacterStats` — 레벨별 이동속도/데미지 배율/스텔스 계수/최대 체력
- `FUS_CharacterSkins` — 캐릭터 머티리얼 세트
- `UDialogueDataAsset` — 대화 노드 그래프

## 리팩토링

프로토타입 동작 이후 네트워크 코드를 점검해 버그 8종과 구조 문제 7종을 수정했습니다.
각 항목은 개별 커밋으로 남아 있습니다.

| 증상 | 원인 | 조치 |
|---|---|---|
| 클라이언트 접속 끊김 | 매 프레임 Reliable RPC 전송 | 스로틀 + 상태 변화 시에만 전송 |
| 스텔스 미작동 | 스프린트가 서버에 미전달 | 입력 시점 Server RPC 호출 |
| AIController 누수 | 사망 시 `UnPossess()` 선호출 | 제거 + 감지 콜백에 사망 가드 |
| 클라이언트 상태 불일치 | `HasAuthority()` 검사 누락 | 서버 권한 검사 추가 |
| 투사체 무한 스폰·누수 | `Triggered` 바인딩, 수명 미설정 | `Started` + 쿨다운 + 수명 설정 |
| 미니언 경보 O(n²) | 매 프레임 전체 액터 검색 | 레지스트리 + 경보 쿨다운 |

---

초기 버전: [Dungeon_Rescuer](https://github.com/apppleManGO/Dungeon_Rescuer)
