// Fill out your copyright notice in the Description page of Project Settings.


#include "US_Character.h"
#include "US_CharacterStats.h"
#include "US_WeaponprojectileComponent.h"
#include "US_Interactable.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "US_CharacterSkins.h"
#include "US_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUS_Character::AUS_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 스프링암 컴포넌트 생성 및 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// 카메라 컴포넌트 생성 및 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
	NoiseEmitter->NoiseLifetime = 0.01f;

	Weapon=CreateDefaultSubobject<UUS_WeaponprojectileComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(RootComponent);
	Weapon->SetRelativeLocation(FVector(120.0f, 70.0f, 0.0f));
	Weapon->SetIsReplicated(true);

	// 컨트롤러 회전 사용 비활성화 (카메라/스프링암으로 대체함)
	bUseControllerRotationPitch=false;
	bUseControllerRotationYaw=false;
	bUseControllerRotationRoll=false;

	GetCapsuleComponent()->InitCapsuleSize(60.f,96.0f);
	GetMesh()->SetRelativeRotation(FRotator(0.0f,0.0f,-91.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SkeletalMeshAsset(TEXT("/Game/KayKit/Characters/rogue"));
	if(SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}
	GetCharacterMovement()->bOrientRotationToMovement=true;
	GetCharacterMovement()->RotationRate=FRotator(0.0f,500.0f,0.0f);
	GetCharacterMovement()->MaxWalkSpeed=500.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed=20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking=2000.f;
}

// Called when the game starts or when spawned
void AUS_Character::BeginPlay()
{
	Super::BeginPlay();
	// Enhanced Input Subsystem을 가져와서 입력 매핑 적용
	if(APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext,0);
		}
	}

	UpdateCharacterStats(1);
}
void AUS_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AUS_PlayerState* PS = GetPlayerState<AUS_PlayerState>())
	{
		ApplySkin(PS->SkinIndex);
	}
}

void AUS_Character::SetSkinIndex_Server_Implementation(int32 NewIndex)
{
	if (AUS_PlayerState* PS = GetPlayerState<AUS_PlayerState>())
	{
		PS->SetSkinIndex(NewIndex);
	}
}

void AUS_Character::UpdateCharacterSkin(int32 InSkinIndex)
{
	if (!CharacterSkinDataTable) return;

	TArray<FUS_CharacterSkins*> Rows;
	CharacterSkinDataTable->GetAllRows<FUS_CharacterSkins>(TEXT("Skin"), Rows);

	if (Rows.Num() == 0) return;

	const int32 Index = FMath::Clamp(InSkinIndex, 0, Rows.Num() - 1);
	const FUS_CharacterSkins* Skin = Rows[Index];

	if (!Skin) return;

	GetMesh()->SetMaterial(0, Skin->Material0);
	GetMesh()->SetMaterial(1, Skin->Material1);
	GetMesh()->SetMaterial(2, Skin->Material2);
	GetMesh()->SetMaterial(4, Skin->Material4);
}

// Called every frame
void AUS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() != ROLE_Authority) return;

	if (GetCharacterStats())
	{
		const bool bIsSprinting = GetCharacterMovement()->MaxWalkSpeed == GetCharacterStats()->SprintSpeed;
		const bool bIsMoving = GetVelocity().Size() > 0.f;
		if (bIsSprinting && bIsMoving)
		{
			float NoiseMagnitude = 1.0f;
			if (GetCharacterStats()->StealthMultiplier > 0.f)
			{
				NoiseMagnitude /= GetCharacterStats()->StealthMultiplier;
			}
			NoiseEmitter->MakeNoise(this, NoiseMagnitude, GetActorLocation());
		}
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastInteractionCheckTime < InteractionCheckInterval) return;
	LastInteractionCheckTime = Now;

	AActor* FoundInteractable = nullptr;
	bool bShouldShowPrompt = false;

	if (!bIsTalking)
	{
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;
		QueryParams.AddIgnoredActor(this);

		const float SphereRadius = 50.f;
		const FVector StartLocation = GetActorLocation() + GetActorForwardVector() * 150.f;
		const FVector EndLocation = GetActorLocation() + GetActorForwardVector() * 500.f;

		const bool bIsHit = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(), StartLocation, EndLocation, SphereRadius,
			UEngineTypes::ConvertToTraceType(ECC_WorldStatic), false,
			TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true);

		if (bIsHit && HitResult.GetActor()
			&& HitResult.GetActor()->GetClass()->ImplementsInterface(UUS_Interactable::StaticClass()))
		{
			FoundInteractable = HitResult.GetActor();
			bShouldShowPrompt = FoundInteractable->ActorHasTag(FName("NPC"))
				|| FoundInteractable->ActorHasTag(FName("Door"));
		}
	}

	InteractableActor = FoundInteractable;

	if (bShouldShowPrompt != bInteractPromptVisible)
	{
		bInteractPromptVisible = bShouldShowPrompt;
		if (bShouldShowPrompt)
		{
			ShowDialogueUI_Client();
		}
		else
		{
			HideDialogueUI_Client();
		}
	}
}

