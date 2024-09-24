// Fill out your copyright notice in the Description page of Project Settings.


#include "BetrayalPlayerController.h"

#include "BetrayalGame/BetrayalGameInstance.h"
#include "BetrayalGame/StaticUtils.h"
#include "BetrayalGame/BetrayalGameMode.h"
#include "BetrayalGame/AI/Controllers/AIPlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "BetrayalGame/BetrayalPlayerState.h"
#include "BetrayalGame/Lobby/BetrayalGameNetworkSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"


void ABetrayalPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABetrayalPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//DOREPLIFETIME(ABetrayalPlayerController, BetrayalPlayerState);
}

void ABetrayalPlayerController::PawnLeavingGame()
{
	if (!HasAuthority())
	{
		Super::PawnLeavingGame();
		return;
	}
	
	const ABetrayalGameMode* GameMode = GetWorld()->GetAuthGameMode<ABetrayalGameMode>();	
	if (!GameMode->UseBots())
	{
		Super::PawnLeavingGame();
		return;
	}	
	
	ReplacePlayerWithBot();
}

void ABetrayalPlayerController::DetermineNewOrReplaceCharacter()
{	
	const auto NetworkSubsystem = GetGameInstance()->GetSubsystem<UBetrayalGameNetworkSubsystem>();
	if (!NetworkSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::DetermineNewOrReplaceCharacter: Network Subsystem is not valid"));
		return;		
	}

	// Finds the total amount of player character actors in the world to determine if
	// it should attempt to create a new character or replace a pre-existing bot by
	// comparing that number to the lobby capacity.
	TArray<AActor*> PlayerCharacters;	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), PlayerCharacters);	
	UE_LOG(LogTemp, Log, TEXT("BetrayalPlayerController::DetermineNewOrReplaceCharacter: PlayerCharacters Count: %i"), PlayerCharacters.Num());
	if (PlayerCharacters.Num() >= NetworkSubsystem->MAX_PLAYERS)
	{
		UE_LOG(LogTemp, Warning, TEXT("BetrayalPlayerController::DetermineNewOrReplaceCharacter: Bot replacement attempted"));
		Server_ReplaceBotWithPlayer();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BetrayalPlayerController::DetermineNewOrReplaceCharacter: New character creation attempted"));
		Server_SetupCharacter(BetrayalGameInstance->CharacterBlueprints.Find(BetrayalGameInstance->GetSelectedCharacter())->GetDefaultObject());
	}
}

void ABetrayalPlayerController::SetupInput()
{
	const auto PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::SetupControllerInput: Couldn't find PlayerCharacter"));
		return;				
	}
	PlayerCharacter->SetupInputSubsystem();
}

void ABetrayalPlayerController::InitStateWithEvent()
{
	OnPlayerSetup();
}

TSubclassOf<AItemActor> ABetrayalPlayerController::FindItemByName(const FString& Name)
{
	if (ItemDataTable)
	{
		FItem* DataTableRow = ItemDataTable->FindRow<FItem>(FName(Name), "");

		if (DataTableRow)
		{
			TSubclassOf<AItemActor> BPActor = DataTableRow->Actor;
			return BPActor;
		}
		else
		{
			if (GEngine)
				PrintWarning("ABetrayalPlayerController::FindItemByName - No item in datatable with that as a row name");
		}
	}
	else
	{
		if (GEngine)
			PrintWarning("ABetrayalPlayerController::FindItemByName - No item datatable attribute in blueprint");
	}

	return NULL;
}

void ABetrayalPlayerController::SpawnItem(const FString& Name)
{
	TSubclassOf<AItemActor> ItemToSpawn = FindItemByName(Name);

	if (ItemToSpawn)
	{
		if (APlayerCharacter* ControllerPawn = Cast<APlayerCharacter>(GetPawn()))
		{
			Server_SpawnItem(ItemToSpawn, ControllerPawn);
		}
	}
}

