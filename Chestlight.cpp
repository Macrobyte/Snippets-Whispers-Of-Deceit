// Fill out your copyright notice in the Description page of Project Settings.


#include "Chestlight.h"

#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"

void AChestlight::SetLightIntensity(float NewIntensity)
{
	Light->SetIntensity(NewIntensity);
}

void AChestlight::Multicast_SetLightIntensity_Implementation(float NewIntensity)
{
	SetLightIntensity(NewIntensity);
}

void AChestlight::Server_SetLightIntensity_Implementation(float NewIntensity)
{
	SetLightIntensity(NewIntensity);
	Multicast_SetLightIntensity(NewIntensity);
}

void AChestlight::DrainBattery(float DrainAmount)
{
	
}

void AChestlight::RechargeBattery(float RechargeAmount)
{
	
}

// Sets default values
AChestlight::AChestlight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void AChestlight::BeginPlay()
{
	Super::BeginPlay();
	
}

void AChestlight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChestlight, Light);
	DOREPLIFETIME(AChestlight, Intensity);
	DOREPLIFETIME(AChestlight, bIsOn);
	
}

// Called every frame
void AChestlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void AChestlight::ToggleLight()
{
	bIsOn = !bIsOn;
	Light->SetVisibility(bIsOn);
}

void AChestlight::Server_ToggleLight_Implementation()
{
	Multicast_ToggleLight();
}

void AChestlight::Multicast_ToggleLight_Implementation()
{
	ToggleLight();
}


