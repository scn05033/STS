#include "STSEnemyCharacter.h"
#include "STSEnemyHPWidget.h"
#include "STSCharacter.h"
#include "STSGameMode.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

ASTSEnemyCharacter::ASTSEnemyCharacter()
{
	// 위젯 컴포넌트 생성 및 설정
	HPWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBar"));
	HPWidgetComp->SetupAttachment(GetRootComponent());

	// 적 머리 위로 위치 조정 (높이는 캐릭터 크기에 맞춰 조절하세요)
	HPWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));

	// 위젯 모드 설정 (Screen: 항상 카메라를 봄, World: 3D 물체처럼 회전)
	
	HPWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HPWidgetComp->SetDrawSize(FVector2D(150.0f, 50.0f));

	// 위젯 컴포넌트 생성
	IntentWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("IntentWidgetComp"));
	IntentWidgetComp->SetupAttachment(RootComponent); // 몬스터 몸통에 부착

	// 3D 월드가 아닌 플레이어의 2D 모니터 화면에 딱 붙어서 보이게 설정
	IntentWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);

	// 몬스터 정수리 위쪽으로 위치 살짝 올리기
	IntentWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));


}

void ASTSEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작하면 현재 체력을 최대 체력으로 설정
	CurrentHealth = MaxHealth;

	// 게임 시작 시 HP바 초기화
	/**if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
	{
		HPWidget->UpdateHP(CurrentHealth, MaxHealth);
	}*/
	UpdateHPUI(CurrentHealth, MaxHealth);
	//UpdateIntentUI(EIntentType::Attack, 15);
}

// 누군가 나를 때리면(ApplyDamage) 이 함수가 자동으로 실행됩니다.
float ASTSEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 부모 클래스의 기본 로직 수행 (필수)
	//float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 들어온 원래 데미지
	float BaseDamage = DamageAmount;

	// 게임모드에서 플레이어의 '힘(Strength)' 가져오기
	if (ASTSGameMode* GameMode = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(this)))
	{

		//BaseDamage += GameMode->CurrentStrength;
		UE_LOG(LogTemp, Warning, TEXT("[스탯 적용] 힘이 더해진 데미지: %f"), BaseDamage);
	}
	float FinalDamage = BaseDamage;

	// [상태이상] 취약(Vulnerable) 스택이 0보다 크면 데미지 1.5배 (반올림)
	if (VulnerableStacks > 0)
	{
		FinalDamage = BaseDamage * 1.5f;
		UE_LOG(LogTemp, Warning, TEXT("[취약 발동] 데미지가 %f 에서 %f 로 증폭되었습니다!"), BaseDamage, FinalDamage);
	}
	float ActualDamage = Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);




	// 데미지가 0보다 클 때만 처리
	if (ActualDamage > 0.0f)
	{
		// 방어도(Block) 연산 시작 
		// float형 데미지를 int32로 변환 (STS는 주로 정수 데미지를 사용하므로 반올림)

		// 블루프린트로 데미지 수치를 보내며 효과(숫자 띄우기, 흔들림) 실행
		PlayHitImpact(FMath::RoundToInt(ActualDamage));
		int32 DamageToHealth = FMath::RoundToInt(ActualDamage);

		if (CurrentBlock > 0)
		{


			if (CurrentBlock >= DamageToHealth)
			{
				// 케이스 A: 방어도가 데미지보다 높거나 같아서 전부 막아냄
				CurrentBlock -= DamageToHealth;
				DamageToHealth = 0; // 체력에 들어갈 데미지 소멸
				UpdateBlockUI(CurrentBlock);
				UE_LOG(LogTemp, Warning, TEXT("[방어 성공] 데미지를 모두 막아냈습니다! 남은 방어도: %d"), CurrentBlock);
			}
			else
			{
				// 케이스 B: 데미지가 너무 세서 방어도가 뚫림
				DamageToHealth -= CurrentBlock; // 방어도만큼 데미지 차감
				UE_LOG(LogTemp, Warning, TEXT("[방어 관통] 방어도가 %d 뚫렸습니다! 남은 데미지: %d"), CurrentBlock, DamageToHealth);

				CurrentBlock = 0; // 방어도는 완전히 박살남
			}
			UpdateBlockUI(CurrentBlock);
			// 위젯에 깎인 방어도 업데이트 (TODO: HPWidget에 함수 추가 필요)
			// if (HPWidget) HPWidget->UpdateBlock(CurrentBlock);
		}

		// 체력(HP) 연산 (방어도를 뚫고 들어온 데미지만 적용) 
		if (DamageToHealth > 0)
		{
			CurrentHealth -= DamageToHealth;

			// 체력이 0 미만으로 내려가지 않게 막음
			if (CurrentHealth < 0.0f) CurrentHealth = 0.0f;

			// 위젯 업데이트 호출!
			/**if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
			{
				HPWidget->UpdateHP(CurrentHealth, MaxHealth);
			}*/
			UpdateHPUI(CurrentHealth, MaxHealth);

			// 애니메이션 재생과 동시에 사운드 재생
			if (HitSoundAsset) 
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSoundAsset, GetActorLocation());
			}

			if (CurrentHealth > 0.0f)
			{
				if (HitReactMontage) { PlayAnimMontage(HitReactMontage); }
			}

			// 사망 처리
			if (CurrentHealth <= 0.0f)
			{
				UE_LOG(LogTemp, Error, TEXT("%s 사망! (Die)"), *GetName());
				Die();
				
			}
		}
	}

	return ActualDamage;
}

