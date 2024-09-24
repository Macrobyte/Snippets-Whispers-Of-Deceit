// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseInteractable.h"

#include "../Player/BaseCharacter.h"
#include "BetrayalGame/StaticUtils.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

// Sets default values
ABaseInteractable::ABaseInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	AIStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AI Stimuli Source Component"));
	if (AIStimuliSourceComponent)
	{
		AIStimuliSourceComponent->bAutoRegister = true;
	}
}

void ABaseInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseInteractable, bIsInteractable);
	
}

void ABaseInteractable::OnInteract(AActor* Interactor)
{
	// INFO: Sets the interactor as the holder of the interactable object
	Holder = Interactor;
	
	InteractEvent(Interactor);
}

void ABaseInteractable::OnBeginFocus(AActor* Interactor)
{
	PrintWarning("OnBeginFocus");
	
	EventBeginFocus(Interactor);
	
	if (!OutlineMesh)
		return;
	
	OutlineMesh->SetRenderCustomDepth(true);
}

void ABaseInteractable::OnEndFocus(AActor* Interactor)
{
	PrintWarning("OnEndFocus");
	
	EventEndFocus(Interactor);
	
	if (!OutlineMesh)
		return;
		
	OutlineMesh->SetRenderCustomDepth(false);
}

// Called when the game starts or when spawned
void ABaseInteractable::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);
	OutlineMesh = StaticMeshComponents.Last();

	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OutlineMesh: %s"), *OutlineMesh->GetName()));
	
	OutlineMesh->CustomDepthStencilValue = 1;
	//OutlineMesh->SetRenderCustomDepth(true);
	
}

void ABaseInteractable::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (AIStimuliSourceComponent)
	{		
		AIStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		AIStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
}

// Called every frame
void ABaseInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

