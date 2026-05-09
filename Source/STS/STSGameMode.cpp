// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSGameMode.h"
#include "STSUserWidget.h"
#include "STSCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/Widget.h"
#include "CardDataStruct.h"
#include "StatusEffectComponent.h"
#include "STSEnemyCharacter.h"
#include "STSGameInstance.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

ASTSGameMode::ASTSGameMode()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
   if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
   }
   
   
    CurrentTurnState = ETurnState::PlayerTurn;
}


void ASTSGameMode::StartCombat(USTSUserWidget* InUIWidget)
{
  

	MainUIWidget = InUIWidget;
    TurnNumber = 1;

   
    if (MainUIWidget)
    {
        MainUIWidget->ClearHandUI(); 
    }

    MainUIWidget->EmptyHand();
    DiscardPile.Empty();  
    

    InitializeDeck();
    StartPlayerTurn();

  
}


void ASTSGameMode::StartPlayerTurn()
{
    
    CurrentTurnState = ETurnState::PlayerTurn;

    CurrentEnergy = MaxEnergy;

    ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (PlayerChar)
    {
        
        PlayerChar->CurrentBlock = 0;

        
        if (UStatusEffectComponent* StatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
        {
            StatusComp->DecreaseAllStatuses();
        }
    }

    

    if (MainUIWidget)
    {
        MainUIWidget->AddCards(5);

        
        MainUIWidget->UpdateEnergyText(CurrentEnergy, MaxEnergy);

       
        if (PlayerChar)
        {
            MainUIWidget->UpdateBlockText(PlayerChar->CurrentBlock);
        }
    }
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), FoundEnemies);

    //턴이 시작되면 적에게 다음 행동을 결정하라고 명령
    for (AActor* Actor : FoundEnemies)
    {
        if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
        {
            if (Enemy->ActorHasTag(FName("CurrentBattle")))
            {
                Enemy->DecideNextIntent();

                //적 상태 이상 감소 
                if (UStatusEffectComponent* EnemyStatusComp = Enemy->FindComponentByClass<UStatusEffectComponent>())
                {
                    EnemyStatusComp->DecreaseAllStatuses();
                }
            }
        }
    }

}

void ASTSGameMode::EndPlayerTurn()
{
   
    if (CurrentTurnState != ETurnState::PlayerTurn) return;

    UE_LOG(LogTemp, Warning, TEXT("<<< 플레이어 턴 종료. 손패를 버립니다."));

    
    if (MainUIWidget)
    {
        MainUIWidget->EmptyHand();
    }
   
    StartEnemyTurn();
}

 

void ASTSGameMode::StartEnemyTurn()
{
    CurrentTurnState = ETurnState::EnemyTurn;

    FTimerHandle ActionTimerHandle;
    // 턴 시작 후 적들이 행동을 시작합니다.
    GetWorld()->GetTimerManager().SetTimer(ActionTimerHandle, [this]()
        {
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), FoundEnemies);

            for (AActor* Actor : FoundEnemies)
            {
                if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
                {
                    if (Enemy->ActorHasTag(FName("CurrentBattle")))
                    {
                        Enemy->CurrentBlock = 0;
                        Enemy->OnBlockChanged.Broadcast(Enemy->CurrentBlock);
                        Enemy->ExecuteIntent(this); 
                    }
                }
            }

           

        }, 1.0f, false);

    // 넉넉하게 4초 뒤에  플레이어 턴으로 넘깁니다.
    FTimerHandle TurnEndTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TurnEndTimerHandle, [this]()
        {
            TurnNumber++;
            StartPlayerTurn();

        }, 4.0f, false);


}

bool ASTSGameMode::TryUseEnergy(int32 Cost)
{
    if (CurrentEnergy >= Cost)
    {
        CurrentEnergy -= Cost;

        if (MainUIWidget)
        {
            MainUIWidget->UpdateEnergyText(CurrentEnergy, MaxEnergy);
        }

        return true; 
    }
    UE_LOG(LogTemp, Warning, TEXT("에너지가 부족합니다!"));
    return false; // 에너지 부족
}



