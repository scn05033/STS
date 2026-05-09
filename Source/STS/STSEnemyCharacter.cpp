#include "STSEnemyCharacter.h"
#include "STSEnemyHPWidget.h"
#include "STSCharacter.h"
#include "STSGameMode.h"
#include "Sound/SoundBase.h"
#include "StatusEffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"


// HP 위젯과 행동 의도 위젯을 생성하고 설정합니다.
ASTSEnemyCharacter::ASTSEnemyCharacter()
{
	
	HPWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBar"));
	HPWidgetComp->SetupAttachment(GetRootComponent());

	
	HPWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));

	
	
	HPWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HPWidgetComp->SetDrawSize(FVector2D(150.0f, 50.0f));

	
	IntentWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("IntentWidgetComp"));
	IntentWidgetComp->SetupAttachment(RootComponent); 

	
	IntentWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);

	
	IntentWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));


}

// 게임이 시작될 때 체력을 최대 체력으로 초기화하고 UI를 업데이트합니다.
void ASTSEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	CurrentHealth = MaxHealth;

	UIIntentType = EIntentType::None;
	UpdateHPUI(CurrentHealth, MaxHealth);
	UpdateIntentUI(UIIntentType, CurrentIntentValue);
}

// 데미지를 받는 함수입니다. 
float ASTSEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	

	
	float BaseDamage = DamageAmount;

	
	if (ASTSGameMode* GameMode = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(this)))
	{

		
	}
	float FinalDamage = BaseDamage;

	
	if (UStatusEffectComponent* StatusComp = FindComponentByClass<UStatusEffectComponent>())
	{
		int32 VulStacks = StatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Vulnerable);

		if (VulStacks > 0)
		{
			
			BaseDamage *= 1.5f;
		}
	}

	
	float ActualDamage = Super::TakeDamage(BaseDamage, DamageEvent, EventInstigator, DamageCauser);




	
	if (ActualDamage > 0.0f)
	{
		
		PlayHitImpact(FMath::FloorToInt(ActualDamage));
		int32 DamageToHealth = FMath::FloorToInt(ActualDamage);

		if (CurrentBlock > 0)
		{


			if (CurrentBlock >= DamageToHealth)
			{
				// 케이스 A: 방어도가 데미지보다 높거나 같아서 전부 막아냄
				CurrentBlock -= DamageToHealth;
				DamageToHealth = 0; 
				UpdateBlockUI(CurrentBlock);
			}
			else
			{
				// 케이스 B: 데미지가 너무 세서 방어도가 뚫림
				DamageToHealth -= CurrentBlock; 

				CurrentBlock = 0; 
			}
			UpdateBlockUI(CurrentBlock);
			
		}

		// 방어도로 막고 남은 데미지가 있다면 체력에서 깎습니다.
		if (DamageToHealth > 0)
		{
			CurrentHealth -= DamageToHealth;

			
			if (CurrentHealth < 0.0f) CurrentHealth = 0.0f;

			
			UpdateHPUI(CurrentHealth, MaxHealth);

			
			if (HitSoundAsset) 
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSoundAsset, GetActorLocation());
			}

			if (CurrentHealth > 0.0f)
			{
				if (HitReactMontage) { PlayAnimMontage(HitReactMontage); }
			}

			
			if (CurrentHealth <= 0.0f)
			{
				Die();
				
			}
		}
	}

	return ActualDamage;
}

void ASTSEnemyCharacter::DecideNextIntent()
{
	int32 RandomChoice = 0;

	
	if (bIsBoss)
	{
		RandomChoice = FMath::RandRange(0, 3);
	}
	else
	{
		
		RandomChoice = FMath::RandRange(0, 2);
	}

	CurrentHitCount = 1; // 매 턴마다 기본 타수를 1로 초기화 

	// UI에 쏴줄 'Enum 타입'을 저장할 임시 변수
	UIIntentType = EIntentType::Attack;


	// 랜덤으로 행동 유형과 값을 결정합니다.
	if (RandomChoice == 0)
	{
		CurrentIntentType = TEXT("Attack");
		CurrentIntentValue = FMath::RandRange(5, 12);
		UIIntentType = EIntentType::Attack;
	}
	else if (RandomChoice == 1)
	{
		CurrentIntentType = TEXT("Defend");
		CurrentIntentValue = FMath::RandRange(4, 8);
		UIIntentType = EIntentType::Defend; 
	}
	else if (RandomChoice == 2)
	{
		CurrentIntentType = TEXT("Debuff");
		CurrentIntentValue = 2;
		UIIntentType = EIntentType::Debuff; 
	}
	
	else if (RandomChoice == 3)
	{
		CurrentIntentType = TEXT("MultiHit");
		CurrentIntentValue = 4; // 1타당 기본 데미지
		CurrentHitCount = 3;    // 3번 연속으로 때림
		UIIntentType = EIntentType::MultiHit; 
	}

	
	UpdateIntentUI(UIIntentType, CurrentIntentValue);
}

