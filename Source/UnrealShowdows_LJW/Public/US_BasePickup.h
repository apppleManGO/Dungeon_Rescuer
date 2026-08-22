// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "US_BasePickup.generated.h"

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_BasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUS_BasePickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	//BlueprintNativeEvent->블루프린트에서 오버라이드 돼 있으면 → 블루프린트 구현 실행 ->없으면 → Pickup_Implementation() 실행
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category="Pickup",meta=(DisplayName="Pickup"))
	void Pickup(class AUS_Character* OwningCharacter);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Components",meta=(AllowPrivateAccess="true"))
	TObjectPtr<class USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UStaticMeshComponent> Mesh;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FORCEINLINE class USphereComponent* GetSphereCollision() const { return SphereCollision; }
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

};
