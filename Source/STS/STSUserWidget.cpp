
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
        //FCardData* CardData = CardDataTable->FindRow<FCardData>(DrawnCardName, ContextString);
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

bool USTSUserWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    //카드가 맞는지 확인
    USTSCardWidget* DroppedCard = Cast<USTSCardWidget>(InOperation->Payload);
    if (!DroppedCard) return false;

    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController) return false;

    // 드롭 이벤트에서 정확한 마우스 화면 좌표 가져오기
    FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();

    //[수정] 화면 좌표 -> 3D 월드 좌표로 변환 (역투영)
    FVector WorldLocation;
    FVector WorldDirection;

    // 플레이어 컨트롤러를 통해 화면 좌표를 월드 좌표와 방향으로 바꿉니다.
    bool bDeprojectSuccess = PlayerController->DeprojectScreenPositionToWorld(
        ScreenPosition.X,
        ScreenPosition.Y,
        WorldLocation,
        WorldDirection
    );

    if (bDeprojectSuccess)
    {
        // [수정] 직접 레이저 발사 (Line Trace)
        FHitResult HitResult;
        FVector TraceStart = WorldLocation;
        FVector TraceEnd = WorldLocation + (WorldDirection * 50000.0f); // 50000cm(500m) 만큼 멀리 쏨

        //플레이어 자신은 무시
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwningPlayerPawn());

        // Visibility 채널을 검사
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

        // 결과 확인
        if (bHit && HitResult.GetActor())
        {
            // 맞은 위치에 하늘색 점과 선 그리기
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 30.0f, FColor::Cyan, false, 10.0f, 0); // 점 크기 30
            DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Cyan, false, 10.0f, 0, 5.0f); // 선 두께 5 

            UE_LOG(LogTemp, Warning, TEXT("레이저 적중! 대상: %s (컴포넌트: %s)"),
                *HitResult.GetActor()->GetName(), *HitResult.GetComponent()->GetName());
            // 적 태그 확인
            if (HitResult.GetActor()->ActorHasTag("Enemy"))
            {
                // 카드 데이터 가져오기
                FCardData CardData = DroppedCard->GetCardData();

                ASTSGameMode* GM = Cast<ASTSGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
                if (!GM) return false;

                // 에너지 사용 시도
                if (!GM->TryUseEnergy(CardData.CostText))
                {
                    // 에너지가 부족하면 실패! (카드는 다시 손패로 돌아감)
                    UE_LOG(LogTemp, Error, TEXT("❌ 에너지가 부족합니다! (필요: %d, 현재: %d)"), CardData.CostText, GM->CurrentEnergy);
                    return false;
                }

                // 카드의 종류(Type)에 따라 분기 처리
                // (CSV 데이터테이블에 'Attack', 'Skill' 등으로 적혀있어야 함)

                if (CardData.Type == FName("Attack"))
                {
                    // === [공격 로직] ===
                    UE_LOG(LogTemp, Warning, TEXT("⚔️ 공격 카드 발동! 데미지: %d"), CardData.BaseDamage);
                    UGameplayStatics::ApplyDamage(HitResult.GetActor(), CardData.BaseDamage, GetOwningPlayer(), GetOwningPlayerPawn(), UDamageType::StaticClass());
                    // 언리얼 내장 함수로 적에게 데미지 전달!
                    UGameplayStatics::ApplyDamage(
                        HitResult.GetActor(),     // 맞는 놈 (적)
                        CardData.BaseDamage,      // 데미지 양
                        GetOwningPlayer(),        // 때린 놈 (컨트롤러)
                        GetOwningPlayerPawn(),    // 때린 놈의 몸체 (캐릭터)
                        UDamageType::StaticClass() // 데미지 유형 (일반)
                    );

                    
                }
                else if (CardData.Type == FName("Defend"))
                {
                    // === [스킬/방어 로직] ===
                    // 수비 카드는 적에게 던져도 효과는 '나'한테 적용되어야 함
                    UE_LOG(LogTemp, Warning, TEXT("방어 카드 발동! 방어도: %d"), CardData.BaseBlock);
                    GM->AddBlock(CardData.BaseBlock);
                    // TODO: 나중에 플레이어 캐릭터의 'AddBlock()' 함수를 호출할 예정
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("알 수 없는 카드 타입: %s"), *CardData.Type.ToString());
                }


                GM->AddToDiscardPile(DroppedCard->CardRowName);

                //카드 삭제 및 정리
                DroppedCard->RemoveFromParent();
                CreatedCards.Remove(DroppedCard);
                UpdateCardLayout();

                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[실패] 맞긴 했는데 Enemy 태그가 없습니다."));
            }
        }
        else
        {
            // 허공에 쏘면 빨간 선 그리기 (어디로 날아가는지 눈으로 확인)
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Magenta, false, 10.0f, 0, 5.0f); // 선 두께 5            
            UE_LOG(LogTemp, Error, TEXT("[실패] 레이저가 허공을 갈랐습니다. (Collision 설정을 다시 확인하세요)"));
        }
    }

    return false;
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