void ASTSGameMode::InitializeDeck()
{
    // MasterDeck에 기본 카드 지급
    // 만약 덱이 0장일 때만 카드를 줍니다. 
    if (MasterDeck.Num() == 0)
    {
        
        for (int i = 0; i < 4; ++i)
        {
            MasterDeck.Add(FName("STRIKE_BASIC"));
        }

        
        for (int i = 0; i < 4; ++i)
        {
            MasterDeck.Add(FName("DEFEND_BASIC"));
        }

        
        MasterDeck.Add(FName("BASH"));
        MasterDeck.Add(FName("Cleave"));
        MasterDeck.Add(FName("TWIN_STRIKE"));
    }

    
    DrawPile.Empty();
    DiscardPile.Empty();

    
    DrawPile = MasterDeck;

    // 뽑을 카드 더미 섞기
    const int32 NumCards = DrawPile.Num();
    for (int32 i = 0; i < NumCards - 1; ++i)
    {
        int32 SwapIdx = FMath::RandRange(i, NumCards - 1);
        DrawPile.Swap(i, SwapIdx);
    }

}

void ASTSGameMode::AddToDiscardPile(FName CardRowName)
{
    DiscardPile.Add(CardRowName);
}


FName ASTSGameMode::DrawCard()
{
    
    if (DrawPile.IsEmpty())
    {
        
        if (DiscardPile.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("덱과 무덤이 모두 비어있습니다!"));
            return NAME_None;
        }

        // 무덤의 카드를 덱으로 모두 가져오고 무덤 비우기
        DrawPile = DiscardPile;
        DiscardPile.Empty();

        // 덱 섞기 (셔플)
        const int32 NumCards = DrawPile.Num();
        for (int32 i = 0; i < NumCards - 1; ++i)
        {
            int32 SwapIdx = FMath::RandRange(i, NumCards - 1);
            DrawPile.Swap(i, SwapIdx);
        }
        UE_LOG(LogTemp, Warning, TEXT("덱이 다 떨어져서 무덤을 섞어 새로운 덱을 만들었습니다!"));
    }

    // 덱의 맨 위(배열의 마지막 요소)를 빼서(Pop) 반환합니다.
    return DrawPile.Pop();
}



void ASTSGameMode::AddStrength(int32 Amount)
{
    CurrentStrength += Amount;
   
    

    
    if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        if (UStatusEffectComponent* StatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
        {
            
            StatusComp->AddStatusEffect(EStatusEffectType::Strength, Amount);
        }
    }
}

void ASTSGameMode::CheckVictory()
{
    TArray<AActor*> FoundEnemies;
   
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("CurrentBattle"), FoundEnemies);

    int32 AliveCount = 0;
    for (AActor* Actor : FoundEnemies)
    {
        if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
        {
            if (Enemy->CurrentHealth > 0)
            {
                AliveCount++; 
            }
        }
    }

    // 살아있는 적이 0명이라면 승리!
    if (AliveCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("모든 적 처치! 전투 승리!"));
       
        if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
        {
            if (UStatusEffectComponent* StatusComp = PlayerChar->FindComponentByClass<UStatusEffectComponent>())
            {
               
                StatusComp->ClearAllStatusEffects();
            }
        }
        if (USTSGameInstance* GI = Cast<USTSGameInstance>(GetGameInstance()))
        {
            
            GI->StopBackgroundMusic(1.5f);

            if (CurrentFloor >= BossFloor)
            {
                // [보스 층 - 엔딩]
                MainUIWidget->ShowGameClear();
               
                GetWorldTimerManager().SetTimer(BGMTimerHandle, this, &ASTSGameMode::PlayEndingBGM, 1.5f, false);
            }
            else
            {
                //  [일반 층 - 다음 층으로]
                MainUIWidget->ShowVictory();
                
                GetWorldTimerManager().SetTimer(BGMTimerHandle, this, &ASTSGameMode::ResumeMapBGM, 1.5f, false);
            }
        }
    }
}


void ASTSGameMode::ResumeMapBGM()
{
    if (USTSGameInstance* GI = Cast<USTSGameInstance>(GetGameInstance()))
    {
        
        if (DefaultBGM)
        {
            GI->PlayBackgroundMusic(DefaultBGM);
        }
    }
}

void ASTSGameMode::PlayEndingBGM()
{
    if (USTSGameInstance* GI = Cast<USTSGameInstance>(GetGameInstance()))
    {
        if (EndingBGM) GI->PlayBackgroundMusic(EndingBGM);
    }
}

void ASTSGameMode::AddWeak(int32 Amount)
{
    WeakStacks += Amount;
}

void ASTSGameMode::DecreasePlayerStatusEffects()
{
    if (WeakStacks > 0)
    {
        WeakStacks--;
    }
}

