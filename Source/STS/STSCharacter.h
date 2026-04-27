// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "STSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBlockChanged, int32, NewBlock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHealthChanged, int32, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

USTRUCT(BlueprintType)
struct FActionCommand
{
	GENERATED_BODY()

	FName ActionType;
	bool bIsAoE;
	AActor* TargetEnemy;
	int32 Damage;
	UAnimMontage* Montage;
	class UNiagaraSystem* VFX;
	FName StatusType;
	int32 StatusAmount = 0;

};



UCLASS(config=Game)
class ASTSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	ASTSCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// 에디터에서 공격 몽타주를 넣을 칸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;

	// 에디터에서 피격 몽타주를 넣을 칸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* HitReactMontage;

	// UI가 호출해 줄 공격 애니메이션 재생 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackAnim();

	// UI가 호출해 줄 피격 애니메이션 재생 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayHitReactAnim();
	// 피격 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* HitEffect;
	

	// C++가 호출하면, 실제 이동과 애니메이션은 블루프린트에서 처리
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void DashAndAttack(AActor* TargetActor, UAnimMontage* MontageToPlay, UNiagaraSystem* VFXToPlay);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void PlayInPlaceAnimation(UAnimMontage* MontageToPlay, UNiagaraSystem* VFXToPlay);
	// 잠시 데미지를 보관할 변수
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 PendingDamage;

	// 상태이상을 기억할 주머니
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FName PendingStatusType;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 PendingStatusAmount;


	AActor* CurrentTarget;

	// AnimBP에서 호출할 실제 타격 함수
	UFUNCTION(BlueprintCallable)
	void ExecuteHit();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentHealth = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 MaxHealth = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentBlock = 0;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerBlockChanged OnBlockChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerDeath OnPlayerDeath;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddBlock(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakePlayerDamage(int32 Damage);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HealPlayer(int32 HealAmount);

	// 캐릭터가 광역 공격 중인지 기억하는 변수
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAoEAttack = false;

	// 광역 공격 시 데미지를 입힐 타겟 리스트 
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 PendingAoEDamage = 0;

	// [완전 방어]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class USoundBase* BlockSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class UNiagaraSystem* BlockVFX;

	// [실드 파괴]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class USoundBase* ShieldBreakSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class UNiagaraSystem* ShieldBreakVFX;

	// [맨몸 피격]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class USoundBase* FleshHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Effects")
	class UNiagaraSystem* BloodVFX;

	//실드가 깨질 때 블루프린트로 신호를 보내는 함수입니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|UI")
	void OnShieldBroken();

	// 액션 큐와 상태 변수
	TArray<FActionCommand> ActionQueue;
	bool bIsProcessingAction = false;

	// 큐 관리 함수들
	void EnqueueAction(const FActionCommand& NewAction);
	void ProcessNextAction();

	UFUNCTION(BlueprintCallable) 
	void CompleteCurrentAction();

	// 현재 진짜로 애니메이션이나 이동이 실행 중인지 확인하는 플래그
	bool bIsExecutingAction = false;



};

