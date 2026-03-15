// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSGameMode.h"
#include "STSUserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Widget.h"
#include "CardDataStruct.h"
#include "STSEnemyCharacter.h"
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
    UE_LOG(LogTemp, Warning, TEXT("[디버그] 게임/전투 시작 직후 MasterDeck 개수: %d"), MasterDeck.Num());
	MainUIWidget = InUIWidget;
    TurnNumber = 1;

    // 전투 시작 시 플레이어 체력바 꽉 채워주기
    if (MainUIWidget)
    {
        MainUIWidget->UpdatePlayerHP(CurrentHealth, MaxHealth);
    }

    InitializeDeck();
    StartPlayerTurn();
}

void ASTSGameMode::StartPlayerTurn()
{
    
    CurrentTurnState = ETurnState::PlayerTurn;

    CurrentEnergy = MaxEnergy;
    CurrentBlock = 0;

    // 내 턴이 시작될 때 디버프가 1 줄어듭니다
    DecreasePlayerStatusEffects();

    if (MainUIWidget)
    {
        MainUIWidget->AddCards(5);

        // UI 수치 초기화 갱신
        MainUIWidget->UpdateEnergyText(CurrentEnergy, MaxEnergy);
        MainUIWidget->UpdateBlockText(CurrentBlock);
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

                Enemy->DecreaseStatusEffects();
            }
        }
    }

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
    

    
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            TArray<AActor*> FoundEnemies;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), FoundEnemies);

            // 모든 적이 일제히 플레이어를 향해 행동(공격/방어)을 실행합니다.
            for (AActor* Actor : FoundEnemies)
            {
                if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
                {
                    if (Enemy->ActorHasTag(FName("CurrentBattle")))
                    {
                        Enemy->CurrentBlock = 0;
                        Enemy->ExecuteIntent(this);

                    }
                }
            }

            TurnNumber++; // 턴 수 증가
            StartPlayerTurn(); // 다시 플레이어 턴으로! (루프)

        }, 2.0f, false);


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

        return true; // 사용 성공
    }
    UE_LOG(LogTemp, Warning, TEXT("에너지가 부족합니다!"));
    return false; // 에너지 부족
}

void ASTSGameMode::AddBlock(int32 Amount)
{
    CurrentBlock += Amount;

    if (MainUIWidget)
    {
        MainUIWidget->UpdateBlockText(CurrentBlock);
    }
   // UE_LOG(LogTemp, Warning, TEXT(" 방어도 획득: %d (현재 총 방어도: %d)"), Amount, CurrentBlock);
}

void ASTSGameMode::InitializeDeck()
{
    // [게임 최초 시작] 내 지갑(MasterDeck)에 기본 카드 지급
    // 만약 덱이 0장일 때만 카드를 줍니다. 
    if (MasterDeck.Num() == 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            MasterDeck.Add(FName("STRIKE_BASIC"));
            // MasterDeck.Add(FName("DEFEND_BASIC")); 

            //MasterDeck.Add(FName("SHRUG_IT_OFF"));

            // MasterDeck.Add(FName("BASH"));

            // MasterDeck.Add(FName("Cleave"));


            MasterDeck.Add(FName("INFLAME"));
            MasterDeck.Add(FName("TWIN_STRIKE"));
            MasterDeck.Add(FName("SEEING_RED"));
        }
        UE_LOG(LogTemp, Warning, TEXT("마스터 덱 최초 생성 완료! 내 전 재산: %d장"), MasterDeck.Num());
    }

    // [전투 준비] 테이블 싹 비우기
    DrawPile.Empty();
    DiscardPile.Empty();

    // [핵심 로직] 내 전 재산을 이번 전투 뽑을 카드 더미로 완벽 복사
    DrawPile = MasterDeck;

    // [셔플] 뽑을 카드 더미 섞기
    const int32 NumCards = DrawPile.Num();
    for (int32 i = 0; i < NumCards - 1; ++i)
    {
        int32 SwapIdx = FMath::RandRange(i, NumCards - 1);
        DrawPile.Swap(i, SwapIdx);
    }

    UE_LOG(LogTemp, Warning, TEXT("전투 덱(DrawPile) 세팅 및 셔플 완료! 전투에 쓸 카드: %d장"), DrawPile.Num());
}

void ASTSGameMode::AddToDiscardPile(FName CardRowName)
{
    DiscardPile.Add(CardRowName);
    UE_LOG(LogTemp, Log, TEXT("무덤으로 감: %s (현재 무덤: %d장)"), *CardRowName.ToString(), DiscardPile.Num());
}

