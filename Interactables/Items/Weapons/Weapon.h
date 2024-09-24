// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/Gameplay/Interactables/IThrowableItem.h"
#include "BetrayalGame/Gameplay/Interactables/Items/ItemActor.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API AWeapon : public AItemActor, public IThrowableItem
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Damage")
	float DamageAmount;
};