// 행동 유형에 따라 실제로 행동을 수행하는 함수입니다. 
void ASTSEnemyCharacter::ExecuteIntent(ASTSGameMode* GM)
{
	if (!GM) return;

	if (CurrentIntentType == TEXT("Attack"))
	{
		
		int32 FinalDamage = CurrentIntentValue + CurrentStrength;

		
		this->PendingDamage = FinalDamage;


		// 플레이어(타겟)를 찾아서 돌진 명령
		if (ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			
			DashAndAttack(PlayerChar);
		}
	}
	else if (CurrentIntentType == TEXT("Defend"))
	{
		AddBlock(CurrentIntentValue);
		PlayAnimMontage(DefendMontage);
	}
	else if (CurrentIntentType == TEXT("Debuff"))
	{
		PlayAnimMontage(DebuffMontage);
		
		if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			
			if (UStatusEffectComponent* StatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
			{
				
				StatusComp->AddStatusEffect(EStatusEffectType::Weak, CurrentIntentValue);

			}
		}
	}
	else if (CurrentIntentType == TEXT("Strength"))
	{
		PlayAnimMontage(BuffMontage);
		
		CurrentStrength += CurrentIntentValue;

	}
	else if (CurrentIntentType == TEXT("MultiHit"))
	{
		// 총 데미지가 아니라 '1타당 데미지'를 계산해서 변수에 "기억"만 해둡니다.
		int32 DamagePerHit = CurrentIntentValue + CurrentStrength;
		this->PendingDamage = DamagePerHit;


		
		if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			ExecuteDashAndMultiHit(PlayerChar); 
		}
	}
}

// 노티파이가 불릴 때마다 실행될 함수 구현
void ASTSEnemyCharacter::DealSingleHitDamage()
{
	// ExecuteIntent에서 기억해둔 PendingDamage를 사용해 진짜로 때립니다.
	if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		PlayerChar->TakePlayerDamage(PendingDamage);

		
	}
}

void ASTSEnemyCharacter::DecreaseStatusEffects()
{
	
	if (VulnerableStacks > 0)
	{
		VulnerableStacks--;
	}
}

// 방어도를 추가하는 함수입니다. 
void ASTSEnemyCharacter::AddBlock(int32 BlockAmount)
{
	CurrentBlock += BlockAmount;


	
	OnBlockChanged.Broadcast(CurrentBlock);


	
	if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
	{
		
	}
}

// 실제로 플레이어에게 데미지를 주는 함수입니다.
void ASTSEnemyCharacter::ExecuteHit()
{
	if (PendingDamage > 0)
	{
		
		if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			
			PlayerChar->TakePlayerDamage(PendingDamage);

			
			PendingDamage = 0;
		}
	}
}

// 적이 사망했을 때 실행되는 함수입니다.
void ASTSEnemyCharacter::Die()
{

	// 충돌 판정 끄기 
	SetActorEnableCollision(false);

	
	if (HPWidgetComp)
	{
		HPWidgetComp->SetVisibility(false);
	}

	Tags.Remove(FName("CurrentBattle"));
	Tags.Remove(FName("Enemy"));

	// 블루프린트에 애니메이션 명령
	PlayDeathVisuals();

	
	GetWorld()->GetTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&ASTSEnemyCharacter::DelayedVictoryCheck,
		DeathDelay,
		false
	);

}

void ASTSEnemyCharacter::DelayedVictoryCheck()
{
	// 타이머가 끝나면 실행되는 실제 게임 종료 로직
	ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->CheckVictory();
	}

	Destroy();
}

void ASTSEnemyCharacter::SetTargetingHighlight(bool bIsTargeted)
{
	if (GetMesh())
	{
		// 언리얼 엔진의 기본 외곽선 그리기 기능 (Custom Depth)을 켜고 끕니다.
		GetMesh()->SetRenderCustomDepth(bIsTargeted);

		
	}
}

// 플레이어가 공격하기 전에, 플레이어의 공격이 적에게 얼마나 데미지를 줄지 예측하는 함수입니다.
int32 ASTSEnemyCharacter::GetPredictedHPChange(float IncomingDamage,int32 HitCount) const
{
	float FinalPerHitDamage = IncomingDamage;

	
	int32 VulStacks = 0;
	if (const UStatusEffectComponent* StatusComp = FindComponentByClass<UStatusEffectComponent>())
	{
		VulStacks = StatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Vulnerable);
	}

	
	if (VulStacks > 0)
	{
		FinalPerHitDamage *= 1.5f;
	}

	int32 DamagePerHit = FMath::FloorToInt(FinalPerHitDamage);
	int32 TotalDamage = DamagePerHit * HitCount;

	int32 DamageToHealth = TotalDamage;

	
	
	if (CurrentBlock > 0)
	{
		if (CurrentBlock >= DamageToHealth) return 0; 
		else DamageToHealth -= CurrentBlock; 
	}

	return DamageToHealth; 
}

// 플레이어가 공격하기 전에, 예측된 데미지로 UI를 업데이트하는 함수입니다.
void ASTSEnemyCharacter::UpdatePredictionUI(float IncomingDamage, int32 HitCount)
{
	float CurrentCalcDamage = IncomingDamage;

	
	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		if (UStatusEffectComponent* PlayerStatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
		{
			
			int32 WeakStacks = PlayerStatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Weak);
			if (WeakStacks > 0)
			{
				
				CurrentCalcDamage *= 0.75f;
			}
		}
	}

	

	
	int32 TotalRealDamage = GetPredictedHPChange(CurrentCalcDamage, HitCount);

	ForwardShowPrediction(TotalRealDamage, CurrentHealth, MaxHealth);
}

// 예측 UI를 초기 상태로 되돌리는 함수입니다.
void ASTSEnemyCharacter::ClearPredictionUI()
{
	ForwardHidePrediction(CurrentHealth, MaxHealth);
}