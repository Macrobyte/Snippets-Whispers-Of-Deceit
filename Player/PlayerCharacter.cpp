// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "BetrayalPlayerController.h"
#include "../Interactables/BaseInteractable.h"
#include "../../BetrayalGameMode.h"
#include "../../BetrayalPlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../Player/Player Components/InventoryComponent.h"
#include "../Player/Player Components/ObjectivesComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "../../AI/Pawns/Monster.h"
#include "BetrayalGame/StaticUtils.h"
#include "BetrayalGame/AI/Controllers/AIPlayerController.h"
#include "BetrayalGame/Gameplay/Chestlight.h"
#include "BetrayalGame/Gameplay/Interactables/IThrowableItem.h"
#include "BetrayalGame/Gameplay/Interactables/Items/Consumables/Consumable.h"
#include "BetrayalGame/Gameplay/Interactables/Items/Weapons/Projectiles/Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"

APlayerCharacter::APlayerCharacter()
{
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("Head"));

	ItemDropLocation = CreateDefaultSubobject<USphereComponent>(TEXT("Item Drop Location"));
	ItemDropLocation->SetupAttachment(GetMesh());

	ItemInspectLocation = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Inspect Location"));
	ItemInspectLocation->SetupAttachment(GetMesh());
	ItemInspectLocation->SetVisibility(false);
	InspectDistance = 100.0f;

	CurrentInteractableInFocus = nullptr;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory Component"));

	ObjectivesComponent = CreateDefaultSubobject<UObjectivesComponent>(TEXT("Objectives Component"));


	ChestlightComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Chestlight"));
	static ConstructorHelpers::FClassFinder<AChestlight> ChestlightBP(TEXT("/Game/Blueprints/BP_Chestlight"));
	if (ChestlightBP.Class)
	{
		ChestlightComponent->SetChildActorClass(ChestlightBP.Class);
		ChestlightComponent->SetupAttachment(GetMesh(), FName("ChestLightSocket"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Chestlight BP not found"));
	}
}

void APlayerCharacter::DebugInput()
{
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, HeldItem);
	DOREPLIFETIME(APlayerCharacter, bIsAttacking);
	DOREPLIFETIME(APlayerCharacter, bIsInteractableInFocus);
}

void APlayerCharacter::Destroyed()
{
	DropAllItems();

	Super::Destroyed();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetOwner(NewController);

	PrintWarning("APlayerCharacter::PossessedBy - Possessed by: " + NewController->GetName());
}

void APlayerCharacter::TurnLook(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (!Controller)
		return;

	const float NewRotation = LookInput.X * BaseTurnRate * GetWorld()->GetDeltaSeconds();
	
	// INFO: Rotate camera, otherwise rotate inspecting object
	if (!bIsInspecting)
	{
		AddControllerYawInput(NewRotation);
	}
	else
	{
		ItemInspectLocation->AddWorldRotation(FRotator(0.0f, NewRotation, 0.0f));
	}
}

void APlayerCharacter::UpDownLook(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (!Controller)
		return;

	const float NewRotation = LookInput.Y * BaseLookUpRate * GetWorld()->GetDeltaSeconds();

	// INFO: Rotate camera, otherwise rotate inspecting object
	if (!bIsInspecting)
	{
		AddControllerPitchInput(-NewRotation); // Value is negated because it's inverted
	}
	else
	{
		ItemInspectLocation->AddWorldRotation(FRotator(-NewRotation, 0.0f, 0.0f));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	Super::Move(Value);

	if (bIsStunned)
		Server_SetMaxWalkSpeed_Implementation(StunnedSpeed);
	else if (bIsRunning)
		Server_SetMaxWalkSpeed_Implementation(RunSpeed);
	else
		Server_SetMaxWalkSpeed_Implementation(WalkSpeed);

	const FVector2D MovementInput = Value.Get<FVector2D>();

	if (Controller != nullptr && !bIsInspecting)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementInput.Y);
		AddMovementInput(RightDirection, MovementInput.X);
	}
}

