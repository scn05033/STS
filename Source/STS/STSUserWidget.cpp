
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
#include "Blueprint/SlateBlueprintLibrary.h"
#include "STSEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CardDataStruct.h"
#include "Components/ProgressBar.h"
#include "DrawDebugHelpers.h"
#include "STSCharacter.h"
#include "Rendering/DrawElements.h"
#include "GameFramework/PlayerController.h" 
#include "Engine/World.h"

void USTSUserWidget::AddCards(int32 Amount)
{
    
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

    // 필수 데이터가 없으면 중단
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

        // 받은 이름표(RowName)로 데이터테이블에서 실제 카드 데이터 찾기
        static const FString ContextString(TEXT("Draw Card Context"));
        
        FCardData* CardData = CardDataTable->FindRow<FCardData>(DrawnCardName, TEXT("Draw Card Context"));
        if (CardData)
        {
            // 카드 위젯 생성 및 데이터 주입 (기존과 동일)
            USTSCardWidget* NewCard = CreateWidget<USTSCardWidget>(GetOwningPlayer(), CardWidgetClass);
            if (NewCard)
            {
                // 방금 뽑은 카드가 뭔지 로그로 확인
                UE_LOG(LogTemp, Log, TEXT("드로우: %s"), *CardData->CardName.ToString());

                NewCard->CardRowName = DrawnCardName;

                NewCard->UpdateCardDesign(*CardData);

                // 손패 패널에 추가하고 배열에 관리
                HandAreaPanel->AddChild(NewCard);
                CreatedCards.Add(NewCard);
            }
        }
    }

    // 손패 예쁘게 정렬
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

void USTSUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (TurnEndButton)
    {
        TurnEndButton->OnClicked.AddDynamic(this, &USTSUserWidget::OnTurnEndClicked);
    }

    // 게임 시작 시 바로 전투 시작을 알림 (테스트용)
    if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->StartCombat(this);
    }
}

void USTSUserWidget::OnTurnEndClicked()
{
    // 게임 모드를 찾아서 "턴 끝낼래요"라고 보고
    if (ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->EndPlayerTurn();
    }
}

void USTSUserWidget::EmptyHand()
{
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    
    //화면에 있는 모든 카드를 순회
    for (USTSCardWidget* Card : CreatedCards)
    {
        if (Card)
        {
            // 카드를 화면에서 지우기 전에 무덤으로 보냄!
            if (GM)
            {
                GM->AddToDiscardPile(Card->CardRowName);
            }

            Card->RemoveFromParent();
        }
    }

    // 관리 목록(배열) 비우기
    CreatedCards.Empty();

    UE_LOG(LogTemp, Log, TEXT("손패를 모두 버렸습니다."));
}

void USTSUserWidget::UpdateEnergyText(int32 CurrentEnergy, int32 MaxEnergy)
{
    if (EnergyText)
    {
        // "에너지: 2 / 3" 형태로 텍스트 변경
        FString NewText = FString::Printf(TEXT("에너지: %d / %d"), CurrentEnergy, MaxEnergy);
        EnergyText->SetText(FText::FromString(NewText));
    }
}

void USTSUserWidget::UpdateBlockText(int32 CurrentBlock)
{
    if (BlockText)
    {
        // 방어도가 0이면 숨기거나 "방어도: 0"으로 표시
        FString NewText = FString::Printf(TEXT("방어도: %d"), CurrentBlock);
        BlockText->SetText(FText::FromString(NewText));
    }
}



void USTSUserWidget::UpdatePlayerHP(int32 CurrentHP, int32 MaxHP)
{
    UE_LOG(LogTemp, Warning, TEXT("UI 체력 업데이트 요청됨! - 현재: %d / 최대: %d"), CurrentHP, MaxHP);
    if (PlayerHPBar && MaxHP > 0)
    {
        float Percent = (float)CurrentHP / (float)MaxHP;
        PlayerHPBar->SetPercent(Percent);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHPBar 위젯을 찾을 수 없습니다! (이름이나 변수 체크 확인)"));
    }
    if (PlayerHPText)
    {
        FString HPString = FString::Printf(TEXT("%d / %d"), CurrentHP, MaxHP);
        PlayerHPText->SetText(FText::FromString(HPString));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHPText 위젯을 찾을 수 없습니다!"));
    }
}

