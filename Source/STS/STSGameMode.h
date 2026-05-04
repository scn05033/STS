// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "STSGameMode.generated.h"

class USTSUserWidget;

// 전투 상태 정의 (신호등)
UENUM(BlueprintType)
enum class ETurnState : uint8
{
    PlayerTurn  UMETA(DisplayName = "Player Turn"), // 내 차례
    EnemyTurn   UMETA(DisplayName = "Enemy Turn"),  // 적 차례
    Victory     UMETA(DisplayName = "Victory"),     // 승리
    Defeat      UMETA(DisplayName = "Defeat")       // 패배
};

UCLASS()
class STS_API ASTSGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASTSGameMode();

    // 현재 턴 상태 확인
    UFUNCTION(BlueprintPure, Category = "Battle")
    ETurnState GetCurrentTurnState() const { return CurrentTurnState; }

    // 게임 시작 (전투 개시)
    void StartCombat(USTSUserWidget* InUIWidget);

    // 턴 넘기기 (UI의 '턴 종료' 버튼이 이걸 호출함)
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void EndPlayerTurn();

    // 플레이어 스탯
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 CurrentEnergy = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 MaxEnergy = 3;

  

    // 현재 힘 수치 (블루프린트 UI에서도 읽을 수 있게 권한 부여)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentStrength = 0;

    // 힘을 올려주는 함수
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddStrength(int32 Amount);

 

    // 행동 함수들
    bool TryUseEnergy(int32 Amount); // 에너지 사용 (부족하면 false)
    

    // 덱 초기화 및 셔플 함수
    void InitializeDeck();

    // 카드를 버리는 함수 (사용한 카드나 턴 종료 시 버려진 카드)
    void AddToDiscardPile(FName CardRowName);

 

    // 카드 1장 뽑아서 이름(RowName) 반환 
    FName DrawCard();
    void CheckVictory();

    // 플레이어의 약화 스택
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    int32 WeakStacks = 0;

    // 약화를 부여하는 함수
    void AddWeak(int32 Amount);

    // 턴 시작 시 디버프를 줄여주는 함수
    void DecreasePlayerStatusEffects();

  
    // 다음 방에서 스폰할 적의 블루프린트 클래스 (언리얼 에디터에서 지정할 수 있게 만듭니다)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class ASTSEnemyCharacter> EnemyClassToSpawn;

    // 덱(뽑을 카드 뭉치)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DrawPile;

    // 무덤(버린 카드 뭉치)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DiscardPile;

    // 내 영구적인 진짜 덱 (전투가 끝날 때마다 이 덱을 기준으로 DrawPile이 리셋)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
    TArray<FName> MasterDeck;

    // 카드가 들어있는 데이터테이블(엑셀) 원본 (여기서 카드를 랜덤으로 뽑아옴)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
    class UDataTable* CardDataTable;

    // 보상으로 제시할 무작위 카드 3장을 뽑는 함수
    UFUNCTION(BlueprintCallable, Category = "Reward")
    TArray<FName> GenerateRandomRewards(int32 Count = 3);

    // 선택한 보상 카드를 내 마스터 덱에 영구 추가하는 함수
    UFUNCTION(BlueprintCallable, Category = "Reward")
    void AddCardToMasterDeck(FName NewCardName);

    // 현재 층수 (1층부터 시작)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Map")
    int32 CurrentFloor = 1;

    // 보스가 등장하는 마지막 층수 (빠른 테스트를 위해 5층으로 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    int32 BossFloor = 3;


    // 보스전에서 스폰할 특별한 적!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class ASTSEnemyCharacter> BossClassToSpawn;

    // 적 스폰 변수들 근처에 모닥불 스폰용 변수를 하나 추가합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TSubclassOf<class AActor> CampfireClassToSpawn;

    //모루 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TSubclassOf<class AActor> AnvilClassToSpawn;

    // 노드를 선택했을 때 다음 스테이지를 세팅하는 함수 
    UFUNCTION(BlueprintCallable, Category = "Map")
    void GoToNextNode(FName NodeType); // "Combat", "Rest", "Boss" 등

    //게임 오버 시 게임을 재시작
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RestartGame();

    // 인덱스 번호와, 새로 끼워넣을 진화형 카드 이름표를 받습니다.
    UFUNCTION(BlueprintCallable, Category = "Deck")
    void UpgradeCardInDeck(int32 CardIndex, FName UpgradedCardID);

    // 카드를 무덤 배열에 추가
    UFUNCTION(BlueprintCallable, Category = "CardData")
    void AddCardToDiscardPile(FName CardName);

    // 전투가 끝난 뒤 돌아갈 기본 맵 BGM
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
    USoundBase* DefaultBGM;

    // 엔딩 BGM 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
    USoundBase* EndingBGM;

protected:
    // 실제 상태 변경 로직
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void StartPlayerTurn();
    void StartEnemyTurn();

    // 상태 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    ETurnState CurrentTurnState;

    // 현재 턴 번호 (1턴, 2턴...)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    int32 TurnNumber = 1;

    
   
    //UI를 기억할 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	USTSUserWidget* MainUIWidget;

    // 덱에 남은 카드 수 반환
    UFUNCTION(BlueprintPure, Category = "CardData")
    int32 GetDeckCount() const { return DrawPile.Num(); }

    // 무덤에 쌓인 카드 수 반환
    UFUNCTION(BlueprintPure, Category = "CardData")
    int32 GetDiscardCount() const { return DiscardPile.Num(); }
  
    // 타이머 핸들과 BGM 복구 함수
    FTimerHandle BGMTimerHandle;
    void ResumeMapBGM();

   // 엔딩 BGM을 재생할 함수 추가
        void PlayEndingBGM();
};



