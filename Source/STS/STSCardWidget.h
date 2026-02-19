// STSCardWidget.h
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardDataStruct.h" 
#include "Layout/Geometry.h"       
#include "Input/Events.h" 
#include "Blueprint/DragDropOperation.h"
#include "STSCardWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class STS_API USTSCardWidget : public UUserWidget
{
    GENERATED_BODY()

protected:

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CardName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CostText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CardDescription;

    UPROPERTY(meta = (BindWidget))
    UImage* CardImage;

    // 마우스가 위젯 영역에 들어왔을 때 
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // 마우스가 위젯 영역에서 나갔을 때 
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // (미리 준비) 마우스 버튼을 눌렀을 때
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

   
    //드래그가 감지되었을 때 실행 
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

    FCardData CurrentCardData;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

 

public:
    // 데이터를 받아서 UI를 갱신하는 함수
    void UpdateCardDesign(const FCardData& Data);

    FText GetCardNameText() const;

    FCardData GetCardData() const { return CurrentCardData; }

    FName CardRowName;
};