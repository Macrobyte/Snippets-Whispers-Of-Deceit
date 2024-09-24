// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "../Player/Player Components/InventoryComponent.h"
#include "../Player/BaseCharacter.h"

#include "PlayerCharacter.generated.h"

class ABetrayalPlayerController;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class AItemActor;


UENUM()
enum EInputActionValue
{
	IAV_None UMETA(DisplayName = "None"),
	IAV_Move UMETA(DisplayName = "Move"),
	IAV_Jump UMETA(DisplayName = "Jump"),
	IAV_Look UMETA(DisplayName = "Look"),
	IAV_Run UMETA(DisplayName = "Run"),
	IAV_Interact UMETA(DisplayName = "Interact"),
	IAV_Inventory1 UMETA(DisplayName = "Inventory1"),
	IAV_Inventory2 UMETA(DisplayName = "Inventory2"),
	IAV_Inventory3 UMETA(DisplayName = "Inventory3"),
	IAV_Inventory4 UMETA(DisplayName = "Inventory4"),
	IAV_TraitorCycleMonster UMETA(DisplayName = "TraitorCycleMonster"),
	IAV_TraitorSpawnMonster UMETA(DisplayName = "TraitorSpawnMonster"),
	IAV_Attack UMETA(DisplayName = "Attack"),
	IAV_DropItem UMETA(DisplayName = "DropItem"),
	IAV_ToggleLight UMETA(DisplayName = "ToggleLight"),
	IAV_ConsumeItem UMETA(DisplayName = "ConsumeItem"),
	IAV_ThrowItem UMETA(DisplayName = "ThrowItem"),
	IAV_InspectItem UMETA(DisplayName = "InspectItem")
};

UENUM()
enum EAnimationMontages
{
	AM_None UMETA(DisplayName = "None"),
	AM_Attack UMETA(DisplayName = "Attack"),
};

USTRUCT(Blueprintable)
struct FMonsterSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster")
	TSubclassOf<class AMonster> Monster;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster")
	int MaxAmount = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster")
	int Count = 0;
};

UCLASS()
class BETRAYALGAME_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:

	APlayerCharacter();

	void DebugInput();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
#pragma region Possession

public:
	virtual void Destroyed() override;

	void PossessedBy(AController* NewController) override;
	
#pragma endregion
	
#pragma region Animation
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Player|Animation")
	TMap<TEnumAsByte<EAnimationMontages>, UAnimMontage*> AnimationMontages;
	
	UFUNCTION()
	void BindMontageEvents();
	
	UFUNCTION()
	void MontageStarted(UAnimMontage* Montage);

	UFUNCTION()
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
#pragma endregion 
	
#pragma region Camera
public:
	UPROPERTY(EditAnywhere,Category="Player|Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Camera")
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Camera")
	float BaseLookUpRate;

	void TurnLook(const FInputActionValue& Value);

	void UpDownLook(const FInputActionValue& Value);
	
private:
#pragma endregion 
	
#pragma region Input
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<EInputActionValue>, UInputAction*> InputAction;

	virtual void PawnClientRestart() override;
	
	UFUNCTION()
	void SetupInputSubsystem();

	UFUNCTION(Server, Reliable)
	void Server_SetupInputSubsystem();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
private:
#pragma endregion

#pragma region Movement
public:
	virtual void Move(const FInputActionValue& Value) override;
	virtual void Move(const FVector2D Value) override;;
	
	UFUNCTION(Server, Reliable)
	void RunStart();

	UFUNCTION(Server, Reliable)
	void RunEnd();
private:
#pragma endregion

