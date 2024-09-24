// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheatTeleportLocation.generated.h"

//This might be temp depending on how progen tagging works
UENUM()
enum EFloor
{
	F_Basement UMETA(DisplayName = "Basement"),
	F_GroundFloor UMETA(DisplayName = "Ground Floor"),
	F_FirstFloor UMETA(DisplayName = "First Floor"),
};

//Doubt this will exist for long but is a substitue for the Procgen taging
UENUM()
enum ERooms
{
	R_Lobby UMETA(DisplayName = "Lobby"),
	R_Bedroom UMETA(DisplayName = "Bedroom"),
	R_Coridoor UMETA(DisplayName = "Coridoor"),
};

UCLASS()
class BETRAYALGAME_API ACheatTeleportLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACheatTeleportLocation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	//Might be replaced by procgen tag
	UPROPERTY(EditAnywhere, Category = "Location")
	TEnumAsByte<EFloor> OnFloor;

	//Temp and will be replaced by procgen tag
	UPROPERTY(EditAnywhere, Category = "Location")
	TEnumAsByte<ERooms> RoomType;
};
