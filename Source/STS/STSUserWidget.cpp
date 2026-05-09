
#include "STSUserWidget.h"
#include "Input/DragAndDrop.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "STSCardWidget.h"
#include "STSGameMode.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "STSEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CardDataStruct.h"
#include "Components/ProgressBar.h"
#include "DrawDebugHelpers.h"
#include "STSCharacter.h"
#include "Rendering/DrawElements.h"
#include "Components/Overlay.h"
#include "GameFramework/PlayerController.h" 
#include "Engine/World.h"

// 카드 추가 함수
void USTSUserWidget::AddCards(int32 Amount)
{
    
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

    
    if (!GM || !CardDataTable || !CardWidgetClass) return;

    for (int32 i = 0; i < Amount; ++i)
    {
        // 덱에서 카드 1장 달라고 요청
        FName DrawnCardName = GM->DrawCard();

        // 받을 카드가 없으면 뽑기 중단
        if (DrawnCardName == NAME_None)
        {
            break;
        }

        
        static const FString ContextString(TEXT("Draw Card Context"));
        
        FCardData* CardData = CardDataTable->FindRow<FCardData>(DrawnCardName, TEXT("Draw Card Context"));
        if (CardData)
        {
            
            USTSCardWidget* NewCard = CreateWidget<USTSCardWidget>(GetOwningPlayer(), CardWidgetClass);
            if (NewCard)
            {
               
                UE_LOG(LogTemp, Log, TEXT("드로우: %s"), *CardData->CardName.ToString());

                NewCard->CardRowName = DrawnCardName;

                NewCard->UpdateCardDesign(*CardData);

                
                HandAreaPanel->AddChild(NewCard);
                CreatedCards.Add(NewCard);

                
                SetInitialPositionToDeck(NewCard);
            }
        }
    }

    
    UpdateCardLayout();
}

// 카드가 추가될 때마다 손패 레이아웃을 업데이트하는 함수
void USTSUserWidget::UpdateCardLayout()
{
	int32 CreateCardsNum = CreatedCards.Num();

	float MiddleIndex = (CreateCardsNum - 1) / 2.0f;
 
    float MaxAngle = 15.0f;
	float HorizontalSpacing = 150.0f; 

    for (int32 i = 0; i < CreateCardsNum; i++)
    {
        UUserWidget* Card = CreatedCards[i];

        float Offset = i - MiddleIndex;
       
      
        FVector2D NewPosition;
		NewPosition.X = Offset * HorizontalSpacing;
	    NewPosition.Y = (FMath::Abs(Offset) * 20.0f); 

        UCanvasPanelSlot* CardSlot = Cast<UCanvasPanelSlot>(Card->Slot);
        if (!CardSlot) return;
        
        FAnchors CenterBottomAnchors(0.5f, 1.0f);
        CardSlot->SetAnchors(CenterBottomAnchors);

        float RotationAngle = Offset * 5.0f;
        CardSlot->SetAlignment(FVector2D(0.5f, 1.0f));
        Card->SetRenderTransformAngle(RotationAngle);
        
        CardSlot->SetSize(FVector2D(200.0f, 300.0f));

        
        if (USTSCardWidget* CardWidget = Cast<USTSCardWidget>(Card))
        {
            CardWidget->Anim_FlyToLocation(NewPosition, false); 
        }

       
    }
}


void USTSUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (TurnEndButton)
    {
        TurnEndButton->OnClicked.AddDynamic(this, &USTSUserWidget::OnTurnEndClicked);
    }
    
    if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        
        PlayerChar->OnPlayerDeath.AddDynamic(this, &USTSUserWidget::ShowGameOver);
    }
    
    if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
       
        GM->MainUIWidget = this;

    }
}



void USTSUserWidget::OnTurnEndClicked()
{
   
    
	
    PlayTurnEndAnimations();

}


// 손패를 모두 버리는 함수
void USTSUserWidget::EmptyHand()
{
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    
    
    for (USTSCardWidget* Card : CreatedCards)
    {
        if (Card)
        {
            
            if (GM)
            {
                GM->AddToDiscardPile(Card->CardRowName);
            }

            Card->RemoveFromParent();
        }
    }

    
    CreatedCards.Empty();

    UE_LOG(LogTemp, Log, TEXT("손패를 모두 버렸습니다."));
}

