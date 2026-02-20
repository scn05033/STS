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
	void UpdateHP(float CurrentHP, float MaxHP);

	//적의 의도 텍스트 업데이트 함수
	void UpdateIntentText(FString NewIntentText);
protected:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* IntentText;
};
