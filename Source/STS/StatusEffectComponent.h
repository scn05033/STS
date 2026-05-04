#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffectComponent.generated.h"

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
    None        UMETA(DisplayName = "없음"),
    Vulnerable  UMETA(DisplayName = "취약 (Vulnerable)"),
    Weak        UMETA(DisplayName = "약화 (Weak)"),
    Strength      UMETA(DisplayName = "힘 (Strength)")
};



// UI 업데이트를 위한 델리게이트(이벤트 디스패처) 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusChangedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))




class STS_API UStatusEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStatusEffectComponent();

    // 상태 이상을 담을 딕셔너리 (그릇)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    TMap<EStatusEffectType, int32> CurrentStatusMap;

    // 상태 이상 부여 함수
    UFUNCTION(BlueprintCallable, Category = "Status")
    void AddStatusEffect(EStatusEffectType EffectType, int32 Amount);

    // 상태가 변할 때마다 호출될 방송국
    UPROPERTY(BlueprintAssignable, Category = "Status|Event")
    FOnStatusChangedSignature OnStatusChanged;

    // 턴이 끝날 때/시작할 때 모든 상태 이상 스택을 1씩 깎는 함수
    void DecreaseAllStatuses();

    // 전투 종료 시 모든 상태 이상을 초기화하는 함수
    UFUNCTION(BlueprintCallable, Category = "Status Effect")
    void ClearAllStatusEffects();
};