void APlayerCharacter::Move(const FVector2D Value)
{
	Super::Move(Value);
}

void APlayerCharacter::SelectSlot(int SlotID)
{
	if (HasAuthority())
	{
		EquipItem(SlotID);
	}
	else
	{
		Server_EquipItem(SlotID);
	}

	FTimerHandle TimerHandle = FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlayerCharacter::ClientUpdateEquippedItem, 0.05f,
	                                       false);
}

void APlayerCharacter::EquipItem(int SlotID)
{
	InventoryComponent->Server_SelectSlot(SlotID);

	//OnSlotSelected(GetBetrayalPlayerController()->WB_HUD, InventoryComponent->GetSelectedSlot());

	AItemActor* Item = InventoryComponent->GetItemInSlot(SlotID).Actor.GetDefaultObject();

	if (HeldItem && !Item)
	{
		UnequipItem();
		HeldItem = nullptr;
		return;
	}

	if (!Item)
		return;

	if (HeldItem && Item->GetClass() == HeldItem->GetClass())
	{
		UnequipItem();
		InventoryComponent->Server_DeselectSlot(SlotID);
		//OnSlotSelected(BetrayalPlayerController->WB_HUD, InventoryComponent->GetSelectedSlot());
		return;
	}
	else
	{
		UnequipItem();
	}

	//Spawn the item in the player's hand
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(Item->GetClass(), SpawnParams))
	{
		ItemActor->NetMulticast_EnableItemPhysics(false);
		ItemActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		                             FName("ItemSocket"));
		ItemActor->SetActorRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		ItemActor->SetActorRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		ItemActor->bIsInteractable = false;
		HeldItem = ItemActor;
	}

	HeldItem->Server_SetCanPickup(false);
}

void APlayerCharacter::UnequipItem()
{
	if (!HeldItem)
		return;

	HeldItem->Destroy();

	HeldItem = nullptr;
}

void APlayerCharacter::DropHeldItem()
{
	Server_DropHeldItem();

	FTimerHandle TimerHandle = FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlayerCharacter::ClientUpdateInventory, 0.05f, false);
}

void APlayerCharacter::Server_DropHeldItem_Implementation()
{
	if (!HeldItem)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(HeldItem->GetClass(), SpawnParams))
	{
		ItemActor->NetMulticast_EnableItemPhysics(true);
		ItemActor->SetActorRelativeLocation(ItemDropLocation->GetComponentLocation());
		ItemActor->SetActorRelativeRotation(ItemDropLocation->GetComponentRotation());
		ItemActor->bIsInteractable = true;
	}

	UnequipItem();

	InventoryComponent->Server_RemoveItemFromInventory(InventoryComponent->GetSelectedSlot().ID);
	InventoryComponent->Server_DeselectSlot(InventoryComponent->GetSelectedSlot().ID);
	//OnItemRemovedFromInventory(BetrayalPlayerController->WB_HUD, InventoryComponent->GetSelectedSlot());

	//GEngine->AddOnScreenDebugMessage(-10, 2.0f, FColor::Green, "Dropping item at: " + ItemDropLocation->GetComponentLocation().ToString());
}

void APlayerCharacter::RemoveHeldItemFromInventory()
{
	Server_RemoveHeldItemFromInventory();

	FTimerHandle TimerHandle = FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlayerCharacter::ClientUpdateInventory, 0.05f, false);
}

void APlayerCharacter::Server_RemoveHeldItemFromInventory_Implementation()
{
	if (!HeldItem)
		return;

	UnequipItem();

	InventoryComponent->Server_RemoveItemFromInventory(InventoryComponent->GetSelectedSlot().ID);
	InventoryComponent->Server_DeselectSlot(InventoryComponent->GetSelectedSlot().ID);
}

