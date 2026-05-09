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
#include "StatusEffectComponent.h"
#include "Components/PanelWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


void USTSCardWidget::UpdateCardDesign(const FCardData& Data)
{   
    CurrentCardData = Data;
    
    if (CardName)
    {
        CardName->SetText(Data.CardName); 
    }

   
   if (Cost)
    {
        Cost->SetText(FText::AsNumber(Data.Cost));
    }
   if (CardDescription)
   {
       FString FinalDesc = Data.CardDescription.ToString();

      
       FString DamageReplacement = FString::Printf(TEXT("<White>%d</>"), Data.BaseDamage);
       FinalDesc = FinalDesc.Replace(TEXT("{Damage}"), *DamageReplacement);

      
       FString BlockReplacement = FString::Printf(TEXT("<White>%d</>"), Data.BaseBlock);
       FinalDesc = FinalDesc.Replace(TEXT("{Block}"), *BlockReplacement);

     
     

      
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
        FText TypeText_KR; 
        FSlateColor TypeColor; 

        if (Data.Type == FName("Attack"))
        {
            TypeText_KR = FText::FromString(TEXT("공격"));
            
            TypeColor = FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
        }
        else if (Data.Type == FName("Skill"))
        {
            TypeText_KR = FText::FromString(TEXT("스킬"));
           
            TypeColor = FSlateColor(FLinearColor(0.2f, 0.7f, 0.4f, 1.0f));
        }
        else if (Data.Type == FName("Defend"))
        {
            TypeText_KR = FText::FromString(TEXT("방어"));
             
            TypeColor = FSlateColor(FLinearColor(0.3f, 0.5f, 0.9f, 1.0f));
        }
        else if (Data.Type == FName("Power"))
        {
            TypeText_KR = FText::FromString(TEXT("파워"));
            
            TypeColor = FSlateColor(FLinearColor(0.9f, 0.7f, 0.1f, 1.0f));
        }
        else
        {
            TypeText_KR = FText::FromString(TEXT("알 수 없음"));
            
            TypeColor = FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
        }

        
        CardType->SetText(TypeText_KR);
        CardType->SetColorAndOpacity(TypeColor); 
    }

}

//마우스를 카드 위에 올렸을 때 확대되고, ZOrder가 올라가서 다른 카드보다 위에 보이도록 합니다.
void USTSCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    
    SetRenderScale(FVector2D(1.2f, 1.2f));

    
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetZOrder(1); 
    }
}

// 마우스가 카드에서 나갔을 때 원래 크기로 돌아가고, ZOrder도 원래대로 낮춥니다.
void USTSCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    
    SetRenderScale(FVector2D(1.0f, 1.0f));

    
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
    {
        CanvasSlot->SetZOrder(0); 
    }
}

// 마우스 버튼을 눌렀을 때 드래그를 시작할지, 아니면 강화 모드에서 클릭 이벤트로 처리할지 결정합니다.
FReply USTSCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    //드래그를 감지하지 않고 클릭만 처리
    if (bIsSmithingMode)
    {
        
        // "나 강화하려고 클릭됐어!" 라고 블루프린트(WBP_TestCard)에 알려줌.
        OnCardClickedForSmithing();

        return FReply::Handled();
    }



    
    else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        
        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// 드래그가 감지되었을 때 실행되는 함수입니다. 
// UMG DragVisual이 카드 UI 충돌을 일으키는 문제를 해결하기 위해,
// 기본 드래그 비주얼은 사용하지 않고, 가짜 카드 위젯을 만들어서 드래그 비주얼로 활용합니다. 
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


// 카드의 대미지 텍스트를 계산된 대미지로 갱신하는 함수입니다. 리치 텍스트 포맷을 활용하여 숫자만 업데이트합니다.
void USTSCardWidget::UpdateDynamicDamageText(int32 CalculatedDamage)
{
    
   
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


// 드래그가 취소되었을 때 실행되는 함수입니다. 여기서 가짜 드래그 비주얼을 제거하고, 원래 카드를 다시 보이게 합니다.
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

// 카드의 최종 데미지를 계산하는 함수입니다. 
int32 USTSCardWidget::CalculateFinalDamage(int32 InBaseDamage, int32 InPlayerStrength, bool bIsTargetVulnerable, bool bIsPlayerWeak)
{
    
    float ExpectedDamage = (float)(InBaseDamage + InPlayerStrength);

    // 최소 데미지는 0으로 보정
    ExpectedDamage = FMath::Max(0.0f, ExpectedDamage);

   
    if (bIsTargetVulnerable)
    {
        ExpectedDamage = FMath::FloorToFloat(ExpectedDamage * 1.5f);
    }

    
    if (bIsPlayerWeak)
    {
        ExpectedDamage = FMath::FloorToFloat(ExpectedDamage * 0.75f);
    }

    
    return FMath::FloorToInt(ExpectedDamage);
}

// 타겟과 상태효과를 반영해 카드 설명의 데미지 텍스트를 갱신
void USTSCardWidget::UpdateTargetAndRefreshText(class ASTSEnemyCharacter* TargetEnemy)
{
    CurrentTargetEnemy = TargetEnemy;
   
    
    if (!CardDescription) return;

    
    int32 MyStrength = 0;
    bool bAmIWeak = false;
    if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        if (UStatusEffectComponent* PlayerStatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
        {
            
            int32 WeakStacks = PlayerStatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Weak);
            bAmIWeak = (WeakStacks > 0);
        }
    }
    if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        MyStrength = GM->CurrentStrength;
    }
    
    bool bIsTargetVuln = false;
    int32 VulStacks = 0; // 로그용 변수
    if (CurrentTargetEnemy)
    {
        
        
        if (UStatusEffectComponent* StatusComp = CurrentTargetEnemy->FindComponentByClass<UStatusEffectComponent>())
        {
            
            VulStacks = StatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Vulnerable);
            bIsTargetVuln = (VulStacks > 0);
        }

        

        
    }


    
    int32 FinalDamage = CalculateFinalDamage(CurrentCardData.BaseDamage, MyStrength, bIsTargetVuln, bAmIWeak);

    
    FString FinalDesc = CurrentCardData.CardDescription.ToString();

    // 데미지 증감에 따라 리치 텍스트 색상 태그 결정
    FString ColorTag = TEXT("White"); 
    if (FinalDamage > CurrentCardData.BaseDamage)
    {
        ColorTag = TEXT("Orange"); 
    }
    else if (FinalDamage < CurrentCardData.BaseDamage)
    {
        ColorTag = TEXT("Red"); 
    }

   
    FString DamageReplacement = FString::Printf(TEXT("<%s>%d</>"), *ColorTag, FinalDamage);

   
    FinalDesc = FinalDesc.Replace(TEXT("{Damage}"), *DamageReplacement);

   
    CardDescription->SetText(FText::FromString(FinalDesc));
    
}