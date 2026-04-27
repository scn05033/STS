// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "STSEnemyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASTSCharacter

ASTSCharacter::ASTSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASTSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASTSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASTSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASTSCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASTSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASTSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


void ASTSCharacter::PlayAttackAnim()
{
	// 몽타주가 세팅되어 있다면 재생해라!
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void ASTSCharacter::PlayHitReactAnim()
{
	// 몽타주가 세팅되어 있다면 재생해라!
	if (AttackMontage)
	{
		PlayAnimMontage(HitReactMontage);
	}
}

void ASTSCharacter::ExecuteHit()
{
	// 광역 공격일 경우
	if (bIsAoEAttack && PendingDamage > 0)
	{
		TArray<AActor*> FoundEnemies;

		// 태그(Tag) 검사를 빼버리고 맵에 있는 '적 캐릭터'는 무조건 다 가져옵니다 (가장 확실함)
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), FoundEnemies);

		for (AActor* Actor : FoundEnemies)
		{
			if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
			{
				UGameplayStatics::ApplyDamage(
					Enemy,
					PendingDamage,
					GetController(),
					this,
					UDamageType::StaticClass()
				);
				if (PendingStatusAmount > 0 && PendingStatusType == FName("Vulnerable"))
				{
					Enemy->VulnerableStacks += PendingStatusAmount;
					UE_LOG(LogTemp, Warning, TEXT("[광역] %s에게 취약 %d 스택 부여! (현재: %d)"), *Enemy->GetName(), PendingStatusAmount, Enemy->VulnerableStacks);
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("모든 적에게 %d 데미지를 입혔습니다!"), PendingDamage);
	}

	// 단일 공격일 경우
	else if (CurrentTarget && PendingDamage > 0)
	{
		UGameplayStatics::ApplyDamage(
			CurrentTarget,
			PendingDamage,
			GetController(),
			this,
			UDamageType::StaticClass()
		);
		if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(CurrentTarget))
		{
			if (PendingStatusAmount > 0 && PendingStatusType == FName("Vulnerable"))
			{
				Enemy->VulnerableStacks += PendingStatusAmount;
				UE_LOG(LogTemp, Warning, TEXT("[단일] %s에게 취약 %d 스택 부여! (현재: %d)"), *Enemy->GetName(), PendingStatusAmount, Enemy->VulnerableStacks);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT(" 적에게 %d 데미지를 입혔습니다."), PendingDamage);
	}
	PendingStatusAmount = 0;
	PendingStatusType = NAME_None;

	
}

void ASTSCharacter::AddBlock(int32 Amount)
{
	CurrentBlock += Amount;
	OnBlockChanged.Broadcast(CurrentBlock); // 방어도 UI 업데이트 방송
}

void ASTSCharacter::TakePlayerDamage(int32 Damage)
{
	int32 ActualDamage = Damage;

	// 방어도가 있다면 먼저 깎기
	if (CurrentBlock > 0)
	{
		if (CurrentBlock >= ActualDamage)
		{
			// 방어도가 충분해서 다 막음
			CurrentBlock -= ActualDamage;
			// 소리 재생
			UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
			// 방어 파티클 재생
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BlockVFX, GetActorLocation());

			ActualDamage = 0;
			UE_LOG(LogTemp, Warning, TEXT("방어도로 완벽히 막았습니다! (남은 방어도: %d)"), CurrentBlock);
		}
		else
		{
			// 방어도가 깨지고 데미지가 관통함
			ActualDamage -= CurrentBlock;
			// 소리 재생
			UGameplayStatics::PlaySoundAtLocation(this, ShieldBreakSound, GetActorLocation());
			//UI 애니메이션 재생
			OnShieldBroken();
			// 파티클 재생
			//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShieldBreakVFX, GetActorLocation());


			UE_LOG(LogTemp, Warning, TEXT("방어도가 파괴되고, %d의 피해가 관통했습니다!"), ActualDamage);
			CurrentBlock = 0;
		}

		// 방어도 UI 업데이트 방송
		OnBlockChanged.Broadcast(CurrentBlock);
	}

	// 남은 데미지를 체력에서 깎기
	if (ActualDamage > 0)
	{
		CurrentHealth -= ActualDamage;
		UE_LOG(LogTemp, Error, TEXT("플레이어가 %d의 피해를 입었습니다! (남은 체력: %d / %d)"), ActualDamage, CurrentHealth, MaxHealth);

		// 소리 재생 
		UGameplayStatics::PlaySoundAtLocation(this, FleshHitSound, GetActorLocation());
		//파티클 재생
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodVFX, GetActorLocation());

		if (CurrentHealth <= 0)
		{
			CurrentHealth = 0;
			//UE_LOG(LogTemp, Error, TEXT("플레이어 사망! 게임 오버!"));

		}
		// 체력 UI 업데이트 방송
		OnHealthChanged.Broadcast(CurrentHealth);
		//피격 애니메이션 재생 
		if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			if (PlayerChar->HitReactMontage)
			{
				PlayerChar->PlayAnimMontage(PlayerChar->HitReactMontage);
			}
			if (PlayerChar->HitEffect)
			{
				// 이펙트를 내 몸통 위치(GetActorLocation)에서 펑 터뜨립니다!
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerChar->HitEffect, PlayerChar->GetActorLocation());
			}
		}





		// 사망 처리
		if (CurrentHealth <= 0)
		{
			//UE_LOG(LogTemp, Error, TEXT("플레이어 사망! 게임 오버!"));
			// 게임 오버 방송
			OnPlayerDeath.Broadcast();
		}
	}
}

