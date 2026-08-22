// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "US_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void UpdateUIForPhase(EGamePhase NewPhase);
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void OnPartyKeyChanged(bool bHasKey);
	
protected:
	// Ready UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> ReadyWidgetClass;

	UPROPERTY()
	UUserWidget* ReadyWidget;

	// Game UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> GameWidgetClass;

	UPROPERTY()
	UUserWidget* GameWidget;
	virtual void OnRep_PlayerState() override;

	UFUNCTION()
	void HandleXpChange(int32 NewXp);

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

	UFUNCTION()
	void HandleReady(bool bIsReady);
protected:
	// 에디터에서 WBP_Fade 클래스를 할당할 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> FadeWidgetClass;

	// 생성된 위젯 인스턴스를 보관
	UPROPERTY()
	UUserWidget* FadeWidgetInstance;

public:
	// 클라이언트에서 페이드 아웃을 시작하는 RPC
	UFUNCTION(Client, Reliable)
	void Client_StartFadeOut();
	
};
