
#include "STSUserWidget.h"
#include "Components/CanvasPanel.h" 
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USTSUserWidget::AddCards(int32 NumCards)
{
    //카드 클래스 지정 안 했으면 튕겨내기
   // if (!CardWidgetClass) return;

    // 카드 생성 반복문
    for (int32 i = 0; i < NumCards; i++)
    {
        
        UUserWidget* NewCard = CreateWidget<UUserWidget>(GetOwningPlayer(), CardWidgetClass);

        
        if (NewCard && HandAreaPanel)
        {
            HandAreaPanel->AddChild(NewCard);
            CreatedCards.Add(NewCard); 
        }
    }

    // 카드 줄 세우기
    UpdateCardLayout();
}
void USTSUserWidget::UpdateCardLayout()
{
	int32 CreateCardsNum = CreatedCards.Num();

	float MiddleIndex = (CreateCardsNum - 1) / 2.0f;
 
    float MaxAngle = 15.0f; // 임시 최대 각도
	float HorizontalSpacing = 150.0f; // 임시 카드 간 가로 간격

    for (int32 i = 0; i < CreateCardsNum; i++)
    {
        UUserWidget* Card = CreatedCards[i];

        float Offset = i - MiddleIndex;
       
      
        FVector2D NewPosition;
		NewPosition.X = Offset * HorizontalSpacing;
	    NewPosition.Y = (FMath::Abs(Offset) * 20.0f); // Y축 위치 조정 (임시값)

        UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(Card->Slot);
        if (!CardSlot) return;
        //앵커를 기준으로 좌표를 잡는다
        FAnchors CenterBottomAnchors(0.5f, 1.0f);
        CardSlot->SetAnchors(CenterBottomAnchors);

        float RotationAngle = Offset * 5.0f;
        CardSlot->SetAlignment(FVector2D(0.5f, 1.0f));
        Card->SetRenderTransformAngle(RotationAngle);
        CardSlot->SetPosition(NewPosition);
        CardSlot->SetSize(FVector2D(200.0f, 300.0f));

       
    }
}


