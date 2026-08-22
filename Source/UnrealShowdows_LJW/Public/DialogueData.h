// DialogueData.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "DialogueData.generated.h" // 반드시 마지막 include

USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FName NextNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FGameplayTag ConditionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bInvertCondition = false;
};

USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine="true"))
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueChoice> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FName NextNodeId_Default = NAME_None;
};