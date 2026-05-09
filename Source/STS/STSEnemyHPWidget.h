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
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")	
	void UpdateHP(float CurrentHP, float MaxHP);

	
	void UpdateIntentText(FString NewIntentText);

	
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
