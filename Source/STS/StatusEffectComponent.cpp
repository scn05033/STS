#include "StatusEffectComponent.h"

UStatusEffectComponent::UStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 틱(매 프레임 연산)은 필요 없으니 꺼줍니다 (최적화)
}

void UStatusEffectComponent::AddStatusEffect(EStatusEffectType EffectType, int32 Amount)
{
    if (EffectType == EStatusEffectType::None || Amount == 0) return;

    // 맵(그릇)에 상태 이상 수치를 더하거나 새로 추가합니다.
    if (CurrentStatusMap.Contains(EffectType))
    {
        CurrentStatusMap[EffectType] += Amount;
    }
    else
    {
        CurrentStatusMap.Add(EffectType, Amount);
    }

    // "내 상태가 변했다!" 라고 델리게이트를 통해 방송(Broadcast)합니다.
    OnStatusChanged.Broadcast();
}

void UStatusEffectComponent::DecreaseAllStatuses()
{
    // 스택이 0이 되어 삭제할 상태 이상들을 모아둘 배열
    TArray<EStatusEffectType> KeysToRemove;

    // 맵을 순회하면서 스택을 1씩 깎음
    for (auto& Elem : CurrentStatusMap)
    {
        Elem.Value -= 1;
        UE_LOG(LogTemp, Warning, TEXT("[상태이상 감소] 타입: %d, 남은 스택: %d"), (int32)Elem.Key, Elem.Value);
        // 스택이 0 이하가 되면 삭제 리스트에 추가
        if (Elem.Value <= 0)
        {
            KeysToRemove.Add(Elem.Key);
        }
    }

    // 0이 된 상태 이상을 맵에서 완전히 제거
    for (EStatusEffectType Key : KeysToRemove)
    {
        CurrentStatusMap.Remove(Key);
        UE_LOG(LogTemp, Warning, TEXT("[상태이상 소멸] 스택이 0이 되어 삭제되었습니다!"));
    }
    OnStatusChanged.Broadcast();
    
}

void UStatusEffectComponent::ClearAllStatusEffects()
{
   

   
    CurrentStatusMap.Empty();
    

    UE_LOG(LogTemp, Warning, TEXT("전투 종료! 플레이어의 모든 상태 이상이 해제되었습니다."));

    OnStatusChanged.Broadcast();
}