// Fill out your copyright notice in the Description page of Project Settings.


#include "US_PlayerController.h"
#include "US_GameInstance.h"
#include "US_GameState.h"
#include "US_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"

void AUS_PlayerController::UpdateUIForPhase(EGamePhase NewPhase)
{
    if (!IsLocalController())
    {
        //UE_LOG(LogTemp, Warning, TEXT("UpdateUIForPhase skipped: Not a local controller"));
        return; 
    }

    //UE_LOG(LogTemp, Warning, TEXT("UpdateUIForPhase called. Phase = %d"), (int32)NewPhase);

    if (NewPhase == EGamePhase::Lobby)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Phase = Lobby"));

        if (ReadyWidgetClass && !ReadyWidget)
        {
            //U//E_LOG(LogTemp, Warning, TEXT("Creating ReadyWidget..."));

            ReadyWidget = CreateWidget<UUserWidget>(this, ReadyWidgetClass);
            if (ReadyWidget)
            {
                if (UUS_GameInstance* MyGI = Cast<UUS_GameInstance>(GetGameInstance()))
                {
                    //퀘스트를 완수하지 않았으면 준비하기 띄우기,완료하면 안띄우기
                    if(!MyGI->bIsMainQuestCleared==true)
                    {
                         ReadyWidget->AddToViewport();
                         //UE_LOG(LogTemp, Warning, TEXT("ReadyWidget created and added to viewport"));
                    }
                    
                }
                
            }
            else
            {
                //UE_LOG(LogTemp, Error, TEXT("CreateWidget returned nullptr (ReadyWidgetClass invalid?)"));
            }
        }

        if (ReadyWidget)
        {
            ReadyWidget->SetVisibility(ESlateVisibility::Visible);
            //UE_LOG(LogTemp, Warning, TEXT("ReadyWidget set to Visible"));
        }

        if (GameWidget)
        {
            GameWidget->SetVisibility(ESlateVisibility::Collapsed);
            //UE_LOG(LogTemp, Warning, TEXT("GameWidget set to Collapsed"));
        }
    }
    else if (NewPhase == EGamePhase::Playing)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Phase = Playing"));

        if (GameWidgetClass && !GameWidget)
        {
            //UE_LOG(LogTemp, Warning, TEXT("Creating GameWidget..."));

            GameWidget = CreateWidget<UUserWidget>(this, GameWidgetClass);
            if (GameWidget)
            {
                GameWidget->AddToViewport();
                //UE_LOG(LogTemp, Warning, TEXT("GameWidget created and added to viewport"));
            }
            else
            {
                //UE_LOG(LogTemp, Error, TEXT("CreateWidget returned nullptr (GameWidgetClass invalid?)"));
            }
        }

        if (GameWidget)
        {
            GameWidget->SetVisibility(ESlateVisibility::Visible);
            //UE_LOG(LogTemp, Warning, TEXT("GameWidget set to Visible"));
        }

        if (ReadyWidget)
        {
            ReadyWidget->SetVisibility(ESlateVisibility::Collapsed);
            //UE_LOG(LogTemp, Warning, TEXT("ReadyWidget set to Collapsed"));
        }
    }
}

void AUS_PlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (AUS_PlayerState* PS = GetPlayerState<AUS_PlayerState>())
	{
		// PlayerState 이벤트 바인딩
		PS->OnXpChange.AddDynamic(this, &AUS_PlayerController::HandleXpChange);
		PS->OnCharacterLevelUp.AddDynamic(this, &AUS_PlayerController::HandleLevelUp);
		PS->OnReady.AddDynamic(this, &AUS_PlayerController::HandleReady);

		//UE_LOG(LogTemp, Warning, TEXT("PlayerController: Bound to PlayerState events for %s"), *PS->GetPlayerName());
	}
}
void AUS_PlayerController::HandleXpChange(int32 NewXp)
{
	//UE_LOG(LogTemp, Warning, TEXT("XP Changed: %d"), NewXp);
}

void AUS_PlayerController::HandleLevelUp(int32 NewLevel)
{
	//UE_LOG(LogTemp, Warning, TEXT("Level Up! New Level: %d"), NewLevel);
}

void AUS_PlayerController::HandleReady(bool bIsReady)
{
	//UE_LOG(LogTemp, Warning, TEXT("Ready State: %s"), bIsReady ? TEXT("READY") : TEXT("NOT READY"));
}

void AUS_PlayerController::Client_StartFadeOut_Implementation()
{
    if (!FadeWidgetClass) return;

    // 1. 위젯이 이미 있다면 생성하지 않음
    if (!FadeWidgetInstance)
    {
        FadeWidgetInstance = CreateWidget<UUserWidget>(this, FadeWidgetClass);
    }

    if (FadeWidgetInstance)
    {
        // 2. 화면에 표시
        FadeWidgetInstance->AddToViewport();

        // 3. 블루프린트에 구현된 'FadeIn' (또는 FadeOut) 이벤트 호출
        // 블루프린트에서 만든 함수 이름을 그대로 적어줍니다.
        UFunction* FadeFunc = FadeWidgetInstance->FindFunction(FName("FadeIn"));
        if (FadeFunc)
        {
            FadeWidgetInstance->ProcessEvent(FadeFunc, nullptr);
        }
    }
}
void AUS_PlayerController::OnPartyKeyChanged(bool bHasKey)
{
    // 여기서 “열쇠 획득!” 위젯 표시/아이콘 On/Off 등 처리
    // ex) HUD 위젯에 SetKeyIconVisible(bHasKey);
}

void AUS_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (AUS_GameState* GS = GetWorld()->GetGameState<AUS_GameState>())
    {
        //UE_LOG(LogTemp, Warning, TEXT("PlayerController BeginPlay → Sync UI with CurrentPhase = %d"), (int32)GS->CurrentPhase);
        UpdateUIForPhase(GS->CurrentPhase);
    }
}
