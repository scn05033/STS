
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

    TArray<UUserWidget*> CreatedCards;
public:
    UFUNCTION(BlueprintCallable)
    void AddCards(int32 NewCards);
    void UpdateCardLayout();
   

};