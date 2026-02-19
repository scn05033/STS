#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "STSEnemyCharacter.generated.h"

UCLASS()
class STS_API ASTSEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASTSEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:
	// 언리얼이 제공하는 데미지 받는 함수 (Override)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 현재 체력을 UI에서 가져갈 수 있게 Getter
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	// 최대 체력
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	//적의 의도 텍스트 업데이트 함수
	void UpdateIntentText(FString NewIntentText);

protected:
	// 최대 체력 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 50.0f;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPWidgetComp;
};