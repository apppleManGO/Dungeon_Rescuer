// Fill out your copyright notice in the Description page of Project Settings.


#include "US_GameMode.h"
#include "US_PlayerController.h"
#include "US_PlayerState.h"
#include "US_Minion.h"
#include "US_Character.h"
#include "UObject/ConstructorHelpers.h"
#include "US_GameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"

AUS_GameMode::AUS_GameMode()
{
	bUseSeamlessTravel = true;
	GameStateClass=AUS_GameState::StaticClass();

	PlayerStateClass=AUS_PlayerState::StaticClass();
	PlayerControllerClass=AUS_PlayerController::StaticClass();
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_Character"));
	if(PlayerPawnBPClass.Class !=nullptr)
	{
		DefaultPawnClass=PlayerPawnBPClass.Class;
	}
}

void AUS_GameMode::AlertMinions(class AActor* AlertInstigator, const FVector& Location, const float Radius)
{
	if (!AlertInstigator) return;

	RegisteredMinions.RemoveAll([](const TWeakObjectPtr<AUS_Minion>& Minion)
	{
		return !Minion.IsValid();
	});

	for (const TWeakObjectPtr<AUS_Minion>& MinionPtr : RegisteredMinions)
	{
		AUS_Minion* Minion = MinionPtr.Get();
		if (!Minion || AlertInstigator == Minion) continue;

		if (FVector::DistSquared(AlertInstigator->GetActorLocation(), Minion->GetActorLocation())
			< FMath::Square(Radius))
		{
			Minion->GoToLocation(Location);
		}
	}
}

void AUS_GameMode::RegisterMinion(AUS_Minion* Minion)
{
	if (Minion)
	{
		RegisteredMinions.AddUnique(Minion);
	}
}

void AUS_GameMode::UnregisterMinion(AUS_Minion* Minion)
{
	RegisteredMinions.Remove(Minion);
}

void AUS_GameMode::CheckAndStartGame()
{
	AUS_GameState* GS = GetGameState<AUS_GameState>();
	if (GS && GS->AreAllPlayersReady())
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AUS_PlayerController* PC = Cast<AUS_PlayerController>(It->Get()))
			{
				// 모든 클라이언트에게 페이드 명령 전달
				PC->Client_StartFadeOut();
			}
		}

		// 1초 뒤 서버 트래블 실행
		FTimerHandle TravelTimer;
		GetWorldTimerManager().SetTimer(TravelTimer, [this]() {
			GetWorld()->ServerTravel("/Game/Maps/Level_01?listen");
		}, 1.0f, false);
	}
}
// US_GameMode.cpp
void AUS_GameMode::BeginPlay()
{
	Super::BeginPlay();

	FString MapName = GetWorld()->GetMapName();

	// PIE 실행 시에는 "UEDPIE_0_Level_01" 이런 식으로 접두사가 붙음
	if (MapName.Contains("Level_01"))
	{
		if (AUS_GameState* GS = GetGameState<AUS_GameState>())
		{
			GS->SetGamePhase(EGamePhase::Playing);
			//UE_LOG(LogTemp, Warning, TEXT("Map [%s] loaded → Phase set to Playing"), *MapName);
		}
	}
	else if(MapName.Contains("Map_02"))
	{
		if (AUS_GameState* GS = GetGameState<AUS_GameState>())
		{
			GS->SetGamePhase(EGamePhase::Lobby);
			//UE_LOG(LogTemp, Warning, TEXT("Map [%s] loaded → Phase set to Lobby"), *MapName);
		}
	}
}
void AUS_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const AUS_Character* DefaultCharacter = DefaultPawnClass
		? Cast<AUS_Character>(DefaultPawnClass->GetDefaultObject())
		: nullptr;
	const UDataTable* SkinDataTable = DefaultCharacter
		? DefaultCharacter->GetCharacterSkinDataTable()
		: nullptr;
	const int32 SkinCount = SkinDataTable ? SkinDataTable->GetRowMap().Num() : 0;

	if (SkinCount <= 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Character skin DataTable is missing or empty."));
		return;
	}

	const int32 SkinIndex = NextSkinIndex;
	NextSkinIndex = (NextSkinIndex + 1) % SkinCount;

	if (AUS_PlayerState* PS = NewPlayer->GetPlayerState<AUS_PlayerState>())
	{
		PS->SetSkinIndex(SkinIndex);
		//UE_LOG(LogTemp, Warning,
	//TEXT("[PS] %s SkinIndex = %d (Role=%d)"),
	//*GetName(), SkinIndex, (int32)GetLocalRole());
	}
}
