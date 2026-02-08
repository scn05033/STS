#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CardDataStruct.generated.h"

/**
 * CSV 데이터를 담을 구조체
 */
USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
	GENERATED_BODY()

public:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FText CardName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 CostText = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FName Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FName Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 BaseDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 BaseBlock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FText CardDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	UTexture2D* Art;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FName EffectId;
};