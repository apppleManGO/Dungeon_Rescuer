// Fill out your copyright notice in the Description page of Project Settings.


#include "US_BasePickup.h"
#include "US_Character.h"
#include "Components/SphereComponent.h"
// Sets default values
AUS_BasePickup::AUS_BasePickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = SphereCollision;
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetSphereRadius(200.0f);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//액터는 기본적으로 복제되지 않으므로 가능으로 설정해준다.
	bReplicates = true;
		
}

// Called when the game starts or when spawned
void AUS_BasePickup::BeginPlay()
{
	Super::BeginPlay();
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this,&AUS_BasePickup::OnBeginOverlap);

	
}
void AUS_BasePickup::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if(const auto Character = Cast<AUS_Character>(OtherActor))
	{
		Pickup(Character);
	}
}


// Called every frame
void AUS_BasePickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



// 블루프린트 또는 자식 클래스에서 오버라이드 가능
void AUS_BasePickup::Pickup_Implementation(class AUS_Character* OwningCharacter)
{
//AActor 클래스에서 상속된 함수로 소유자를 설정하는 함수
	SetOwner(OwningCharacter);
}
