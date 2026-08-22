// DialogueDataAsset.cpp
#include "DialogueDataAsset.h"

void UDialogueDataAsset::BuildSampleForGuildMasterAndRescuer()
{
	NodesById.Empty();

	// ========== 길드장 (Start: N1) ==========
	{
		FDialogueNode N1;
		N1.Line = FText::FromString(TEXT("안녕하신가 모험가."));
		N1.NextNodeId_Default = FName(TEXT("N2"));
		NodesById.Add(FName(TEXT("N1")), N1);

		FDialogueNode N2;
		N2.Line = FText::FromString(TEXT("퀘스트를 받겠는가?"));
		{
			FDialogueChoice CYes;
			CYes.Label = FText::FromString(TEXT("예"));
			CYes.NextNodeId = FName(TEXT("N3_Yes"));
			N2.Choices.Add(CYes);

			FDialogueChoice CNo;
			CNo.Label = FText::FromString(TEXT("아니오"));
			CNo.NextNodeId = FName(TEXT("N3_No"));
			N2.Choices.Add(CNo);
		}
		NodesById.Add(FName(TEXT("N2")), N2);

		FDialogueNode N3_No;
		N3_No.Line = FText::FromString(TEXT("준비가 되면 다시 오게나."));
		N3_No.NextNodeId_Default = NAME_None;
		NodesById.Add(FName(TEXT("N3_No")), N3_No);

		FDialogueNode N3_Yes;
		N3_Yes.Line = FText::FromString(TEXT("파티가 준비되었나?"));
		{
			FDialogueChoice CCheck;
			CCheck.Label = FText::FromString(TEXT("확인"));
			CCheck.NextNodeId = FName(TEXT("N4_Check"));
			N3_Yes.Choices.Add(CCheck);
		}
		NodesById.Add(FName(TEXT("N3_Yes")), N3_Yes);

		FDialogueNode N4_Check;
		N4_Check.Line = FText::FromString(TEXT(""));
		{
			FDialogueChoice CReady;
			CReady.Label = FText::FromString(TEXT("계속"));
			CReady.NextNodeId = FName(TEXT("N4_Ready"));
			CReady.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Party.Ready")));
			N4_Check.Choices.Add(CReady);

			FDialogueChoice CNotReady;
			CNotReady.Label = FText::FromString(TEXT("계속"));
			CNotReady.NextNodeId = FName(TEXT("N4_NotReady"));
			CNotReady.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Party.Ready")));
			CNotReady.bInvertCondition = true;
			N4_Check.Choices.Add(CNotReady);
		}
		NodesById.Add(FName(TEXT("N4_Check")), N4_Check);

		FDialogueNode N4_NotReady;
		N4_NotReady.Line = FText::FromString(TEXT("아직 준비가 안된모양이군."));
		N4_NotReady.NextNodeId_Default = NAME_None;
		NodesById.Add(FName(TEXT("N4_NotReady")), N4_NotReady);

		FDialogueNode N4_Ready;
		N4_Ready.Line = FText::FromString(TEXT("그럼 던전으로 보내주겠네."));
		N4_Ready.NextNodeId_Default = NAME_None;
		NodesById.Add(FName(TEXT("N4_Ready")), N4_Ready);
	}

	// ========== 구조자 (Start: R1) ==========
	{
		FDialogueNode R1;
		R1.Line = FText::FromString(TEXT("구해주러 왔구나!"));
		{
			FDialogueChoice CConfirm;
			CConfirm.Label = FText::FromString(TEXT("확인"));
			CConfirm.NextNodeId = FName(TEXT("R2_Check"));
			R1.Choices.Add(CConfirm);
		}
		NodesById.Add(FName(TEXT("R1")), R1);

		FDialogueNode R2_Check;
		R2_Check.Line = FText::FromString(TEXT(""));
		{
			FDialogueChoice CRescued;
			CRescued.Label = FText::FromString(TEXT("계속"));
			CRescued.NextNodeId = FName(TEXT("R2_Rescued"));
			CRescued.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Rescue.Done")));
			R2_Check.Choices.Add(CRescued);

			FDialogueChoice CNotRescued;
			CNotRescued.Label = FText::FromString(TEXT("계속"));
			CNotRescued.NextNodeId = FName(TEXT("R2_NotRescued"));
			CNotRescued.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Rescue.Done")));
			CNotRescued.bInvertCondition = true;
			R2_Check.Choices.Add(CNotRescued);
		}
		NodesById.Add(FName(TEXT("R2_Check")), R2_Check);

		FDialogueNode R2_Rescued;
		R2_Rescued.Line = FText::FromString(TEXT("도와줘서 고마워!"));
		R2_Rescued.NextNodeId_Default = NAME_None;
		NodesById.Add(FName(TEXT("R2_Rescued")), R2_Rescued);

		FDialogueNode R2_NotRescued;
		R2_NotRescued.Line = FText::FromString(TEXT("아프다고 빨리 탈출시켜줘!"));
		R2_NotRescued.NextNodeId_Default = NAME_None;
		NodesById.Add(FName(TEXT("R2_NotRescued")), R2_NotRescued);
	}

	StartNodeId = FName(TEXT("N1")); // 기본 시작점(길드장)
}