void ASTSEnemyCharacter::DecideNextIntent()
{
	int32 RandomChoice = 0;

	// 보스일 경우 5개의 패턴을 뽑음.
	if (bIsBoss)
	{
		RandomChoice = FMath::RandRange(0, 4);
	}
	else
	{
		// 일반 몬스터일 경우: 기존대로 0, 1, 2 중 하나만 뽑음
		RandomChoice = FMath::RandRange(0, 2);
	}

	CurrentHitCount = 1; // 매 턴마다 기본 타수를 1로 초기화 (중요!)

	// UI에 쏴줄 'Enum 타입'을 저장할 임시 변수
	EIntentType UIIntentType = EIntentType::Attack;

	if (RandomChoice == 0)
	{
		CurrentIntentType = TEXT("Attack");
		CurrentIntentValue = FMath::RandRange(5, 12);
		UIIntentType = EIntentType::Attack; // 공격 아이콘
	}
	else if (RandomChoice == 1)
	{
		CurrentIntentType = TEXT("Defend");
		CurrentIntentValue = FMath::RandRange(4, 8);
		UIIntentType = EIntentType::Defend; // 방어 아이콘
	}
	else if (RandomChoice == 2)
	{
		CurrentIntentType = TEXT("Debuff");
		CurrentIntentValue = 2;
		UIIntentType = EIntentType::Debuff; // 약화 아이콘 
	}
	else if (RandomChoice == 3)
	{
		CurrentIntentType = TEXT("BuffStrength");
		CurrentIntentValue = 2;
		
		UIIntentType = EIntentType::Buff;
	}
	else if (RandomChoice == 4)
	{
		CurrentIntentType = TEXT("MultiHit");
		CurrentIntentValue = 4; // 1타당 기본 데미지
		CurrentHitCount = 3;    // 3번 연속으로 때림
		UIIntentType = EIntentType::Attack; // 연타도 일단 공격 아이콘을 띄움
	}

	// UI 업데이트 이벤트 호출 
	UpdateIntentUI(UIIntentType, CurrentIntentValue);
}

