// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSCharacter.h"
#include "STSUserWidget.h"
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
#include "StatusEffectComponent.h"
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
	
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void ASTSCharacter::PlayHitReactAnim()
{
	
	if (AttackMontage)
	{
		PlayAnimMontage(HitReactMontage);
	}
}

// 데미지 계산과 상태이상 적용을 담당하는 함수
void ASTSCharacter::ExecuteHit()
{
	
	float FinalActualDamage = PendingDamage;

	
	if (UStatusEffectComponent* MyStatusComp = FindComponentByClass<UStatusEffectComponent>())
	{
		int32 WeakStacks = MyStatusComp->CurrentStatusMap.FindRef(EStatusEffectType::Weak);
		if (WeakStacks > 0)
		{
			
			FinalActualDamage *= 0.75f;
		}
	}

	
	if (bIsAoEAttack && FinalActualDamage > 0)
	{
		TArray<AActor*> FoundEnemies;
		
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTSEnemyCharacter::StaticClass(), FoundEnemies);

		for (AActor* Actor : FoundEnemies)
		{
			if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(Actor))
			{
				
				UGameplayStatics::ApplyDamage(
					Enemy,
					FinalActualDamage,
					GetController(),
					this,
					UDamageType::StaticClass()
				);

				
				if (PendingStatusAmount > 0)
				{
					EStatusEffectType TypeToApply = EStatusEffectType::None;
					if (PendingStatusType == FName("Vulnerable")) TypeToApply = EStatusEffectType::Vulnerable;
					else if (PendingStatusType == FName("Weak")) TypeToApply = EStatusEffectType::Weak; 

					if (TypeToApply != EStatusEffectType::None)
					{
						if (UStatusEffectComponent* StatusComp = Enemy->FindComponentByClass<UStatusEffectComponent>())
						{
							StatusComp->AddStatusEffect(TypeToApply, PendingStatusAmount);
						}
					}
				}
			}
		}
	}

	
	else if (CurrentTarget && FinalActualDamage > 0)
	{
		
		UGameplayStatics::ApplyDamage(
			CurrentTarget,
			FinalActualDamage,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		if (ASTSEnemyCharacter* Enemy = Cast<ASTSEnemyCharacter>(CurrentTarget))
		{
			if (PendingStatusAmount > 0)
			{
				EStatusEffectType TypeToApply = EStatusEffectType::None;
				if (PendingStatusType == FName("Vulnerable")) TypeToApply = EStatusEffectType::Vulnerable;
				else if (PendingStatusType == FName("Weak")) TypeToApply = EStatusEffectType::Weak;

				if (TypeToApply != EStatusEffectType::None)
				{
					if (UStatusEffectComponent* StatusComp = Enemy->FindComponentByClass<UStatusEffectComponent>())
					{
						StatusComp->AddStatusEffect(TypeToApply, PendingStatusAmount);
					}
				}
			}
		}
		
	}

	
	PendingStatusAmount = 0;
	PendingStatusType = NAME_None;
}

void ASTSCharacter::AddBlock(int32 Amount)
{
	CurrentBlock += Amount;
	OnBlockChanged.Broadcast(CurrentBlock); 
}

// 데미지를 받아서 방어도로 먼저 막고, 남은 데미지는 체력에서 깎는 함수
void ASTSCharacter::TakePlayerDamage(int32 Damage)
{
	int32 ActualDamage = Damage;

	// 방어도가 0보다 크면 먼저 방어도로 막습니다
	if (CurrentBlock > 0)
	{
		if (CurrentBlock >= ActualDamage)
		{
			
			CurrentBlock -= ActualDamage;
			
			UGameplayStatics::PlaySoundAtLocation(this, BlockSound, GetActorLocation());
			
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BlockVFX, GetActorLocation());

			ActualDamage = 0;
		}
		else
		{
			
			ActualDamage -= CurrentBlock;
			
			UGameplayStatics::PlaySoundAtLocation(this, ShieldBreakSound, GetActorLocation());
			
			OnShieldBroken();
			


			CurrentBlock = 0;
		}

		
		OnBlockChanged.Broadcast(CurrentBlock);
	}

	// 남은 데미지가 0보다 크면 체력에서 깎습니다
	if (ActualDamage > 0)
	{
		CurrentHealth -= ActualDamage;

		
		UGameplayStatics::PlaySoundAtLocation(this, FleshHitSound, GetActorLocation());
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodVFX, GetActorLocation());

		if (CurrentHealth <= 0)
		{
			CurrentHealth = 0;
			

		}
		
		OnHealthChanged.Broadcast(CurrentHealth);
		
		if (ASTSCharacter* PlayerChar = Cast<ASTSCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			if (PlayerChar->HitReactMontage)
			{
				PlayerChar->PlayAnimMontage(PlayerChar->HitReactMontage);
			}
			if (PlayerChar->HitEffect)
			{
				
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerChar->HitEffect, PlayerChar->GetActorLocation());
			}
		}





		// 사망 처리
		if (CurrentHealth <= 0)
		{
			
			
			OnPlayerDeath.Broadcast();
		}
	}
}

// 체력을 회복시키는 함수
void ASTSCharacter::HealPlayer(int32 HealAmount)
{
	
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0, MaxHealth);

	
	OnHealthChanged.Broadcast(CurrentHealth);
}

// 행동 명령서를 받아서 큐에 넣고, 캐릭터가 놀고 있으면 즉시 실행하는 함수
void ASTSCharacter::EnqueueAction(const FActionCommand& NewAction)
{
	
	ActionQueue.Add(NewAction);

	
	

	if (!bIsProcessingAction)
	{
		
		ProcessNextAction();
	}
	else
	{
	}
}

// 큐의 맨 앞 행동을 꺼내서 실행하는 함수
void ASTSCharacter::ProcessNextAction()
{

	
	
	if (ActionQueue.Num() == 0)
	{
		
		bIsProcessingAction = false;
		bIsExecutingAction = false; 
		return;
	}
	if (bIsExecutingAction) return;
	
	bIsProcessingAction = true;
	bIsExecutingAction = true;

	
	FActionCommand CurrentAction = ActionQueue[0];

	
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
			
		}
		else
		{
			DashAndAttack(CurrentTarget, CurrentAction.Montage, CurrentAction.VFX);
		}
	}
	else if (CurrentAction.ActionType == FName("Defend"))
	{
		
		PlayInPlaceAnimation(CurrentAction.Montage, CurrentAction.VFX);
		
	}
	else
	{

		PlayInPlaceAnimation(CurrentAction.Montage, CurrentAction.VFX);
		
	}
}

// 행동이 끝났다고 보고하는 함수. 
void ASTSCharacter::CompleteCurrentAction()
{

	// 만약 실행 중인 액션이 없는데 호출됐다면 무시한다
	if (!bIsExecutingAction)
	{
				return;
	}
	
	bIsExecutingAction = false;
	
	if (ActionQueue.Num() > 0)
	{
		ActionQueue.RemoveAt(0);
	}

	// 다음 명령서가 있는지 확인하고 실행합니다.
	ProcessNextAction();
}
	