void USTSUserWidget::UpdateEnergyText(int32 CurrentEnergy, int32 MaxEnergy)
{
    if (EnergyText)
    {
       
        FString NewText = FString::Printf(TEXT("에너지: %d / %d"), CurrentEnergy, MaxEnergy);
        EnergyText->SetText(FText::FromString(NewText));
    }
}




void USTSUserWidget::UpdatePlayerHP(int32 CurrentHP, int32 MaxHP)
{
    UE_LOG(LogTemp, Warning, TEXT("UI 체력 업데이트 요청됨! - 현재: %d / 최대: %d"), CurrentHP, MaxHP);
    if (HPBar && MaxHP > 0)
    {
        float Percent = (float)CurrentHP / (float)MaxHP;
        HPBar->SetPercent(Percent);
    }
    else
    {
    }
    if (HPText)
    {
        FString HPString = FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP);
        HPText->SetText(FText::FromString(HPString));
    }
    else
    {
    }
}

void USTSUserWidget::ShowGameOver()
{
   
    if (GameOverPanel)
    {
        
        GameOverPanel->SetVisibility(ESlateVisibility::Visible);
    }
    if (CombatUIPanel)
    {
        
        CombatUIPanel->SetVisibility(ESlateVisibility::Collapsed);
    }

    // 더 이상 카드를 못 내게 클릭 막기
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

bool USTSUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    APlayerController* PC = GetOwningPlayer();
    
    bIsTargeting = false;
    HideAoEPrediction();


    USTSCardWidget* DroppedCard = Cast<USTSCardWidget>(InOperation->Payload);
    if (!DroppedCard) return false;

    FCardData CardData = DroppedCard->GetCardData();
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM) return false;

    AActor* TargetEnemy = nullptr;

    // 공격 카드일 경우: 타겟팅 검사
    if (CardData.Type == FName("Attack"))
    {
        if (CardData.bIsAoE)
        {
            UE_LOG(LogTemp, Warning, TEXT("광역 공격 카드 발동 준비!"));

            HideAoEPrediction();
        }
        else
        {
            
            if (CurrentHoveredEnemy)
            {
                TargetEnemy = CurrentHoveredEnemy; 
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("단일 공격 카드는 적을 타겟팅해야 합니다!"));
                return false; // 허공에 놨으니 공격 취소
            }
        }
    }

    // 타겟을 확정지었으니, 이제 안심하고 몬스터의 불을 끄고 변수를 초기화합니다.
    if (CurrentHoveredEnemy)
    {
        CurrentHoveredEnemy->SetTargetingHighlight(false);
        CurrentHoveredEnemy->ClearPredictionUI();
        CurrentHoveredEnemy = nullptr;
    }


    // 에너지 결제
    if (!GM->TryUseEnergy(CardData.Cost)) return false;
    UpdateEnergyText(GM->CurrentEnergy, GM->MaxEnergy);
   

    
    ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(PC->GetPawn());
    if (!PlayerChar)
    {
        
        return false;
    }
    FActionCommand NewCmd;
    NewCmd.ActionType = CardData.Type;  
    NewCmd.bIsAoE = CardData.bIsAoE;
    NewCmd.TargetEnemy = TargetEnemy;
    NewCmd.Montage = CardData.CardMontage.LoadSynchronous();
    NewCmd.VFX = CardData.CardVFX.LoadSynchronous();
    NewCmd.Damage = 0; 

    // 카드 종류에 따라 로직 처리
    if (CardData.Type == FName("Attack"))
    {
        
        int32 DamageWithStrength = CardData.BaseDamage + (GM ? GM->CurrentStrength : 0);
        float OutgoingDamage = (float)DamageWithStrength;

        
        if (GM && GM->WeakStacks > 0)
        {
            OutgoingDamage *= 0.75f;
        }

        int32 FinalPlayerDamage = FMath::FloorToInt(OutgoingDamage);

       
        NewCmd.Damage = FinalPlayerDamage;

        
        NewCmd.StatusType = CardData.StatusType;    
        NewCmd.StatusAmount = CardData.StatusAmount; 

       
        PlayerChar->EnqueueAction(NewCmd);
        
    }
    else if (CardData.Type == FName("Defend"))
    {
        
        PlayerChar->AddBlock(CardData.BaseBlock);

       
        PlayerChar->EnqueueAction(NewCmd);

        if (CardData.DrawAmount > 0) AddCards(CardData.DrawAmount);
    }
    else if (CardData.Type == FName("Power"))
    {
        
        if (CardData.StatusType == FName("STRENGTH") && GM)
            GM->AddStrength(CardData.StatusAmount);
        PlayerChar->EnqueueAction(NewCmd);

    }
    else if (CardData.Type == FName("Skill"))
    {
        
        if (CardData.BaseBlock > 0) PlayerChar->AddBlock(CardData.BaseBlock);
        if (CardData.StatusAmount > 0 && CardData.StatusType == FName("ENERGY") && GM)
        {
            GM->CurrentEnergy += CardData.StatusAmount;
            UpdateEnergyText(GM->CurrentEnergy, GM->MaxEnergy);
        }
        if (CardData.DrawAmount > 0) AddCards(CardData.DrawAmount);
        PlayerChar->EnqueueAction(NewCmd);
    }

    
    GM->AddToDiscardPile(DroppedCard->CardRowName);

    // 뷰포트에 떠있는 가짜 카드 있으면 제거
    if (USTSCardWidget* FakeCard = DroppedCard->GetFakeDragVisual())
    {
        FakeCard->RemoveFromParent();
    }

    DroppedCard->RemoveFromParent();
    CreatedCards.Remove(DroppedCard);
    UpdateCardLayout();

    return true;
}

