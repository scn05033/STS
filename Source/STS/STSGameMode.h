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

    // 카드 뽑기 내부 로직 (UI에게 "이 이름의 카드를 생성해!" 라고 명령할 예정)
    void DrawCardsFromDeck(int32 Amount);

    // 카드 1장 뽑아서 이름(RowName) 반환 
    FName DrawCard();

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

    // 덱(뽑을 카드 뭉치)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DrawPile;

    // 무덤(버린 카드 뭉치)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deck")
    TArray<FName> DiscardPile;

   
};