FName ASTSGameMode::DrawCard()
{
    // 덱이 비어있는지 확인
    if (DrawPile.IsEmpty())
    {
        // 무덤도 비어있으면 줄 카드가 없음 (탈진 상태)
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

void ASTSGameMode::TakePlayerDamage(int32 Damage)
{
    int32 ActualDamage = Damage;

    // 방어도가 있다면 먼저 깎기
    if (CurrentBlock > 0)
    {
        if (CurrentBlock >= ActualDamage)
        {
            // 방어도가 충분해서 다 막음
            CurrentBlock -= ActualDamage;
            ActualDamage = 0;
            UE_LOG(LogTemp, Warning, TEXT("방어도로 완벽히 막았습니다! (남은 방어도: %d)"), CurrentBlock);
        }
        else
        {
            // 방어도가 깨지고 데미지가 관통함
            ActualDamage -= CurrentBlock;
            UE_LOG(LogTemp, Warning, TEXT("방어도가 파괴되고, %d의 피해가 관통했습니다!"), ActualDamage);
            CurrentBlock = 0;
        }
    }

    // 남은 데미지를 체력에서 깎기
    if (ActualDamage > 0)
    {
        CurrentHealth -= ActualDamage;
        UE_LOG(LogTemp, Error, TEXT("플레이어가 %d의 피해를 입었습니다! (남은 체력: %d / %d)"), ActualDamage, CurrentHealth, MaxHealth);

        if (CurrentHealth <= 0)
        {
            CurrentHealth = 0;
            //UE_LOG(LogTemp, Error, TEXT("플레이어 사망! 게임 오버!"));
            
        }
        //ui 갱신
        if (MainUIWidget)
        {
            MainUIWidget->UpdatePlayerHP(CurrentHealth, MaxHealth);
        }
        // 사망 처리
        if (CurrentHealth <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("플레이어 사망! 게임 오버!"));
            if (MainUIWidget)
            {
                MainUIWidget->ShowGameOver(); // 게임 오버 화면 띄우기
            }
        }
    }

    // UI에 바뀐 방어도/체력 알려주기
    if (MainUIWidget)
    {
        MainUIWidget->UpdateBlockText(CurrentBlock);
    }
}

void ASTSGameMode::AddStrength(int32 Amount)
{
    CurrentStrength += Amount;

    // 로그로 잘 올라갔는지 확인!
    UE_LOG(LogTemp, Warning, TEXT("힘 증가! 현재 힘: %d"), CurrentStrength);
}

void ASTSGameMode::CheckVictory()
{
    TArray<AActor*> FoundEnemies;
    // 이 전투에 참여한 적들만 긁어옵니다.
    UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ASTSEnemyCharacter::StaticClass(), FName("CurrentBattle"), FoundEnemies);

    int32 AliveCount = 0;
    for (AActor* Actor : FoundEnemies)
    {
        if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
        {
            if (Enemy->CurrentHealth > 0)
            {
                AliveCount++; // 살아있는 적 숫자 세기
            }
        }
    }

    // 살아있는 적이 0명이라면 승리!
    if (AliveCount == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("모든 적 처치! 전투 승리!"));
        UE_LOG(LogTemp, Error, TEXT("[디버그] 전투 종료 직후 MasterDeck 개수: %d"), MasterDeck.Num());
        // 지금이 보스 층(5층)이거나 그 이상이라면 엔딩 화면
        if (CurrentFloor >= BossFloor)
        {
            MainUIWidget->ShowGameClear();
        }
        else
        {
            // 일반 층이라면? -> 평소처럼 카드 보상 화면!
            MainUIWidget->ShowVictory();
        }
    }
}

void ASTSGameMode::AddWeak(int32 Amount)
{
    WeakStacks += Amount;
    UE_LOG(LogTemp, Warning, TEXT("플레이어가 '약화'에 걸렸습니다! (현재 %d 스택)"), WeakStacks);
}

void ASTSGameMode::DecreasePlayerStatusEffects()
{
    if (WeakStacks > 0)
    {
        WeakStacks--;
        UE_LOG(LogTemp, Warning, TEXT("플레이어의 약화 스택이 감소했습니다. (남은 스택: %d)"), WeakStacks);
    }
}

