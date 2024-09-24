// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor.h"

#include "../../Player/BaseCharacter.h"
#include "../../Player/Player Components/InventoryComponent.h"
#include "../../Player/PlayerCharacter.h"
#include "../../Player/BetrayalPlayerController.h"
#include "BetrayalGame/StaticUtils.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetSimulatePhysics(true);

	// INFO: Default size of an inspect-able item
	InspectItemScale = FVector::One();
}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);
	
	bCanPickup = true;

	
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActor, ItemData);
	DOREPLIFETIME(AItemActor, bIsObjectiveItem);
	DOREPLIFETIME(AItemActor, bCanPickup);
	DOREPLIFETIME(AItemActor, ItemMesh);
	
}

void AItemActor::OnInteract(AActor* Interactor)
{
	Super::OnInteract(Interactor);
	
	OnPickup(Interactor);
}

void AItemActor::NetMulticast_EnableItemPhysics_Implementation(bool bState)
{
	if(bState)
	{
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
	}
	else
	{
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}


void AItemActor::Server_SetCanPickup_Implementation(const bool CanPickup)
{
	SetCanPickup(CanPickup);
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

