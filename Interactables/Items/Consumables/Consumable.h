// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/Gameplay/Interactables/Items/ItemActor.h"
#include "Consumable.generated.h"

USTRUCT(BlueprintType)
struct FConsumableItemData : public FItem
{
	GENERATED_BODY()

	FConsumableItemData()
	{
		Value = 0.0f;
		bIsPassive = false;
		EffectDuration = 0.0f;
		EffectProc = 0.0f;
	}

#pragma region Consumable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable")
	TSoftObjectPtr<UAnimMontage> ConsumeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable")
	TSoftObjectPtr<USoundBase> ConsumeSFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable")
	float Value; // INFO: This is the value that the consumable item will give to the player e.g. (Health amount, Stamina amount, etc.)
#pragma endregion Consumable

#pragma region PassiveEffect
	// INFO: Determines if the consumable item has a passive effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable|PassiveEffect")
	bool bIsPassive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable|PassiveEffect", meta = (EditCondition = "bIsPassive"))
	float EffectDuration;

	// INFO: How often the effect occurs every n seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Consumable|PassiveEffect", meta = (EditCondition = "bIsPassive"))
	float EffectProc;
#pragma endregion PassiveEffect

	// INFO: Overload the == operator to compare two FConsumableItemData structs
	bool operator==(const FConsumableItemData& Other) const
	{
		return FItem::operator==(Other) && Value == Other.Value && bIsPassive == Other.bIsPassive && EffectDuration == Other.EffectDuration && EffectProc == Other.EffectProc;
	}
};

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API AConsumable : public AItemActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Consumable")
	void OnConsume();

private:
	
public:
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
	// FDataTableRowHandle ConsumableItemData;

};
