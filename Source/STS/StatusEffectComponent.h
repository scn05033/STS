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



// UI 업데이트를 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusChangedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))




class STS_API UStatusEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStatusEffectComponent();

    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Status")
    TMap<EStatusEffectType, int32> CurrentStatusMap;

    
    UFUNCTION(BlueprintCallable, Category = "Status")
    void AddStatusEffect(EStatusEffectType EffectType, int32 Amount);

    
    UPROPERTY(BlueprintAssignable, Category = "Status|Event")
    FOnStatusChangedSignature OnStatusChanged;

    
    void DecreaseAllStatuses();

    
    UFUNCTION(BlueprintCallable, Category = "Status Effect")
    void ClearAllStatusEffects();
};