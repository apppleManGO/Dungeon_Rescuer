// Fill out your copyright notice in the Description page of Project Settings.


#include "US_PlayerState.h"

#include "US_Character.h"
#include "US_CharacterStats.h"
#include "US_GameState.h"
#include "US_GameMode.h"
#include "Net/UnrealNetwork.h"

void AUS_PlayerState::OnRep_CharacterLevelUp(int32 OldValue) const
{
	//Broadcast 모든 코드에 이 델리게이트 함수와 연관된 모든 코드를 (인자값)을 전달하면서 호출
	OnCharacterLevelUp.Broadcast(CharacterLevel);
}

void AUS_PlayerState::OnRep_Xp(int32 OldValue) const
{
	//Broadcast 모든 코드에 이 델리게이트 함수와 연관된 모든 코드를 (인자값)을 전달하면서 호출
	OnXpChange.Broadcast(Xp);
}

void AUS_PlayerState::OnRep_IsReady(bool OldValue) const
{
	OnReady.Broadcast(bIsReady);
	// 로그 찍기
	//스크린에 로그찍기
}

void AUS_PlayerState::AddXp(int32 Value)
{
	if (!HasAuthority()) return;

	Xp += Value;
	//Broadcast 모든 코드에 이 델리게이트 함수와 연관된 모든 코드를 (인자값)을 전달하면서 호출
	OnXpChange.Broadcast(Value);

	if (const auto Character = Cast<AUS_Character>(GetPawn()))
	{
		const FUS_CharacterStats* Stats = Character->GetCharacterStats();
		if(Stats && Stats->NextLevelXp < Xp)
		{

			CharacterLevel++;
			Character->UpdateCharacterStats(CharacterLevel);
			OnCharacterLevelUp.Broadcast(CharacterLevel);
		}
		
	}
}

//변수들이 네트워크를 통해 조건에 따라 복제될지 정의 하는 함수
void AUS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//특정 변수에 대해 복제 + 조건 지정(복제할 변수가 소속된 클래스,복제 대상 변수,이 오브젝트의 Owner(소유자)에게만 복제됨)
	DOREPLIFETIME(AUS_PlayerState, SkinIndex);
	DOREPLIFETIME_CONDITION(AUS_PlayerState,Xp,COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AUS_PlayerState,CharacterLevel,COND_OwnerOnly);
	DOREPLIFETIME(AUS_PlayerState, bIsReady);
}
void AUS_PlayerState::SetSkinIndex(int32 NewIndex)
{
	if (HasAuthority())
	{
		SkinIndex = NewIndex;

		// 서버 자기 자신도 즉시 적용
		if (APawn* Pawn = GetPawn())
		{
			if (AUS_Character* Character = Cast<AUS_Character>(Pawn))
			{
				Character->ApplySkin(SkinIndex);
			}
		}
	}
}

void AUS_PlayerState::OnRep_SkinIndex(int32 OldValue)
{
	//UE_LOG(LogTemp, Warning,
		//TEXT("[OnRep_SkinIndex] %s : %d -> %d"),
		//*GetName(), OldValue, SkinIndex);

	if (APawn* Pawn = GetPawn())
	{
		if (AUS_Character* Char = Cast<AUS_Character>(Pawn))
		{
			Char->ApplySkin(SkinIndex);
		}
	}
}

void AUS_PlayerState::ServerSetReady_Implementation(bool bReady)
{
	// bIsReady = bReady;
	// if(bIsReady)bIsReady=false;
	// else
	// {
	// 	bIsReady=true;
	// }
	// OnRep_IsReady(bIsReady);
	bIsReady = bReady;
	OnRep_IsReady(bIsReady);
	// 서버에서만 GameMode 존재
	if (AUS_GameMode* GM = GetWorld()->GetAuthGameMode<AUS_GameMode>())
	{
		GM->CheckAndStartGame();
	}
}
