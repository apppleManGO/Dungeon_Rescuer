// Fill out your copyright notice in the Description page of Project Settings.


#include "US_WeaponprojectileComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "US_BaseWeaponProjectile.h"
#include "US_Character.h"

// Sets default values for this component's properties
UUS_WeaponprojectileComponent::UUS_WeaponprojectileComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	ProjectileClass = AUS_BaseWeaponProjectile::StaticClass();
	// ...
}


// Called when the game starts
void UUS_WeaponprojectileComponent::BeginPlay()
{
	Super::BeginPlay();
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)return;
	if(const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(WeaponMappingContext,1);
		}
		if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(ThrowAction,ETriggerEvent::Triggered,this,&UUS_WeaponprojectileComponent::Throw);
		}
	}
	// ...
	
}

void UUS_WeaponprojectileComponent::Throw()
{
	Throw_Server();
}

void UUS_WeaponprojectileComponent::Throw_Client_Implementation()
{
	const auto  Character = Cast<AUS_Character>(GetOwner());
	if(ThrowAnimation != nullptr)
	{
		if(const auto AnimInstance = Character->GetMesh()->GetAnimInstance();AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(ThrowAnimation,1.f);
		}
	}
}

void UUS_WeaponprojectileComponent::Throw_Server_Implementation()
{
	if(ProjectileClass)
	{
		Throw_Client();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,[&]()
		{
			const auto Character = Cast<AUS_Character>(GetOwner());
			const auto ProjectileSpawnLocation = GetComponentLocation();
			const auto ProjectileSpawnRotation = GetComponentRotation();
			auto ProjectileSpawnParams = FActorSpawnParameters();
				
			ProjectileSpawnParams.Owner =GetOwner();
			ProjectileSpawnParams.Instigator=Character;
			GetWorld()->SpawnActor<AUS_BaseWeaponProjectile>(ProjectileClass,ProjectileSpawnLocation,ProjectileSpawnRotation,ProjectileSpawnParams);
		},.4f,false);

		}
}


// Called every frame
void UUS_WeaponprojectileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUS_WeaponprojectileComponent::SetProjectileClass(TSubclassOf<class AUS_BaseWeaponProjectile> NewProjectileClass)
{
	ProjectileClass = NewProjectileClass;
}

