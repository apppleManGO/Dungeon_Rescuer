// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "US_MinionSpawner.generated.h"

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_MinionSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUS_MinionSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void Spawn();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	TArray<TSubclassOf<class AUS_Minion>> SpawnableMinions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	float SpawnDelay =15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	int32 NunMinionsAtStart = 2;

	// 이 스포너가 동시에 유지할 수 있는 최대 미니언 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	int32 MaxAliveMinions = 8;

private:
	// 이 스포너가 스폰한 미니언 추적
	UPROPERTY()
	TArray<TWeakObjectPtr<class AUS_Minion>> SpawnedMinions;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Spawn System",meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UBoxComponent> SpawnArea;

	UPROPERTY()
	FTimerHandle SpawnTimerHandle;

};
