// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "BaseCharacter.h"

UPlayerAnimInstance::UPlayerAnimInstance()
	: MoveSpeed(0.0f), bSprinting(false), Character(nullptr)
{
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	Character = Cast<ABaseCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::CustomAnimUpdate()
{
	if (!Character)		
		Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	if (Character)
	{
		bSprinting = Character->bIsRunning;
		FVector Speed = Character->GetVelocity();
		Speed.Z = 0.0f;
		MoveSpeed = Speed.Size();		
	}
}
