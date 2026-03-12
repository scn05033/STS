// STSCardWidget.cpp
#include "STSCardWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


void USTSCardWidget::UpdateCardDesign(const FCardData& Data)
{   
    CurrentCardData = Data;
    // 이름 설정
    if (CardName)
    {
        CardName->SetText(Data.CardName); 
    }

    //비용 설정 (int를 Text로 변환)
   if (Cost)
    {
        Cost->SetText(FText::AsNumber(Data.Cost));
    }
   
    //설명 설정
    if (CardDescription)
    {
        CardDescription->SetText(Data.CardDescription);
    }

 /**   // 이미지 설정 (이미지가 있을 때만)
    if (CardImage && Data.CardIcon)
    {
        CardImage->SetBrushFromTexture(Data.CardIcon);
    }*/


}
void USTSCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    // 크기 키우기 
    SetRenderScale(FVector2D(1.2f, 1.2f));

    // ZOrder 바꾸기 (슬롯을 가져와서 변경)
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetZOrder(1); // 제일 앞으로
    }
}

void USTSCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // 크기 원상복구
    SetRenderScale(FVector2D(1.0f, 1.0f));

    // ZOrder 원상복구
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetZOrder(0); //원상복귀
    }
}


FReply USTSCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    //드래그를 감지하지 않고 클릭만 처리!
    if (bIsSmithingMode)
    {
        
        // "나 강화하려고 클릭됐어!" 라고 블루프린트(WBP_TestCard)에 알려줌.
        OnCardClickedForSmithing();

        return FReply::Handled();
    }



    // 왼쪽 마우스 버튼 감지
    else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // 드래그 감지 시작
        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USTSCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    // 드래그 오퍼레이션 생성
    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

	// payload에 자기 자신(카드 위젯) 넣기 
    DragOp->Payload = this;

	USTSCardWidget* DragVisualWidget = CreateWidget<USTSCardWidget>(GetOwningPlayer(), GetClass());
   
    if (DragVisualWidget) {
        DragVisualWidget->UpdateCardDesign(CurrentCardData);
		DragVisualWidget->SetRenderTransformAngle(0.0f); // 회전 초기화  

        DragVisualWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		DragOp->DefaultDragVisual = DragVisualWidget;

    }
        
    


    // 마우스가 카드의 중앙을 잡게 함 
    DragOp->Pivot = EDragPivot::CenterCenter;

    // 시스템에 전달
    OutOperation = DragOp;

    this->SetVisibility(ESlateVisibility::Hidden);

    UE_LOG(LogTemp, Log, TEXT("드래그 시작! 카드: %s"), *CardName->GetText().ToString());
}

FText USTSCardWidget::GetCardNameText() const
{
    if (CardName)
    {
        return CardName->GetText();
    }
    return FText::FromString("Unknown");
}



void USTSCardWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    // 드래그가 실패하거나 취소되면, 숨겨뒀던 나를 다시 보여줍니다.
    this->SetVisibility(ESlateVisibility::Visible);

    // 다시 나타날 때 ZOrder나 크기 초기화
    SetRenderScale(FVector2D(1.0f, 1.0f));
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetZOrder(0);
    }

    UE_LOG(LogTemp, Warning, TEXT("드래그 취소됨. 원본 복귀."));
}