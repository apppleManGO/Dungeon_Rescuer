// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "US_GameState.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Lobby,
	Playing
};
UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_GameState : public AGameState
{
	GENERATED_BODY()
public:
	//모든플레이어가 게임준비가 되었는지
	UFUNCTION(BlueprintCallable)
	bool AreAllPlayersReady() const;

	//현재 게임상태 - 로비 , 게임중 , 게임오버
	UPROPERTY(ReplicatedUsing=OnRep_GamePhase, BlueprintReadOnly, Category="Game Phase")
	EGamePhase CurrentPhase;

	UFUNCTION()
	void OnRep_GamePhase();

	void SetGamePhase(EGamePhase NewPhase);
	UPROPERTY(ReplicatedUsing="OnRep_PartyHasPrisonKey", BlueprintReadOnly, Category="Dungeon")
	bool bPartyHasPrisonKey =false;

	UFUNCTION()
	void OnRep_PartyHasPrisonKey();

	// 서버에서 공용 열쇠 지급
	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	void GrantPartyPrisonKey();

	// 열쇠를 사용할 때(문 열 때) 호출
	UFUNCTION(BlueprintCallable, Category = "Game Logic")
	bool ConsumePartyPrisonKey();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 이번 스테이지에서 구조해야 할 총 NPC 수 (에디터에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 TotalNPCsToRescue = 3;

	// 현재까지 구조한 NPC 수 (서버에서 클라이언트로 동기화)
	UPROPERTY(ReplicatedUsing = OnRep_RescuedNPCCount, BlueprintReadOnly, Category = "Quest")
	int32 RescuedNPCCount = 0;

	// 모든 NPC 구출 시 탈출 포탈이 생성될 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector EscapePortalLocation = FVector(-445.56f, -3613.61f, -1.90f);

	// NPC 구조 시 서버에서 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AddRescuedNPC();

	// 동기화 시 호출될 함수 (UI 업데이트 등에 활용)
	UFUNCTION()
	void OnRep_RescuedNPCCount();

	// 모든 NPC를 구했는지 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsAllNPCsRescued() const { return RescuedNPCCount >= TotalNPCsToRescue; }
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAllNPCsRescued(FVector SpawnLocation);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Quest")
	void BP_OnAllNpCsRescued(FVector SpawnLocation);

	// 현재 접속 플레이어 수. BP_Potal이 이 변수를 읽으므로 이름을 바꾸면 BP 컴파일이 깨진다.
	// 주의: 원본은 `= PlayerArray.Num()`로 선언돼 있었으나 멤버 초기화식은 생성 시점에
	// 평가되므로 항상 0이었다(갱신 주체도 없었음). 실제 값이 필요하면 GameMode의
	// PostLogin/Logout에서 PlayerArray.Num()으로 갱신하고 복제를 걸어야 한다.
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	int32 TotalPlayers = 0;
};
