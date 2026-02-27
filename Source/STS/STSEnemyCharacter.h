#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "STSEnemyCharacter.generated.h"

class ASTSGameMode;

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

	// 다음 행동을 결정하는 함수
	void DecideNextIntent();

	// 결정된 행동을 실행하는 함수 (GameMode에게 데미지를 주기 위해 포인터를 받음)
	void ExecuteIntent(ASTSGameMode* GM);
	
	// 최대 체력 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 50.0f;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	// 취약 스택 (0이면 정상, 1 이상이면 취약 상태)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	int32 VulnerableStacks = 0;

	// 턴이 끝날 때 상태이상을 1씩 줄여주는 함수
	void DecreaseStatusEffects();


protected:
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPWidgetComp;

	// 현재 결정된 행동 종류 ("Attack" 또는 "Defend")
	FString CurrentIntentType;

	// 현재 결정된 수치 (데미지량 또는 방어도량)
	int32 CurrentIntentValue;
};