void USTSUserWidget::ShowGameOver()
{
    if (GameOverPanel)
    {
        // 숨겨뒀던 게임 오버 화면을 띄움
        GameOverPanel->SetVisibility(ESlateVisibility::Visible);
    }

    // 더 이상 카드를 못 내게 클릭 막기
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

bool USTSUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    APlayerController* PC = GetOwningPlayer();
    // 화살표 선 긋기 끄기
    bIsTargeting = false;

    UE_LOG(LogTemp, Warning, TEXT("==== [디버그] NativeOnDrop 이 실행되었습니다! ===="));

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
        }
        else
        {
            // 자석 타겟팅 핵심: 레이저를 쏠 필요 없이 DragOver에서 잡아둔 녀석을 그대로 씁니다!
            if (CurrentHoveredEnemy)
            {
                TargetEnemy = CurrentHoveredEnemy; // 타겟 확정!
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
        CurrentHoveredEnemy = nullptr;
    }


    // 에너지 결제
    if (!GM->TryUseEnergy(CardData.Cost)) return false;
    UpdateEnergyText(GM->CurrentEnergy, GM->MaxEnergy);

    // 카드 효과 발동!
    if (CardData.Type == FName("Attack"))
    {
        // [힘(Strength)] 기본 데미지 + 내 힘
        int32 DamageWithStrength = CardData.BaseDamage;
        if (GM) { DamageWithStrength += GM->CurrentStrength; }

        // [약화(Weak)] 데미지 감소
        int32 FinalPlayerDamage = DamageWithStrength;
        if (GM && GM->WeakStacks > 0)
        {
            FinalPlayerDamage = FMath::FloorToInt(DamageWithStrength * 0.75f);
            UE_LOG(LogTemp, Warning, TEXT("[약화 발동] 힘 적용 데미지 %d 가 %d 로 감소!"), DamageWithStrength, FinalPlayerDamage);
        }

        // [단일 공격일 때]
        if (!CardData.bIsAoE && TargetEnemy)
        {
            if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(PC->GetPawn()))
            {
                // 캐릭터에게 '데미지'와 '타겟(C++)'을 완벽하게 기억시킵니
                PlayerChar->PendingDamage = FinalPlayerDamage;
                PlayerChar->CurrentTarget = TargetEnemy;

				// 돌진해서 공격하는 애니메이션 재생 (데미지 적용은 애니메이션 노티파이에서)   
                PlayerChar->DashAndAttack(TargetEnemy);
            }

            // [상태이상 부여] 
            if (CardData.StatusAmount > 0 && CardData.StatusType == FName("Vulnerable"))
            {
                if (ASTSEnemyCharacter* EnemyChar = Cast<ASTSEnemyCharacter>(TargetEnemy))
                {
                    EnemyChar->VulnerableStacks += CardData.StatusAmount;
                }
            }
        }
        // [광역 공격일 때] 
        else if (CardData.bIsAoE)
        {
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("CurrentBattle"), FoundEnemies);

            for (AActor* Actor : FoundEnemies)
            {
                if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
                {
                    // 광역은 당장 애니메이션 세팅이 안 되어 있으니 예전처럼 즉시 데미지 처리
                    UGameplayStatics::ApplyDamage(Enemy, FinalPlayerDamage, PC, PC->GetPawn(), UDamageType::StaticClass());
                }
            }
        }
    }
    // 수비 카드일 때
    else if (CardData.Type == FName("Defend"))
    {
        GM->AddBlock(CardData.BaseBlock);
        UE_LOG(LogTemp, Warning, TEXT("방어도 증가: %d"), CardData.BaseBlock);
        if (CardData.DrawAmount > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("카드 드로우 효과 발동! %d 장 드로우"), CardData.DrawAmount);
            AddCards(CardData.DrawAmount);
		}
    }
    else if (CardData.Type == FName("Power"))
    {
        // 엑셀에 적어둔 StatusType이 "STRENGTH" 라면?
        if (CardData.StatusType == FName("STRENGTH"))
        {
            // 플레이어의 힘을 StatusAmount(2) 만큼 올려줍니다!
            GM->AddStrength(CardData.StatusAmount);
        }
    }
    else if (CardData.Type == FName("Skill"))
    {
        // 방어도(Block) 획득 로직이 있다면 
        if (CardData.BaseBlock > 0 && GM)
        {
            // 예: GM->CurrentBlock += CardData.BaseBlock;
            UE_LOG(LogTemp, Warning, TEXT("[방어] 방어도를 %d 얻었습니다!"), CardData.BaseBlock);
        }

        // StatusType이 ENERGY 라면 에너지 펌핑
        if (CardData.StatusAmount > 0 && CardData.StatusType == FName("ENERGY"))
        {
            if (GM)
            {
                // 플레이어의 현재 에너지에 StatusAmount(2)를 추가
                GM->CurrentEnergy += CardData.StatusAmount;

                UE_LOG(LogTemp, Warning, TEXT("[에너지 펌핑] 에너지를 %d 얻었습니다! 현재 에너지: %d"), CardData.StatusAmount, GM->CurrentEnergy);

                UpdateEnergyText(GM->CurrentEnergy, GM->MaxEnergy);
            }
        }

        //  드로우 효과가 있다면 여기서 처리!
        if (CardData.DrawAmount > 0)
        {
            // DrawCards(CardData.DrawAmount);
        }
    }

    // 카드 버리기
    GM->AddToDiscardPile(DroppedCard->CardRowName);
    DroppedCard->RemoveFromParent();
    CreatedCards.Remove(DroppedCard);
    UpdateCardLayout();

    return true;
}

