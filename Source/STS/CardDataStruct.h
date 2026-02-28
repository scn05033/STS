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
	int32 Cost = 0;

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

	// 상태이상 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatusType;

	// 상태이상 스택 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatusAmount;

	// 광역기(AoE) 여부 체크! (true면 전체 공격, false면 단일 공격)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAoE;
};