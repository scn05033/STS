// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSGameMode.h"
#include "STSUserWidget.h"

#include "Kismet/GameplayStatics.h"

ASTSGameMode::ASTSGameMode()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // 기본값 설정
    CurrentTurnState = ETurnState::PlayerTurn;
}

void ASTSGameMode::StartCombat(USTSUserWidget* InUIWidget)
{
    UE_LOG(LogTemp, Warning, TEXT("=== 전투 시작! (Start Combat) ==="));
	MainUIWidget = InUIWidget;
    TurnNumber = 1;
    StartPlayerTurn();
}

void ASTSGameMode::StartPlayerTurn()
{
    
    CurrentTurnState = ETurnState::PlayerTurn;
    UE_LOG(LogTemp, Warning, TEXT(">>> turn start!"), TurnNumber);

	  
    // TODO: 여기서 UI에게 "카드 5장 뽑아"라고 명령해야 함
    if (MainUIWidget)
    {
        MainUIWidget->AddCards(5);
	}

    // TODO: 에너지(마나) 초기화
}

void ASTSGameMode::EndPlayerTurn()
{
    // 플레이어 턴이 아닐 때 버튼 누르면 무시
    if (CurrentTurnState != ETurnState::PlayerTurn) return;

    UE_LOG(LogTemp, Warning, TEXT("<<< 플레이어 턴 종료. 손패를 버립니다."));

    // TODO: 여기서 UI에게 "손패 다 버려"라고 명령해야 함
    if (MainUIWidget)
    {
        MainUIWidget->EmptyHand();
    }
    // 바로 적 턴으로 넘김
    StartEnemyTurn();
}

void ASTSGameMode::StartEnemyTurn()
{
    CurrentTurnState = ETurnState::EnemyTurn;
    UE_LOG(LogTemp, Warning, TEXT("!!! enemy turn start !!!"));

    // 적의 행동은 시간이 좀 걸려야 함 (연출)
    // 2초 뒤에 다시 플레이어 턴으로 넘기는 타이머 설정
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            UE_LOG(LogTemp, Warning, TEXT("!!! enemy turn end ."));

            TurnNumber++; // 턴 수 증가
            StartPlayerTurn(); // 다시 플레이어 턴으로! (루프)

        }, 2.0f, false);
}
