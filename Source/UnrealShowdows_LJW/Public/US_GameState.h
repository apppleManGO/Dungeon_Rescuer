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
	Playing,
	GameOver
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
	int32 GetAlivePlayerCount() const;

	// 이번 스테이지에서 구조해야 할 총 NPC 수 (에디터에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 TotalNPCsToRescue = 3;

	// 현재까지 구조한 NPC 수 (서버에서 클라이언트로 동기화)
	UPROPERTY(ReplicatedUsing = OnRep_RescuedNPCCount, BlueprintReadOnly, Category = "Quest")
	int32 RescuedNPCCount = 0;

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
	
	//플레이어 수 확인
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPlayers = PlayerArray.Num();
};
