// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "STSGameMode.generated.h"

class USTSUserWidget;

// 전투 상태 정의 
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

    
    UFUNCTION(BlueprintPure, Category = "Battle")
    ETurnState GetCurrentTurnState() const { return CurrentTurnState; }

    UFUNCTION(BlueprintCallable)
    void StartCombat(USTSUserWidget* InUIWidget);

    
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void EndPlayerTurn();

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 CurrentEnergy = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
    int32 MaxEnergy = 3;

  

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentStrength = 0;

   
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddStrength(int32 Amount);

 

   
    bool TryUseEnergy(int32 Amount); 
    

    
    void InitializeDeck();

   
    void AddToDiscardPile(FName CardRowName);

 

    
    FName DrawCard();
    void CheckVictory();

    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    int32 WeakStacks = 0;

    
    void AddWeak(int32 Amount);

   
    void DecreasePlayerStatusEffects();

  
    // 다음 방에서 스폰할 적의 블루프린트 클래스 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class ASTSEnemyCharacter> EnemyClassToSpawn;

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DrawPile;

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DiscardPile;

    // 내 영구적인 진짜 덱 (전투가 끝날 때마다 이 덱을 기준으로 DrawPile이 리셋)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
    TArray<FName> MasterDeck;

    // 카드가 들어있는 데이터테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
    class UDataTable* CardDataTable;

    
    UFUNCTION(BlueprintCallable, Category = "Reward")
    TArray<FName> GenerateRandomRewards(int32 Count = 3);

    
    UFUNCTION(BlueprintCallable, Category = "Reward")
    void AddCardToMasterDeck(FName NewCardName);

    // 현재 층수 (1층부터 시작)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Map")
    int32 CurrentFloor = 1;

    // 보스가 등장하는 마지막 층수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    int32 BossFloor = 3;


    // 보스전에서 스폰할 특별한 적
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TSubclassOf<class ASTSEnemyCharacter> BossClassToSpawn;

  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TSubclassOf<class AActor> CampfireClassToSpawn;

   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TSubclassOf<class AActor> AnvilClassToSpawn;

    
    UFUNCTION(BlueprintCallable, Category = "Map")
    void GoToNextNode(FName NodeType); 

    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void RestartGame();

    
    UFUNCTION(BlueprintCallable, Category = "Deck")
    void UpgradeCardInDeck(int32 CardIndex, FName UpgradedCardID);

    
    UFUNCTION(BlueprintCallable, Category = "CardData")
    void AddCardToDiscardPile(FName CardName);

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
    USoundBase* DefaultBGM;

     
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
    USoundBase* EndingBGM;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    USTSUserWidget* MainUIWidget;

protected:
   
    UFUNCTION(BlueprintCallable, Category = "Battle")
    void StartPlayerTurn();
    void StartEnemyTurn();

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    ETurnState CurrentTurnState;

   
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle")
    int32 TurnNumber = 1;

    
   

    
    UFUNCTION(BlueprintPure, Category = "CardData")
    int32 GetDeckCount() const { return DrawPile.Num(); }

    
    UFUNCTION(BlueprintPure, Category = "CardData")
    int32 GetDiscardCount() const { return DiscardPile.Num(); }
  
    
    FTimerHandle BGMTimerHandle;
    void ResumeMapBGM();

   
        void PlayEndingBGM();
};