// Called to bind functionality to input
void AUS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// EnhancedInputComponent로 캐스팅 후 액션 바인딩
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AUS_Character::Move);
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&AUS_Character::Look);
		EnhancedInputComponent->BindAction(InteractAction,ETriggerEvent::Triggered,this,&AUS_Character::Interact);
		EnhancedInputComponent->BindAction(SprintAction,ETriggerEvent::Started,this,&AUS_Character::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction,ETriggerEvent::Completed,this,&AUS_Character::SprintEnd);
	}

}

// 캐릭터 스탯 설정 함수 (레벨 기반으로 DataTable에서 값 불러옴)
void AUS_Character::UpdateCharacterStats(int32 CharacterLevel)
{
	//현재 뛰고있는지 확인 하는 변수
	auto IsSprinting = false;
	if (GetCharacterStats())
	{
		//현재 캐릭터 이동속도 == 맥스 스피드랑 같은경우 ->트루(뛰는중)
		IsSprinting=GetCharacterMovement()->MaxWalkSpeed==GetCharacterStats()->SprintSpeed;
	}
	if (CharacterDataTable)
	{
		TArray<FUS_CharacterStats*> CharacterStatsRows;
		CharacterDataTable->GetAllRows<FUS_CharacterStats>(TEXT("US_Character"),CharacterStatsRows);

		if (CharacterStatsRows.Num() > 0)
		{
			const auto NewCharacterLevel  = FMath::Clamp(CharacterLevel,1,CharacterStatsRows.Num());
			const FUS_CharacterStats* StatsRow = CharacterStatsRows[NewCharacterLevel - 1];
			if (!StatsRow) return;

			CharacterStats = *StatsRow;
			bStatsLoaded = true;

			GetCharacterMovement()->MaxWalkSpeed = CharacterStats.WalkSpeed;

			// [추가] 데이터 테이블에 MaxHealth가 있다면 가져오고, 아니면 기본값 사용
			// 만약 FUS_CharacterStats 구조체 안에 MaxHealth 변수가 있다면 아래 주석 해제하여 사용
			MaxHealth = CharacterStats.MaxHealth;
            
			// 레벨업/초기화 시 체력을 최대치로 회복
			CurrentHealth = MaxHealth;
			
			//현재 뛰는 중에 레벨업을 하면 뛰어가던것이 걷는 속도로 변한다.
			//현재 속도를 레벨업한 기본 속도로 업데이트 하기 때문에 서버에 호출해서 재설정하기
			if(IsSprinting)
			{
				//서버에다가 호출하기
				SprintStart_Server();
			}
		}
	}
}

void AUS_Character::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUS_Character, CurrentHealth);
	DOREPLIFETIME(AUS_Character, bIsDead);
	//DOREPLIFETIME(AUS_Character, SkinIndex);
}


void AUS_Character::Move(const struct FInputActionValue& Value)
{
	const auto MovementVector = Value.Get<FVector2D>();

	if(Controller != nullptr)
	{
		const auto Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0,Rotation.Yaw,0);

		const auto ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const auto RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection,MovementVector.Y);
		AddMovementInput(RightDirection,MovementVector.X);
	}
}