void USTSUserWidget::ShowVictory()
{
   

    if (VictoryPanel)
    {
        VictoryPanel->SetVisibility(ESlateVisibility::Visible);


        // UI가 켜지자마자 블루프린트에게 카드 3장 뽑으라고 명령
        InitVictoryRewards();
    }
    else
    {
        
    }

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    OnCombatEnded();
}


bool USTSUserWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
    CurrentDragOp = InOperation;
    DraggedCard = Cast<USTSCardWidget>(InOperation->Payload);
    if (!DraggedCard) return true;

    FCardData CardData = DraggedCard->GetCardData();

    
    // 드래그 비주얼에서 실제 카드 위젯 꺼내기
    
    USTSCardWidget* VisualCard = nullptr;
    if (DraggedCard)
    {
        // 원본 카드가 뷰포트에 띄워둔 가짜 카드를 직접 가져옵니다.
        VisualCard = DraggedCard->GetFakeDragVisual();
    }

    
   // 단일 공격 로직
    if (CardData.Type == FName("Attack") && !CardData.bIsAoE)
    {
        bIsTargeting = true;
        CurrentDragScreenPos = InDragDropEvent.GetScreenSpacePosition();
        FVector2D MouseLocalPos = InGeometry.AbsoluteToLocal(CurrentDragScreenPos);

        ASTSEnemyCharacter* BestTarget = nullptr;
        APlayerController* PC = GetOwningPlayer();
        float MinDistance = 400.0f;

        if (PC)
        {
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("Enemy"), FoundEnemies);

            for (AActor* Actor : FoundEnemies)
            {
                ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor);
                if (Enemy && Enemy->CurrentHealth > 0)
                {
                    FVector TargetLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
                    FVector2D EnemyScreenPos;

                    if (PC->ProjectWorldLocationToScreen(TargetLocation, EnemyScreenPos))
                    {
                        FVector2D EnemyLocalPos;
                        USlateBlueprintLibrary::ScreenToWidgetLocal(this, InGeometry, EnemyScreenPos, EnemyLocalPos);

                        float Dist = FVector2D::Distance(MouseLocalPos, EnemyLocalPos);
                        if (Dist < MinDistance)
                        {
                            MinDistance = Dist;
                            BestTarget = Enemy;
                        }
                    }
                }
            }
        }

        int32 ExpectedPlayerDamage = CardData.BaseDamage;
        if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            ExpectedPlayerDamage += GM->CurrentStrength;
            if (GM->WeakStacks > 0)
            {
                ExpectedPlayerDamage = FMath::FloorToInt(ExpectedPlayerDamage * 0.75f);
            }
        }

        if (CurrentHoveredEnemy != BestTarget)
        {
            if (CurrentHoveredEnemy)
            {
                CurrentHoveredEnemy->SetTargetingHighlight(false);
                CurrentHoveredEnemy->ClearPredictionUI();
            }

            CurrentHoveredEnemy = BestTarget;

            if (CurrentHoveredEnemy)
            {
                CurrentHoveredEnemy->SetTargetingHighlight(true);
                CurrentHoveredEnemy->UpdatePredictionUI(ExpectedPlayerDamage, CardData.HitCount);
            }

            if (VisualCard)
            {
                VisualCard->UpdateTargetAndRefreshText(CurrentHoveredEnemy);
            }
        }
    }

    
    //  광역 공격(AoE) 로직 
   
    else if (CardData.Type == FName("Attack") && CardData.bIsAoE)
    {
        bIsTargeting = false;

        int32 ExpectedPlayerDamage = CardData.BaseDamage;
        if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            ExpectedPlayerDamage += GM->CurrentStrength;
            if (GM->WeakStacks > 0)
            {
                ExpectedPlayerDamage = FMath::FloorToInt(ExpectedPlayerDamage * 0.75f);
            }
        }

        TArray<AActor*> FoundEnemies;
        UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("Enemy"), FoundEnemies);

        for (AActor* Actor : FoundEnemies)
        {
            if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
            {
                if (Enemy->CurrentHealth > 0)
                {
                    Enemy->SetTargetingHighlight(true);
                    Enemy->UpdatePredictionUI(ExpectedPlayerDamage, CardData.HitCount);
                }
            }
        }

        if (VisualCard)
        {
            VisualCard->UpdateTargetAndRefreshText(nullptr);
        }
    }

   
    // 타겟팅 취소 로직 
    
    else
    {
        bIsTargeting = false;

        if (CurrentHoveredEnemy)
        {
            CurrentHoveredEnemy->SetTargetingHighlight(false);
            CurrentHoveredEnemy->ClearPredictionUI();
            CurrentHoveredEnemy = nullptr;

            if (VisualCard)
            {
                VisualCard->UpdateTargetAndRefreshText(nullptr);
            }
        }
    }



    
	// 드래그 중인 카드의 가짜 비주얼이 있다면, 그 위치를 타겟(몬스터 또는 마우스)으로 이동시킵니다.
    if (DraggedCard && DraggedCard->GetFakeDragVisual())
    {
        FVector2D TargetViewportPos;

        
        FVector2D CardSize = DraggedCard->GetFakeDragVisual()->GetDesiredSize();
        if (CardSize.X == 0 || CardSize.Y == 0) CardSize = FVector2D(200.0f, 300.0f);

        // 타겟팅 유무에 따라 카드를 띄울 추가 오프셋
        FVector2D HoverOffset = FVector2D(0.0f, 0.0f);

        if (CurrentHoveredEnemy)
        {
            // 타겟팅 중: 몬스터(Endpoint)를 추적
            FVector TargetLoc = CurrentHoveredEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

            // 3D 월드 좌표를 UMG 전용 뷰포트 좌표로 한 방에 변환
            bool bSuccess = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), TargetLoc, TargetViewportPos, false);

            // 카메라 각도 등의 이유로 변환에 실패하면, 안전하게 마우스 좌표를 사용
            if (!bSuccess)
            {
                FVector2D PixelPos;
                USlateBlueprintLibrary::AbsoluteToViewport(this, InDragDropEvent.GetScreenSpacePosition(), PixelPos, TargetViewportPos);
            }

            // 몬스터 체력바를 가리지 않도록 카드를 화살표 끝보다 살짝 아래로 내립니다.
            HoverOffset = FVector2D(0.0f, 150.0f);
        }
        else
        {
            // 허공 드래그 중: 마우스를 추적
            FVector2D PixelPos;
            USlateBlueprintLibrary::AbsoluteToViewport(this, InDragDropEvent.GetScreenSpacePosition(), PixelPos, TargetViewportPos);

            // 허공일 때는 마우스 정중앙에 두기 위해 오프셋을 0으로 둡니다.
            HoverOffset = FVector2D(0.0f, 0.0f);
        }

        
        // 최종 적용: 타겟 좌표 - (카드 크기 절반) + (가림 방지용 오프셋)
        // false를 넣어 엔진이 좌표를 이중으로 나누는 것을 막습니다.
        
        FVector2D FinalPos = TargetViewportPos - (CardSize * 0.5f) + HoverOffset;
        DraggedCard->GetFakeDragVisual()->SetPositionInViewport(FinalPos, false);
    }

    return true;
}

