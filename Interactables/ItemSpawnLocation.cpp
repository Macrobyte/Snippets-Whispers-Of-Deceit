// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawnLocation.h"

// Sets default values
AItemSpawnLocation::AItemSpawnLocation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlaceholderActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("Placeholder Actor"));
	if (PlaceholderActor)
		PlaceholderActor->SetupAttachment(RootComponent);
}

void AItemSpawnLocation::UpdatePlaceholderActor()
{
	if (!ItemFilter.Get())
		return;
	PlaceholderActor->SetChildActorClass(ItemFilter.Get());
}

// Called when the game starts or when spawned
void AItemSpawnLocation::BeginPlay()
{
	Super::BeginPlay();
	
	GetPlaceholderActor()->DestroyChildActor();
}

// Called every frame
void AItemSpawnLocation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void AItemSpawnLocation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AItemSpawnLocation, ItemFilter))
	{
		UpdatePlaceholderActor();
	}
}
#endif