void AUS_Character::Look(const FInputActionValue& Value)
{
	const auto LookAxisVector = Value.Get<FVector2D>();

	if(Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUS_Character::SprintStart(const FInputActionValue& Value)
{
	if(GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed=GetCharacterStats()->SprintSpeed;
	}
	SprintStart_Server();
}

void AUS_Character::SprintEnd(const FInputActionValue& Value)
{
	if(GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed=GetCharacterStats()->WalkSpeed;
	}
	SprintEnd_Server();
}

void AUS_Character::Interact(const FInputActionValue& Value)
{
	Interact_Server();
	
}

void AUS_Character::Interact_Server_Implementation()
{
	if (InteractableActor)
	{
		// 상호작용 시작 (I키 누름)
		// NPC일 경우 대화 중 상태로 진입
		if (InteractableActor->ActorHasTag(FName("NPC")))
		{
			bIsTalking = true;
			HideDialogueUI_Client(); // 대화가 시작되었으니 '상호작용 [I]' UI는 숨김
		}

		IUS_Interactable::Execute_Interact(InteractableActor, this);
	}
}
void AUS_Character::SprintStart_Server_Implementation()
{
	SprintStart_Client();
}
void AUS_Character::SprintEnd_Server_Implementation()
{
	SprintEnd_Client();
}
void AUS_Character::SprintStart_Client_Implementation()
{
	if(GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed=GetCharacterStats()->SprintSpeed;
	}
}

void AUS_Character::SprintEnd_Client_Implementation()
{
	if(GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed=GetCharacterStats()->WalkSpeed;
	}
}
void AUS_Character::ShowDialogueUI()
{
	if (DialogueWidget == nullptr && DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<UUserWidget>(GetWorld(), DialogueWidgetClass);
		if (DialogueWidget)
		{
			DialogueWidget->AddToViewport();
		}
	}
}

void AUS_Character::HideDialogueUI()
{
	if (DialogueWidget)
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
}
void AUS_Character::ShowDialogueUI_Client_Implementation()
{
	if (IsLocallyControlled()) // 내 캐릭터만
	{
		ShowDialogueUI();
	}
}

void AUS_Character::HideDialogueUI_Client_Implementation()
{
	if (IsLocallyControlled()) // 내 캐릭터만
	{
		HideDialogueUI();
	}
}
void AUS_Character::ApplySkin(int32 InSkinIndex)
{
	//SkinIndex = InSkinIndex;
	UpdateCharacterSkin(InSkinIndex);
	UE_LOG(LogTemp, Warning, TEXT("ApplySkin called: %d"), InSkinIndex);
}
void AUS_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (AUS_PlayerState* PS = GetPlayerState<AUS_PlayerState>())
		{
			ApplySkin(PS->SkinIndex);
		}
	}
}

// US_Character.cpp

float AUS_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. 부모 클래스 로직 실행
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority()) return ActualDamage;

	// 데미지가 0 이하거나 이미 죽었으면 무시
	if (ActualDamage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	// 2. 체력 감소 (0 이하로 내려가지 않게 Clamp)
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("Player Took Damage: %f, Current HP: %f"), ActualDamage, CurrentHealth);

	// 3. 사망 체크
	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	else
	{
		// ▼ 체력이 남아있다면 피격 몽타주 재생 ▼
		if (HitReactMontage)
		{
			PlayAnimMontage(HitReactMontage);
		}
	}
	return ActualDamage;
}

void AUS_Character::EndDialogue()
{
	bIsTalking = false;
}

void AUS_Character::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("Player Died!"));

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DisableInput(PC);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	else
	{
		// 사망 몽타주가 없을 때만 래그돌
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	}
}
// 체력이 변경될 때 클라이언트에서 호출됨 (UI 업데이트 등에 사용)
void AUS_Character::OnRep_CurrentHealth()
{
	// 예: HUD 업데이트 함수 호출
	// if (DialogueWidget) { ... } 
	UE_LOG(LogTemp, Log, TEXT("Health Updated on Client: %f"), CurrentHealth);
}
