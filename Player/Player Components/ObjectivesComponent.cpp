// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectivesComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UObjectivesComponent::UObjectivesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
}


void UObjectivesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UObjectivesComponent, CurrentEventObjective);
	DOREPLIFETIME(UObjectivesComponent, CurrentHauntObjective);
}

// Called when the game starts
void UObjectivesComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UObjectivesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UObjectivesComponent::Server_SetEventObjective_Implementation(FObjective NewObjective)
{
	
}

void UObjectivesComponent::Server_SetHauntObjective_Implementation(FObjective NewObjective)
{
	SetHauntObjective(NewObjective);
}