void ABetrayalPlayerController::AddItemToInventory(const FString& Name)
{
	FItem* DataTableRow = ItemDataTable->FindRow<FItem>(FName(Name), "");

	if (DataTableRow)
	{
		if (APlayerCharacter* ControllerPawn = Cast<APlayerCharacter>(GetPawn()))
		{
			const int SlotID = ControllerPawn->InventoryComponent->GetFirstEmptySlotID();

			if (SlotID == -1)
			{
				PrintWarning("ABetrayalPlayerController::AddItemToInventory - Inventory is full");
				return;
			}

			ControllerPawn->InventoryComponent->AddItemToSlot(*DataTableRow, SlotID);
			ControllerPawn->ClientUpdateInventory();

			ControllerPawn->InventoryComponent->Server_AddItemToSlot(*DataTableRow, SlotID);

			// Update inventory again to reflect server-side changes
			ControllerPawn->ClientUpdateInventory();
		}
	}
}

void ABetrayalPlayerController::SetHealth(const float& NewCurrentHealth)
{
	if (APlayerCharacter* ControllerPawn = Cast<APlayerCharacter>(GetPawn()))
	{
		float Amount;

		if (ControllerPawn->GetCurrentHealth() > NewCurrentHealth)
		{
			Amount = ControllerPawn->GetCurrentHealth() - NewCurrentHealth;

			ControllerPawn->Server_TakeDamage(Amount);
		}
		else if (ControllerPawn->GetCurrentHealth() < NewCurrentHealth)
		{
			Amount = NewCurrentHealth - ControllerPawn->GetCurrentHealth();

			ControllerPawn->Server_Heal(Amount);
		}
	}

}

void ABetrayalPlayerController::ForceStartHaunt()
{
	Server_ForceStartHaunt();
}

//Absolutely horrible but it's going to get changed in future to use procgen tags
//Gets to live for now as this is rudementry
//Would instead just call the Server_TeleportTo and take a single string as a parameter
void ABetrayalPlayerController::TeleportTo(FString Floor, FString Room)
{
	EFloor WantedFloor = EFloor::F_FirstFloor;
	ERooms WantedRoom = ERooms::R_Lobby;

	if (Room.Equals("Lobby", ESearchCase::IgnoreCase))
	{
		WantedRoom = ERooms::R_Lobby;
		Server_TeleportTo(WantedFloor, WantedRoom);
		return;
	}

	if (Floor.Equals("g", ESearchCase::IgnoreCase))
	{
		WantedFloor = EFloor::F_GroundFloor;
	}
	else if (Floor.Equals("b", ESearchCase::IgnoreCase))
	{
		WantedFloor = EFloor::F_Basement;
	}
	else if (Floor.Equals("f", ESearchCase::IgnoreCase))
	{
		WantedFloor = EFloor::F_FirstFloor;
	}
	else
	{
		return;
	}

	if (Room.Equals("bedroom", ESearchCase::IgnoreCase))
	{
		WantedRoom = ERooms::R_Bedroom;
	}
	else
	{
		return;
	}

	Server_TeleportTo(WantedFloor, WantedRoom);
}

//Will be changed in the future but this system is rudementary
//Would instead take a string as a parameter
//And would check the string against the procgen tag of the cheatteleportlocation
void ABetrayalPlayerController::Server_TeleportTo_Implementation(EFloor WantedFloor, ERooms WantedRoom)
{
	APawn* ControlledPawn = GetPawn();

	if (WantedRoom == ERooms::R_Lobby)
	{
		ABetrayalGameMode* GameMode = Cast<ABetrayalGameMode>(GetWorld()->GetAuthGameMode<ABetrayalGameMode>());

		ControlledPawn->TeleportTo(GameMode->FindPlayerStart(this)->GetActorLocation(), ControlledPawn->GetActorRotation());

		return;
	}

	TArray<AActor*> FoundTeleportLocations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheatTeleportLocation::StaticClass(), FoundTeleportLocations);

	ACheatTeleportLocation* CurrentLocation;

	for (AActor* Location : FoundTeleportLocations)
	{
		CurrentLocation = Cast<ACheatTeleportLocation>(Location);

		if (CurrentLocation)
		{
			if (WantedFloor == CurrentLocation->OnFloor && WantedRoom == CurrentLocation->RoomType)
			{
				ControlledPawn->TeleportTo(Location->GetActorLocation(), ControlledPawn->GetActorRotation());
				break;
			}
		}
	}
}

