#include "STSEnemyCharacter.h"
#include "STSEnemyHPWidget.h"
#include "STSGameMode.h"
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
}

void ASTSEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작하면 현재 체력을 최대 체력으로 설정
	CurrentHealth = MaxHealth;

	// 게임 시작 시 HP바 초기화
	if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
	{
		HPWidget->UpdateHP(CurrentHealth, MaxHealth);
	}
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
		int32 DamageToHealth = FMath::RoundToInt(ActualDamage);

		if (CurrentBlock > 0)
		{
			if (CurrentBlock >= DamageToHealth)
			{
				// 케이스 A: 방어도가 데미지보다 높거나 같아서 전부 막아냄
				CurrentBlock -= DamageToHealth;
				DamageToHealth = 0; // 체력에 들어갈 데미지 소멸

				UE_LOG(LogTemp, Warning, TEXT("[방어 성공] 데미지를 모두 막아냈습니다! 남은 방어도: %d"), CurrentBlock);
			}
			else
			{
				// 케이스 B: 데미지가 너무 세서 방어도가 뚫림
				DamageToHealth -= CurrentBlock; // 방어도만큼 데미지 차감
				UE_LOG(LogTemp, Warning, TEXT("[방어 관통] 방어도가 %d 뚫렸습니다! 남은 데미지: %d"), CurrentBlock, DamageToHealth);

				CurrentBlock = 0; // 방어도는 완전히 박살남
			}

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
			if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
			{
				HPWidget->UpdateHP(CurrentHealth, MaxHealth);
			}

			// 사망 처리
			if (CurrentHealth <= 0.0f)
			{
				UE_LOG(LogTemp, Error, TEXT("%s 사망! (Die)"), *GetName());

				// 승리 확인 요청
				ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
				if (GM)
				{
					GM->CheckVictory();
				}

				Destroy();
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

	USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject());
	CurrentHitCount = 1; // 매 턴마다 기본 타수를 1로 초기화 (중요!)

	if (RandomChoice == 0)
	{
		CurrentIntentType = TEXT("Attack");
		CurrentIntentValue = FMath::RandRange(5, 12); // 5~12 사이의 랜덤 데미지

		if (HPWidget)
		{
			HPWidget->UpdateIntentText(FString::Printf(TEXT("공%d"), CurrentIntentValue));
		}
	}
	else if (RandomChoice == 1)
	{
		CurrentIntentType = TEXT("Defend");
		CurrentIntentValue = FMath::RandRange(4, 8); // 4~8 사이의 랜덤 방어도

		if (HPWidget)
		{
			HPWidget->UpdateIntentText(FString::Printf(TEXT("방%d"), CurrentIntentValue));
		}
	}
	else if (RandomChoice == 2)
	{
		CurrentIntentType = TEXT("Debuff");
		CurrentIntentValue = 2; // 플레이어에게 줄 약화 스택 (예: 2스택)

		if (HPWidget)
		{
			// 머리 위에 "약화2" 라고 띄워줍니다
			HPWidget->UpdateIntentText(FString::Printf(TEXT("약화%d"), CurrentIntentValue));
		}
	}
	else if (RandomChoice == 3)
	{
		// 보스 추가 패턴 A: 힘 증가
		CurrentIntentType = TEXT("BuffStrength");
		CurrentIntentValue = 2; // 한 번에 오르는 힘 수치
		if (HPWidget) HPWidget->UpdateIntentText(FString::Printf(TEXT("힘+%d"), CurrentIntentValue));
	}
	else if (RandomChoice == 4)
	{
		// 보스 추가 패턴 B: 연속 타격
		CurrentIntentType = TEXT("MultiHit");
		CurrentIntentValue = 4; // 1타당 기본 데미지
		CurrentHitCount = 3;    // 3번 연속으로 때림

		// UI 표시: "연타 6x3" (기본데미지+힘 x 타수) 형태로 띄워줍니다.
		if (HPWidget) HPWidget->UpdateIntentText(FString::Printf(TEXT("연타 %dx%d"), CurrentIntentValue + CurrentStrength, CurrentHitCount));
	}
}

void ASTSEnemyCharacter::ExecuteIntent(ASTSGameMode* GM)
{
	if (!GM) return;

	if (CurrentIntentType == TEXT("Attack"))
	{
		int32 FinalDamage = CurrentIntentValue + CurrentStrength; // 공격력에 힘(Strength) 합산
		UE_LOG(LogTemp, Warning, TEXT(" 적이 %d의 데미지로 공격합니다!"), CurrentIntentValue);
		//애니메이션 재생 로직 추가
		//
		if (AttackMontage)
		{
			PlayAnimMontage(AttackMontage);
		}
		GM->TakePlayerDamage(CurrentIntentValue);
	}
	else if (CurrentIntentType == TEXT("Defend"))
	{
		UE_LOG(LogTemp, Warning, TEXT(" 적이 %d의 방어도를 올립니다! (적 방어도 로직은 추후 추가)"), CurrentIntentValue);
		AddBlock(CurrentIntentValue);
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

		// For 루프를 돌려서 플레이어에게 데미지를 입힘.
		for (int i = 0; i < CurrentHitCount; i++)
		{
			GM->TakePlayerDamage(FinalDamage);
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

	// UI 업데이트 
	if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
	{
		// TODO: HPWidget에 방어도를 표시하는 함수를 미리 만들어두셨다면 여기서 호출
		
	}
}