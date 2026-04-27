// STSCardWidget.cpp
#include "STSCardWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/DragDropOperation.h"
#include "STSEnemyCharacter.h"
#include "STSGamemode.h"
#include "Components/RichTextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Kismet/GameplayStatics.h"
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
   if (!LeftText || !ValueText || !RightText) return;
   FString RawDesc = Data.CardDescription.ToString();
   FString LStr, RStr;

   // 공격 카드일 때 ({Damage} 기준)
   if (RawDesc.Split(TEXT("{Damage}"), &LStr, &RStr))
   {
       LeftText->SetText(FText::FromString(LStr));

       ValueText->SetText(FText::AsNumber(Data.BaseDamage));
       // 손패에 들어올 때부터 무조건 핫핑크로 칠해버립니다
       //ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 1.0f, 1.0f)));

       RightText->SetText(FText::FromString(RStr));
   }
    // 방어 카드일 때 ({Block} 기준)
    else if (RawDesc.Split(TEXT("{Block}"), &LStr, &RStr))
    {
        LeftText->SetText(FText::FromString(LStr));
        LeftText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        ValueText->SetText(FText::AsNumber(Data.BaseBlock));
        ValueText->SetColorAndOpacity(FSlateColor(FLinearColor::White)); // 기본 하얀색
        ValueText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        RightText->SetText(FText::FromString(RStr));
        RightText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    // 태그가 없는 일반 카드일 때 (가운데, 오른쪽 텍스트 숨김)
    else
    {
        LeftText->SetText(FText::FromString(RawDesc));
        LeftText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        ValueText->SetVisibility(ESlateVisibility::Collapsed);
        RightText->SetVisibility(ESlateVisibility::Collapsed);
    }


    if (CardArtImage && !Data.Art.IsNull())
    {
        // 소프트 포인터는 바로 사용할 수 없으므로 동기식 로드를 수행합니다.
        UTexture2D* LoadedTexture = Data.Art.LoadSynchronous();

        if (LoadedTexture)
        {
            CardArtImage->SetBrushFromTexture(LoadedTexture);
        }
    }

    if (CardType)
    {
        FText TypeText_KR; // 화면에 띄울 한글 텍스트 변수
        FSlateColor TypeColor; // 화면에 띄울 텍스트 색상 변수

        if (Data.Type == FName("Attack"))
        {
            TypeText_KR = FText::FromString(TEXT("공격"));
            // 붉은색 (강렬한 타격 느낌)
            TypeColor = FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
        }
        else if (Data.Type == FName("Skill"))
        {
            TypeText_KR = FText::FromString(TEXT("스킬"));
            // 청록색/초록색 (보조 및 기술 느낌)
            TypeColor = FSlateColor(FLinearColor(0.2f, 0.7f, 0.4f, 1.0f));
        }
        else if (Data.Type == FName("Defend"))
        {
            TypeText_KR = FText::FromString(TEXT("방어"));
            // 파란색 (단단한 방패 느낌)
            TypeColor = FSlateColor(FLinearColor(0.3f, 0.5f, 0.9f, 1.0f));
        }
        else if (Data.Type == FName("Power"))
        {
            TypeText_KR = FText::FromString(TEXT("파워"));
            // 금색/보라색 (희귀하고 강력한 버프 느낌)
            TypeColor = FSlateColor(FLinearColor(0.9f, 0.7f, 0.1f, 1.0f));
        }
        else
        {
            TypeText_KR = FText::FromString(TEXT("알 수 없음"));
            // 기본 텍스트 색상 (밝은 회색)
            TypeColor = FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
        }

        // 최종적으로 번역된 한글 텍스트와 색상을 위젯에 적용
        CardType->SetText(TypeText_KR);
        CardType->SetColorAndOpacity(TypeColor); // 여기서 색상이 적용됩니다
    }

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

        DragVisualWidget->ForceLayoutPrepass();

        

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


int32 USTSCardWidget::CalculateFinalDamage(int32 InBaseDamage, int32 InPlayerStrength, bool bIsTargetVulnerable, bool bIsPlayerWeak)
{
    // 합연산 (기본 데미지 + 힘)
    // 힘이 마이너스면 자연스럽게 데미지가 깎입니다.
    float ExpectedDamage = (float)(InBaseDamage + InPlayerStrength);

    // 최소 데미지는 0으로 보정
    ExpectedDamage = FMath::Max(0.0f, ExpectedDamage);

    //  곱연산 1 (적 취약 적용, 1.5배)
    if (bIsTargetVulnerable)
    {
        ExpectedDamage = FMath::FloorToFloat(ExpectedDamage * 1.5f);
    }

    // 곱연산 2 (플레이어 약화 적용, 0.75배)
    if (bIsPlayerWeak)
    {
        ExpectedDamage = FMath::FloorToFloat(ExpectedDamage * 0.75f);
    }

    // 최종 정수로 변환하여 반환
    return FMath::FloorToInt(ExpectedDamage);
}

void USTSCardWidget::UpdateTargetAndRefreshText(class ASTSEnemyCharacter* TargetEnemy)
{
    CurrentTargetEnemy = TargetEnemy;
    // 레고 조각 3개가 모두 잘 연결되었는지 확인
    if (!LeftText || !ValueText || !RightText) return;

    //  내 상태 / 적 상태 가져오기
    int32 MyStrength = 0;
    bool bAmIWeak = false;
    if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        MyStrength = GM->CurrentStrength;
        bAmIWeak = (GM->WeakStacks > 0);
    }
    bool bIsTargetVuln = (CurrentTargetEnemy && CurrentTargetEnemy->VulnerableStacks > 0);

    if (CurrentTargetEnemy)
    {
        
        UE_LOG(LogTemp, Warning, TEXT("타겟 적 이름: %s, 취약 스택: %d"), *CurrentTargetEnemy->GetName(), CurrentTargetEnemy->VulnerableStacks);

        //bool bIsTargetVuln = (CurrentTargetEnemy->VulnerableStacks > 0);
    }


    // 최종 데미지 계산
    int32 FinalDamage = CalculateFinalDamage(CurrentCardData.BaseDamage, MyStrength, bIsTargetVuln, bAmIWeak);

    // 텍스트 쪼개고 조립하기
    FString RawDesc = CurrentCardData.CardDescription.ToString();
    FString LStr, RStr;

    // 공격 카드일 때 "{Damage}"를 기준으로 양옆을 쪼갭니다
    if (RawDesc.Split(TEXT("{Damage}"), &LStr, &RStr))
    {
        // 왼쪽 텍스트 세팅 ("피해를 ")
        LeftText->SetText(FText::FromString(LStr));
        LeftText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        // 가운데 숫자 세팅 ("6" -> "4" 등)
        ValueText->SetText(FText::AsNumber(FinalDamage));
        ValueText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 1.0f, 1.0f)));
        // 데미지에 따른 색상 칠하기
        if (FinalDamage > CurrentCardData.BaseDamage) {
            // 데미지 증가 (주황색)
            ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f)));
        }
        else if (FinalDamage < CurrentCardData.BaseDamage) {
            // 약화 등으로 데미지 감소 (빨간색)
            ValueText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f)));
        }
        else {
            // 기본 상태 (하얀색)
            ValueText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }

        // 오른쪽 텍스트 세팅 (" 줍니다.")
        RightText->SetText(FText::FromString(RStr));
        RightText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
    {
        LeftText->SetText(FText::FromString(TEXT("오류: 데이터테이블에 {Damage}가 없음!")));
        // 방어 카드이거나 타겟팅 태그가 없는 경우 (왼쪽 텍스트 하나만 써서 전체 출력)
        LeftText->SetText(FText::FromString(RawDesc));
        LeftText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        ValueText->SetVisibility(ESlateVisibility::Collapsed); // 가운데 숫자 숨김
        RightText->SetVisibility(ESlateVisibility::Collapsed); // 오른쪽 텍스트 숨김
    }
}