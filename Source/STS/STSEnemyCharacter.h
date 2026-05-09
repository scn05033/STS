#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "STSEnemyCharacter.generated.h"

class ASTSGameMode;

UENUM(BlueprintType)
enum class EIntentType : uint8
{
	None  UMETA(DisplayName = "없음"),
	Attack  UMETA(DisplayName = "공격"),
	Defend  UMETA(DisplayName = "방어"),
	Debuff  UMETA(DisplayName = "약화"),
	Buff  UMETA(DisplayName = "강화"),
	MultiHit  UMETA(DisplayName = "강공")
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyBlockChanged, int32, NewBlock);

UCLASS()
class STS_API ASTSEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASTSEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	// 알람 시계 역할을 할 핸들
	FTimerHandle DeathTimerHandle;

	// 실제 승리 체크를 수행할 함수 
	UFUNCTION()
	void DelayedVictoryCheck();
public:
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyBlockChanged OnBlockChanged;

	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	
	void DecideNextIntent();

	
	void ExecuteIntent(ASTSGameMode* GM);
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 30.0f;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	int32 VulnerableStacks = 0;

	
	void DecreaseStatusEffects();

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentBlock = 0;
	int32 PendingDamage = 0;
	
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddBlock(int32 BlockAmount);

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsBoss = false;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentStrength = 0;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentHitCount = 1;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;

	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* HitReactMontage;
	USoundBase* HitSoundAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Animation")
	UAnimMontage* DefendMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Animation")
	UAnimMontage* DebuffMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Animation")
	UAnimMontage* BuffMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy Animation")
	UAnimMontage* MultiHitMontage;

	// 몽타주 노티파이에서 호출할 실제 타격 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DealSingleHitDamage();


	EIntentType UIIntentType;
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void DashAndAttack(AActor* TargetActor);

	

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteHit();

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* IntentWidgetComp;

	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateIntentUI(EIntentType IntentType, int32 IntentValue);

	

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateHPUI(float CurrentHP, float MaxHP);

	// 블루프린트에서 만든 타격 효과(플로팅 데미지, 흔들림)를 실행하는 수신기
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void PlayHitImpact(int32 Damage);

	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBlockUI(int32 NewBlock);

	
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayDeathVisuals();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay = 1.0f;

	
	UFUNCTION(BlueprintCallable, Category = "Combat|Visuals")
	void SetTargetingHighlight(bool bIsTargeted);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetPredictedHPChange(float IncomingDamage, int32 HitCount = 1) const;

	
	void UpdatePredictionUI(float IncomingDamage, int32 HitCount = 1);
	void ClearPredictionUI();

	
	 //데미지 계산이 끝나면 블루프린트의 대시 연출을 실행하라고 지시하는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void ExecuteDashAndMultiHit(AActor* TargetPlayer);


protected:
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPWidgetComp;

	
	FString CurrentIntentType;

	
	int32 CurrentIntentValue;

	
	void Die();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ForwardShowPrediction(int32 RealDamage, float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ForwardHidePrediction(float CurrentHP, float MaxHP);
};