void APlayerCharacter::DropItem(FInventorySlot Slot)
{
	for (auto InventorySlot : InventoryComponent->GetInventorySlots())
	{
		if (Slot != InventorySlot)
			continue;

		if (InventorySlot.bIsEquipped)
		{
			DropHeldItem();
			continue;
		}

		if (InventorySlot.bIsEmpty)
			continue;

		AItemActor* SlotItem = InventoryComponent->GetItemInSlot(InventorySlot.ID).Actor.GetDefaultObject();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		if (AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(SlotItem->GetClass(), SpawnParams))
		{
			ItemActor->NetMulticast_EnableItemPhysics(true);
			ItemActor->SetActorRelativeLocation(ItemDropLocation->GetComponentLocation());
			ItemActor->SetActorRelativeRotation(ItemDropLocation->GetComponentRotation());
			ItemActor->bIsInteractable = true;
		}

		InventoryComponent->Server_RemoveItemFromInventory(InventorySlot.ID);

		if (InventorySlot.bIsSelected)
			InventoryComponent->Server_DeselectSlot(InventorySlot.ID);

		// FTimerHandle TimerHandle = FTimerHandle();
		// GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlayerCharacter::ClientUpdateInventory, 0.5f, false);

		//ClientUpdateInventory();
	}
}

void APlayerCharacter::Server_DropItem_Implementation(FInventorySlot Slot)
{
	DropItem(Slot);
}

void APlayerCharacter::ConsumeHeldItem()
{
	if(!HeldItem)
		return;

	// INFO: Need to re-set holder on item usage as it becomes null 
	if(const TSoftObjectPtr<ABaseInteractable> InteractableItemData = Cast<ABaseInteractable>(HeldItem))
	{
		InteractableItemData->Holder = this;
	}
	
	if(const TSoftObjectPtr<AConsumable> Consumable = Cast<AConsumable>(HeldItem))
	{
		// INFO: Access consumable item data info
		if (const FConsumableItemData* ConsumableItemData = Consumable->ItemData.DataTable->FindRow<FConsumableItemData>(Consumable->ItemData.RowName, ""))
		{
			// INFO: Logic for Effect Changing Item
			if (ConsumableItemData->bIsPassive)
			{
				PrintLog("Consuming Effect Changing Item");
				// More logic may go here
			}
			// INFO: Logic for Stat Changing Item
			else
			{
				PrintLog("Consuming Stat Changing Item");
				// More logic may go here
			}
		}
		
		Consumable->OnConsume();
		Server_ConsumeHeldItem();
		Server_RemoveHeldItemFromInventory();
	}
	
	
	
}

void APlayerCharacter::Server_ConsumeHeldItem_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Blue,TEXT("Player has consumed item"));
}

void APlayerCharacter::ThrowHeldItem()
{
	Server_ThrowHeldItem();
}

void APlayerCharacter::Server_ThrowHeldItem_Implementation()
{
	// INFO: Returns if the player isn't holding an item or if the item isn't throwable
	if (!HeldItem || HeldItem && !HeldItem->GetClass()->ImplementsInterface(UThrowableItem::StaticClass()))
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(HeldItem->GetClass(), SpawnParams))
	{
		ItemActor->NetMulticast_EnableItemPhysics(true);
		ItemActor->SetActorRelativeLocation(ItemDropLocation->GetComponentLocation());
		ItemActor->SetActorRelativeRotation(ItemDropLocation->GetComponentRotation());
		ItemActor->bIsInteractable = false;

		const FVector ThrowVector = CameraComponent->GetForwardVector() * ThrowForce;

		ItemActor->ItemMesh->AddImpulse(ThrowVector);
		
		if (AProjectile* Projectile = Cast<AProjectile>(ItemActor))
		{
			Projectile->Execute_OnThrow(Projectile);
		}
	}

	UnequipItem();

	InventoryComponent->Server_RemoveItemFromInventory(InventoryComponent->GetSelectedSlot().ID);
	InventoryComponent->Server_DeselectSlot(InventoryComponent->GetSelectedSlot().ID);
}



void APlayerCharacter::DropAllItems()
{
	for (auto Slot : InventoryComponent->GetInventorySlots())
	{
		DropItem(Slot);
	}


}

