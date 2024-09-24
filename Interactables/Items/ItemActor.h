// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BaseInteractable.h"
#include "BetrayalGame/Gameplay/Interactables/InteractableItem.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ItemActor.generated.h"

USTRUCT(BlueprintType)
struct FItem : public FTableRowBase
{
	GENERATED_BODY()

	FItem()
	{
		Name = "None";
		Description = FText::FromString("None");
		Image = nullptr;
		Actor = nullptr;

	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* Image;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AItemActor> Actor;

	// Overload the == operator to compare two FItem structs
	bool operator==(const FItem& Other) const
	{
		return Name == Other.Name && Description.EqualTo(Other.Description) && Image == Other.Image;
	}
};


UCLASS()
class BETRAYALGAME_API AItemActor : public ABaseInteractable , public IInteractableItem
{
	GENERATED_BODY()

#pragma region Base Functions
public:
	// Sets default values for this actor's properties
	AItemActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:

	
#pragma endregion

#pragma region Networking
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
#pragma endregion 
	
#pragma region Interactable Overrides
public:
	
	virtual void OnInteract(class AActor* Interactor) override;

private:
#pragma endregion
	
#pragma region Item
public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Item")
	UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item")
	FDataTableRowHandle ItemData;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsObjectiveItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FVector InspectItemScale;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
	void OnPickup(class AActor* Interactor);

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	bool bCanPickup;

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool CanPickup() const { return bCanPickup; }
	
	void SetCanPickup(const bool CanPickup) { bCanPickup = CanPickup; }

	UFUNCTION(Server, Reliable)
	void Server_SetCanPickup(const bool CanPickup);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_EnableItemPhysics(bool bState);

#pragma endregion 

};
