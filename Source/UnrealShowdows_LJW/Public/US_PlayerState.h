// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "US_PlayerState.generated.h"

/**
 * 
 */
//델리게이트 선언 (변수명선언,인자값선언,인자변수명선언)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnXpChange, int32, NewXp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLevelUp, int32, NewLevelXp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReady, bool, Ready);

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_PlayerState : public APlayerState
{
	GENERATED_BODY()
protected:
	//ReplicatedUsing 네트워크에서 동기화될 때,값이 복제,변경 될 때 자동으로 이 함수를 호출한다.함수(변경값);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, ReplicatedUsing = "OnRep_Xp",Category = "Experience")
	int Xp =0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, ReplicatedUsing = "OnRep_CharacterLevelUp",Category = "Experience")
	int CharacterLevel =1;
	
	
	//델리게이트 함수 선언
	UFUNCTION()
	void OnRep_CharacterLevelUp(int32 OldValue) const;
	
	UFUNCTION()
	void OnRep_Xp(int32 OldValue) const;

	UFUNCTION()
	void OnRep_IsReady(bool OldValue) const;
public:
	UFUNCTION(BlueprintCallable,Category = "Experience")
	void AddXp(int32 Value);

	//변수들이 네트워크를 통해 조건에 따라 복제될지 정의 하는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetSkinIndex(int32 NewIndex);

	// 델리게이트 타입의 인스턴스 뱀버 변수 선언
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnXpChange OnXpChange;
	
	// 델리게이트 타입의 인스턴스 뱀버 변수 선언
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterLevelUp OnCharacterLevelUp;
	
	// 델리게이트 타입의 인스턴스 뱀버 변수 선언
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnReady OnReady;
	
	//플레이어 게임준비
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,ReplicatedUsing="OnRep_IsReady",Category="Gameplay")
	bool bIsReady = false;
	
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void ServerSetReady(bool bReady);

	//플레이어 스킨 색 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,
	ReplicatedUsing = OnRep_SkinIndex,
	Category = "Appearance")
	int32 SkinIndex = 0;

	UFUNCTION()
	void OnRep_SkinIndex(int32 OldValue);

};
