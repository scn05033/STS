#include "STSEnemyCharacter.h"
#include "STSEnemyHPWidget.h"
#include "Components/WidgetComponent.h"

ASTSEnemyCharacter::ASTSEnemyCharacter()
{
	// 1. 위젯 컴포넌트 생성 및 설정
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

// ⭐ 누군가 나를 때리면(ApplyDamage) 이 함수가 자동으로 실행됩니다.
float ASTSEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 부모 클래스의 기본 로직 수행 (필수)
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 데미지가 0보다 클 때만 처리
	if (ActualDamage > 0.0f)
	{
		// 체력 감소
		CurrentHealth -= ActualDamage;

		// 체력이 0 미만으로 내려가지 않게 막음
		if (CurrentHealth < 0.0f) CurrentHealth = 0.0f;

		// ⭐ 위젯 업데이트 호출!
		if (USTSEnemyHPWidget* HPWidget = Cast<USTSEnemyHPWidget>(HPWidgetComp->GetUserWidgetObject()))
		{
			HPWidget->UpdateHP(CurrentHealth, MaxHealth);
		}

		//UE_LOG(LogTemp, Warning, TEXT("아야! %s가 %f의 피해를 입었습니다. (남은 체력: %f / %f)"),
		//	*GetName(), ActualDamage, CurrentHealth, MaxHealth);

		// 사망 처리
		if (CurrentHealth <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("%s 사망! (Die)"), *GetName());
			// TODO: 나중에 사망 애니메이션 재생 및 파괴 로직 추가
		}
	}

	return ActualDamage;
}