void USTSUserWidget::ShowVictory()
{
    // UI가 호출되었는지 확인하는 로그
    UE_LOG(LogTemp, Warning, TEXT("UI: 승리 화면 출력 요청됨!"));

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

    USTSCardWidget* DraggedCard = Cast<USTSCardWidget>(InOperation->Payload);
    // 마우스의 절대 좌표를 로컬 좌표로 변환합니다.
    FVector2D MouseLocalPos = InGeometry.AbsoluteToLocal(CurrentDragScreenPos);
    if (!DraggedCard) return true;

    FCardData CardData = DraggedCard->GetCardData();

    if (CardData.Type == FName("Attack") && !CardData.bIsAoE)
    {
        bIsTargeting = true;
        CurrentDragScreenPos = InDragDropEvent.GetScreenSpacePosition();

        ASTSEnemyCharacter* BestTarget = nullptr;
        APlayerController* PC = GetOwningPlayer();

        // 자석 반경 설정 (이 픽셀 반경 안에 들어와야 타겟팅 됨. 너무 크면 화면 끝에서도 타겟팅 됩니다)
        float MinDistance = 400.0f;

        if (PC)
        {
            // 모든 적을 가져옵니다. 
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("Enemy"), FoundEnemies);

            for (AActor* Actor : FoundEnemies)
            {
                ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor);
                if (Enemy && Enemy->CurrentHealth > 0) // 살아있는 적만 검사
                {
                    FVector2D EnemyScreenPos;
                    // 적의 3D 위치를 2D 화면(모니터) 좌표로 변환합니다
                    // (발끝이 아닌 가슴/머리 쪽을 타겟팅하도록 Z축으로 +100 정도 올려주면 좋습니다)
                    FVector TargetLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

                    if (PC->ProjectWorldLocationToScreen(TargetLocation, EnemyScreenPos))
                    {
                        //몬스터의 창 기준 픽셀 좌표를 UI 로컬 좌표로 변환합니다
                        FVector2D EnemyLocalPos;
                        USlateBlueprintLibrary::ScreenToWidgetLocal(this, InGeometry, EnemyScreenPos, EnemyLocalPos);

                        // 둘 다 로컬 좌표로 통일되었으니, 이제 거리가 100% 정확하게 계산됩니다.
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

        // 기존 하이라이트 교체 로직 (HoveredEnemy 대신 BestTarget 사용)
        if (CurrentHoveredEnemy != BestTarget)
        {
            if (CurrentHoveredEnemy) CurrentHoveredEnemy->SetTargetingHighlight(false);
            CurrentHoveredEnemy = BestTarget;
            if (CurrentHoveredEnemy) CurrentHoveredEnemy->SetTargetingHighlight(true);
        }
    }
    else
    {
        bIsTargeting = false;
        if (CurrentHoveredEnemy)
        {
            CurrentHoveredEnemy->SetTargetingHighlight(false);
            CurrentHoveredEnemy = nullptr;
        }
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
            // 남은 카드의 이름을 다시 덱(DrawPile)에 집어넣습니다.
            if (GM)
            {
                GM->DrawPile.Add(Card->CardRowName);
            }
            // 화면에서 카드를 삭제합니다.
            Card->RemoveFromParent();
        }
    }

    // 관리 목록을 비워줍니다.
    CreatedCards.Empty();
}

void USTSUserWidget::ApplyRewardCardsData(TArray<FName> RewardNames)
{
    // 데이터테이블(엑셀)이 없으면 중단
    if (!CardDataTable) return;

    // 1번 카드 세팅
    if (RewardNames.IsValidIndex(0) && RewardCard1)
    {
        FCardData* Data1 = CardDataTable->FindRow<FCardData>(RewardNames[0], TEXT("Reward1"));
        if (Data1)
        {
            RewardCard1->CardRowName = RewardNames[0]; // 클릭할 때를 대비해 이름표 달아주기
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
                return HitResult.GetActor(); // 적을 찾으면 반환
            }
        }
    }
    return nullptr; // 못 찾으면 널 반환
}