void ASTSEnemyCharacter::ExecuteIntent(ASTSGameMode* GM)
{
	if (!GM) return;

	if (CurrentIntentType == TEXT("Attack"))
	{
		// 데미지 계산 및 '기억' 
		int32 FinalDamage = CurrentIntentValue + CurrentStrength;

		
		this->PendingDamage = FinalDamage;

		UE_LOG(LogTemp, Warning, TEXT("적이 %d의 데미지로 공격을 준비합니다!"), PendingDamage);

		// 플레이어(타겟)를 찾아서 돌진 명령
		if (ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			
			DashAndAttack(PlayerChar);
		}
	}
	else if (CurrentIntentType == TEXT("Defend"))
	{
		//UE_LOG(LogTemp, Warning, TEXT(" 적이 %d의 방어도를 올립니다! (적 방어도 로직은 추후 추가)"), CurrentIntentValue);
		AddBlock(CurrentIntentValue);
		//UpdateBlockUI(CurrentIntentValue);
	}
	else if (CurrentIntentType == TEXT("Debuff"))
	{
		UE_LOG(LogTemp, Warning, TEXT("적이 플레이어에게 기분 나쁜 저주를 겁니다!"));

		if (GM)
		{
			// 플레이어에게 약화 2스택을 줍니다.
			GM->AddWeak(CurrentIntentValue);
		}
	}
	else if (CurrentIntentType == TEXT("BuffStrength"))
	{
		// 보스 추가 패턴 A: 힘 획득
		CurrentStrength += CurrentIntentValue;
		UE_LOG(LogTemp, Warning, TEXT(" [보스 패턴] 보스가 포효하며 힘을 %d 얻었습니다! (현재 힘: %d)"), CurrentIntentValue, CurrentStrength);
	}
	else if (CurrentIntentType == TEXT("MultiHit"))
	{
		// 보스 추가 패턴 B: 연속 타격 실행
		int32 FinalDamage = CurrentIntentValue + CurrentStrength; // 1타당 데미지 계산
		UE_LOG(LogTemp, Warning, TEXT(" [보스 패턴] 보스가 %d의 데미지로 %d번 연속 공격합니다!"), FinalDamage, CurrentHitCount);

		if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			for (int i = 0; i < CurrentHitCount; i++)
			{
				PlayerChar->TakePlayerDamage(FinalDamage);
			}
		}
	}
}

void ASTSEnemyCharacter::DecreaseStatusEffects()
{
	// 취약 스택이 남아있다면 1 감소
	if (VulnerableStacks > 0)
	{
		VulnerableStacks--;
		UE_LOG(LogTemp, Warning, TEXT("적의 취약 스택이 1 감소했습니다. (남은 스택: %d)"), VulnerableStacks);
	}
}

void ASTSEnemyCharacter::AddBlock(int32 BlockAmount)
{
	CurrentBlock += BlockAmount;

	UE_LOG(LogTemp, Warning, TEXT("[전투] 적이 방어도를 %d 획득했습니다. (총 방어도: %d)"), BlockAmount, CurrentBlock);

	// "내 방어도가 변했어!" UI에 알려줌
	OnBlockChanged.Broadcast(CurrentBlock);


	// UI 업데이트 
	if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
	{
		// TODO: HPWidget에 방어도를 표시하는 함수를 미리 만들어두셨다면 여기서 호출
		
	}
}

void ASTSEnemyCharacter::ExecuteHit()
{
	if (PendingDamage > 0)
	{
		// PlayerCharacter를 불러옵니다.
		if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			// 캐릭터의 함수를 호출하여 데미지를 줍니다.
			PlayerChar->TakePlayerDamage(PendingDamage);

			// 데미지 초기화
			PendingDamage = 0;
		}
	}
}

void ASTSEnemyCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s 사망! 쓰러지는 연출 시작"), *GetName());

	// 충돌 판정 끄기 (플레이어가 죽은 시체를 타겟팅하거나 때리지 못하게 함)
	SetActorEnableCollision(false);

	// 머리 위 체력바 UI 숨기기 
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

	UE_LOG(LogTemp, Warning, TEXT("%s 시체 소멸!"), *GetName());
	Destroy();
}

void ASTSEnemyCharacter::SetTargetingHighlight(bool bIsTargeted)
{
	if (GetMesh())
	{
		// 언리얼 엔진의 기본 외곽선 그리기 기능 (Custom Depth)을 켜고 끕니다.
		GetMesh()->SetRenderCustomDepth(bIsTargeted);

		// (선택) 만약 발밑에 빨간색 타겟팅 마크(Decal)나 파티클을 띄우고 싶다면 여기서 켜고 끕니다
		// if (TargetingParticle) TargetingParticle->SetVisibility(bIsTargeted);
	}
}