void USTSUserWidget::ClearHandUI()
{
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

    for (USTSCardWidget* Card : CreatedCards)
    {
        if (Card)
        {
            
            if (GM)
            {
                GM->DrawPile.Add(Card->CardRowName);
            }
            
            Card->RemoveFromParent();
        }
    }

   
    CreatedCards.Empty();
}

void USTSUserWidget::ApplyRewardCardsData(TArray<FName> RewardNames)
{
   
    if (!CardDataTable) return;

    // 1번 카드 세팅
    if (RewardNames.IsValidIndex(0) && RewardCard1)
    {
        FCardData* Data1 = CardDataTable->FindRow<FCardData>(RewardNames[0], TEXT("Reward1"));
        if (Data1)
        {
            RewardCard1->CardRowName = RewardNames[0]; 
            RewardCard1->UpdateCardDesign(*Data1);     
        }
    }

    // 2번 카드 세팅
    if (RewardNames.IsValidIndex(1) && RewardCard2)
    {
        FCardData* Data2 = CardDataTable->FindRow<FCardData>(RewardNames[1], TEXT("Reward2"));
        if (Data2)
        {
            RewardCard2->CardRowName = RewardNames[1];
            RewardCard2->UpdateCardDesign(*Data2);
        }
    }

    // 3번 카드 세팅
    if (RewardNames.IsValidIndex(2) && RewardCard3)
    {
        FCardData* Data3 = CardDataTable->FindRow<FCardData>(RewardNames[2], TEXT("Reward3"));
        if (Data3)
        {
            RewardCard3->CardRowName = RewardNames[2];
            RewardCard3->UpdateCardDesign(*Data3);
        }
    }
}

