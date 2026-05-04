#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "STSEnemyCharacter.generated.h"

class ASTSGameMode;

UENUM(BlueprintType)
enum class EIntentType : uint8
{
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
	// 위젯이 연결할 수 있는 방어도 변경 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyBlockChanged OnBlockChanged;

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

	// 적의 현재 방어도
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentBlock = 0;
	int32 PendingDamage = 0;
	
	// 방어도를 획득하는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddBlock(int32 BlockAmount);

	// 보스 여부 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsBoss = false;

	// 적의 현재 힘(Strength) 스탯
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentStrength = 0;

	// 다단히트 타수 저장용
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentHitCount = 1;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;

	// 블루프린트에서 피격 몽타주를 넣을 수 있는 칸 만들기
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

	// 몬스터 머리 위에 UI를 띄워줄 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* IntentWidgetComp;

	//C++에서 호출하면 블루프린트에서 실행
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateIntentUI(EIntentType IntentType, int32 IntentValue);

	

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateHPUI(float CurrentHP, float MaxHP);

	// 블루프린트에서 만든 타격 효과(플로팅 데미지, 흔들림)를 실행하는 수신기
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void PlayHitImpact(int32 Damage);

	//방어도가 변할 때 UI에 업데이트를 요청하는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBlockUI(int32 NewBlock);

	// 블루프린트에서 쓰러지는 애니메이션과 이펙트를 틀도록 지시하는 알람
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayDeathVisuals();

	// 블루프린트에서 수정 가능한 사망 대기 시간 변수! 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay = 1.0f;

	// 타겟팅 시각 효과를 켜고 끄는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Visuals")
	void SetTargetingHighlight(bool bIsTargeted);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	int32 GetPredictedHPChange(int32 IncomingDamage) const;

	// 카드 위젯이 호출할 함수 (일반 함수로 선언)
	void UpdatePredictionUI(int32 IncomingDamage);
	void ClearPredictionUI();

	
	 //데미지 계산이 끝나면 블루프린트의 대시 연출을 실행하라고 지시하는 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void ExecuteDashAndMultiHit(AActor* TargetPlayer);


protected:
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HPWidgetComp;

	// 현재 결정된 행동 종류 ("Attack" 또는 "Defend")
	FString CurrentIntentType;

	// 현재 결정된 수치 (데미지량 또는 방어도량)
	int32 CurrentIntentValue;

	// 사망 처리를 전담할 함수
	void Die();

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ForwardShowPrediction(int32 RealDamage, float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void ForwardHidePrediction(float CurrentHP, float MaxHP);
};