// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interactables/Items/ItemActor.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	// If the slot is empty, the item is null and the ID is 15
	FInventorySlot()
	{
		ID = 15;
		bIsEmpty = true;
		bIsEquipped = false;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	FItem Item;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	int ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	bool bIsEmpty;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	bool bIsEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slot")
	bool bIsSelected;

	//Overload the == operator to compare two FInventorySlot structs
	bool operator==(const FInventorySlot& Other) const
	{
		return ID == Other.ID && Item == Other.Item && bIsEmpty == Other.bIsEmpty && bIsEquipped == Other.bIsEquipped;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BETRAYALGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:	
	// Sets default values for this component's properties
	UInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	FInventorySlot SelectedSlot;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	FInventorySlot LastSlotAdded;
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	bool bIsInventoryInitialized = false;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int MaxInventorySlots;
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	bool bIsInventoryFull;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory")
	int FilledSlotCount;
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetSlot(int ID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetSlotWithItem(FItem Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetFirstEmptySlotID();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItem GetItemInSlot(int SlotID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventorySlot> GetInventorySlots() const { return InventorySlots; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetSelectedSlot() const { return SelectedSlot; }
	
	UFUNCTION()
	void SelectSlot(int SlotID);
	UFUNCTION(Server, Reliable)
	void Server_SelectSlot(int SlotID);
	
	UFUNCTION()
	void DeselectSlot(int SlotID);
	UFUNCTION(Server, Reliable)
	void Server_DeselectSlot(int SlotID);

#pragma region RPCs
	UFUNCTION()
	void AddItemToSlot(FItem Item, int SlotID);
	UFUNCTION(Server, Reliable)
	void Server_AddItemToSlot(FItem Item, int SlotID);
	
	UFUNCTION(Server, Reliable, Blueprintable, Category = "Inventory")
	void Server_AddItemToInventory(FItem Item);
	UFUNCTION()
	void AddItemToInventory(FItem Item);

	UFUNCTION(Server, Reliable)
	void Server_RemoveItemFromInventory(int ID);
	UFUNCTION()
	void RemoveItemFromInventory(int ID);

	UFUNCTION(Server, Reliable)
	void Server_InitializeInventory();
	UFUNCTION()
	void InitializeInventory();
#pragma endregion
	
#pragma region Getters and Setters
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsInventoryFull() const { return bIsInventoryFull; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetLastSlotAdded() const { return LastSlotAdded; }
#pragma endregion
};
