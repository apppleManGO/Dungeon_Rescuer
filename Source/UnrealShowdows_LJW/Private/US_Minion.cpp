// Fill out your copyright notice in the Description page of Project Settings.
#include "US_Minion.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "US_Character.h"
#include "US_GameMode.h"
#include "US_BasePickup.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AUS_Minion::AUS_Minion()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 회전 설정 (움직이는 방향으로만 회전)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// AI 자동 소유 - 월드에 있거나 스폰되었을때 ai할당
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass=AAIController::StaticClass();

	// Pawn 감지 컴포넌트 생성 및 설정
	PawnSense = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSense"));
	//시야,청각 감지 업데이트 시간
	PawnSense->SensingInterval=.8f;
	//시야각 설 , 실제 총 시야각은 90
	PawnSense->SetPeripheralVisionAngle(45.f);
	//시야 거리 설정
	PawnSense->SightRadius=1500.f;
	//청각 감지 설정
	PawnSense->HearingThreshold=500.f;
	//시야에 방해물이 없는 상태에서의 청각 감지 거리
	PawnSense->LOSHearingThreshold=1000.f;

	//// 충돌 컴포넌트 생성
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(100);
	Collision->SetupAttachment(RootComponent);

	// 캡슐 컴포넌트 초기화
	GetCapsuleComponent()->InitCapsuleSize(60.f,96.f);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	// 메시 위치 조정 및 메시 로드
	GetMesh()->SetRelativeLocation(FVector(0.0f,0.0f,-91.0f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SkeletalMeshAsset(TEXT("/Game/KayKit/Skeletons/skeleton_minion"));
	if(SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}

	// 이동 컴포넌트 설정
	//이동 방향으로 자동 회전 설정
	GetCharacterMovement()->bOrientRotationToMovement=true;
	//회전 속도 설정
	GetCharacterMovement()->RotationRate=FRotator(0.0f,500.f,0.f);
	GetCharacterMovement()->MaxWalkSpeed=200.f;
	GetCharacterMovement()->MinAnalogWalkSpeed =20.f;
	////걷기 상태에서의 감속(브레이크) 속도
	GetCharacterMovement()->BrakingDecelerationWalking=2000.f;

	//스폰 아이템 가져오기 및 확률 설정
	static ConstructorHelpers::FClassFinder<AUS_BasePickup> GoldCoinAsset(TEXT("/Game/BP/BP_GoldCoinPickUp"));
	if (GoldCoinAsset.Succeeded())
	{
		FDropItemInfo GoldInfo;
		GoldInfo.ItemClass = GoldCoinAsset.Class;
		GoldInfo.DropChance = 20.0f; // 골드 코인 20%
		DropItems.Add(GoldInfo);
	}

	static ConstructorHelpers::FClassFinder<AUS_BasePickup> SilverCoinAsset(TEXT("/Game/BP/BP_SilverCoinPickUp_Child")); 
	if (SilverCoinAsset.Succeeded())
	{
		FDropItemInfo SilverInfo;
		SilverInfo.ItemClass = SilverCoinAsset.Class;
		SilverInfo.DropChance = 80.0f; // 실버 코인 80%
		DropItems.Add(SilverInfo);
	}
}


// Called when the game starts or when spawned
void AUS_Minion::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (AUS_GameMode* GameMode = Cast<AUS_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->RegisterMinion(this);
		}
	}
	// 메쉬가 가진 모든 머티리얼 슬롯을 순회하며 동적 인스턴스 생성
	int32 NumMaterials = GetMesh()->GetNumMaterials();
	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* BaseMat = GetMesh()->GetMaterial(i);
		if (BaseMat)
		{
			// 각 슬롯에 대해 동적 머티리얼 생성 후 배열에 저장
			UMaterialInstanceDynamic* DynMat = GetMesh()->CreateDynamicMaterialInstance(i, BaseMat);
			if (DynMat)
			{
				DynamicMaterials.Add(DynMat);
			}
		}
	}
	// 시작 시 순찰 위치 지정
	SetNextPatrolLocation();
}

void AUS_Minion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (AUS_GameMode* GameMode = Cast<AUS_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->UnregisterMinion(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}
//소리들었을때 호출 하는함수
void AUS_Minion::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	GoToLocation(Location);
}

