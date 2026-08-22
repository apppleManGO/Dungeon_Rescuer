// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "US_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALSHOWDOWS_LJW_API UUS_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	public:

	// 메인 퀘스트(모든 NPC 구출)를 완료했는지 여부
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bIsMainQuestCleared = false;

	// "총 3명 중 2명 구했네!"등 다양한 대사 변수
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 SavedRescuedNPCCount = 0;
};
