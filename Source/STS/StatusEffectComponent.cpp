#include "StatusEffectComponent.h"

UStatusEffectComponent::UStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 틱(매 프레임 연산)은 필요 없으니 꺼줍니다 
}

// 상태 이상을 추가하는 함수입니다. EffectType이 None이거나 Amount가 0이면 아무 작업도 하지 않습니다.
void UStatusEffectComponent::AddStatusEffect(EStatusEffectType EffectType, int32 Amount)
{
    if (EffectType == EStatusEffectType::None || Amount == 0) return;

    if (CurrentStatusMap.Contains(EffectType))
    {
        CurrentStatusMap[EffectType] += Amount;
    }
    else
    {
        CurrentStatusMap.Add(EffectType, Amount);
    }

    
    OnStatusChanged.Broadcast();
}

// 모든 상태 이상 스택을 1씩 감소시키는 함수입니다. 스택이 0이하가 된 상태 이상은 맵에서 제거됩니다.
void UStatusEffectComponent::DecreaseAllStatuses()
{
    
    TArray<EStatusEffectType> KeysToRemove;

    
    for (auto& Elem : CurrentStatusMap)
    {
        if (Elem.Key == EStatusEffectType::Vulnerable || Elem.Key == EStatusEffectType::Weak)
        {
            Elem.Value -= 1;

            if (Elem.Value <= 0)
            {
                KeysToRemove.Add(Elem.Key);
            }
        }
    }

    
    for (EStatusEffectType Key : KeysToRemove)
    {
        CurrentStatusMap.Remove(Key);
       
    }
    OnStatusChanged.Broadcast();
    
}

// 모든 상태 이상을 제거하는 함수입니다. 전투 종료 시 호출되어 플레이어의 상태 이상이 초기화됩니다.
void UStatusEffectComponent::ClearAllStatusEffects()
{
   

   
    CurrentStatusMap.Empty();
    

    UE_LOG(LogTemp, Warning, TEXT("전투 종료! 플레이어의 모든 상태 이상이 해제되었습니다."));

    OnStatusChanged.Broadcast();
}