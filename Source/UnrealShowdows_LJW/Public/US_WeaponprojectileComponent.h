// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "US_WeaponprojectileComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALSHOWDOWS_LJW_API UUS_WeaponprojectileComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUS_WeaponprojectileComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void Throw();
	
	UFUNCTION(Server,Reliable)
	void Throw_Server();

	UFUNCTION(NetMulticast,Unreliable)
	void Throw_Client();
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Projectile",meta=(AllowPrivateAccess="true"))
	TSubclassOf<class AUS_BaseWeaponProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Input",meta=(AllowPrivateAccess="true"))
	class UInputMappingContext* WeaponMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Input",meta=(AllowPrivateAccess="true"))
	class UInputAction* ThrowAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Projectile",meta=(AllowPrivateAccess="true"))
	UAnimMontage* ThrowAnimation;

	// 투척 쿨다운 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta=(AllowPrivateAccess="true"))
	float ThrowCooldown = 0.8f;

	// 마지막 투척 시각 (서버 기준)
	float LastThrowTime = -999.f;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetProjectileClass(TSubclassOf<class AUS_BaseWeaponProjectile>NewProjectileClass);
		
};
