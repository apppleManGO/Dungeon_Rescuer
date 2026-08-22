// DialogueDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueData.h"

#include "DialogueDataAsset.generated.h" // 반드시 마지막 include

UCLASS(BlueprintType)
class UNREALSHOWDOWS_LJW_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TMap<FName, FDialogueNode> NodesById;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FName StartNodeId = NAME_None;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="Dialogue")
	void BuildSampleForGuildMasterAndRescuer();
};