void APlayerCharacter::Client_AddItemToInventory_Implementation(int SlotID, AItemActor* Item)
{
	// This does the same as Server_AddItemToInventory, but locally.

	// Check if the data table is valid.
	if (!IsValid(Item->ItemData.DataTable))
	{
		PrintError("APlayerCharacter::Client_AddItemToInventory - Item: " + Item->GetName() + " data table is invalid");
		return;
	}

	// Get item data from the item actor.
	// It grabs data from the data table in AItemActor by finding the row with the same name.
	const FItem ItemData = *Item->ItemData.DataTable->FindRow<FItem>(Item->ItemData.RowName, "");

	InventoryComponent->AddItemToSlot(ItemData, SlotID);

	ClientUpdateInventory();
}

void APlayerCharacter::Server_AddItemToInventory_Implementation(int SlotID, AItemActor* Item,
                                                                APlayerCharacter* NewOwner)
{
	// Check if the data table is valid.
	if (!IsValid(Item->ItemData.DataTable))
	{
		PrintError("APlayerCharacter::Server_AddItemToInventory - Item: " + Item->GetName() + " data table is invalid");
		return;
	}

	// Get item data from the item actor.
	// It grabs data from the data table in AItemActor by finding the row with the same name.
	const FItem ItemData = *Item->ItemData.DataTable->FindRow<FItem>(Item->ItemData.RowName, "");

	NewOwner->InventoryComponent->Server_AddItemToSlot(ItemData, SlotID);

	Item->OnPickup(NewOwner); // This is only being called on server side.

	Item->Destroy();
}

void APlayerCharacter::ClientUpdateEquippedItem()
{
	if (!IsPlayerControlled())
		return;
	auto PlayerController = GetController<ABetrayalPlayerController>();
	if (!PlayerController)
	{
		PrintError("APlayerCharacter::ClientUpdateEquippedItem - Controller is invalid");
		return;
	}

	OnSlotSelected(PlayerController->WB_HUD, InventoryComponent->GetSelectedSlot());
}

void APlayerCharacter::InventoryUIUpdateDirtyFix()
{
	if(!IsLocallyControlled())
		return;
	
	//only run function every 2 seconds
	if (GetWorld()->GetTimeSeconds() - LastInventoryUpdate > 1.0f)
	{
		ClientUpdateInventory();
		ClientUpdateEquippedItem();
		LastInventoryUpdate = GetWorld()->GetTimeSeconds();
	}
}

void APlayerCharacter::InspectItem()
{
	// INFO: If we are inspecting an item, stop inspecting it and return
	if (bIsInspecting)
	{
		bIsInspecting = false;
		ItemInspectLocation->SetVisibility(false);

		// INFO: Clear all materials ready for next inspection
		const int NumMaterials = ItemInspectLocation->GetNumMaterials();
				
		for (int i = 0; i < NumMaterials; i++)
		{
			if (UMaterialInterface* Material = ItemInspectLocation->GetMaterial(i))
			{
				ItemInspectLocation->SetMaterial(i, nullptr);
			}
		}
		
		ItemInspectLocation->SetStaticMesh(nullptr);

		return;
	}
	
	if (CurrentInteractableInFocus)
	{
		if (CurrentInteractableInFocus->Implements<UInteractableItem>())
		{
			const AItemActor* Item = Cast<AItemActor>(CurrentInteractableInFocus);

			if (!Item)
			{
				PrintError("APlayerCharacter::InspectItem - Interactable failed to cast to AItemActor");
				return;
			}

			if (!bIsInspecting)
			{
				bIsInspecting = true;

				FVector CameraForward = CameraComponent->GetForwardVector();
				FVector CameraLocation = CameraComponent->GetComponentLocation();

				ItemInspectLocation->SetWorldLocation(CameraLocation + (CameraForward * InspectDistance));
				ItemInspectLocation->SetWorldRotation(CameraComponent->GetComponentRotation());
				
				ItemInspectLocation->SetVisibility(true);
				ItemInspectLocation->SetRelativeScale3D(Item->InspectItemScale);
				ItemInspectLocation->SetStaticMesh(Item->ItemMesh->GetStaticMesh());

				// INFO: Assign all materials from the item to the inspecting object
				const int NumMaterials = Item->ItemMesh->GetNumMaterials();
				
				for (int i = 0; i < NumMaterials; i++)
				{
					if (UMaterialInterface* Material = Item->ItemMesh->GetMaterial(i))
					{
						ItemInspectLocation->SetMaterial(i, Material);
					}
				}
			}
		}
	}
}

