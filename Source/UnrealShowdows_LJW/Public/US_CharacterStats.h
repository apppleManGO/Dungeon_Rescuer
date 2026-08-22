#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "US_CharacterStats.generated.h"

USTRUCT(BlueprintType)
struct UNREALSHOWDOWS_LJW_API FUS_CharacterStats : public  FTableRowBase//언리얼엔진에서 데이터 테이블을 생성할수있도록 확장
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed =200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed =400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultipier=1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NextLevelXp=10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StealthMultiplier=1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth=20.f;
};
