// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BetrayalGame/BetrayalGameState.h"
#include "Components/ActorComponent.h"
#include "ObjectivesComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BETRAYALGAME_API UObjectivesComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FObjective CurrentEventObjective;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Objectives")
	FObjective CurrentHauntObjective;

public:
	UObjectivesComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Objectives")
	void SetEventObjective(FObjective NewObjective) { CurrentEventObjective = NewObjective; };

	UFUNCTION(Server, Reliable)
	void Server_SetEventObjective(FObjective NewObjective);
	
	UFUNCTION(BlueprintCallable, Category="Objectives")
	void SetHauntObjective(FObjective NewObjective) { CurrentHauntObjective = NewObjective; }

	UFUNCTION(Server, Reliable)
	void Server_SetHauntObjective(FObjective NewObjective);
	
	UFUNCTION(BlueprintCallable, Category="Objectives")
	FObjective GetEventObjective() { return CurrentEventObjective; };

	UFUNCTION(BlueprintCallable, Category="Objectives")
	FObjective GetHauntObjective() { return CurrentHauntObjective; };
		
};
