// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "US_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AUS_GameMode();
	UFUNCTION(BlueprintCallable,Category="Minions")
	void AlertMinions(class AActor* AlertInstigator, const FVector& Location, const float Radius);
	void RegisterMinion(class AUS_Minion* Minion);
	void UnregisterMinion(class AUS_Minion* Minion);
	UFUNCTION(BlueprintCallable,Category="Gameplay")
	void CheckAndStartGame();
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY()
	int32 NextSkinIndex = 0;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<class AUS_Minion>> RegisteredMinions;
};
