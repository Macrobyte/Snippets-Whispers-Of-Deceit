// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Melee.h"
#include "Sword.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API ASword : public AMelee
{
	GENERATED_BODY()

public:
	virtual void OnAttack() override;
};