void APlayerCharacter::Server_DropAllItems_Implementation()
{
	DropAllItems();

	
}


void APlayerCharacter::Server_EquipItem_Implementation(int SlotID)
{
	EquipItem(SlotID);
}

void APlayerCharacter::RunStart_Implementation()
{
	if (bIsStunned)
		return;

	bIsRunning = true;
	Server_SetMaxWalkSpeed_Implementation(RunSpeed);
}

void APlayerCharacter::RunEnd_Implementation()
{
	if (bIsStunned)
		return;

	bIsRunning = false;
	Server_SetMaxWalkSpeed_Implementation(WalkSpeed);
}

void APlayerCharacter::Server_UnequipItem_Implementation()
{
	UnequipItem();
}

void APlayerCharacter::Server_SetInteractableInFocus_Implementation(bool bNewValue)
{
	bIsInteractableInFocus = bNewValue;
}

void APlayerCharacter::TraceForInteractables()
{
	if (!IsLocallyControlled())
		return;

	FVector TraceStart = CameraComponent->GetComponentLocation();
	FVector TraceEnd = TraceStart + (CameraComponent->GetForwardVector() * (!bIsInspecting ? InteractDistance : 0.0f));
	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("")), false, this);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd,ECC_PhysicsBody, TraceParams))
	{
		AActor* HitActor = HitResult.GetActor();
		
		if(!HitActor)
			return;
		
		if(HitActor->Implements<UInteractable>() && HitActor != CurrentInteractableInFocus)
		{			
			CurrentInteractableInFocus = Cast<ABaseInteractable>(HitActor);
			CurrentInteractableInFocus->OnBeginFocus(this);

			if(PreviousInteractableInFocus && PreviousInteractableInFocus != CurrentInteractableInFocus)
				PreviousInteractableInFocus->OnEndFocus(this);
			
			PreviousInteractableInFocus = CurrentInteractableInFocus;
			
			if(CurrentInteractableInFocus && !CurrentInteractableInFocus->bIsInteractable)
			{
				CurrentInteractableInFocus = nullptr;

				if(PreviousInteractableInFocus)
				{
					PreviousInteractableInFocus->OnEndFocus(this);
					
					PreviousInteractableInFocus = nullptr;
				}
			}
				
		}
		else if (!HitActor->Implements<UInteractable>())
		{
			CurrentInteractableInFocus = nullptr;
			
			if(PreviousInteractableInFocus)
			{
				PreviousInteractableInFocus->OnEndFocus(this);
				PreviousInteractableInFocus = nullptr;
			}
		}
	}
	else
	{
		CurrentInteractableInFocus = nullptr;
		
		if(PreviousInteractableInFocus)
		{
			PreviousInteractableInFocus->OnEndFocus(this);
			PreviousInteractableInFocus = nullptr;
		}
	}
	
}

void APlayerCharacter::Interact()
{
	if (CurrentInteractableInFocus)
	{
		// If the interactable is an item, pick it up.
		if (CurrentInteractableInFocus->Implements<UInteractableItem>())
		{
			AItemActor* Item = Cast<AItemActor>(CurrentInteractableInFocus);
			if (!Item)
			{
				PrintError("APlayerCharacter::Interact - Interactable failed to cast to AItemActor");
				return;
			}

			if (!Item->CanPickup())
			{
				PrintWarning("APlayerCharacter::Interact - Item cannot be picked up. bCanPickup is false");
				return;
			}

			const int SlotID = InventoryComponent->GetFirstEmptySlotID();

			if (SlotID == -1)
			{
				PrintWarning("APlayerCharacter::Interact - Inventory is full");
				return;
			}

			//APlayerCharacter* NewOwner = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			
			Client_AddItemToInventory(SlotID, Item);
			// Originally commented out new owner was set to third param, changed
			// as I feel this makes more sense and works with AI but if it breaks, try reverting - Joseph.
			Server_AddItemToInventory(SlotID, Item, this);

			// Update inventory again to reflect server-side changes
			ClientUpdateInventory();
		}

		// Run the OnInteract function on the interactable.
		CurrentInteractableInFocus->OnInteract(this);
	}
}

