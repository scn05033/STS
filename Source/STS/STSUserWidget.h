
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/DragAndDrop.h"
#include "STSUserWidget.generated.h"

class UCanvasPanel;
class UButton;
class UTextBlock;
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

	// 카드 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "STS")
	TSubclassOf<UUserWidget> CardWidgetClass;
	//UPROPERTY()
	TArray<UUserWidget*> CreatedCards;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* CardDataTable;

	

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


};