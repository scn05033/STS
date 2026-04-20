// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
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
	if (CurrentTarget && PendingDamage > 0)
	{
		// 여기서 진짜 ApplyDamage를 실행
		UGameplayStatics::ApplyDamage(CurrentTarget, PendingDamage, GetController(), this, UDamageType::StaticClass());

		// 이펙트나 사운드를 C++에서 터뜨려도 좋습니다.
		UE_LOG(LogTemp, Warning, TEXT("노티파이 발생! 적에게 %d 데미지를 입혔습니다."), PendingDamage);
	}
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

		// 방어도 UI 업데이트 방송
		OnBlockChanged.Broadcast(CurrentBlock);
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
	