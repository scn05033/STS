// STSCardWidget.h
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardDataStruct.h" 
#include "Layout/Geometry.h"       
#include "Input/Events.h" 
#include "Blueprint/DragDropOperation.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "STSCardWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class STS_API USTSCardWidget : public UUserWidget
{
    GENERATED_BODY()

protected:

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* CardName;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Cost;

    UPROPERTY(meta = (BindWidget))
    URichTextBlock* CardDescription;

    


    UPROPERTY(meta = (BindWidget))
    class UImage* CardArtImage;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* CardType;

    // 마우스가 위젯 영역에 들어왔을 때 
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // 마우스가 위젯 영역에서 나갔을 때 
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // (미리 준비) 마우스 버튼을 눌렀을 때
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

   
    //드래그가 감지되었을 때 실행 
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

    FCardData CurrentCardData;
   // void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    UPROPERTY()
    class ASTSEnemyCharacter* CurrentTargetEnemy;

public:
    // 데이터를 받아서 UI를 갱신하는 함수
    UFUNCTION(BlueprintCallable)
    void UpdateCardDesign(const FCardData& Data);

    FText GetCardNameText() const;

    FCardData GetCardData() const { return CurrentCardData; }

    UPROPERTY(BlueprintReadWrite)
    FName CardRowName;

    // 이 카드가 현재 대장간(강화 창)에 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card State")
    bool bIsSmithingMode = false;

    // 블루프린트에서 구현할 이벤트 (카드가 대장간에서 클릭되었을 때 발동)
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCardClickedForSmithing();

    // 블루프린트에 만들어둔 비행 이벤트를 C++에서 호출하기 위한 껍데기
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Animation")   
    void Anim_FlyToLocation(FVector2D TargetPosition, bool bIsDiscarding);

    // 데미지를 계산해주는 전용 함수
    UFUNCTION(BlueprintCallable, Category = "Card|Calculate")
    int32 CalculateFinalDamage(int32 InBaseDamage, int32 InPlayerStrength, bool bIsTargetVulnerable, bool bIsPlayerWeak);

    // 타겟을 전달받고 텍스트를 갱신하는 함수
    UFUNCTION(BlueprintCallable, Category = "Card|Targeting")
    void UpdateTargetAndRefreshText(class ASTSEnemyCharacter* TargetEnemy);

    // 드래그 비주얼로 사용할 래퍼 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Drag")
    TSubclassOf<class UUserWidget> DragVisualClass;

    // 드래그 시작 전, 원래 담겨있던 부모 패널(Canvas Panel 등)을 기억해둡니다.
    UPROPERTY()
    class UPanelWidget* OriginalParentPanel;

    UPROPERTY()
    class USTSCardWidget* FakeDragVisual;

    

    void UpdateDynamicDamageText(int32 CalculatedDamage);

    USTSCardWidget* GetFakeDragVisual() const { return FakeDragVisual; }
};