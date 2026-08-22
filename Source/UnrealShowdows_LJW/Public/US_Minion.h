// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "US_Minion.generated.h"


USTRUCT(BlueprintType)
struct FDropItemInfo
{
    GENERATED_BODY()

    // 스폰할 픽업 아이템 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    TSubclassOf<class AUS_BasePickup> ItemClass;

    // 드랍 확률 (예: 20.0 = 20%, 80.0 = 80%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DropChance = 0.0f;
};

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_Minion : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AUS_Minion();
    //순찰 지점을 새로 설정하는 함수
    UFUNCTION(BlueprintCallable, Category = "Minion AI")
    void SetNextPatrolLocation();
    
    //플레이어를 추격하는 함수
    UFUNCTION(BlueprintCallable, Category = "Minion AI")
    void Chase(APawn* Pawn);
    void AttemptAttack(AActor* InTarget);

    virtual void PostInitializeComponents() override;

    FORCEINLINE UPawnSensingComponent* GetPawnSense() const { return PawnSense; }

    FORCEINLINE USphereComponent* GetCollision() const {return Collision;}

    //시야에 Pawn이 들어왔을 때 호출되는 함수
    UFUNCTION()
    void OnPawnDetected(APawn* Pawn);

    //지정된 위치로 이동하는 함수
    UFUNCTION(BlueprintCallable, Category = "Minion AI")
    void GoToLocation(const FVector& Location);

public: 
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    //순찰 기본 속도
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Minion AI")
    float patrolSpeed = 150.f;

    //추격 속도
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Minion AI")
    float ChaseSpeed = 250;

    //순찰 반경
    UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Minion AI")
    float PatrolRadius = 50000.f;
    
    
private:
    //시야, 청각 감지를 위한 PawnSensingComponent
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Minion Perception", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UPawnSensingComponent> PawnSense;

    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Minion Perception", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USphereComponent> Collision;

    //현재 이동할 순찰 위치
    UPROPERTY()
    FVector PatrolLocation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minion AI",meta = (AllowPrivateAccess = "true"))
    float AlertRadius = 6000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health",meta = (AllowPrivateAccess = "true"))
    float Health = 5.f;

    // ▼ 2. 기존 배열(SpawnablePickups)을 지우고 새로운 구조체 배열로 교체합니다 ▼
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
    TArray<FDropItemInfo> DropItems;
    // ▲ 교체 끝 ▲

    //공격 쿨타임
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minion AI",meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = -999.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minion AI",meta = (AllowPrivateAccess = "true"))
    float AttackCooldown = 2.5f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Minion AI",meta = (AllowPrivateAccess = "true"))
    int32 AttackDamage=1;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    //소리들었을때 호출 하는함수
    UFUNCTION()
    void OnHearNoise(APawn* PawnInstigator,const FVector& Location, float Volume);

    UFUNCTION()
    void OnDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange = 150.f; // 공격 가능 거리

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ChaseRadius = 1000.f; // 추격을 포기하는 최대 거리
    
    UPROPERTY()
    AActor* TargetPlayer; // 현재 추격/공격 중인 대상
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 900.f; // 뒤로 밀려나는 힘

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StunDuration = 0.2f; // 스턴 지속 시간 (초)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsStunned = false; // 현재 스턴 상태인지 체크

    FTimerHandle StunTimerHandle; // 스턴 해제 타이머

    UFUNCTION()
    void RecoverFromStun(); // 스턴 해제 함수
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitReactMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage;

    // 모든 슬롯의 동적 머티리얼을 저장할 배열
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

    FTimerHandle HitFlashTimerHandle;
    void ResetMaterialColor();

    UPROPERTY(EditAnywhere, Category = "Appearance")
    FLinearColor HitFlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, Category = "Appearance")
    float FlashDuration = 0.15f;

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayHitFlash();
};