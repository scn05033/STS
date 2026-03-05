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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 CurrentBlock = 0;

    // 플레이어 체력
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 CurrentHealth = 50;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 MaxHealth = 50;

    // 플레이어가 데미지를 받는 함수 (방어도를 먼저 깎는 로직 포함)
    void TakePlayerDamage(int32 Damage);

    // 행동 함수들
    bool TryUseEnergy(int32 Amount); // 에너지 사용 (부족하면 false)
    void AddBlock(int32 Amount);     // 방어도 획득

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

    // 승리 후 다음 방으로 넘어가는 함수
   // UFUNCTION(BlueprintCallable, Category = "Combat")
   // void StartNextStage();

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
    int32 BossFloor = 5;


    // 보스전에서 스폰할 특별한 적!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class ASTSEnemyCharacter> BossClassToSpawn;

    // 노드를 선택했을 때 다음 스테이지를 세팅하는 함수 
    UFUNCTION(BlueprintCallable, Category = "Map")
    void GoToNextNode(FName NodeType); // "Combat", "Rest", "Boss" 등

protected:
    // 실제 상태 변경 로직
    void StartPlayerTurn();
    void StartEnemyTurn();

    // 상태 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    ETurnState CurrentTurnState;

    // 현재 턴 번호 (1턴, 2턴...)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    int32 TurnNumber = 1;

    
   
    //UI를 기억할 변수
    UPROPERTY()
	USTSUserWidget* MainUIWidget;



   

   
};



