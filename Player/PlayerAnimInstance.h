// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPlayerAnimInstance();

	virtual void NativeInitializeAnimation() override;

#pragma region Animation Conditions
	// Exposed Variables
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Interp, Category = "Monster | Animation", meta = (AllowPrivateAccess = "true"))
	float MoveSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Monster | Animation", meta = (AllowPrivateAccess = "true"))
	bool bSprinting;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Monster | Animation", meta = (AllowPrivateAccess = "true"))
	class ABaseCharacter* Character;

	// Exposed Functions
public:
	UFUNCTION(BlueprintCallable, Category = "Monster | Animation")
	void CustomAnimUpdate();
	
#pragma endregion
	
};