void AUS_Minion::OnDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    Health -= Damage;
	// 서버에서만 실행되는 이 함수가 모든 클라이언트에게 "색 바꿔!"라고 명령합니다.
	MulticastPlayHitFlash();

	// 타이머 설정 (한 번만 호출하면 됨)
	GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AUS_Minion::ResetMaterialColor, FlashDuration, false);
    // 1. 살아있을 때 (연속으로 맞더라도 매번 넉백 적용!)
    if (Health > 0)
    {
        // 스턴 상태 갱신 (이미 true여도 덮어씌움)
        bIsStunned = true;
          
        // 하던 행동(이동) 멈추기
        if (GetController())
        {
            GetController()->StopMovement();
			ActiveChaseTarget = nullptr;
        }

        // 넉백 방향 계산 (나를 때린 놈의 반대 방향으로)
        FVector KnockbackDir = -GetActorForwardVector();
        if (DamageCauser)
        {
            KnockbackDir = GetActorLocation() - DamageCauser->GetActorLocation();
            KnockbackDir.Z = 0.f; // 공중으로 너무 날아가지 않게 수평만 맞춤
            KnockbackDir.Normalize();
        }

        // 살짝 위로 뜨게 Z축 추가 후 날려버리기
        KnockbackDir.Z = 0.5f; 
        
        // LaunchCharacter의 2, 3번째 인자(true, true)가 기존 이동 속도를 무시하고 
        // 넉백 힘을 새로 덮어씌우기 때문에 연속으로 맞아도 예쁘게 날아갑니다.
        LaunchCharacter(KnockbackDir * KnockbackForce, true, true);
    	if (HitReactMontage)
    	{
    		PlayAnimMontage(HitReactMontage);
    	}
        // 일정 시간(StunDuration) 뒤에 스턴 해제 함수 호출
        // (연속으로 맞으면 타이머가 계속 0초로 리셋되어서 계속 스턴 상태가 유지됨!)
        GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AUS_Minion::RecoverFromStun, StunDuration, false);
        
        return; // 아직 살아있으므로 여기서 함수 종료
    } 

    // 2. 죽었을 때 (체력이 0 이하)
    // 죽었을 때도 스턴을 걸어서 안 움직이게 확실히 못 박아둡니다.
    bIsStunned = true;
	if (GetController())
	{
		GetController()->StopMovement();
		ActiveChaseTarget = nullptr;
		GetController()->UnPossess();
	}
	GetCharacterMovement()->DisableMovement();
	//사망 애니메이션
	if (DeathMontage)
	{
		// 애니메이션을 재생하고 나중에 삭제(Destroy)되도록 설정하는 것이 좋습니다.
		// 바로 Destroy() 해버리면 애니메이션을 볼 틈도 없이 사라지기 때문입니다.
		PlayAnimMontage(DeathMontage);
        
		// 캡슐 콜리전을 꺼서 시체 위로 지나갈 수 있게 만듦
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
    // ▼ 확률 기반 아이템 드랍 로직 ▼
    if (DropItems.Num() > 0)
    {
       // 0.0 ~ 100.0 사이의 랜덤한 소수(퍼센트) 뽑기
       float RandomValue = FMath::FRandRange(0.0f, 100.0f);
       float CumulativeChance = 0.0f; // 확률 누적 변수

       for (const FDropItemInfo& DropInfo : DropItems)
       {
          CumulativeChance += DropInfo.DropChance;

          // 주사위 값이 누적 확률 안에 들어오면 해당 아이템 스폰
          if (RandomValue <= CumulativeChance)
          {
             if (DropInfo.ItemClass) // 클래스가 비어있지 않은지 안전 검사
             {
                FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
                AUS_BasePickup* SpawnedItem = GetWorld()->SpawnActor<AUS_BasePickup>(DropInfo.ItemClass, SpawnLocation, GetActorRotation());
                
                if (SpawnedItem)
                {
                   UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(SpawnedItem->GetRootComponent());
                   
                   if (PrimitiveRoot && PrimitiveRoot->IsSimulatingPhysics())
                   {
                      float RandomX = FMath::RandRange(-300.0f, 300.0f);
                      float RandomY = FMath::RandRange(-300.0f, 300.0f);
                      float UpwardForce = FMath::RandRange(400.0f, 600.0f);
                      
                      FVector ImpulseDirection(RandomX, RandomY, UpwardForce);
                      PrimitiveRoot->AddImpulse(ImpulseDirection, NAME_None, true); 
                   }
                }
             }  
             break; // 아이템을 하나 떨궜으니 반복문 종료
          }
       }
    }

	// 사망 연출을 볼 시간을 준 뒤 소멸한다.
	SetLifeSpan(5.0f);
}
void AUS_Minion::RecoverFromStun()
{
	bIsStunned = false;
}

void AUS_Minion::ResetMaterialColor()
{
	for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
	{
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("TintColor"), FLinearColor::White);
		}
	}
}

