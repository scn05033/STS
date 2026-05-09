
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
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* HandAreaPanel;

	
	UPROPERTY(meta = (BindWidget))
	UButton* TurnEndButton;

	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnergyText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "UI")
	UTextBlock* BlockText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STS")
	TSubclassOf<UUserWidget> CardWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "Cards")
	TArray<USTSCardWidget*> CreatedCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* CardDataTable;

	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;

	
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* GameOverPanel;

	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void PlayTurnEndAnimations();
	UFUNCTION()
	void OnTurnEndClicked();
	virtual void NativeConstruct() override;
	
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	FLinearColor LineColor = FLinearColor::Red; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	float LineThickness = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting Line")
	float ArrowHeadSize = 30.0f; 

	
	UPROPERTY(BlueprintReadOnly)
	bool bIsTargeting = false;

	
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	
	UPROPERTY(BlueprintReadOnly)
	FVector2D CurrentDragScreenPos;

	

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
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

	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdatePlayerHP(int32 CurrentHP, int32 MaxHP);
	UPROPERTY(meta = (BindWidget),BlueprintReadWrite)
	class UCanvasPanel* CombatUIPanel;
	
	UFUNCTION()
	void ShowGameOver();

	
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	

	UPROPERTY(BlueprintReadOnly)
	class ASTSEnemyCharacter* CurrentHoveredEnemy;
	void ShowVictory();

	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* VictoryPanel;

	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* GameClearPanel;



	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameClear();

	
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