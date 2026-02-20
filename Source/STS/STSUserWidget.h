
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

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlockText;
	// 카드 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STS")
	TSubclassOf<UUserWidget> CardWidgetClass;
	UPROPERTY()
	TArray<USTSCardWidget*> CreatedCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* CardDataTable;

	// UI 에디터의 체력바와 텍스트
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerHPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerHPText;

	// 게임 오버 패널 
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* GameOverPanel;

	

	UFUNCTION()
	void OnTurnEndClicked();
	virtual void NativeConstruct() override;
	//드롭을 감지하는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
public:
	UFUNCTION(BlueprintCallable)
	void AddCards(int32 NewCards);
	void UpdateCardLayout();
	
	void EmptyHand();
	void UpdateEnergyText(int32 CurrentEnergy, int32 MaxEnergy);
	void UpdateBlockText(int32 CurrentBlock);

	// 플레이어 체력 갱신 함수
	void UpdatePlayerHP(int32 CurrentHP, int32 MaxHP);

	// 게임 오버 연출 함수
	void ShowGameOver();

};