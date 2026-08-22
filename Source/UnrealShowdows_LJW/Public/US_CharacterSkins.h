#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "US_CharacterSkins.generated.h"

USTRUCT(BlueprintType)
struct UNREALSHOWDOWS_LJW_API FUS_CharacterSkins : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* Material4;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* Material0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* Material1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* Material2;
	
};
