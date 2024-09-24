// Fill out your copyright notice in the Description page of Project Settings.


#include "HiddenAsymmetricalHaunt.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AHiddenAsymmetricalHaunt::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AHiddenAsymmetricalHaunt, ObjectiveInteractable);
	DOREPLIFETIME(AHiddenAsymmetricalHaunt, InteractableSpawnTransform);
	DOREPLIFETIME(AHiddenAsymmetricalHaunt, ItemSpawnTransforms);
	DOREPLIFETIME(AHiddenAsymmetricalHaunt, ObjectiveItems);
}

AHiddenAsymmetricalHaunt::AHiddenAsymmetricalHaunt()
{
	HauntCategory = EHauntCategory::Hc_HiddenAsymmetric;
	bHasTraitor = true;
}

void AHiddenAsymmetricalHaunt::BeginPlay()
{
	Super::BeginPlay();
}

void AHiddenAsymmetricalHaunt::SetupSpawns()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		if(Actor->ActorHasTag("InteractableSpawnTransform"))
		{
			InteractableSpawnTransform = Actor;
		}
		else if(Actor->ActorHasTag("ItemSpawnTransform"))
		{
			ItemSpawnTransforms.Add(Actor);
		}
	}

	// Spawn the interactable
	if(InteractableSpawnTransform)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ObjectiveInteractable = GetWorld()->SpawnActor<ABaseInteractable>(ObjectiveInteractableClass, InteractableSpawnTransform->GetActorLocation(), InteractableSpawnTransform->GetActorRotation(), SpawnParams);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No InteractableSpawnTransform found for %s"), *GetName());
	}

	// Spawn the items
	for (auto Item : ItemSpawnTransforms)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ObjectiveItems.Add(GetWorld()->SpawnActor<AItemActor>(ObjectiveItemClass, Item->GetActorLocation(), Item->GetActorRotation(), SpawnParams));
	}
}

void AHiddenAsymmetricalHaunt::DestroySpawnTransforms()
{
	InteractableSpawnTransform->Destroy();

	for (auto ItemSpawn : ItemSpawnTransforms)
		ItemSpawn->Destroy();
}

void AHiddenAsymmetricalHaunt::Server_SetupSpawns_Implementation()
{
	//TODO: Items being spawned for every client and not replicating properly
	SetupSpawns();
}
