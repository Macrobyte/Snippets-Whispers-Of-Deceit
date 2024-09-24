// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/Gameplay/Interactables/Items/Weapons/Weapon.h"
#include "Melee.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class BETRAYALGAME_API AMelee : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void OnAttack()	PURE_VIRTUAL(AMelee::OnAttack);

protected:
	
#pragma region Stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Stats")
	float AttackCooldown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Stats")
	FTimerHandle AttackTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Stats")
	bool bCanAttack;
#pragma endregion Stats

#pragma region SFX
	UPROPERTY(EditAnywhere, Category = "Weapon|Melee|SFX")
	TSoftObjectPtr<USoundBase> SlashSFX;

	UPROPERTY(EditAnywhere, Category = "Weapon|Melee|SFX")
	TSoftObjectPtr<USoundBase> MeleeDamageSFX;
#pragma endregion SFX

#pragma region Animations
	UPROPERTY(EditAnywhere, Category = "Weapon|Melee|Animations")
	TSoftObjectPtr<UAnimMontage> SwingMontage;

	UPROPERTY(EditAnywhere, Category = "Weapon|Melee|Animations")
	TSoftObjectPtr<UAnimMontage> BlockMontage;

#pragma endregion	Animations
	
};