void ABetrayalPlayerController::Server_ForceStartHaunt_Implementation()
{
	ABetrayalGameMode* BetrayalGameMode = GetWorld()->GetAuthGameMode<ABetrayalGameMode>();
	if (BetrayalGameMode)
	{
		if(BetrayalGameMode->GetMatchStage() == EMatchStage::Haunting)
		{
			return;
		}

		BetrayalGameMode->SetMatchStage(static_cast<TEnumAsByte<EMatchStage>>(EMatchStage::Haunting));
	}
	else
	{
		PrintWarning("ABetrayalPlayerController::ForceStartHaunt - No GameMode Found");
	}
}

void ABetrayalPlayerController::Server_SpawnItem_Implementation(TSubclassOf<AItemActor> ItemToSpawn, APlayerCharacter* ControllerPawn)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ControllerPawn;
	SpawnParams.Instigator = ControllerPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AItemActor* ItemActor = GetWorld()->SpawnActor<AItemActor>(ItemToSpawn, SpawnParams))
	{

		ItemActor->NetMulticast_EnableItemPhysics(true);
		ItemActor->SetActorRelativeLocation(ControllerPawn->ItemDropLocation->GetComponentLocation());
		ItemActor->SetActorRelativeRotation(ControllerPawn->ItemDropLocation->GetComponentRotation());
		ItemActor->bIsInteractable = true;

	}
}

bool ABetrayalPlayerController::InitBetrayalPlayerState()
{
	if(!PlayerState)
	{
		PrintError("BetrayerPlayerController::InitBetrayalPlayerState - PlayerState is not valid" );
		return false;
	}
	
	// BetrayalPlayerState = Cast<ABetrayalPlayerState>(PlayerState);
	// if(!BetrayalPlayerState)
	// {
	// 	PrintError("BetrayerPlayerController::InitBetrayalPlayerState - BetrayalPlayerState is not valid");
	// 	// print error depending on server or client
	// 	return false;
	// }
	
	PrintWarning("BetrayerPlayerController::InitBetrayalPlayerState - BetrayalPlayerState IS VALID");
	return true;
}

bool ABetrayalPlayerController::InitBetrayalGameInstance()
{
	BetrayalGameInstance = GetGameInstance<UBetrayalGameInstance>();
	if(!BetrayalGameInstance)
	{
		PrintError("BetrayalPlayerController::InitBetrayalGameInstance - GameInstance is not valid");
		return false;
	}

	PrintWarning("BetrayalPlayerController::InitBetrayalGameInstance - GameInstance IS VALID");
	return true;
}

