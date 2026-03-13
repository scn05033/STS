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
		// 체력 감소
		CurrentHealth -= ActualDamage;

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
			// TODO: 나중에 사망 애니메이션 재생 및 파괴 로직 추가

			// 승리 확인 요청
			ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (GM)
			{
				GM->CheckVictory();
			}

			// 내 캐릭터 월드에서 삭제
			Destroy();
		}
	}

	return ActualDamage;
}

void ASTSEnemyCharacter::DecideNextIntent()
{
	// 0: 공격, 1: 방어, 2: 약화 (각각 33% 확률)
	int32 RandomChoice = FMath::RandRange(0, 2);

	USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject());

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
	else 
	{
		CurrentIntentType = TEXT("Debuff");
		CurrentIntentValue = 2; // 플레이어에게 줄 약화 스택 (예: 2스택)

		if (HPWidget)
		{
			// 머리 위에 "약화2" 라고 띄워줍니다
			HPWidget->UpdateIntentText(FString::Printf(TEXT("약화%d"), CurrentIntentValue));
		}
	}
}

void ASTSEnemyCharacter::ExecuteIntent(ASTSGameMode* GM)
{
	if (!GM) return;

	if (CurrentIntentType == TEXT("Attack"))
	{
		UE_LOG(LogTemp, Warning, TEXT(" 적이 %d의 데미지로 공격합니다!"), CurrentIntentValue);
		GM->TakePlayerDamage(CurrentIntentValue);
	}
	else if (CurrentIntentType == TEXT("Defend"))
	{
		UE_LOG(LogTemp, Warning, TEXT(" 적이 %d의 방어도를 올립니다! (적 방어도 로직은 추후 추가)"), CurrentIntentValue);
		// TODO: 나중에 적 자신의 방어도를 올리는 함수 호출
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