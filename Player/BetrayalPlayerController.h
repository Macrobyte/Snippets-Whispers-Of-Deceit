// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "BetrayalGame/BetrayalPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "BetrayalGame/Gameplay/CheatTeleportLocation.h"
#include "BetrayalPlayerController.generated.h"


class ABetrayalPlayerState;
class APlayerCharacter;

UCLASS()
class BETRAYALGAME_API ABetrayalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PawnLeavingGame() override;

#pragma region References 
protected:

	UPROPERTY()
	UBetrayalGameInstance* BetrayalGameInstance;

public:

	UFUNCTION(Server, Reliable)
	void Server_InitializeReferences();
	
private:
#pragma endregion

#pragma region Input

	
	
#pragma endregion 

#pragma region AI Integration
private:
	// Checks if the lobbies full and if it is the controller replaces a bot controller, otherwise it creates a new character.
	void DetermineNewOrReplaceCharacter();
		
	// Replaces the player with an AI controller.
	void ReplacePlayerWithBot();
	
	// Takes control of an AI controller's pawn.
	UFUNCTION(Server, Reliable)
	void Server_ReplaceBotWithPlayer();

	// Runs the 'SetupInput' function on the owning client.
	UFUNCTION(Client, Reliable)
	void Client_SetupInput();

	// Finds a bot, removes/deletes its AI controller and returns its character.
	TWeakObjectPtr<APlayerCharacter> GetBotPawn();
	
#pragma endregion

#pragma region UI
protected:
	
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Controller|UI")
	UUserWidget* WB_HUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Controller|UI")
	UUserWidget* WB_Interact;
		
private:
#pragma endregion 

#pragma region Cheats
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cheats")
	UDataTable* ItemDataTable;

	UFUNCTION()
	TSubclassOf<AItemActor> FindItemByName(const FString& Name);

	UFUNCTION(Exec)
	void SpawnItem(const FString& Name);
	UFUNCTION(Server, Reliable)
	void Server_SpawnItem(TSubclassOf<AItemActor> ItemToSpawn, APlayerCharacter* ControllerPawn);

	UFUNCTION(Exec)
	void AddItemToInventory(const FString& Name);

	UFUNCTION(Exec)
	void SetHealth(const float& NewCurrentHealth);

	UFUNCTION(Exec)
	void ForceStartHaunt();
	UFUNCTION(Server, Reliable)
	void Server_ForceStartHaunt();

	UFUNCTION(Exec)
	void TeleportTo(FString Floor, FString Room);
	UFUNCTION(Server, Reliable)
	void Server_TeleportTo(EFloor WantedFloor, ERooms WantedRoom);

public:

private:

#pragma endregion 


// ========================================== NEW VASCO SHIT =============================================


	// This function has two purposes:
	// 1 - It initializes the player state, sets up the player input and calls the OnReferenceInitialized event
	// 2 - It causes the PlayerState to be replicated. This calls the OnRep_PlayerState and initializes the
	//	   BetrayalPlayerState in case you didn't in the fist try.
	UFUNCTION()
	bool InitBetrayalPlayerState();

	UFUNCTION()
	bool InitBetrayalGameInstance();
	
	// This OnRep for the player state is used to set the BetrayalPlayerState reference.
	virtual void OnRep_PlayerState() override;

	virtual void OnRep_Pawn() override;

	void OnPossess(APawn* InPawn) override;

	
	UFUNCTION()
	void SetupCharacter(APlayerCharacter* CharacterToSpawn);
	UFUNCTION(Server, Reliable)
	void Server_SetupCharacter(APlayerCharacter* CharacterToSpawn);
	
	// Sets up the pawns input system.
	void SetupInput();

	void InitStateWithEvent();
	
public:
	
	UFUNCTION()
	void Setup();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Controller|References")
	void OnPlayerSetup();
	
	
	
	
};
