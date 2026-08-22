// US_NPCDialogueActor.cpp
#include "US_NPCDialogueActor.h"
#include "GameFramework/PlayerController.h"

AUS_NPCDialogueActor::AUS_NPCDialogueActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AUS_NPCDialogueActor::BeginPlay()
{
	Super::BeginPlay();
}

void AUS_NPCDialogueActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUS_NPCDialogueActor::StartDialogue(APlayerController* Requestor)
{
	if (!Requestor)
	{
		return;
	}
	if (NodesById.Num() == 0 || StartNodeId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC '%s' has empty dialogue or no StartNodeId"), *GetName());
		return;
	}

	// 플레이어 컨트롤러에 Blueprint 이벤트 "OpenDialogue"가 있다고 가정하고 호출
	// 시그니처 예시 (BP에서 구현):
	//   OpenDialogue(NodesById: TMap<Name, FDialogueNode>, StartNodeId: Name)
	static const FName OpenDialogueFuncName(TEXT("OpenDialogue"));
	if (UFunction* Func = Requestor->FindFunction(OpenDialogueFuncName))
	{
		struct FOpenDialogueParams
		{
			TMap<FName, FDialogueNode> NodesById;
			FName StartNodeId;
		};

		FOpenDialogueParams Params;
		Params.NodesById = NodesById;
		Params.StartNodeId = StartNodeId;

		Requestor->ProcessEvent(Func, &Params);
	}
	else
	{
		// 대체: 함수가 없으면 로그로 알려주기
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is missing BP event 'OpenDialogue'. Implement it to show the widget."));
	}
}