void APlayerCharacter::Server_Interact_Implementation(class AActor* NewOwner, class ABaseInteractable* Interactable)
{
	if (Interactable)
		Interactable->OnInteract(NewOwner);

	NetMulticast_Interact(NewOwner, Interactable);
}

void APlayerCharacter::Client_Interact_Implementation(AActor* NewOwner, ABaseInteractable* Interactable)
{
	if (Interactable)
		Interactable->OnInteract(NewOwner);

	ClientUpdateInventory();
}

void APlayerCharacter::ClientUpdateInventory()
{
	if (!IsPlayerControlled())
		return;
	
	if (!InventoryComponent)
	{
		PrintError("APlayerCharacter::ClientUpdateInventory - Inventory Component is invalid");
		return;
	}

	auto PlayerController = GetController<ABetrayalPlayerController>();
	if (!PlayerController)
	{
		PrintError("APlayerCharacter::ClientUpdateInventory - Controller is invalid");
		return;
	}
	OnInventoryUpdated(PlayerController->WB_HUD, InventoryComponent->GetInventorySlots());

	//PrintWarning("APlayerCharacter::ClientUpdateInventory - Inventory updated");
}

void APlayerCharacter::CycleSelectedMonster()
{
	Server_CycleSelectedMonster();
}

void APlayerCharacter::Server_CycleSelectedMonster_Implementation()
{
	const ABetrayalPlayerState* BetrayalPlayerState = GetPlayerState<ABetrayalPlayerState>();
	if (!BetrayalPlayerState->IsTraitor() || Monsters.Num() == 0)
		return;
	if (SelectedMonsterIndex == INDEX_NONE)
		return;
	if (SelectedMonsterIndex == Monsters.Num() - 1)
		SelectedMonsterIndex = 0;
	else
		SelectedMonsterIndex++;
}

void APlayerCharacter::SpawnSelectedMonster()
{
	if (Monsters[SelectedMonsterIndex].Count >= Monsters[SelectedMonsterIndex].MaxAmount)
		return;

	Server_SpawnMonster(Monsters[SelectedMonsterIndex].Monster);
	Monsters[SelectedMonsterIndex].Count++;
}

void APlayerCharacter::Server_SpawnMonster_Implementation(TSubclassOf<AMonster> MonsterType)
{
	const ABetrayalPlayerState* BetrayalPlayerState = GetPlayerState<ABetrayalPlayerState>();
	if (!BetrayalPlayerState->IsTraitor())
		return;
	UWorld* World = GetWorld();
	if (!World)
		return;

	FVector Location = GetActorLocation();
	//Location.Y += 30;
	const FRotator Rotation = GetActorRotation();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	if (AMonster* Monster = World->SpawnActor<AMonster>(MonsterType, Location, Rotation, SpawnInfo))
		Monster->SpawnDefaultController();
}


void APlayerCharacter::NetMulticast_Interact_Implementation(class AActor* NewOwner,
                                                            class ABaseInteractable* Interactable)
{
	// if(NewOwner)
	// 	GEngine->AddOnScreenDebugMessage(-11, 2.0f, FColor::Green, "Interactable Owner: " + NewOwner->GetActorLabel());
}

void APlayerCharacter::BindMontageEvents()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &APlayerCharacter::MontageEnded);
		AnimInstance->OnMontageStarted.AddDynamic(this, &APlayerCharacter::MontageStarted);
	}
}

