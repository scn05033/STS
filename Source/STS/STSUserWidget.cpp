
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
#include "STSEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CardDataStruct.h"
#include "Components/ProgressBar.h"
#include "DrawDebugHelpers.h"
#include "STSCharacter.h"
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

    USTSCardWidget* DroppedCard = Cast<USTSCardWidget>(InOperation->Payload);
    if (!DroppedCard) return false;

    FCardData CardData = DroppedCard->GetCardData();
    ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM) return false;

    APlayerController* PC = GetOwningPlayer();
    AActor* TargetEnemy = nullptr;

    // 공격 카드일 경우: 타겟팅 검사
    if (CardData.Type == FName("Attack"))
    {
        if (CardData.bIsAoE)
        {
            // 광역기면 레이저 검사 없이 프리패스!
            UE_LOG(LogTemp, Warning, TEXT("광역 공격 카드 발동 준비!"));
        }
        else
        {
            // 단일 공격기면 기존처럼 레이저로 적을 맞췄는지 깐깐하게 검사
            FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
            FVector WorldLoc, WorldDir;
            if (UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, WorldLoc, WorldDir))
            {
                FVector End = WorldLoc + (WorldDir * 10000.0f);
                FHitResult HitResult;

                if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, End, ECC_Visibility))
                {
                    if (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag(FName("Enemy")))
                    {
                        TargetEnemy = HitResult.GetActor();
                    }
                }
            }

            if (!TargetEnemy)
            {
                UE_LOG(LogTemp, Warning, TEXT("단일 공격 카드는 적을 타겟팅해야 합니다!"));
                return false;
            }
        }
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

    FVector WorldLocation, WorldDirection;
    APlayerController* PC = GetOwningPlayer();

    if (PC && PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
    {
        FVector Start = WorldLocation;
        FVector End = Start + (WorldDirection * 10000.0f);
        FHitResult HitResult;

        // 마우스 위치로 레이저를 쏴서 적이 있는지 확인
        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
        {
            if (HitResult.GetActor() && HitResult.GetActor()->ActorHasTag("Enemy"))
            {
                // 적 위에 마우스가 있으면, 적의 위치에 빨간색 구체를 0.1초 동안 그림! (조준점 역할)
                DrawDebugSphere(GetWorld(), HitResult.Location, 40.0f, 16, FColor::Red, false, 0.1f, 0, 3.0f);
            }
        }
    }
    return true; // 드래그 이벤트 정상 처리
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


