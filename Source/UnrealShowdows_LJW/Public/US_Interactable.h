#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "US_Interactable.generated.h"
//UINTERFACE - 언리얼 헤더 툴에게 인터페이스를 구현할라고 알려줌
//minimalapi 경량 API, 외부 모듈에 최소한으로 노출
UINTERFACE(minimalapi,Blueprintable)
//UUS_Interactable 언리얼 리플렉션, 블루프린트용 클래스 (실제 로직 없음)
class UUS_Interactable : public UInterface 

{
	GENERATED_BODY()
};
// 실제 상속 받아 구현할 C++ 인터페이스 정의부

class UNREALSHOWDOWS_LJW_API IUS_Interactable
{
	GENERATED_BODY()
public:
	// 블루프린트와 C++ 양쪽에서 구현 가능 (BlueprintNativeEvent)
	// 상호작용 실행 함수 (예: 아이템 줍기, 문 열기 등)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,Category="Interaction", meta=(DisplayName="Interact"))
	void Interact(class AUS_Character* CharacterInstigator);

	// 상호작용 가능한지 여부를 반환하는 함수 (예: 조건 충족 여부 체크)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent,Category="Interaction", meta=(DisplayName="can Interact"))
	bool CanInteract(AUS_Character* CharacterInstigator) const;
};
