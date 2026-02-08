
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
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h" 
#include "Engine/World.h"
void USTSUserWidget::AddCards(int32 NumCards)
{
  


    if (!CardWidgetClass || !CardDataTable) return; // 데이터 테이블 없을 때 튕겨내기
    UE_LOG(LogTemp, Warning, TEXT("AddCards 호출됨! 장수: %d"), NumCards);

    if (!CardWidgetClass)
    {
        // 클래스 누락 확인
        UE_LOG(LogTemp, Error, TEXT("CardWidgetClass가 비어있습니다! 블루프린트 확인하세요."));
        return;
    }
    if (!CardDataTable)
    {
        // 테이블 누락 확인
        UE_LOG(LogTemp, Error, TEXT("CardDataTable이 비어있습니다! 블루프린트 확인하세요."));
        return;
    }


    // 데이터 테이블의 모든 행 이름 가져오기
    TArray<FName> RowNames = CardDataTable->GetRowNames();

    // 데이터가 하나도 없을 때 튕겨내기
    if (RowNames.Num() == 0) return;


    // 카드 생성 반복문
    for (int32 i = 0; i < NumCards; i++)
    {
        

        // 랜덤으로 이름 하나 뽑기
        int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
        FName SelectedRowName = RowNames[RandomIndex];

        // 그 이름으로 데이터 구조체 찾기 
        // ContextString은 에러 났을 때 로그에 찍힐 메시지.
        static const FString ContextString(TEXT("Card Allocation"));
        FCardData* CardData = CardDataTable->FindRow<FCardData>(SelectedRowName, ContextString);

        // 위젯 생성
        UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), CardWidgetClass);

        USTSCardWidget* NewCard = Cast<USTSCardWidget>(NewWidget);
        
        if (NewCard && HandAreaPanel && CardData)
        {
            UE_LOG(LogTemp, Warning, TEXT("카드 생성 성공! 이름: %s"), *CardData->CardName.ToString());
            NewCard->UpdateCardDesign(*CardData);
            HandAreaPanel->AddChild(NewCard);
            CreatedCards.Add(NewCard); 
        }
        else {
            if (!NewCard)
                UE_LOG(LogTemp, Error, TEXT("실패 원인: NewCard가 null입니다. (형변환 실패)"));

            if (!HandAreaPanel)
                UE_LOG(LogTemp, Error, TEXT("실패 원인: HandAreaPanel이 null입니다. (BindWidget 실패)"));

            if (!CardData)
                UE_LOG(LogTemp, Error, TEXT("실패 원인: CardData가 null입니다. (데이터 못 찾음)"));
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

        // Visibility 채널을 검사
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility
        );

        // 결과 확인
        if (bHit && HitResult.GetActor())
        {
            // 디버그용: 맞은 위치에 초록색 점 찍기 (3초간 유지)
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 20.0f, FColor::Green, false, 3.0f);
            UE_LOG(LogTemp, Warning, TEXT("레이저 적중! 대상: %s"), *HitResult.GetActor()->GetName());

            // 적 태그 확인
            if (HitResult.GetActor()->ActorHasTag("Enemy"))
            {
                // 카드 데이터 가져오기
                FCardData CardData = DroppedCard->GetCardData();

                // 카드의 종류(Type)에 따라 분기 처리
                // (CSV 데이터테이블에 'Attack', 'Skill' 등으로 적혀있어야 함)

                if (CardData.Type == FName("Attack"))
                {
                    // === [공격 로직] ===
                    UE_LOG(LogTemp, Warning, TEXT("⚔️ 공격 카드 발동! 데미지: %d"), CardData.BaseDamage);

                    // 언리얼 내장 함수로 적에게 데미지 전달!
                    UGameplayStatics::ApplyDamage(
                        HitResult.GetActor(),     // 맞는 놈 (적)
                        CardData.BaseDamage,      // 데미지 양
                        GetOwningPlayer(),        // 때린 놈 (컨트롤러)
                        GetOwningPlayerPawn(),    // 때린 놈의 몸체 (캐릭터)
                        UDamageType::StaticClass() // 데미지 유형 (일반)
                    );
                }
                else if (CardData.Type == FName("Skill"))
                {
                    // === [스킬/방어 로직] ===
                    // 수비 카드는 적에게 던져도 효과는 '나'한테 적용되어야 함
                    UE_LOG(LogTemp, Warning, TEXT("🛡️ 스킬 카드 발동! 방어도: %d"), CardData.BaseBlock);

                    // TODO: 나중에 플레이어 캐릭터의 'AddBlock()' 함수를 호출할 예정
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("❓ 알 수 없는 카드 타입: %s"), *CardData.Type.ToString());
                }

                // 카드 삭제 및 정리
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
            DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 3.0f);
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
    // 1. 화면에서 지우기
    for (UUserWidget* Card : CreatedCards)
    {
        if (Card)
        {
            Card->RemoveFromParent();
        }
    }

    // 2. 관리 목록(배열) 비우기
    CreatedCards.Empty();

    UE_LOG(LogTemp, Log, TEXT("손패를 모두 버렸습니다."));
}


