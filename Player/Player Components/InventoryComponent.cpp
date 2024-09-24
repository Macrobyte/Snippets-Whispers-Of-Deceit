// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "../PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventorySlots);
	DOREPLIFETIME(UInventoryComponent, FilledSlotCount);
	DOREPLIFETIME(UInventoryComponent, bIsInventoryFull);
	DOREPLIFETIME(UInventoryComponent, bIsInventoryInitialized);
	DOREPLIFETIME(UInventoryComponent, SelectedSlot);
	DOREPLIFETIME(UInventoryComponent, LastSlotAdded);
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetOwnerRole() == ROLE_Authority)
		InitializeInventory();
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
		Server_InitializeInventory();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::Server_AddItemToInventory_Implementation(FItem Item)
{
	AddItemToInventory(Item);
}

void UInventoryComponent::AddItemToInventory(FItem Item)
{
	for (auto& slot : InventorySlots)
	{
		if(slot.bIsEmpty)
		{
			slot.Item = Item;
			slot.bIsEmpty = false;
			LastSlotAdded = slot;
			FilledSlotCount++;
			break;
		}
	}

	if(InventorySlots.Num() == FilledSlotCount)
	{
		bIsInventoryFull = true;
	}
}

void UInventoryComponent::RemoveItemFromInventory(int ID)
{
	for (auto& slot : InventorySlots)
	{
		if(slot.ID == ID)
		{
			slot.Item = FItem();
			slot.bIsSelected = false;
			slot.bIsEmpty = true;
			FilledSlotCount--;
			break;
		}
	}

	if(FilledSlotCount < InventorySlots.Num())
	{
		bIsInventoryFull = false;
	}
}

void UInventoryComponent::Server_RemoveItemFromInventory_Implementation(int ID)
{
	RemoveItemFromInventory(ID);
}

void UInventoryComponent::InitializeInventory()
{
	for ( int i = 0; i < MaxInventorySlots; i++)
	{
		FInventorySlot Slot;
		Slot.ID = i;
		Slot.bIsEmpty = true;
		InventorySlots.Add(Slot);
	}
	
	bIsInventoryInitialized = true;
}

void UInventoryComponent::Server_InitializeInventory_Implementation()
{
	InitializeInventory();
}

FInventorySlot UInventoryComponent::GetSlot(int ID)
{
	for (auto slot : InventorySlots)
		if(slot.ID == ID)
			return slot;

	return FInventorySlot();
}

FInventorySlot UInventoryComponent::GetSlotWithItem(FItem Item)
{
	for (auto slot : InventorySlots)
		if(slot.Item.Name.ToString() == Item.Name.ToString())
			return slot;

	return FInventorySlot();	
}

int UInventoryComponent::GetFirstEmptySlotID()
{
	for (auto slot : InventorySlots)
		if(slot.bIsEmpty)
			return slot.ID;

	return -1;
}

FItem UInventoryComponent::GetItemInSlot(int SlotID)
{
	for (auto slot : InventorySlots)
		if(slot.ID == SlotID)
			return slot.Item;

	return FItem();
}

void UInventoryComponent::SelectSlot(int SlotID)
{
	for (auto& slot : InventorySlots)
	{
		if(SlotID == slot.ID)
		{
			slot.bIsSelected = true;

			if(slot.Item.Actor)
			{
				slot.bIsEquipped = true;
				
			}

			SelectedSlot = slot;
		}
		else
		{
			slot.bIsSelected = false;
			slot.bIsEquipped = false;
		}
	}
}

void UInventoryComponent::Server_SelectSlot_Implementation(int SlotID)
{
	SelectSlot(SlotID);
}

void UInventoryComponent::DeselectSlot(int SlotID)
{
	for (auto& slot : InventorySlots)
	{
		if(SlotID == slot.ID)
		{
			slot.bIsSelected = false;
			slot.bIsEquipped = false;

			SelectedSlot = slot;
		}
	}
}

void UInventoryComponent::AddItemToSlot(FItem Item, int SlotID)
{
	for (auto& slot : InventorySlots)
	{
		if(SlotID == slot.ID)
		{
			slot.Item = Item;
			slot.bIsEmpty = false;
			LastSlotAdded = slot;
			FilledSlotCount++;
			break;
		}
	}

	if(InventorySlots.Num() == FilledSlotCount)
	{
		bIsInventoryFull = true;
	}
}

void UInventoryComponent::Server_AddItemToSlot_Implementation(FItem Item, int SlotID)
{
	AddItemToSlot(Item, SlotID);
}

void UInventoryComponent::Server_DeselectSlot_Implementation(int SlotID)
{
	DeselectSlot(SlotID);
}
