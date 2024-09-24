// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "BetrayalPlayerController.h"
#include "PlayerCharacter.h"
#include "BetrayalGame/StaticUtils.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentHealth);
	DOREPLIFETIME(ABaseCharacter, bIsDead);
	DOREPLIFETIME(ABaseCharacter, bIsRunning);
	DOREPLIFETIME(ABaseCharacter, bIsStunned);
	DOREPLIFETIME(ABaseCharacter, StunTimerHandle);
	DOREPLIFETIME(ABaseCharacter, WalkSpeed);
	DOREPLIFETIME(ABaseCharacter, RunSpeed);
	DOREPLIFETIME(ABaseCharacter, StunnedSpeed);
}

bool ABaseCharacter::SweepTraceForCharacter(ABaseCharacter*& HitCharacterOut)
{
	FVector HitLocation = GetMesh()->GetSocketLocation("ItemSocket");
	
	TArray<FHitResult> HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	
	GetWorld()->SweepMultiByChannel(HitResult, HitLocation, HitLocation, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(50.0f), CollisionParams);
	
	for (auto Pawn : HitResult)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Pawn.GetActor());
		if(!Character)
			continue;
		
		HitCharacterOut = Character;
		return true;
	}
	
	return false;
}

void ABaseCharacter::NetDebugging()
{
	if(!Controller)
		return;
	
	if (Controller->IsLocalPlayerController())
	{
		// Execute debug code only for the local player
		if (HasAuthority())
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, "You are the Server!");
			
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, "You are the Client!");
		}
	}
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	
}

void ABaseCharacter::Move(const FVector2D Value)
{
	
}

void ABaseCharacter::SetMaxWalkSpeed(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ABaseCharacter::ResetMovementSpeeds()
{
	WalkSpeed = DefaultWalkSpeed;
	RunSpeed = DefaultRunSpeed;
	StunnedSpeed = DefaultStunnedSpeed;
}

void ABaseCharacter::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	SetMaxWalkSpeed(NewSpeed);
}

void ABaseCharacter::AdjustMovementSpeeds(float adjustmentAmount, float duration)
{
	WalkSpeed += adjustmentAmount;
	RunSpeed += adjustmentAmount;
	StunnedSpeed += adjustmentAmount;

	// INFO: Prevents camera glitching out due to new movement speed being applied
	float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (CurrentSpeed == WalkSpeed)
	{
		Server_SetMaxWalkSpeed(WalkSpeed);
	}
	else if (CurrentSpeed == RunSpeed)
	{
		Server_SetMaxWalkSpeed(RunSpeed);
	}
	else if (CurrentSpeed == StunnedSpeed)
	{
		Server_SetMaxWalkSpeed(StunnedSpeed);
	}

	FTimerHandle AdjustmentTimerHandle;
	
	GetWorld()->GetTimerManager().SetTimer(AdjustmentTimerHandle, this, &ABaseCharacter::ResetMovementSpeeds, duration, false);
}

void ABaseCharacter::Server_AdjustMovementSpeeds_Implementation(float adjustmentAmount, float duration)
{
	AdjustMovementSpeeds(adjustmentAmount, duration);
}

void ABaseCharacter::StunAttack_Implementation()
{
	ABaseCharacter* HitCharacter = nullptr;	
	if(SweepTraceForCharacter(HitCharacter))
	{
		if(HitCharacter)
		{
			HitCharacter->Stun(this, StunDuration);
			OnStunAttack(HitCharacter);
		}
	}
		
		//HitCharacter->Server_Stun(StunDuration);
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                 AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABaseCharacter::TakeDamage(float Damage)
{
	CurrentHealth -= Damage;
	
	if(CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		bIsDead = true;
		OnDeath();
		return;
	}

	if(auto PlayerController = GetController<ABetrayalPlayerController>())
	{
		OnDamageTaken(PlayerController->WB_HUD, Damage);
	}
	
	
}

void ABaseCharacter::Heal(float Amount)
{
	if(CurrentHealth + Amount > MaxHealth)
		CurrentHealth = MaxHealth;
	else
		CurrentHealth += Amount;

	if(auto PlayerController = GetController<ABetrayalPlayerController>())
	{
		OnHeal(PlayerController->WB_HUD, Amount);
	}
}

void ABaseCharacter::Server_Heal_Implementation(float Amount)
{
	Heal(Amount);
}

void ABaseCharacter::Server_TakeDamage_Implementation(float Damage)
{
	TakeDamage(Damage);
}

void ABaseCharacter::Server_SetMaxHealth_Implementation(float NewMaxHealth)
{
	SetMaxHealth(NewMaxHealth);
}

void ABaseCharacter::OnStunAttack(AActor* StunnedTarget)
{
}

void ABaseCharacter::Stun(AActor* Attacker, float Duration)
{
	bIsStunned = true;

	Server_SetMaxWalkSpeed_Implementation(StunnedSpeed);
	
	GetWorld()->GetTimerManager().SetTimer(StunTimerHandle, this, &ABaseCharacter::StopStun, Duration, false);

	OnStun();
}

void ABaseCharacter::Server_Stun_Implementation(AActor* Attacker, float Duration)
{
	Stun(Attacker, Duration);
}

void ABaseCharacter::StopStun()
{
	bIsStunned = false;

	if (bIsRunning)
		Server_SetMaxWalkSpeed_Implementation(RunSpeed);
	else
		Server_SetMaxWalkSpeed_Implementation(WalkSpeed);
	
	OnStunEnd();
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;

	DefaultWalkSpeed = WalkSpeed;
	DefaultRunSpeed = RunSpeed;
	DefaultStunnedSpeed = StunnedSpeed;
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	NetDebugging();
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}

