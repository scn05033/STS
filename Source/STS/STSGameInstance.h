#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "STSGameInstance.generated.h"

UCLASS()
class STS_API USTSGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    //맵 이동에도 살아남을 BGM 재생기 변수
    UPROPERTY()
    UAudioComponent* BackgroundMusicPlayer;

    // 블루프린트에서 BGM을 트는 함수
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void PlayBackgroundMusic(USoundBase* MusicToPlay);

    // 블루프린트에서 BGM을 페이드 아웃/중지하는 함수
    UFUNCTION(BlueprintCallable, Category = "BGM")
    void StopBackgroundMusic(float FadeOutDuration);
};