int32 USTSUserWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    // 부모 함수의 기본 Paint 로직을 먼저 실행합니다.
    int32 NextLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    if (bIsTargeting)
    {
        // 시작점과 끝점
        FVector2D StartPoint = FVector2D(AllottedGeometry.GetLocalSize().X * 0.5f, AllottedGeometry.GetLocalSize().Y * 0.9f);
       // FVector2D EndPoint = AllottedGeometry.AbsoluteToLocal(CurrentDragScreenPos);
        FVector2D EndPoint;

        //  자석 타겟팅 시각화: 누구를 따라갈 것인가?
        if (CurrentHoveredEnemy)
        {
            // 타겟팅된 적이 있다면? -> 화살표 끝을 몬스터의 몸통에 강제로 고정
            FVector2D EnemyScreenPos;

            // 몬스터의 발끝이 아닌 가슴팍 쪽에 꽂히도록 Z축으로 100만큼 올려줍니다.
            FVector TargetLocation = CurrentHoveredEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 0.0f);

            if (GetOwningPlayer() && GetOwningPlayer()->ProjectWorldLocationToScreen(TargetLocation, EnemyScreenPos))
            {
                // 몬스터의 화면 2D 좌표를 도화지 로컬 좌표로 변환
               //EndPoint = AllottedGeometry.AbsoluteToLocal(EnemyScreenPos);
                USlateBlueprintLibrary::ScreenToWidgetLocal((UObject*)this, AllottedGeometry, EnemyScreenPos, EndPoint);
            }
        }
        else
        {
            // 타겟팅된 적이 없다면? -> 평소처럼 마우스 커서를 따라갑니다.
            EndPoint = AllottedGeometry.AbsoluteToLocal(CurrentDragScreenPos);
        }

        // STS 스타일의 베지어 곡선(Bezier Curve) 계산
        TArray<FVector2D> Points;


        // 슬레이 더 스파이어 스타일의 컨트롤 포인트
        // ControlPoint1: 시작점(카드)에서 일단 하늘 위로 아주 높게(-800) 솟구치게 합니다.
        FVector2D ControlPoint1 = StartPoint + FVector2D(0.0f, -800.0f);

        // ControlPoint2: 도착점(적)의 바로 위 하늘(-800)에서 수직으로 내리꽂히게 합니다.
        FVector2D ControlPoint2 = EndPoint + FVector2D(0.0f, -800.0f);

        // 곡선을 30개의 짧은 직선으로 쪼개서 아주 부드럽게 만들기
        int32 Segments = 30;
        for (int32 i = 0; i <= Segments; i++)
        {
            float t = (float)i / Segments;
            float u = 1.0f - t;

            // 3차 베지어 곡선 공식
            FVector2D Point = (u * u * u) * StartPoint +
                3 * (u * u) * t * ControlPoint1 +
                3 * u * (t * t) * ControlPoint2 +
                (t * t * t) * EndPoint;

            Points.Add(Point);
        }

        // 선 그리기
        FSlateDrawElement::MakeLines(
            OutDrawElements, NextLayerId + 100, AllottedGeometry.ToPaintGeometry(),
            Points, ESlateDrawEffect::None, LineColor, true, LineThickness
        );

        // 화살표 머리 그리기 (곡선의 마지막 각도를 따라감)
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
   

    // 다음 레이어 ID를 반환합니다.
    return NextLayerId;
}

// 드래그 리브: 마우스가 위젯 밖으로 나가면 OFF
void USTSUserWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // 타겟팅 중이던 몬스터의 불을 끄고 초기화
    if (CurrentHoveredEnemy)
    {
        CurrentHoveredEnemy->SetTargetingHighlight(false);
        CurrentHoveredEnemy = nullptr;
    }

    Super::NativeOnDragLeave(InDragDropEvent, InOperation);
    bIsTargeting = false;
    CurrentHoveredEnemy = nullptr;
}