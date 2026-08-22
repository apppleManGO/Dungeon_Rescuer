// Fill out your copyright notice in the Description page of Project Settings.


#include "US_GameState.h"

#include "US_PlayerController.h"
#include "GameFramework/GameState.h"
#include "US_PlayerState.h"
#include "Net/UnrealNetwork.h"

bool AUS_GameState::AreAllPlayersReady() const
{
	for (APlayerState* PS : PlayerArray)
	{
		AUS_PlayerState* MyPS = Cast<AUS_PlayerState>(PS);
		if (MyPS && !MyPS->bIsReady)
		{
			return false;
		}
	}
	return true;
}

void AUS_GameState::OnRep_GamePhase()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AUS_PlayerController* MyPC = Cast<AUS_PlayerController>(PC))
		{
			if (MyPC->IsLocalController())  // 🔑 로컬 컨트롤러일 때만 UI 업데이트
			{
				MyPC->UpdateUIForPhase(CurrentPhase);
			}
		}
	}
}

void AUS_GameState::SetGamePhase(EGamePhase NewPhase)
{
	if (HasAuthority()) // 서버에서만 상태 변경
	{
		CurrentPhase = NewPhase;      // Phase 변경
		OnRep_GamePhase();             // 서버에서도 UI 갱신
	}
}

void AUS_GameState::OnRep_PartyHasPrisonKey()
{
	// 여기서는 "공용 상태가 바뀌었다"는 사실만 처리하면 됨
	// (UI는 각 클라에서 자기 PC로 처리하는 게 안전)
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AUS_PlayerController* MyPC = Cast<AUS_PlayerController>(PC))
		{
			if (MyPC->IsLocalController())
			{
				MyPC->OnPartyKeyChanged(bPartyHasPrisonKey); // (아래 2)에서 만들어줄 함수)
			}
		}
	}
}

void AUS_GameState::GrantPartyPrisonKey()
{
	if (!HasAuthority()) return;

	if (!bPartyHasPrisonKey)
	{
		bPartyHasPrisonKey = true;
		OnRep_PartyHasPrisonKey(); // 서버에서도 처리
	}
}

bool AUS_GameState::ConsumePartyPrisonKey()
{
	if (!HasAuthority()) return false;

	if (bPartyHasPrisonKey)
	{
		bPartyHasPrisonKey = false;
		OnRep_PartyHasPrisonKey();
		return true;
	}
	return false;
}
void AUS_GameState::AddRescuedNPC()
{
	if (!HasAuthority()) return;

	RescuedNPCCount++;
	OnRep_RescuedNPCCount();

	if (IsAllNPCsRescued())
	{
		BP_OnAllNpCsRescued(EscapePortalLocation);
		Multicast_OnAllNPCsRescued(EscapePortalLocation);
	}
}
void AUS_GameState::OnRep_RescuedNPCCount()
{
	// NPC 카운트가 변경되었을 때 각 플레이어의 UI를 갱신하는 로직을 넣을 수 있습니다.
	// 예: MyPC->UpdateRescueUI(RescuedNPCCount, TotalNPCsToRescue);
}
// Replication 설정
void AUS_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUS_GameState, CurrentPhase);
	DOREPLIFETIME(AUS_GameState, bPartyHasPrisonKey);
	DOREPLIFETIME(AUS_GameState, RescuedNPCCount);
}
void AUS_GameState::Multicast_OnAllNPCsRescued_Implementation(FVector SpawnLocation)
{
	UE_LOG(LogTemp, Log, TEXT("All NPCs rescued. Escape portal at %s"), *SpawnLocation.ToString());
}
// AUS_GameState.cpp 내부 예시 함수
int32 AUS_GameState::GetAlivePlayerCount() const
{
	int32 AliveCount = 0;
	for (APlayerState* PS : PlayerArray)
	{
		// PlayerState나 Character에 bIsDead 같은 변수가 있다면 체크
		// AUS_PlayerState* MyPS = Cast<AUS_PlayerState>(PS);
		// if (MyPS && !MyPS->bIsDead) AliveCount++;
	}
	return AliveCount;
}
