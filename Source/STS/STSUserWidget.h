
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
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdatePlayerHP(int32 CurrentHP, int32 MaxHP);

	// 게임 오버 연출 함수
	void ShowGameOver();

	// 드래그 중일 때 매 프레임 실행되는 함수
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

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
};