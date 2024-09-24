// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseHaunt.h"

#include "../../BetrayalGameMode.h"
#include "../Player/Player Components/ObjectivesComponent.h"
#include "BetrayalGame/BetrayalGameState.h"
#include "BetrayalGame/StaticUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABaseHaunt::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseHaunt, HauntName);
	DOREPLIFETIME(ABaseHaunt, HauntDescription);
	DOREPLIFETIME(ABaseHaunt, HauntCategory);
	DOREPLIFETIME(ABaseHaunt, HauntDuration);
	DOREPLIFETIME(ABaseHaunt, TraitorObjective);
	DOREPLIFETIME(ABaseHaunt, TraitorMonsters);
	DOREPLIFETIME(ABaseHaunt, SurvivorObjective)
	DOREPLIFETIME(ABaseHaunt, Traitor);
	DOREPLIFETIME(ABaseHaunt, Survivors);
	
}

void ABaseHaunt::SpawnHauntItems_Implementation(bool bInitialize)
{
	if (HauntActorSpawnPoints.Num() == 0)
	{
		PrintError("Haunt items spawn failed, can't find any spawn points in world..");
		return;
	}	
	
	// Spawn Haunt Items
	if (HauntActors.Num() > 0)
	{
		for (const FHauntActor& HauntActor : HauntActors)
		{
			if (HauntActor.bSpawnAtStart != bInitialize)
				continue;

			FVector Location = FVector::Zero();
			FRotator Rotation = FRotator::ZeroRotator;
			AItemSpawnLocation* ChosenSpawnPoint = nullptr;

			for (AItemSpawnLocation* SpawnPoint : HauntActorSpawnPoints)
			{
				if (!SpawnPoint)
					continue;
				if (HauntActor.Actor.Get() != SpawnPoint->GetItemFilter().Get())
				{
					PrintError("Haunt item spawn failed, invalid spawn point.." + HauntActor.Actor.Get()->GetName() + " and " + SpawnPoint->GetItemFilter().Get()->GetName());
					continue;
				}
				ChosenSpawnPoint = SpawnPoint;
			}

			if (!ChosenSpawnPoint)
			{
				PrintError( "Haunt item spawn failed, invalid chosen spawn point..");
				continue;				
			}
			
			Location = ChosenSpawnPoint->GetActorLocation();
			Rotation = ChosenSpawnPoint->GetActorRotation();
			
			HauntActorSpawnPoints.Remove(ChosenSpawnPoint);
			ChosenSpawnPoint->Destroy();
			
			FActorSpawnParameters SpawnParams;
			AActor* Actor = GetWorld()->SpawnActor(HauntActor.Actor, &Location, &Rotation, SpawnParams);
			
			PrintWarning("Haunt item spawned..");
		}
	}
}

ABaseHaunt::ABaseHaunt(): HauntCategory(Hc_Asymmetric), bHasTimer(false), Traitor(nullptr), bHasTraitor(false)
{
	bReplicates = true;
}

void ABaseHaunt::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemSpawnLocation::StaticClass(), SpawnPoints);
	for (AActor* Point : SpawnPoints)
		if (AItemSpawnLocation* SpawnPoint = Cast<AItemSpawnLocation>(Point))
			HauntActorSpawnPoints.Add(SpawnPoint);

	
	SpawnHauntItems(true);
	
}

void ABaseHaunt::ConfigureHaunt(FName NewName, const FText& NewDescription, TEnumAsByte<EHauntCategory> NewCategory,
                                bool bUsesTimer, float NewDuration, bool bUsesTraitor, TArray<TSubclassOf<AMonster>> NewTraitorMonsters/*, const FDataTableRowHandle& NewTraitorObjective,
	, const FDataTableRowHandle& NewSurvivorObjective*/)
{
	HauntName = NewName;
	HauntDescription = NewDescription;
	HauntCategory = NewCategory;
	bHasTimer = bUsesTimer;
	HauntDuration = NewDuration;
	bHasTraitor = bUsesTraitor;
	// TraitorObjective = NewTraitorObjective;
	TraitorMonsters = NewTraitorMonsters;
	// SurvivorObjective = NewSurvivorObjective;

	
}

void ABaseHaunt::StartHaunt()
{
	TraitorSetup();
		
	SurvivorSetup();
	
	OnHauntStart();

	if (HasAuthority())
	{
		SpawnHauntItems(false);
	}
}

void ABaseHaunt::Server_StartHaunt_Implementation()
{
	StartHaunt();
}

void ABaseHaunt::EndHaunt()
{
	OnHauntEnd();
}

void ABaseHaunt::Server_EndHaunt_Implementation()
{
	EndHaunt();
}

void ABaseHaunt::TraitorSetup()
{
	if(!bHasTraitor)
		return;

	const ABetrayalGameMode* BMode = Cast<ABetrayalGameMode>(GetWorld()->GetAuthGameMode());
	if(!BMode)
		return;
	
	ABetrayalPlayerState* RandomPlayer = BMode->GetRandomPlayer();
	RandomPlayer->SetIsTraitor(true);

	const FObjective* TraitorObjectiveData = TraitorObjective.DataTable->FindRow<FObjective>(TraitorObjective.RowName, "");
	if(!TraitorObjectiveData)
		return;

	const APlayerCharacter* TraitorCharacter = Cast<APlayerCharacter>(RandomPlayer->GetPawn());
	if(!TraitorCharacter)
		return;

	TraitorCharacter->ObjectivesComponent->Server_SetHauntObjective(*TraitorObjectiveData);
	
	// TODO Not working fix later
	Traitor = TraitorCharacter;
	
	OnTraitorChosen(TraitorCharacter);
	
	PrintWarning("Traitor Setup");
}

void ABaseHaunt::SurvivorSetup()
{
	const ABetrayalGameMode* BMode = Cast<ABetrayalGameMode>(GetWorld()->GetAuthGameMode());
	if(!BMode)
		return;

	for (const auto Player : BMode->GetAllPlayerStates())
	{
		if(Player->IsTraitor())
			continue;
		
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Player->GetPawn());
		if(!PlayerCharacter)
			continue;
		
		const FObjective* SurvivorObjectiveData = SurvivorObjective.DataTable->FindRow<FObjective>(SurvivorObjective.RowName, "");
		if(!SurvivorObjectiveData)
			continue;

		PlayerCharacter->ObjectivesComponent->Server_SetHauntObjective(*SurvivorObjectiveData);

		// TODO Not working fix later
		Survivors.Add(PlayerCharacter);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Survivor Added"));
		
	}

	PrintWarning("Survivors Setup");
}