void ABetrayalPlayerController::ReplacePlayerWithBot()
{
	const TWeakObjectPtr<UWorld> World = GetWorld();
	
	const TWeakObjectPtr<APawn> ControlledPawn = GetPawn();
	if (!ControlledPawn.Get())
	{
		UE_LOG(LogGameMode, Warning, TEXT("BetrayalPlayerController::ReplacePlayerWithBot: Failed to find controlled pawn."));
		return;
	}
	
	// Removes this as the pawns controller and then spawns it's default controller which
	// is whatever AI controller it has assigned in blueprint.
	UnPossess();
	ControlledPawn->SpawnDefaultController();
	if (const auto BotCtrl = ControlledPawn->GetController<AAIPlayerController>())
	{
		// Not all bots have a player state, this is something manually assigned in it's blueprint.
		const auto AIState = BotCtrl->GetPlayerState<ABetrayalPlayerState>();

		// This gets the bot up-to-date with whatever stage the game is in as that can define how it should be behaving.
		if (const auto BetrayalGameMode = World->GetAuthGameMode<ABetrayalGameMode>())
			BetrayalGameMode->UpdateAIPlayerMode(BotCtrl);
		
		// Sets AI state to correct values.
		AIState->SetIsABot(true);
		AIState->SetControlState(CS_AI);

		// Makes sure to retain the leaving players role, ensuring no roles are removed mid-play.
		if (const auto State = GetPlayerState<ABetrayalPlayerState>())
			AIState->SetIsTraitor(State->IsTraitor());
		UE_LOG(LogGameMode, Warning, TEXT("BetrayalPlayerController::ReplacePlayerWithBot: Success."));		
	}
	else
	{		
		UE_LOG(LogGameMode, Error, TEXT("BetrayalPlayerController::ReplacePlayerWithBot: Couldn't spawn AI controller."));
	}
}

void ABetrayalPlayerController::Server_ReplaceBotWithPlayer_Implementation()
{
	// One core reason this is a server RPC, possession calls must be made server-side otherwise they'll be ignored.
	// The other reason being that AI controllers don't exist outside the server, so attempting to call them from
	// clients will yield null pointers.
	
	UE_LOG(LogTemp, Warning, TEXT("BetrayalPlayerController::ReplaceBotWithPlayer: ATTEMPTED"));

	// Attempts to retrieve a pre-existing character that was controlled by a bot.
	const TWeakObjectPtr<APlayerCharacter> PlayerCharacter = GetBotPawn();
	if (!PlayerCharacter.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::ReplaceBotWithPlayer Aborted: No valid bot pawns"));
		return;		
	}
	
	// The character selection system leaves a sphere default pawn in the level as an artefact that typically gets
	// deleted in the repossession process, as that can't be called here it is manually cleaned up instead.
	
	const TWeakObjectPtr<APawn> OldCharacter = GetPawn();
	if (OldCharacter.Get())
	{
		UnPossess();
		OldCharacter->Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::ReplaceBotWithPlayer: Failed to destroy sphere pawn"));
	}

	// The order of operations here is very important as attempting to setup input anywhere else outside of after the
	// server-side possession can lead to a race condition where the player character you're attempting to setup input for,
	// isn't assigned yet on the client.
	
	Possess(PlayerCharacter.Get());	
	PlayerCharacter->PlayerStateReferece = GetPlayerState<ABetrayalPlayerState>();
	UE_LOG(LogTemp, Log, TEXT("BetrayalPlayerController::ReplaceBotWithPlayer: Successfully set player state ref."));
	
	Client_SetupInput();
	
	UE_LOG(LogTemp, Warning, TEXT("BetrayalPlayerController::ReplaceBotWithPlayer: Success."));
}

void ABetrayalPlayerController::Client_SetupInput_Implementation()
{
	if (!IsLocalController())
		return;
	SetupInput();
}

TWeakObjectPtr<APlayerCharacter> ABetrayalPlayerController::GetBotPawn()
{
	// As this function deals with AI controllers, it has to be ran server-side only - could be argued it should be an RPC
	// but as it's only being called server -> server, it felt slightly redundant so instead I left a simple error handle.
	if (!HasAuthority())
	{	
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::GetBotPawn: Does not have authority."));
		return nullptr;			
	}
	
	const auto NetworkSubsystem = GetGameInstance()->GetSubsystem<UBetrayalGameNetworkSubsystem>();
	if (!NetworkSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::GetBotPawn: Network Subsystem is not valid"));
		return nullptr;		
	}

	// Checks through every player state until it finds one owned by a player bot.
	// AI controllers don't typically have player states but it can be enabled in their blueprint.
	const int32 PlayerCount = UGameplayStatics::GetNumPlayerStates(GetWorld());
	TWeakObjectPtr<ABetrayalPlayerState> AIState = nullptr;
	for (int32 i = 0; i < PlayerCount; ++i)
	{
		AIState = Cast<ABetrayalPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), i));
		if (!AIState.Get())
			continue;
		if (AIState->IsABot())
			break;
	}	
	if (!AIState.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("BetrayalPlayerController::GetBotPawn: No available bot Player States"));
		return nullptr;			
	}

	// Shifts the previous bots role onto the newly joined player, ensuring no roles are accidentally removed.
	if (const auto State = GetPlayerState<ABetrayalPlayerState>())
		State->SetIsTraitor(AIState->IsTraitor());

	// Retrieves the bots pawn and destroys its controller.
	const TWeakObjectPtr<APawn>  ControlledPawn = AIState->GetPawn();
	if (const auto AICont = Cast<AAIPlayerController>(AIState->GetOwningController()))
	{
		AICont->UnPossess();
		AICont->Destroy();
	}
	
	return Cast<APlayerCharacter>(ControlledPawn);
}

void ABetrayalPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//PrintWarning( "BetrayalPlayerController::OnRep_PlayerState - Player State Replicated" );

	// When the player state is replicated, if the BetrayalPlayerState is not valid, then initialize it.
	// The ValidatePlayerSate also this but it triggers the state to be replicated again.
	// This resolves a latency issue with the client not having a valid/correct player state.
	
	// if(InitBetrayalPlayerState())
	// {
	// 	PrintWarning("BetrayalPlayerController::OnRep_PlayerState - BetrayalPlayerState IS VALID");
	// }
}

void ABetrayalPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	
	
}

void ABetrayalPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
}

void ABetrayalPlayerController::Setup()
{
	if(!IsLocalController())
		return;
	
	PrintWarning("===================================");
	
	PrintWarning(HasAuthority() ? "BetrayalPlayerController::BeginPlay - Server" : "BetrayalPlayerController::BeginPlay - Client");
	
	InitBetrayalGameInstance();
			
	DetermineNewOrReplaceCharacter();
	
	// THE UI FOR THE INVENTORY DOES NOT WORK IF THE TIMER IS SET TO SAME AS ^ABOVE^.
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer( TimerHandle,this, &ABetrayalPlayerController::OnPlayerSetup, 0.2f, false);

	OnPlayerSetup();
	
	PrintWarning("===================================");
}

void ABetrayalPlayerController::SetupCharacter(APlayerCharacter* CharacterToSpawn)
{
	if(!GetPawn())
	{
		PrintError("BetrayalPlayerController::SetupCharacter - GetPawn is not valid" );
		return;
	}

	// Cache the pawns position.
	const FVector PreviousPawnLocation = GetPawn()->GetActorLocation();

	// Destroy the old pawn.
	GetPawn()->Destroy();

	// Spawn the new pawn. Which is the selected character.
	APlayerCharacter* NewPlayerCharacter = GetWorld()->SpawnActor<APlayerCharacter>(CharacterToSpawn->GetClass(), PreviousPawnLocation, FRotator::ZeroRotator);

	// Possess the new pawn.
	Possess(NewPlayerCharacter);

	// Set the new pawn as the player character. Don't know if its necessary but the Possess function doesn't seem to do it.
	SetPawn(NewPlayerCharacter);

	PrintWarning("BetrayalPlayerController::SetupCharacter - New Pawn is: " + GetPawn()->GetName());
	
	// Setup the input for the new pawn.
	SetupInput();

	NewPlayerCharacter->PlayerStateReferece = GetPlayerState<ABetrayalPlayerState>();

	PrintWarning("BetrayalPlayerController::SetupCharacter - Set the state reference: " + NewPlayerCharacter->PlayerStateReferece->GetName());
}

void ABetrayalPlayerController::Server_SetupCharacter_Implementation(APlayerCharacter* CharacterToSpawn)
{
	SetupCharacter(CharacterToSpawn);
}


void ABetrayalPlayerController::Server_InitializeReferences_Implementation()
{
	//InitBetrayalPlayerState();
}