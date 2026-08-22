// Fill out your copyright notice in the Description page of Project Settings.


#include "US_MainMenuPawn.h"
#include "US_CharacterSkins.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AUS_MainMenuPawn::AUS_MainMenuPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	RootComponent = Arrow;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(450.0f, 90.0f, 160.0f));
	Camera->SetRelativeRotation(FRotator(-10.0f, 180.0f, 0.0f));

	Mesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeLocation(FVector(0.0f, -30.0f, 90.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/KayKit/Characters/rogue"));
	if(SkeletalMeshAsset.Succeeded())
	{
		Mesh->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}
}

// Called when the game starts or when spawned
void AUS_MainMenuPawn::BeginPlay()
{
	Super::BeginPlay();
	if(IsLocallyControlled())
	{
		RandomizeCharacterSkin();
	}
}

void AUS_MainMenuPawn::RandomizeCharacterSkin()
{
	if(CharacterSkinDataTable)
	{
		TArray<FUS_CharacterSkins*> CharacterSkinRows;
		CharacterSkinDataTable->GetAllRows<FUS_CharacterSkins>(TEXT("US_Character"),CharacterSkinRows);
		if(CharacterSkinRows.Num()>0)
		{
			const auto NewIndex = FMath::RandRange(0, CharacterSkinRows.Num() - 1);
			const FUS_CharacterSkins* Skin = CharacterSkinRows[NewIndex];
			if (!Skin) return;

			Mesh->SetMaterial(4, Skin->Material4);
			Mesh->SetMaterial(0, Skin->Material0);
			Mesh->SetMaterial(1, Skin->Material1);
			Mesh->SetMaterial(2, Skin->Material2);
		}
	}
}

// Called every frame
void AUS_MainMenuPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUS_MainMenuPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