#pragma region Inventory
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory")
	class USphereComponent* ItemDropLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory")
	AItemActor* HeldItem;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Player|Inventory")
	bool bWasItemUnequipped;

	UFUNCTION()
	void SelectSlot(int SlotID);
	
	UFUNCTION()
	void EquipItem(int SlotID);
	UFUNCTION(Server, Reliable)
	void Server_EquipItem(int SlotID);

	UFUNCTION()
	void UnequipItem();
	UFUNCTION(Server, Reliable)
	void Server_UnequipItem();

	UFUNCTION()
	void DropHeldItem();
	UFUNCTION(Server, Reliable)
	void Server_DropHeldItem();

	// Removes the currently held item from the inventory, does not spawn it in the world. Do NOT use when trying to drop an item.
	UFUNCTION()
	void RemoveHeldItemFromInventory();
	// Removes the currently held item from the inventory, does not spawn it in the world. Do NOT use when trying to drop an item.
	UFUNCTION(Server, Reliable)
	void Server_RemoveHeldItemFromInventory();

	
	UFUNCTION()
	void DropItem(FInventorySlot Slot);
	UFUNCTION(Server, Reliable)
	void Server_DropItem(FInventorySlot Slot);
	
	UFUNCTION()
	void DropAllItems();
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DropAllItems();

	UFUNCTION()
	void ConsumeHeldItem();
	UFUNCTION(Server, Reliable)
	void Server_ConsumeHeldItem();

	UFUNCTION()
	void ThrowHeldItem();
	UFUNCTION(Server, Reliable)
	void Server_ThrowHeldItem();
	
	UFUNCTION(Server, Reliable)
	void Server_AddItemToInventory(int SlotID, AItemActor* Item, APlayerCharacter* NewOwner);
	UFUNCTION(Client, Reliable)
	void Client_AddItemToInventory(int SlotID, AItemActor* Item);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Inventory")
	void OnSlotSelected(UUserWidget* HUD, FInventorySlot Slot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Inventory")
	void OnItemRemovedFromInventory(UUserWidget* HUD, FInventorySlot Slot);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Inventory")
	void OnInventoryUpdated(UUserWidget* HUD, const TArray<FInventorySlot>& InventorySlots);
	
	void ClientUpdateInventory(); // TODO: Make sure to remove all timers and use other method.

	void ClientUpdateEquippedItem();
	
	double LastInventoryUpdate = 0.0f;
	void InventoryUIUpdateDirtyFix(); // TODO: Remove this from existence as soon as possible.
	
private:	
#pragma endregion 

#pragma region Inspection
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inspection")
	UStaticMeshComponent* ItemInspectLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Inspection")
	float InspectDistance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Inspection")
	bool bIsInspecting;

	UFUNCTION()
	void InspectItem();
#pragma endregion Inspection

#pragma region Throwing
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Throwing")
	float ThrowForce = 1000.0f;
#pragma endregion Throwing
	
#pragma region Interaction
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Interaction")
	class ABaseInteractable* CurrentInteractableInFocus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Interaction")
	ABaseInteractable* PreviousInteractableInFocus;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Player|Interaction")
	bool bIsInteractableInFocus;

	UFUNCTION(Server, Reliable)
	void Server_SetInteractableInFocus(bool bNewValue);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Interaction")
	float InteractDistance = 600.0f;
	
	void TraceForInteractables();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Interaction")
	void OnItemPickedUp(UUserWidget* HUD, FInventorySlot Slot);
	
	void Interact();
	
	UFUNCTION(Server, Reliable)
	void Server_Interact(class AActor* NewOwner, class ABaseInteractable* Interactable);

	UFUNCTION(Client, Reliable)
	void Client_Interact(class AActor* NewOwner, class ABaseInteractable* Interactable);
	
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Interact(class AActor* NewOwner, class ABaseInteractable* Interactable);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Interaction")
	void OnInteractableInFocus(UUserWidget* InteractWidget, bool bIsInFocus);
	
private:
#pragma endregion 

#pragma region Traitor Actions
public:
	UFUNCTION()
	void CycleSelectedMonster();
	UFUNCTION(Server, Reliable)
	void Server_CycleSelectedMonster();
	
	UFUNCTION()
	void SpawnSelectedMonster();
	UFUNCTION(Server, Reliable)
	void Server_SpawnMonster(TSubclassOf<class AMonster> MonsterType);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Player|Traitor", meta = (AllowPrivateAccess))
	TArray<FMonsterSpawnInfo> Monsters;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Traitor", meta = (AllowPrivateAccess = "true"))
	int SelectedMonsterIndex = 0;
	
#pragma endregion
	
#pragma region Objectives
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Objectives")
	class UObjectivesComponent* ObjectivesComponent;

	
private:
#pragma endregion

#pragma region Combat System
protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Player|Combat")
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat")
	UAnimMontage* AttackMontage;
	
public:
	UFUNCTION()
	void Attack();

	UFUNCTION(Server, Reliable)
	void Server_Attack();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_Attack();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Player|Combat")
	void OnAttack();

	virtual void Stun(AActor* Attacker, float Duration) override;
	
private:
#pragma endregion

#pragma region Chestlight
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Chestlight")
	class UChildActorComponent* ChestlightComponent;

public:
	UFUNCTION()
	void ToggleLight();
	UFUNCTION(Server, Reliable)
	void Server_ToggleLight();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Chestlight")
	void OnLightToggled(UUserWidget* HUD, bool bIsOn);
	
private:	
#pragma endregion

#pragma region References
public:
	UPROPERTY(VisibleAnywhere, Category="Player|References")
	class ABetrayalPlayerState* PlayerStateReferece;
private:	
#pragma endregion 
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Player|Combat")
	void OnTraitorStateChanged(UUserWidget* HUD, bool bIsTraitor);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	
};
