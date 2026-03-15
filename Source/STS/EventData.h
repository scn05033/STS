#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EventData.generated.h" 

USTRUCT(BlueprintType)
struct FEventData : public FTableRowBase
{
    GENERATED_BODY()

    // ==========================================
    // 1. 이벤트 기본 텍스트
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventDescription;

    // ==========================================
    // 2. [선택지 1]
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 1")
    FString Choice1_Text;

    // 어떤 행동을 할지 결정 (예: "Heal", "TakeDamage", "AddCard", "RemoveCard", "None")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 1")
    FName Choice1_ActionType;

    // 데미지나 회복량 등 수치가 필요할 때 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 1")
    int32 Choice1_Value;

    // 카드를 주거나 뺏을 때 사용할 카드 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 1")
    FName Choice1_CardID;

    // ==========================================
    // 3. [선택지 2]
    // ==========================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 2")
    FString Choice2_Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 2")
    FName Choice2_ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 2")
    int32 Choice2_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice 2")
    FName Choice2_CardID;
};