// Fill out your copyright notice in the Description page of Project Settings.


#include "US_BaseWeaponProjectile.h"
#include "US_Character.h"
#include "US_CharacterStats.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DataWrappers/ChaosVDParticleDataWrapper.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUS_BaseWeaponProjectile::AUS_BaseWeaponProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetSphereRadius(10);
	SphereCollision->BodyInstance.SetCollisionProfileName("BlockAll");
	SphereCollision->OnComponentHit.AddDynamic(this,&AUS_BaseWeaponProjectile::OnHit);

	RootComponent = SphereCollision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Mesh->SetRelativeLocation(FVector(-40, 0, 0));
	Mesh->SetRelativeRotation(FRotator(-90, 0, 0));
	static  ConstructorHelpers::FObjectFinder<UStaticMesh>StaticMesh(TEXT("/Game/KayKit/DungeonElements/dagger_common"));
	if (StaticMesh.Succeeded())
	{
		GetMesh()->SetStaticMesh(StaticMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->ProjectileGravityScale=0;
	ProjectileMovement->InitialSpeed=3000;
	ProjectileMovement->MaxSpeed = 3000;
	ProjectileMovement->bRotationFollowsVelocity=true;
	ProjectileMovement->bShouldBounce=false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AUS_BaseWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUS_BaseWeaponProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	auto ComputedDamage = Damage;
	if(const auto Character = Cast<AUS_Character>(GetInstigator()))
	{
		ComputedDamage *=Character->GetCharacterStats()->DamageMultipier;
	}

	if(OtherActor && OtherActor != this)
	{
		const FDamageEvent Event(UDamageType::StaticClass());
		OtherActor->TakeDamage(ComputedDamage,Event,GetInstigatorController(),this);
	}
	Destroy();
}

// Called every frame
void AUS_BaseWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

