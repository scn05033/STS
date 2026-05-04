#include "STSGameInstance.h"
#include "Kismet/GameplayStatics.h" 

void USTSGameInstance::PlayBackgroundMusic(USoundBase* MusicToPlay)
{
    // 기존에 플레이어가 있고 소리가 나오고 있다면 중지
    if (BackgroundMusicPlayer && BackgroundMusicPlayer->IsPlaying())
    {
        BackgroundMusicPlayer->Stop();
    }

    // 사운드를 스폰하면서 해당 게임 인스턴스에 종속시킵니다.
    
    BackgroundMusicPlayer = UGameplayStatics::SpawnSound2D(this, MusicToPlay, 1.0f, 1.0f, 0.0f, nullptr, true);
}

void USTSGameInstance::StopBackgroundMusic(float FadeOutDuration)
{
    if (BackgroundMusicPlayer && BackgroundMusicPlayer->IsPlaying())
    {
        //맵 이동 직전에 호출해서 페이드를 겁니다.
        BackgroundMusicPlayer->FadeOut(FadeOutDuration, 0.0f);
    }
}