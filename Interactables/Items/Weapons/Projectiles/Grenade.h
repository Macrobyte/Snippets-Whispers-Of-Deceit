// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API AGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	virtual void OnThrow_Implementation() override;

	UFUNCTION()
	void OnExplosion();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Explosion();

protected:

#pragma region Stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|Stats")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|Stats")
	float TimeUntilDetination;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|Stats")
	FTimerHandle ExplosionTimerHandle;
#pragma endregion Stats

#pragma region SFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|SFX")
	TSoftObjectPtr<USoundBase> ExplosionSFX;
#pragma endregion SFX

#pragma region VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|VFX")
	TSoftObjectPtr<UParticleSystem> ExplosionVfx;
#pragma endregion VFX
	
};
