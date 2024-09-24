// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../BaseInteractable.h"
#include "ItemActor.h"
#include "PianoInteractable.generated.h"

/**
 * 
 */
UCLASS()
class BETRAYALGAME_API APianoInteractable : public ABaseInteractable
{
	GENERATED_BODY()

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piano")
	int NecessarySheets;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Piano")
	int CurrentSheets;

	virtual void OnInteract(AActor* Interactor) override;
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category= "Piano")
	void OnPianoComplete();
	
	
};