void USTSUserWidget::ShowGameClear()
{
    UE_LOG(LogTemp, Warning, TEXT("보스 처치! 게임 클리어 화면 출력!"));

    if (GameClearPanel)
    {
        GameClearPanel->SetVisibility(ESlateVisibility::Visible);
    }

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    OnCombatEnded();
}

AActor* USTSUserWidget::GetEnemyUnderCursor(FVector2D ScreenPos)
{
    FVector WorldLoc, WorldDir;
    APlayerController* PC = GetOwningPlayer();

    if (PC && UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, WorldLoc, WorldDir))
    {
        FVector End = WorldLoc + (WorldDir * 10000.0f);
        FHitResult HitResult;

        if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, End, ECC_Visibility))
        {
            if (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("Enemy")))
            {
                return HitResult.GetActor(); 
            }
        }
    }
    return nullptr; 
}


int32 USTSUserWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    // 부모 함수의 기본 Paint 로직 실행
    int32 NextLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    if (bIsTargeting)
    {
        
        
        
        
        FVector2D StartPoint;
        
        if (DraggedCard)
        {
            // 투명해진 원본 카드의 렌더링/위치 정보(Geometry)를 가져옵니다.
            FGeometry CardGeometry = DraggedCard->GetCachedGeometry();

           
            FVector2D CardLocalCenter = CardGeometry.GetLocalSize() * 0.5f;

            
            FVector2D AbsoluteCardCenter = CardGeometry.LocalToAbsolute(CardLocalCenter);

           
            StartPoint = AllottedGeometry.AbsoluteToLocal(AbsoluteCardCenter);
        }
        else
        {
           
            StartPoint = FVector2D(AllottedGeometry.GetLocalSize().X * 0.5f, AllottedGeometry.GetLocalSize().Y * 0.9f);
        }
        
        
        
        FVector2D EndPoint;
        if (CurrentHoveredEnemy)
        {
            
            FVector2D EnemyScreenPos;

            // 몬스터의 발밑이 아닌 몸통을 향하도록 Z축을 올려줌
            FVector TargetLocation = CurrentHoveredEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

            if (GetOwningPlayer() && GetOwningPlayer()->ProjectWorldLocationToScreen(TargetLocation, EnemyScreenPos))
            {
                USlateBlueprintLibrary::ScreenToWidgetLocal((UObject*)this, AllottedGeometry, EnemyScreenPos, EndPoint);
            }
        }
        else
        {
            // 타겟팅된 적이 없다면 마우스 커서를 따라감
            EndPoint = AllottedGeometry.AbsoluteToLocal(CurrentDragScreenPos);
        }

        
        // 베지어 곡선 (Bezier Curve) 그리기
        
        TArray<FVector2D> Points;

        
        FVector2D ControlPoint1 = StartPoint + FVector2D(0.0f, -800.0f);
        FVector2D ControlPoint2 = EndPoint + FVector2D(0.0f, -800.0f);

        int32 Segments = 30;
        for (int32 i = 0; i <= Segments; i++)
        {
            float t = (float)i / Segments;
            float u = 1.0f - t;

            FVector2D Point = (u * u * u) * StartPoint +
                3 * (u * u) * t * ControlPoint1 +
                3 * u * (t * t) * ControlPoint2 +
                (t * t * t) * EndPoint;

            Points.Add(Point);
        }

        FSlateDrawElement::MakeLines(
            OutDrawElements, NextLayerId + 100, AllottedGeometry.ToPaintGeometry(),
            Points, ESlateDrawEffect::None, LineColor, true, LineThickness
        );

        
        // 화살표 머리 (Arrow Head): 도착점(EndPoint)에, 적을 향해 그리기
        
        if (Points.Num() >= 2)
        {
            
            FVector2D Dir = (EndPoint - Points[Points.Num() - 2]).GetSafeNormal();
            FVector2D RightDir = FVector2D(-Dir.Y, Dir.X);

            TArray<FVector2D> ArrowPoints;
           
            ArrowPoints.Add(EndPoint);
            ArrowPoints.Add(EndPoint - (Dir * ArrowHeadSize) + (RightDir * ArrowHeadSize * 0.5f));
            ArrowPoints.Add(EndPoint - (Dir * ArrowHeadSize) - (RightDir * ArrowHeadSize * 0.5f));
            ArrowPoints.Add(EndPoint); 

            FSlateDrawElement::MakeLines(
                OutDrawElements, NextLayerId + 100, AllottedGeometry.ToPaintGeometry(),
                ArrowPoints, ESlateDrawEffect::None, LineColor, true, LineThickness
            );
        }
    }

    return NextLayerId;
}

