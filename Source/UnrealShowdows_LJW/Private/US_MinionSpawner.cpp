// Fill out your copyright notice in the Description page of Project Settings.


#include "US_MinionSpawner.h"
#include "US_Minion.h"
#include "Components/BoxComponent.h"

// Sets default values
AUS_MinionSpawner::AUS_MinionSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetBoxExtent(FVector(1000.f, 1000.f, 1000.f));

}

// Called when the game starts or when spawned
void AUS_MinionSpawner::BeginPlay()
{
	Super::BeginPlay();

	if(SpawnableMinions.IsEmpty())return;
	if(GetLocalRole() != ROLE_Authority)return;
	for(int32 i=0;i<NunMinionsAtStart;i++)
	{
		Spawn();
	}
	GetWorldTimerManager().SetTimer(SpawnTimerHandle,this,&AUS_MinionSpawner::Spawn,SpawnDelay,true,SpawnDelay);
}

void AUS_MinionSpawner::Spawn()
{
	FActorSpawnParameters spawnParams;
	// 1. 충돌 시 위치를 조정하되 "항상 스폰(AlwaysSpawn)" 하도록 변경
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	auto Minion = SpawnableMinions[FMath::RandRange(0,SpawnableMinions.Num()-1)];

	const auto Rotation = FRotator(0.0f, FMath::RandRange(0,360), 0.0f);
    
	// 2. Z축 좌표를 바닥에 박히지 않게 충분히 띄워줍니다. (예: 100.f ~ 150.f)
	// BoxExtent의 Z값을 활용해서 띄워줘도 좋습니다.
	const auto Location = SpawnArea->GetComponentLocation() +
	   FVector(
		   FMath::RandRange(-SpawnArea->GetScaledBoxExtent().X, SpawnArea->GetScaledBoxExtent().X),
		   FMath::RandRange(-SpawnArea->GetScaledBoxExtent().Y, SpawnArea->GetScaledBoxExtent().Y),
		   150.0f // <-- 0 대신 높이를 주어 공중에서 떨어지도록 설정
	   );

	GetWorld()->SpawnActor<AUS_Minion>(Minion, Location, Rotation, spawnParams);
}

// Called every frame
void AUS_MinionSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