void ASTSGameMode::GoToNextNode(FName NodeType)
{
    // 층수 증가 및 스탯 초기화
    CurrentFloor++;
    CurrentBlock = 0;
    WeakStacks = 0;
    CurrentEnergy = MaxEnergy;

    DrawPile = MasterDeck;
    DiscardPile.Empty();

    if (MainUIWidget)
    {
        // 맵 UI나 기타 창들을 닫아주는 로직 (나중에 추가)
        MainUIWidget->ClearHandUI();
    }

    // 노드 종류에 따른 분기 처리
    if (NodeType == FName("Rest"))
    {
        UE_LOG(LogTemp, Warning, TEXT("모닥불 방 진입! 스폰 시도 중..."));

        // 블루프린트에 모닥불 클래스가 제대로 들어있는지 확인
        if (!CampfireClassToSpawn)
        {
            UE_LOG(LogTemp, Error, TEXT("에러: CampfireClassToSpawn이 비어있습니다! BP_STSGameMode에서 다시 넣어주세요."));
            return;
        }

        FActorSpawnParameters SpawnParams;
        // 바닥에 살짝 겹쳐도 무조건 강제로 스폰
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (CampfireClassToSpawn) {
            FVector SpawnLocation(-9420.0f, 5140.0f, 590.0f);
            FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

            AActor* Campfire = GetWorld()->SpawnActor<AActor>(CampfireClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
        }
        // 스폰이 진짜로 성공했는지 확인
       /*if (Campfire)
        {
            UE_LOG(LogTemp, Warning, TEXT("모닥불 스폰 완벽 성공!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("에러: 모닥불 스폰 함수가 실패했습니다! "));
        }
        */
        if (AnvilClassToSpawn)
        {
            FVector AnvilLoc(-9420.0f, 4640.0f, 590.0f); // Y축을 500으로 (오른쪽)
            GetWorld()->SpawnActor<AActor>(AnvilClassToSpawn, AnvilLoc, FRotator::ZeroRotator, SpawnParams);
        }

        return; // 전투를 막기 위해 함수 종료
    }

    // 전투 노드라면 적 소환 (보스인지 확인)
    TSubclassOf<class ASTSEnemyCharacter> TargetEnemyClass = EnemyClassToSpawn;

    

    if (CurrentFloor == BossFloor)
    {
        TargetEnemyClass = BossClassToSpawn;
        UE_LOG(LogTemp, Error, TEXT("보스 등장! 최종 전투 준비!"));
    }

    //  적 소환! 
    if (TargetEnemyClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        FVector SpawnLocation(-9420.0f, 5140.0f, 590.0f);
        FRotator SpawnRotation(0.0f, 180.0f, 0.0f);


        AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(TargetEnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (NewEnemy)
        {
            NewEnemy->Tags.Add(FName("CurrentBattle"));
            NewEnemy->Tags.Add(FName("Enemy"));
        }
    }

    // 새 턴 시작
    StartPlayerTurn();
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
        if (ValidCards.Num() == 0) break; // 더 이상 뽑을 카드가 없으면 멈춤

        int32 RandomIndex = FMath::RandRange(0, ValidCards.Num() - 1);
        Rewards.Add(ValidCards[RandomIndex]);

        // 뽑힌 카드는 후보 목록에서 제외 (똑같은 카드가 3장 뜨는 것 방지)
        ValidCards.RemoveAt(RandomIndex);
    }

    return Rewards;
}

void ASTSGameMode::AddCardToMasterDeck(FName NewCardName)
{
    // 내 영구 덱에 카드를 추가합니다!
    MasterDeck.Add(NewCardName);
    //UE_LOG(LogTemp, Warning, TEXT("덱 강화 완료! 새 카드가 마스터 덱에 추가되었습니다: %s"), *NewCardName.ToString());
    UE_LOG(LogTemp, Warning, TEXT("[디버그] 방금 카드를 Add 했습니다. 현재 MasterDeck 총 카드 수: %d"), MasterDeck.Num());
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
    //인덱스 유효성 검사
    if (MasterDeck.IsValidIndex(CardIndex))
    {
        FName OldCard = MasterDeck[CardIndex];
		MasterDeck[CardIndex] = UpgradedCardID; // 해당 인덱스의 카드를 업그레이드된 카드 ID로 교체

        UE_LOG(LogTemp, Warning, TEXT("카드 강화 완료! [%s] ➡️ [%s]"), *OldCard.ToString(), *UpgradedCardID.ToString());
    }
}