void ASTSGameMode::GoToNextNode(FName NodeType)
{
    // 층수 증가 및 스탯 초기화
    CurrentFloor++;
    WeakStacks = 0;
    CurrentEnergy = MaxEnergy;

    DrawPile = MasterDeck;
    DiscardPile.Empty();

    if (MainUIWidget)
    {
       
        MainUIWidget->ClearHandUI();
    }

    // 노드 종류에 따른 분기 처리
    if (NodeType == FName("Rest"))
    {
        UE_LOG(LogTemp, Warning, TEXT("모닥불 방 진입! 스폰 시도 중..."));

        
        if (!CampfireClassToSpawn)
        {
            return;
        }

        FActorSpawnParameters SpawnParams;
       
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (CampfireClassToSpawn) {
            FVector SpawnLocation(-9420.0f, 5140.0f, 590.0f);
            FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

            AActor* Campfire = GetWorld()->SpawnActor<AActor>(CampfireClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
        }
       
        if (AnvilClassToSpawn)
        {
            FVector AnvilLoc(-9420.0f, 4640.0f, 400.0f); 
            GetWorld()->SpawnActor<AActor>(AnvilClassToSpawn, AnvilLoc, FRotator::ZeroRotator, SpawnParams);
        }

        return; 
    }

    
    TSubclassOf<class ASTSEnemyCharacter> TargetEnemyClass = EnemyClassToSpawn;

    

    FVector SpawnLocation(-9000.0f, 5140.0f, 520.0f);

    if (CurrentFloor == BossFloor)
    {
        TargetEnemyClass = BossClassToSpawn;

        
        SpawnLocation = FVector(-2390.0f, 4540.0f, 200.0f);

    }

    //  적 소환
    if (TargetEnemyClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        

        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();

            
            SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, PlayerLocation);
        }
        AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(TargetEnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (NewEnemy)
        {
            NewEnemy->Tags.Add(FName("CurrentBattle"));
            NewEnemy->Tags.Add(FName("Enemy"));
        }
    }

    
    
}


TArray<FName> ASTSGameMode::GenerateRandomRewards(int32 Count)
{
    TArray<FName> Rewards;
    if (!CardDataTable) return Rewards;

    TArray<FName> AllRowNames = CardDataTable->GetRowNames();
    TArray<FName> ValidCards; // 보상으로 줄 수 있는 진짜 후보들

    // 엑셀을 쫙 훑으면서 '보상 가능(bIsRewardable)' 체크가 된 카드만 걸러냄
    for (FName RowName : AllRowNames)
    {
        FCardData* CardData = CardDataTable->FindRow<FCardData>(RowName, TEXT(""));
        if (CardData && CardData->bIsRewardable)
        {
            ValidCards.Add(RowName);
        }
    }

    // 걸러진 카드들 중에서 중복 없이 3장을 뽑음
    for (int32 i = 0; i < Count; i++)
    {
        if (ValidCards.Num() == 0) break; 

        int32 RandomIndex = FMath::RandRange(0, ValidCards.Num() - 1);
        Rewards.Add(ValidCards[RandomIndex]);

        // 뽑힌 카드는 후보 목록에서 제외 
        ValidCards.RemoveAt(RandomIndex);
    }

    return Rewards;
}

void ASTSGameMode::AddCardToMasterDeck(FName NewCardName)
{
    
    MasterDeck.Add(NewCardName);
    
}

void ASTSGameMode::RestartGame()
{
    UE_LOG(LogTemp, Warning, TEXT("게임을 리셋하고 1층부터 다시 시작합니다!"));

    // 현재 열려있는 레벨을 다시 열어서 게임을 초기 상태로 리셋
    FString CurrentLevelName = GetWorld()->GetName();
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName), false);
}

void ASTSGameMode::UpgradeCardInDeck(int32 CardIndex, FName UpgradedCardID)
{
    
    if (MasterDeck.IsValidIndex(CardIndex))
    {
        FName OldCard = MasterDeck[CardIndex];
		MasterDeck[CardIndex] = UpgradedCardID; // 해당 인덱스의 카드를 업그레이드된 카드 ID로 교체

    }
}

// 카드가 버려질 때마다 무덤에 추가하는 함수입니다.
void ASTSGameMode::AddCardToDiscardPile(FName CardName)
{
    
    DiscardPile.Add(CardName);

}
