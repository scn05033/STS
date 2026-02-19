#include "STSEnemyHPWidget.h"

void USTSEnemyHPWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPBar && MaxHP > 0.0f)
	{
		// 퍼센트 계산 (0.0 ~ 1.0)
		HPBar->SetPercent(CurrentHP / MaxHP);
	}

	if (HPText)
	{
		// 텍스트 갱신 (예: "43 / 50")
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP)));
	}
}