void ASTSCharacter::HealPlayer(int32 HealAmount)
{
	// 체력을 회복시키고, MaxHealth를 넘지 않게 잘라줍니다
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0, MaxHealth);

	// 체력 UI 업데이트 방송
	OnHealthChanged.Broadcast(CurrentHealth);
}


void ASTSCharacter::EnqueueAction(const FActionCommand& NewAction)
{
	// 명령서를 큐에 넣습니다.
	ActionQueue.Add(NewAction);

	// 큐에 잘 들어왔는가지 로그로 확인
	UE_LOG(LogTemp, Warning, TEXT("큐에 명령 접수됨! 현재 대기열 수: %d"), ActionQueue.Num());

	if (!bIsProcessingAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("캐릭터가 놀고 있으므로 즉시 실행합니다!"));
		ProcessNextAction();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[경고] 캐릭터가 아직 이전 행동을 끝내지 않아 대기합니다! (Complete 호출 누락 의심)"));
	}
}

void ASTSCharacter::ProcessNextAction()
{

	UE_LOG(LogTemp, Warning, TEXT("현재 큐 잔량: %d"), ActionQueue.Num());
	// 큐가 비어있으면 퇴근
	if (ActionQueue.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("큐가 비었습니다. Idle 상태로 복귀합니다."));
		bIsProcessingAction = false;
		bIsExecutingAction = false; // 일 끝남
		return;
	}
	if (bIsExecutingAction) return;
	// 일 시작
	bIsProcessingAction = true;
	bIsExecutingAction = true; // 이제부터 진짜 일 시작!

	// 큐의 맨 앞(0번) 명령서를 꺼내서 읽습니다.
	FActionCommand CurrentAction = ActionQueue[0];

	// 캐릭터의 상태를 명령서대로 세팅합니다.
	bIsAoEAttack = CurrentAction.bIsAoE;
	CurrentTarget = CurrentAction.TargetEnemy;
	PendingDamage = CurrentAction.Damage;
	CurrentTarget = CurrentAction.TargetEnemy;
	PendingStatusType = CurrentAction.StatusType;     
	PendingStatusAmount = CurrentAction.StatusAmount;

	if (CurrentAction.ActionType == FName("Attack"))
	{
		if (CurrentAction.bIsAoE)
		{
			PlayInPlaceAnimation(CurrentAction.Montage, CurrentAction.VFX);
			//CompleteCurrentAction();
		}
		else
		{
			DashAndAttack(CurrentTarget, CurrentAction.Montage, CurrentAction.VFX);
		}
	}
	else if (CurrentAction.ActionType == FName("Defend"))
	{
		// 방어 애니메이션은 제자리에서 재생
		PlayInPlaceAnimation(CurrentAction.Montage, CurrentAction.VFX);
		//CompleteCurrentAction();
	}
	else
	{
		// 애니메이션이 없는 기타 카드라면 즉시 넘김
		CompleteCurrentAction();
	}
}

void ASTSCharacter::CompleteCurrentAction()
{

	UE_LOG(LogTemp, Log, TEXT("액션 완료 보고 접수됨."));
	// 만약 실행 중인 액션이 없는데 호출됐다면 무시한다
	if (!bIsExecutingAction)
	{
		UE_LOG(LogTemp, Error, TEXT("오류: 실행 중인 액션이 없는데 Complete가 호출됨!"));		return;
	}
	// 플래그를 끄고 큐를 정리합니다.
	bIsExecutingAction = false;
	// 방금 끝낸 행동을 큐에서 지워버립니다.
	if (ActionQueue.Num() > 0)
	{
		ActionQueue.RemoveAt(0);
	}

	// 다음 명령서가 있는지 확인하고 실행합니다.
	ProcessNextAction();
}
	