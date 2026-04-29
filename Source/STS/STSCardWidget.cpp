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
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/PanelWidget.h"
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
   if (CardDescription)
   {
       FString FinalDesc = Data.CardDescription.ToString();

       // 공격 카드라면 {Damage}를 <White>숫자</>로 변경
       FString DamageReplacement = FString::Printf(TEXT("<White>%d</>"), Data.BaseDamage);
       FinalDesc = FinalDesc.Replace(TEXT("{Damage}"), *DamageReplacement);

       // 방어 카드라면 {Block}을 <White>숫자</>로 변경
       FString BlockReplacement = FString::Printf(TEXT("<White>%d</>"), Data.BaseBlock);
       FinalDesc = FinalDesc.Replace(TEXT("{Block}"), *BlockReplacement);

     
     

       // 최종 적용
       CardDescription->SetText(FText::FromString(FinalDesc));
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

    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
    DragOp->Payload = this;

    USTSCardWidget* DragVisualWidget = CreateWidget<USTSCardWidget>(GetOwningPlayer(), GetClass());

    if (DragVisualWidget)
    {
        DragVisualWidget->UpdateCardDesign(CurrentCardData);
        DragVisualWidget->SetRenderTransformAngle(0.0f);
        DragVisualWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

        // 뷰포트에 띄우고 크기 강제 고정
        //DragVisualWidget->AddToViewport(999);
        DragVisualWidget->AddToPlayerScreen(999);
        FVector2D FixedSize = FVector2D(200.0f, 300.0f);
        DragVisualWidget->SetDesiredSizeInViewport(FixedSize);

        // 만든 가짜 위젯을 멤버 변수에 저장해둡니다.
        FakeDragVisual = DragVisualWidget;

        
        //복잡한 나눗셈 다 버리고, 엔진 내장 변환 함수를 씁니다.
        FVector2D PixelPos, ViewportPos;
        USlateBlueprintLibrary::AbsoluteToViewport(this, InMouseEvent.GetScreenSpacePosition(), PixelPos, ViewportPos);

        // 카드 크기의 절반을 빼서 정중앙에 정확히 꽂습니다.
        FVector2D CardSize = FVector2D(200.0f, 300.0f);
        FakeDragVisual->SetPositionInViewport(ViewportPos - (CardSize * 0.5f), false);

       
        DragOp->DefaultDragVisual = nullptr;
    }

    DragOp->Pivot = EDragPivot::CenterCenter;
    OutOperation = DragOp;

    this->SetRenderOpacity(0.0f);
    this->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}



void USTSCardWidget::UpdateDynamicDamageText(int32 CalculatedDamage)
{
    // 리치 텍스트 포맷을 사용하여 대미지 숫자만 갱신
    // "피해를 <Red>9</> 줍니다." 처럼 색상 태그를 넣으면 더 직관적입니다.
    FString NewDesc = FString::Printf(TEXT("피해를 <%s>%d</> 줍니다."), TEXT("Default"), CalculatedDamage);
    CardDescription->SetText(FText::FromString(NewDesc));
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


    // 드래그가 취소되면 화면에 떠있던 가짜 카드를 파괴합니다.
    if (FakeDragVisual)
    {
        FakeDragVisual->RemoveFromParent();
        FakeDragVisual = nullptr;
    }

    // 숨겼던 원본 카드를 다시 보이게 (손패 원복)
    this->SetRenderOpacity(1.0f);
    this->SetVisibility(ESlateVisibility::Visible);
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
    // 리치 텍스트 블록이 바인딩 안 되었으면 리턴
    if (!CardDescription) return;

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

    // 원본 설명 텍스트 가져오기 (예: "피해를 {Damage} 줍니다.")
    FString FinalDesc = CurrentCardData.CardDescription.ToString();

    // 데미지 증감에 따라 리치 텍스트 색상 태그 결정
    FString ColorTag = TEXT("White"); // 기본 하얀색
    if (FinalDamage > CurrentCardData.BaseDamage)
    {
        ColorTag = TEXT("Orange"); // 버프: 주황색
    }
    else if (FinalDamage < CurrentCardData.BaseDamage)
    {
        ColorTag = TEXT("Red"); // 디버프: 빨간색
    }

    // 변경할 문자열 만들기 (예: "<Orange>8</>")
    FString DamageReplacement = FString::Printf(TEXT("<%s>%d</>"), *ColorTag, FinalDamage);

    // "{Damage}" 글자를 방금 만든 색상 숫자로 통째로 바꿔치기
    FinalDesc = FinalDesc.Replace(TEXT("{Damage}"), *DamageReplacement);

    // 리치 텍스트 위젯에 한 방에 적용!
    CardDescription->SetText(FText::FromString(FinalDesc));
    
}