void APlayerCharacter::MontageStarted(UAnimMontage* Montage)
{
	if (Montage == *AnimationMontages.Find(AM_Attack))
		bIsAttacking = true;
}

void APlayerCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == *AnimationMontages.Find(AM_Attack))
		bIsAttacking = false;
}

void APlayerCharacter::Attack()
{
	OnAttack();

	Server_Attack();
}

void APlayerCharacter::Server_Attack_Implementation()
{
	NetMulticast_Attack();
}

void APlayerCharacter::NetMulticast_Attack_Implementation()
{
	if (!bIsAttacking && !HeldItem && !bIsInspecting) // Don't attack if player has item on hand.
		PlayAnimMontage(AttackMontage, 1.0f, NAME_None);
}


void APlayerCharacter::Stun(AActor* Attacker, float Duration)
{
	Server_DropAllItems();
	Super::Stun(Attacker, Duration);
}

void APlayerCharacter::ToggleLight()
{
	Server_ToggleLight();

	// auto PlayerController = GetController<ABetrayalPlayerController>();
	// if(!PlayerController)
	// {
	// 	PrintError("APlayerCharacter::ToggleLight - Controller is invalid");
	// 	return;
	// }
	//
	// if(AChestlight* Chestlight = Cast<AChestlight>(ChestlightComponent->GetChildActor()))
	// {
	// 	
	// 	OnLightToggled(PlayerController->WB_HUD, Chestlight->IsOn());
	// }
}

void APlayerCharacter::Server_ToggleLight_Implementation()
{
	if (AChestlight* Chestlight = Cast<AChestlight>(ChestlightComponent->GetChildActor()))
		Chestlight->Server_ToggleLight();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	if(IsLocallyControlled())
		BindMontageEvents();
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	TraceForInteractables();

	auto PlayerController = GetController<ABetrayalPlayerController>();
	if (PlayerController)
	{
		bool bIsInteractable = CurrentInteractableInFocus ? true : false;

		OnInteractableInFocus(PlayerController->WB_Interact, bIsInteractable);
	}

	InventoryUIUpdateDirtyFix();
	

}

void APlayerCharacter::Server_SetupInputSubsystem_Implementation()
{
	SetupInputSubsystem();
}

void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (ABetrayalPlayerState* State = GetPlayerState<ABetrayalPlayerState>())
	{
		if (State->IsABot())
			return;
		SetupInputSubsystem();
	}
}

void APlayerCharacter::SetupInputSubsystem()
{
	if (APlayerController* PlayerController = Cast<ABetrayalPlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Move), ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::Move);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Jump), ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::Jump);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Look), ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::TurnLook);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Look), ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::UpDownLook);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Run), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::RunStart);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Run), ETriggerEvent::Completed, this,
		                                   &APlayerCharacter::RunEnd);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Interact), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::Interact);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Inventory1), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::SelectSlot, 0);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Inventory2), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::SelectSlot, 1);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Inventory3), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::SelectSlot, 2);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Inventory4), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::SelectSlot, 3);


		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_TraitorCycleMonster), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::CycleSelectedMonster);
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_TraitorSpawnMonster), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::SpawnSelectedMonster);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_Attack), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::Attack);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_DropItem), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::DropHeldItem);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_ToggleLight), ETriggerEvent::Started, this,
		                                   &APlayerCharacter::ToggleLight);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_ConsumeItem),ETriggerEvent::Started,this,
			&APlayerCharacter::ConsumeHeldItem);
		
		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_ThrowItem), ETriggerEvent::Started, this,
			&APlayerCharacter::ThrowHeldItem);

		EnhancedInputComponent->BindAction(*InputAction.Find(IAV_InspectItem), ETriggerEvent::Started, this,
			&APlayerCharacter::InspectItem);
	}

#if WITH_EDITOR
	if (!HasAuthority())
		return;
	if (ABetrayalGameMode* BetrayalGameMode = GetWorld()->GetAuthGameMode<ABetrayalGameMode>())
		PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, BetrayalGameMode, &ABetrayalGameMode::SetNextStage);
#endif
}
