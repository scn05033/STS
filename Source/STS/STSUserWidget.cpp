
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

    AActor* TargetEnemy = nullptr;
    APlayerController* PC = GetOwningPlayer();

    // 공격 카드일 경우
    if (CardData.Type == FName("Attack"))
    {
        // [핵심 해결] 드래그 이벤트 전용 마우스 좌표 가져오기!
        FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();
        FVector WorldLoc, WorldDir;

        // 드래그 중인 화면 좌표를 3D 월드 좌표로 변환
        if (UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, WorldLoc, WorldDir))
        {
            FVector End = WorldLoc + (WorldDir * 10000.0f);
            FHitResult HitResult;

            // 눈에 보이는 빨간 레이저 빔 그리기!
            DrawDebugLine(GetWorld(), WorldLoc, End, FColor::Red, false, 3.0f, 0, 5.0f);

            // 레이저 발사
            if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLoc, End, ECC_Visibility))
            {
                AActor* HitActor = HitResult.GetActor();
                if (HitActor)
                {
                    UE_LOG(LogTemp, Warning, TEXT("레이저 적중! 맞은 물체: %s"), *HitActor->GetName());

                    if (HitActor->ActorHasTag(FName("Enemy")))
                    {
                        TargetEnemy = HitActor;
                    }
                }
            }
        }

        // 공격 카드인데 적을 못 맞췄다면 취소
        if (!TargetEnemy)
        {
            UE_LOG(LogTemp, Warning, TEXT("공격 카드는 적을 타겟팅해야 합니다!"));
            return false;
        }
    }

    // 에너지 사용
    if (!GM->TryUseEnergy(CardData.Cost))
    {
        return false;
    }

    // 에너지가 깎였으니 현재 UI의 숫자를 강제로 바로 업데이트
    UpdateEnergyText(GM->CurrentEnergy, GM->MaxEnergy);

    // 카드 효과 발동
    if (CardData.Type == FName("Attack") && TargetEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎯 공격 성공! 대상: %s, 데미지: %d"), *TargetEnemy->GetName(), CardData.BaseDamage);
        UGameplayStatics::ApplyDamage(TargetEnemy, CardData.BaseDamage, PC, PC->GetPawn(), UDamageType::StaticClass());
        // [상태이상 부여] 
        if (CardData.StatusAmount > 0 && CardData.StatusType == FName("Vulnerable"))
        {
            // 맞은 적을 ASTSEnemyCharacter로 변환해서 스택을 쌓아줍니다.
            if (ASTSEnemyCharacter* EnemyChar = Cast<ASTSEnemyCharacter>(TargetEnemy))
            {
                EnemyChar->VulnerableStacks += CardData.StatusAmount;
                UE_LOG(LogTemp, Warning, TEXT("적에게 '취약' %d 스택 부여! (현재 총 %d 스택)"), CardData.StatusAmount, EnemyChar->VulnerableStacks);
            }
        }
    }
    else if (CardData.Type == FName("Defend"))
    {
        GM->AddBlock(CardData.BaseBlock);
        UE_LOG(LogTemp, Warning, TEXT("방어도 증가: %d"), CardData.BaseBlock);
    }

    // 사용한 카드 처리
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
    }
    else
    {
        
    }

    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

