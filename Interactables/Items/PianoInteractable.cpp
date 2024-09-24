// Fill out your copyright notice in the Description page of Project Settings.


#include "PianoInteractable.h"

#include "../../Player/PlayerCharacter.h"
#include "../../Player/BetrayalPlayerController.h"

#include "Net/UnrealNetwork.h"


void APianoInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APianoInteractable, CurrentSheets);
}

void APianoInteractable::OnInteract(AActor* Interactor)
{
	Super::OnInteract(Interactor);
	
	APlayerCharacter* Player = Cast<APlayerCharacter>(Interactor);
	if(!Player)
		return;

	for (auto InventorySlot : Player->InventoryComponent->GetInventorySlots())
	{
		if(InventorySlot.Item.Name == "Music Sheet")
		{
			CurrentSheets++;
			Player->InventoryComponent->RemoveItemFromInventory(InventorySlot.ID);
			
			if(Player->HeldItem)
				Player->UnequipItem();

			Player->InventoryComponent->DeselectSlot(InventorySlot.ID);
			
			//Player->OnItemRemovedFromInventory(Player->GetBetrayalPlayerController()->WB_HUD, Player->InventoryComponent->GetSelectedSlot());
			
			if(CurrentSheets == NecessarySheets)
				OnPianoComplete();
			
			break;
		}
	}
}
