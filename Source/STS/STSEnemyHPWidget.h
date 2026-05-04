// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "STSEnemyHPWidget.generated.h"

UCLASS()
class STS_API USTSEnemyHPWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 체력바 업데이트 함수
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")	
	void UpdateHP(float CurrentHP, float MaxHP);

	//적의 의도 텍스트 업데이트 함수
	void UpdateIntentText(FString NewIntentText);

	// 블루프린트(UMG)에서 이벤트로 구현할 수 있도록 BlueprintImplementableEvent 사용
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")	
	void ShowPredictedHP(int32 PredictedDamage, float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")	
	void HidePredictedHP(float CurrentHP, float MaxHP);


protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* HPText;

	


	
};
