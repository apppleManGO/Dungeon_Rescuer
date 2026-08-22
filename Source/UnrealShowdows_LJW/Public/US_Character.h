// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "US_Character.generated.h"

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUS_Character();
	
private:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UInputMappingContext>DefaultMappingContext;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UInputAction>MoveAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UInputAction>LookAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UInputAction>SprintAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UInputAction>InteractAction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Character Data",meta=(AllowPrivateAccess=true))
	class UDataTable* CharacterDataTable;

	struct FUS_CharacterStats* CharacterStats;

	UPROPERTY()
	AActor* InteractableActor;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Stealth",meta=(AllowPrivateAccess=true))
	TObjectPtr<UPawnNoiseEmitterComponent> NoiseEmitter;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UUS_WeaponprojectileComponent>Weapon;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Character Data",meta=(AllowPrivateAccess=true))
	UDataTable* CharacterSkinDataTable;

	struct FUS_CharacterSkins* CharacterSkin;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 상호작용 탐색 주기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckInterval = 0.15f;

	float LastInteractionCheckTime = 0.f;
	bool bInteractPromptVisible = false;

	void Move(const struct FInputActionValue& Value);
	void Look(const  FInputActionValue& Value);
	void SprintStart(const  FInputActionValue& Value);
	void SprintEnd(const  FInputActionValue& Value);
	void Interact(const  FInputActionValue& Value);
	UFUNCTION(Server,Reliable)
	void SprintStart_Server();
	UFUNCTION(Server,Reliable)
	void SprintEnd_Server();
	UFUNCTION(Server,Reliable)
	void Interact_Server();
	UFUNCTION(NetMulticast,Reliable)
	void SprintStart_Client();
	UFUNCTION(NetMulticast,Reliable)
	void SprintEnd_Client();
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,ReplicatedUsing="OnRep_SkinChaged",Category="Skin")
	//int32 SkinIndex=-1;

	//UFUNCTION()
	//void OnRep_SkinChaged();
	// UFUNCTION(Server,Reliable)
	// void SetSkinIndex_Server(int32 Value);
	UFUNCTION()
	void UpdateCharacterSkin(int32 InSkinIndex);

	// [추가] 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_CurrentHealth", Category = "Health")
	float CurrentHealth;

	// [추가] 최대 체력 (기본값 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	// 사망 처리 중복 방지 (서버 권위)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Health")
	bool bIsDead = false;
	
	UFUNCTION()
	void OnRep_CurrentHealth();
	// [추가] 사망 처리 함수
	virtual void Die();
	// [추가] 현재 대화 중인지 체크하는 상태 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bIsTalking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* DeathMontage;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 'FORCEINLINE'은 컴파일러에게 이 함수를 호출하는 곳에 코드를 직접 삽입(인라인화)하도록 요청합니다.
	// 함수 호출 오버헤드를 줄여 성능을 최적화할 수 있습니다.
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const{return FollowCamera;}
	FORCEINLINE FUS_CharacterStats* GetCharacterStats() const {return CharacterStats;}
	FORCEINLINE UUS_WeaponprojectileComponent* GetWeapon() const {return Weapon;}
	FORCEINLINE FUS_CharacterSkins* GetCharacterSkins() const {return CharacterSkin;}
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Camera",meta=(AllowPrivateAccess=true))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Camera",meta=(AllowPrivateAccess=true))
	TObjectPtr<class UCameraComponent> FollowCamera;

	void UpdateCharacterStats(int32 CharacterLevel);

	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// UI 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> DialogueWidgetClass;

	UUserWidget* DialogueWidget;

	void ShowDialogueUI();
	void HideDialogueUI();
	void ApplySkin(int32 InSkinIndex);
	virtual void PossessedBy(AController* NewController) override;
	UFUNCTION(Client, Reliable)
	void ShowDialogueUI_Client();

	UFUNCTION(Client, Reliable)
	void HideDialogueUI_Client();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanTalk = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDialogueDone = false;

	virtual void OnRep_PlayerState() override;
	UFUNCTION(Server, Reliable)
	void SetSkinIndex_Server(int32 NewIndex);
	

	// [추가] 데미지 받는 함수 오버라이드
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// [추가] 체력 반환 (UI 등에서 사용)
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();
	
};
