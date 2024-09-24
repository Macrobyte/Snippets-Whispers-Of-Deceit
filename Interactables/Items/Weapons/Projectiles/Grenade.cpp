// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"

#include "BetrayalGame/StaticUtils.h"
#include "BetrayalGame/Gameplay/Player/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void AGrenade::OnThrow_Implementation()
{
	Super::OnThrow_Implementation();
	
	// INFO: Set the timer for the grenade to explode after a certain amount of time
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AGrenade::OnExplosion, TimeUntilDetination, false);
}

void AGrenade::OnExplosion()
{
	TArray<FHitResult> HitResults;
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation;

	FCollisionShape CollisionShape;
	CollisionShape.ShapeType = ECollisionShape::Sphere;
	CollisionShape.SetSphere(ExplosionRadius);

	// INFO: Sweep for any actors within the explosion radius
	if (GetWorld()->SweepMultiByChannel(HitResults, StartLocation, EndLocation, FQuat(), ECC_PhysicsBody, CollisionShape))
	{
		// INFO: Keep track of the characters that have been hit to prevent them from being hit multiple times
		TArray<TWeakObjectPtr<ABaseCharacter>> HitCharacters;
		
		for (auto It  = HitResults.CreateIterator(); It; ++It)
		{
			TWeakObjectPtr<ABaseCharacter> HitCharacter = Cast<ABaseCharacter>(It->GetActor());
			if (HitCharacter != nullptr)
			{
				bool bAlreadyHit = false;
				
				for (int i = 0; i < HitCharacters.Num(); i++)
				{
					if (HitCharacters[i] == HitCharacter)
					{
						bAlreadyHit = true;
					}

					// INFO: Break out of the loop if the character has already been hit
					if (bAlreadyHit)
						break;
				}

				if (!bAlreadyHit)
				{
					PrintLog("Damaged a actor");
					HitCharacter->Server_TakeDamage(DamageAmount);
					HitCharacters.Emplace(HitCharacter);
				}
			}
		}
	}

	NetMulticast_Explosion();
	
	Destroy();
}

void AGrenade::NetMulticast_Explosion_Implementation()
{
	if (ExplosionVfx)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVfx.Get(), GetActorLocation());
	}
	else
	{
		PrintWarning("AGrenade::OnExplosion - No Explosion VFX set for the grenade");
	}

	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX.Get(), GetActorLocation());
	}
	else
	{
		PrintWarning("AGrenade::OnExplosion - No Explosion SFX set for the grenade");
	}
}
