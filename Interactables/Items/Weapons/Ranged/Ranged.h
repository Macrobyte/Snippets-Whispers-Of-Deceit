// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/Gameplay/Interactables/Items/Weapons/Weapon.h"
#include "Ranged.generated.h"

class AProjectile;

UENUM()
enum EWeaponType
{
	None = 0,
	
	Primitive,
	Modern
};

/**
 * 
 */
UCLASS(Abstract)
class BETRAYALGAME_API ARanged : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void OnShoot() PURE_VIRTUAL(ARanged::OnShoot);
	
protected:

#pragma region Stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Stats")
	float AmmoAmount; // INFO: Amount of ammo currently in the clip/ready to be fired
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Stats")
	float ReloadTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Stats")
	FTimerHandle ReloadTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Stats")
	bool bCanShoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Stats")
	TEnumAsByte<EWeaponType> WeaponType;
#pragma endregion Stats

#pragma region Primitive
	// INFO: The projectile that will be spawned when the weapon is fired
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Primitive", meta = (EditCondition = "Type == EWeaponType::Primitive"))
	TSoftObjectPtr<AProjectile> ProjectileAsset;
#pragma endregion Primitive

#pragma region Modern
	// INFO: The amount of ammo that can be held in a single clip
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Modern", meta = (EditCondition = "Type == EWeaponType::Modern"))
	float ClipSize;

	// INFO: The maximum amount of clips that can be held
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Modern", meta = (EditCondition = "Type == EWeaponType::Modern"))
	float MaxClips;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Modern", meta = (EditCondition = "Type == EWeaponType::Modern"))
	float ClipsRemaining;
#pragma endregion Modern

#pragma region SFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|SFX")
	TSoftObjectPtr<USoundBase> ShootSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|SFX")
	TSoftObjectPtr<USoundBase> RangedDamageSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|SFX")
	TSoftObjectPtr<USoundBase> ReloadSFX;
#pragma endregion SFX

#pragma region Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Animations")
	TSoftObjectPtr<UAnimMontage> ShootMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Animations")
	TSoftObjectPtr<UAnimMontage> AimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Animations")
	TSoftObjectPtr<UAnimMontage> ReloadMontage;
#pragma endregion Animations
	
};
