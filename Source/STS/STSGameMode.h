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



