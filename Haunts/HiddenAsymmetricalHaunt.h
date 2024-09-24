// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseHaunt.h"
#include "HiddenAsymmetricalHaunt.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API AHiddenAsymmetricalHaunt : public ABaseHaunt
{
	GENERATED_BODY()

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
#pragma region Haunt Interactable
	UPROPERTY(EditAnywhere, Category = "Haunt|Hidden Asymmetric|Interactable", meta = (EditCondition = "bAutoSpawnInteractable"))
	TSubclassOf<ABaseInteractable> ObjectiveInteractableClass;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Haunt|Hidden Asymmetric|Interactable", meta = (EditCondition = "bAutoSpawnInteractable"));
	AActor* InteractableSpawnTransform;
	
	UPROPERTY(Replicated, EditAnywhere, Category = "Haunt|Hidden Asymmetric|Interactable", meta = (EditCondition = "!bAutoSpawnInteractable"));
	ABaseInteractable* ObjectiveInteractable;


#pragma endregion

#pragma region Haunt Items
	UPROPERTY(EditAnywhere, Category="Haunt|Hidden Asymmetric|Items");
	TSubclassOf<AItemActor> ObjectiveItemClass;
	
	UPROPERTY(Replicated, VisibleAnywhere, Category="Haunt|Hidden Asymmetric|Items");
	TArray<AActor*> ItemSpawnTransforms;
	
	UPROPERTY(Replicated, EditAnywhere, Category="Haunt|Hidden Asymmetric|Items");
	TArray<AItemActor*> ObjectiveItems;

	
	
#pragma endregion 
	
	
public:
	AHiddenAsymmetricalHaunt();

	virtual void BeginPlay() override;

	UFUNCTION()
	void SetupSpawns();

	UFUNCTION(Server, Reliable)
	void Server_SetupSpawns();

	UFUNCTION()
	void DestroySpawnTransforms();

	
};