// 드래그 리브: 마우스가 위젯 밖으로 나가면 OFF
void USTSUserWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // 타겟팅 중이던 몬스터의 불을 끄고 초기화
    if (CurrentHoveredEnemy)
    {
        CurrentHoveredEnemy->SetTargetingHighlight(false);
        CurrentHoveredEnemy->ClearPredictionUI();
        CurrentHoveredEnemy = nullptr;
    }

    Super::NativeOnDragLeave(InDragDropEvent, InOperation);
    bIsTargeting = false;
    CurrentHoveredEnemy = nullptr;
    DraggedCard = nullptr;
}

void USTSUserWidget::ShowAoEPrediction(int32 FinalDamage) 
{
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), AllEnemies);

    for (AActor* EnemyActor : AllEnemies)
    {
        if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(EnemyActor))
        {
            

            
            Enemy->UpdatePredictionUI(FinalDamage);
        }
    }
}

// 광역 공격 예측 UI를 모두 끄는 함수
void USTSUserWidget::HideAoEPrediction()
{
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), AllEnemies);

    for (AActor* EnemyActor : AllEnemies)
    {
        if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(EnemyActor))
        {
            
            Enemy->ClearPredictionUI();
        }
    }
}

void USTSUserWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

    DraggedCard = Cast<USTSCardWidget>(InOperation->Payload);
    if (!DraggedCard) return;

    FCardData CardData = DraggedCard->GetCardData();

    // 광역 공격 카드라면 드래그 시작 즉시 모든 적에게 예측 UI를 띄우라고 명령
    if (CardData.Type == FName("Attack") && CardData.bIsAoE)
    {
        ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
        int32 FinalDamage = CardData.BaseDamage + (GM ? GM->CurrentStrength : 0);

        
        ShowAoEPrediction(FinalDamage);

    }
}

void USTSUserWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    // 카드를 쓰지 않고 취소했을 때도 끄기
    HideAoEPrediction();

    
    if (CurrentHoveredEnemy)
    {
        CurrentHoveredEnemy->SetTargetingHighlight(false);
        CurrentHoveredEnemy->ClearPredictionUI();
        CurrentHoveredEnemy = nullptr;
    }

    bIsTargeting = false;
    DraggedCard = nullptr;

}