void AUS_Minion::MulticastPlayHitFlash_Implementation()
{
	// 모든 동적 머티리얼 순회하며 빨간색으로 변경
	for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
	{
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("TintColor"), HitFlashColor);
		}
	}

	// 각자의 컴퓨터에서 타이머를 돌려 원래 색으로 복구
	GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AUS_Minion::ResetMaterialColor, FlashDuration, false);
}

// 컴포넌트 초기화 후 호출
void AUS_Minion::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(GetLocalRole() != ROLE_Authority){return;}
	

	//데미지받으면 호출
	OnTakeAnyDamage.AddDynamic(this,&AUS_Minion::OnDamage);
	
	// 시야/청각 이벤트 바인딩
	GetPawnSense()->OnSeePawn.AddDynamic(this,&AUS_Minion::OnPawnDetected);
	GetPawnSense()->OnHearNoise.AddDynamic(this,&AUS_Minion::OnHearNoise);
}


// Called every frame
void AUS_Minion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() != ROLE_Authority) return;
	if (bIsStunned) return;
	if(Health<=0)return;
	// 1. 타겟 플레이어 유효성 검사 (시야 감지 등을 통해 할당된 타겟이 있는지)
	// 현재 코드에서는 OnPawnDetected에서 타겟을 특정 변수에 저장하는 로직이 필요합니다.
	// (헤더에 AActor* TargetPlayer; 추가 권장)

	if (TargetPlayer)
	{
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

		// 2. 공격 범위 내에 플레이어가 있는가? (예: 150.0f)
		if (DistanceToPlayer <= AttackRange)
		{
			// 3. 공격 시도 (공격 딜레이 체크 포함)
			AttemptAttack(TargetPlayer);
            
			// 공격 중에는 이동 중지 (선택 사항)
			if (GetController())
			{
				GetController()->StopMovement();
				ActiveChaseTarget = nullptr;
			}
		}
		// 4. 공격 범위 밖이지만 시야/추격 범위 내에 있는가?
		else if (DistanceToPlayer <= ChaseRadius)
		{
			// 계속 추격
			Chase(Cast<APawn>(TargetPlayer));
		}
		else
		{
			// 5. 플레이어가 범위를 완전히 벗어남 -> 타겟 상실 및 순찰 복귀
			TargetPlayer = nullptr;
			ActiveChaseTarget = nullptr;
			SetNextPatrolLocation();
		}
	}
	else
	{
		// 6. 타겟이 없을 때: 목표 지점에 도달하면 다음 순찰지로 이동
		if ((GetActorLocation() - PatrolLocation).Size() < 200.f)
		{
			SetNextPatrolLocation();
		}
	}
}

// Called to bind functionality to input
void AUS_Minion::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// 시야에 Pawn이 들어왔을 때 호출
void AUS_Minion::OnPawnDetected(APawn* Pawn)
{
	// 플레이어 캐릭터만 인식
	if(!Pawn->IsA<AUS_Character>())return;
	TargetPlayer = Pawn;

	// 추격 속도가 아닐 때만 추격 시작
	if(GetCharacterMovement()->MaxWalkSpeed!=ChaseSpeed)
	{
		Chase(Pawn);
	}
}



void AUS_Minion::GoToLocation(const FVector& Location)
{
	ActiveChaseTarget = nullptr;
	PatrolLocation = Location;
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(),PatrolLocation);
}

void AUS_Minion::SetNextPatrolLocation()
{
	if(!HasAuthority()) return;

	GetCharacterMovement()->MaxWalkSpeed = patrolSpeed;
    
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation RandomLocation;
		// 실제 랜덤 지점을 찾고 그 결과를 RandomLocation에 저장
		if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), PatrolRadius, RandomLocation))
		{
			PatrolLocation = RandomLocation.Location; // 실제 찾은 위치로 업데이트
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), PatrolLocation);
		}
	}
}
// 추격 로직
void AUS_Minion::Chase(APawn* Pawn)
{
	if (GetLocalRole() != ROLE_Authority || !Pawn) return;
	GetCharacterMovement()->MaxWalkSpeed=ChaseSpeed;

	if (ActiveChaseTarget != Pawn)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->MoveToActor(Pawn, AttackRange * 0.9f);
			ActiveChaseTarget = Pawn;
		}
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAlertTime >= AlertCooldown)
	{
		LastAlertTime = Now;
		if (AUS_GameMode* GameMode = Cast<AUS_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AlertMinions(this, Pawn->GetActorLocation(), AlertRadius);
		}
	}
}
void AUS_Minion::AttemptAttack(AActor* InTarget)
{
	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return;

	LastAttackTime = Now;
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
	// 데미지 입히기
	UGameplayStatics::ApplyDamage(
		InTarget, 
		AttackDamage, 
		GetController(), 
		this, 
		UDamageType::StaticClass()
	);

}
