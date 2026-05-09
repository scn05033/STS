#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "NiagaraSystem.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardData")
	TSoftObjectPtr<class UTexture2D> Art;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FName EffectId;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatusType;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StatusAmount;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAoE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRewardable = true; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	FName UpgradedCardID;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DrawAmount;

	// 타격 횟수 (기본값은 무조건 1번 때리도록 1로 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card Data")
	int32 HitCount = 1;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UAnimMontage> CardMontage;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<class UNiagaraSystem> CardVFX;

};