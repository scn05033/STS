
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/DragAndDrop.h"
#include "STSUserWidget.generated.h"

class UCanvasPanel;
class UButton;
class UTextBlock;
class USTSCardWidget;
class UProgressBar;

UCLASS()
class STS_API USTSUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 카드가 놓일 가운데 공간 
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* HandAreaPanel;

	// 턴 종료 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* TurnEndButton;

	// 에너지 텍스트 
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnergyText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	UTextBlock* BlockText;
	// 카드 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STS")
	TSubclassOf<UUserWidget> CardWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "Cards")
	TArray<USTSCardWidget*> CreatedCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* CardDataTable;

	// UI 에디터의 체력바와 텍스트
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;

	// 게임 오버 패널 
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* GameOverPanel;

	
	// C++이 블루프린트에게 애니메이션 재생을 지시하는 무전기
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void PlayTurnEndAnimations();
	UFUNCTION()
	void OnTurnEndClicked();
	virtual void NativeConstruct() override;
	//드롭을 감지하는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// 오버라이드할 NativePaint 함수 선언
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// 블루프린트에서 조절할 수 있도록 변수 선언
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	FLinearColor LineColor = FLinearColor::Red; // 선 색상

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	float LineThickness = 5.0f; // 선 두께

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	float ArrowHeadSize = 30.0f; // 화살표 머리 크기

	// 적중 여부와 상관없이 '현재 공격 카드를 쥐고 있는지'를 판단할 변수
	UPROPERTY(BlueprintReadOnly)
	bool bIsTargeting = false;

	// 카드를 화면 밖으로 놓쳤을 때를 대비한 함수
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// UI에서 선을 그릴 때 사용할 마우스/타겟 좌표 변수들
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDragScreenPos;

	

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	// 현재 진행 중인 드래그 정보를 저장할 변수
	UPROPERTY()
	class UDragDropOperation* CurrentDragOp;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UFUNCTION(BlueprintCallable)
	void AddCards(int32 NewCards);
	void UpdateCardLayout();
	
	void EmptyHand();
	void UpdateEnergyText(int32 CurrentEnergy, int32 MaxEnergy);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateBlockText(int32 CurrentBlock);

	// 플레이어 체력 갱신 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdatePlayerHP(int32 CurrentHP, int32 MaxHP);
	UPROPERTY(meta = (BindWidget),BlueprintReadWrite)
	class UCanvasPanel* CombatUIPanel;
	// 게임 오버 연출 함수
	UFUNCTION()
	void ShowGameOver();

	// 드래그 중일 때 매 프레임 실행되는 함수
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	

	UPROPERTY(BlueprintReadOnly)
	class ASTSEnemyCharacter* CurrentHoveredEnemy;
	void ShowVictory();

	// UI 패널 연결
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* VictoryPanel;

	//엔딩 화면 패널 연결
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* GameClearPanel;



	// 엔딩 화면을 켜는 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameClear();

	// 다음 방으로 갈 때 화면의 카드를 지우는 함수
	void ClearHandUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Reward")
	void InitVictoryRewards();

	


	//보상 카드 3개
	UPROPERTY(meta = (BindWidget))
	class USTSCardWidget* RewardCard1;

	UPROPERTY(meta = (BindWidget))
	class USTSCardWidget* RewardCard2;

	UPROPERTY(meta = (BindWidget))
	class USTSCardWidget* RewardCard3;

	// 뽑힌 3장의 카드 이름을 받아서 엑셀 데이터를 쏴주는 함수
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void ApplyRewardCardsData(TArray<FName> RewardNames);

	// 전투 종료를 알리는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "State")
	void OnCombatEnded();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	AActor* GetEnemyUnderCursor(FVector2D ScreenPos);

	// 생성된 카드의 시작 위치를 '덱'으로 강제 이동시키는 블루프린트 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void SetInitialPositionToDeck(USTSCardWidget* NewCard);

	void ShowAoEPrediction(int32 FinalDamage);

	void HideAoEPrediction();

	
	UPROPERTY()
	class USTSCardWidget* DraggedCard = nullptr;
};