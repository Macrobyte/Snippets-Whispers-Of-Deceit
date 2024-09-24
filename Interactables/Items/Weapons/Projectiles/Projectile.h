// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/Gameplay/Interactables/Items/Weapons/Weapon.h"
#include "Projectile.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class BETRAYALGAME_API AProjectile : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void OnThrow_Implementation() override;
	
protected:

#pragma region Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|Animations")
	TSoftObjectPtr<UAnimMontage> ThrowMontage;
#pragma endregion Animations

#pragma region SFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile|SFX")
	TSoftObjectPtr<USoundBase> ThrowSFX;
#pragma endregion SFX
	
};
