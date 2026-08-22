// US_NPCDialogueActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueData.h" // FDialogueNode, FDialogueChoice
#include "US_NPCDialogueActor.generated.h"

UCLASS()
class UNREALSHOWDOWS_LJW_API AUS_NPCDialogueActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUS_NPCDialogueActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// NPC가 가진 대화 노드들 (에디터에서 채우기)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	TMap<FName, FDialogueNode> NodesById;

	// 시작 노드 ID (예: "N1")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
	FName StartNodeId = NAME_None;

	// 상호작용 시 호출 (캐릭터에서 라인트레이스 Hit 후 이 함수 호출)
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void StartDialogue(APlayerController* Requestor);
};