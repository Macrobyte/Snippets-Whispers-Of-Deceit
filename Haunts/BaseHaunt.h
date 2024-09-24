// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Player/PlayerCharacter.h"
#include "BetrayalGame/Gameplay/Interactables/ItemSpawnLocation.h"
#include "BaseHaunt.generated.h"

UENUM()
enum EHauntCategory
{
	Hc_Asymmetric UMETA(DisplayName = "Asymmetric"),
	Hc_HiddenAsymmetric UMETA(DisplayName = "Hidden Asymmetric"),
	Hc_FreeForAll UMETA(DisplayName = "Free For All"),
};

USTRUCT(BlueprintType)
struct FHauntActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AActor> Actor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bSpawnAtStart = false;
};

UCLASS(Blueprintable)
class BETRAYALGAME_API ABaseHaunt : public AActor
{
	GENERATED_BODY()

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(Replicated, EditAnywhere, Category = "Haunt")
	FName HauntName;
	
	UPROPERTY(Replicated, EditAnywhere, Category = "Haunt", meta = (MultiLine = true));
	FText HauntDescription;
	
	UPROPERTY(Replicated, EditAnywhere, Category = "Haunt")
	TEnumAsByte<EHauntCategory> HauntCategory;
	
	UPROPERTY(EditAnywhere, Category = "Haunt")
	bool bHasTimer;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Haunt", meta = (EditCondition = "bHasTimer"))
	float HauntDuration = 0.0f;

#pragma region Traitor Properties

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Haunt|Traitor")
	const APlayerCharacter* Traitor;
	
	UPROPERTY(EditAnywhere, Category = "Haunt|Traitor")
	bool bHasTraitor;
	
	UPROPERTY(Replicated, EditAnywhere,
			BlueprintReadOnly, Category = "Haunt|Traitor",
			meta = (EditCondition = "HauntCategory == EHauntCategory::Hc_Asymmetric || HauntCategory == EHauntCategory::Hc_HiddenAsymmetric",
					Tooltip = "Traitors only have an objective if the haunt is Asymmetric or Hidden Asymmetric"))
	FDataTableRowHandle TraitorObjective;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category="Haunt|Traitor",
			meta = (EditCondition = "HauntCategory == EHauntCategory::Hc_Asymmetric || HauntCategory == EHauntCategory::Hc_HiddenAsymmetric",
					Tooltip = "Traitors only have a list of monsters if the haunt is Asymmetric or Hidden Asymmetric"))
	TArray<TSubclassOf<AMonster>> TraitorMonsters;
#pragma endregion

#pragma region Survivor Properties

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Haunt|Survivors")
	TArray<APlayerCharacter*> Survivors;
	
	UPROPERTY(Replicated, EditAnywhere,
			BlueprintReadOnly, Category = "Haunt|Survivors",
			meta=(Tooltip = "Survivors will always have an objective"))
	FDataTableRowHandle SurvivorObjective;
#pragma endregion

#pragma region Game
	UPROPERTY(EditDefaultsOnly, Category = "Haunt|Items")
	TArray<FHauntActor> HauntActors;

	UPROPERTY(VisibleAnywhere, Category = "Haunt|Items")
	TArray<AItemSpawnLocation*> HauntActorSpawnPoints;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Haunt|Items")
	void SpawnHauntItems(bool bInitialize = false);
	
#pragma endregion
	
public:
	ABaseHaunt();

	virtual void BeginPlay() override;
	
	void ConfigureHaunt(FName NewName, const FText& NewDescription, TEnumAsByte<EHauntCategory> NewCategory, bool bUsesTimer,
					   float NewDuration, bool bUsesTraitor, TArray<TSubclassOf<AMonster>> NewTraitorMonsters/*, const FDataTableRowHandle& NewTraitorObjective, const TArray<AMonster*>& NewTraitorMonsters,
					   const FDataTableRowHandle& NewSurvivorObjective*/);
	
	// TODO: Add cutscene references(start haunt, end haunt, traitor picked)
	
	UFUNCTION(Blueprintable, Category = "Haunt")
	virtual void StartHaunt();

	UFUNCTION(Server, Reliable)
	void Server_StartHaunt();

	UFUNCTION(BlueprintImplementableEvent, Category = "Haunt")
	void OnHauntStart();

	UFUNCTION(Blueprintable, Category = "Haunt")
	virtual void EndHaunt();

	UFUNCTION(Server, Reliable)
	void Server_EndHaunt();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Haunt")
	void OnHauntEnd();

	void TraitorSetup();

	UFUNCTION(BlueprintImplementableEvent, Category = "Haunt")
	void OnTraitorChosen(const APlayerCharacter* TraitorPlayerCharacter);
	
	void SurvivorSetup();


#pragma region Setters

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	FName GetHauntName() const { return HauntName; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetHauntName(const FName& NewName) { HauntName = NewName; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	FText GetHauntDescription() const { return HauntDescription; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetHauntDescription(const FText& NewDescription) { HauntDescription = NewDescription; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	TEnumAsByte<EHauntCategory> GetHauntCategory() const { return HauntCategory; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetHauntCategory(const TEnumAsByte<EHauntCategory>& NewCategory) { HauntCategory = NewCategory; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	bool HasTimer() const { return bHasTimer; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	float GetHauntDuration() const { return HauntDuration; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetHauntDuration(const float NewDuration) { HauntDuration = NewDuration; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	bool HasTraitor() const { return bHasTraitor; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	FDataTableRowHandle GetTraitorObjective() const { return TraitorObjective; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetTraitorObjective(const FDataTableRowHandle& NewObjective) { TraitorObjective = NewObjective; }
	
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	TArray<TSubclassOf<AMonster>> GetTraitorMonsters() const { return TraitorMonsters; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetTraitorMonsters(TArray<TSubclassOf<AMonster>> NewMonsters) { TraitorMonsters = NewMonsters; }

	UFUNCTION(BlueprintCallable, Category = "Haunt")
	FDataTableRowHandle GetSurvivorObjective() const { return SurvivorObjective; }
	UFUNCTION(BlueprintCallable, Category = "Haunt")
	void SetSurvivorObjective(const FDataTableRowHandle& NewObjective) { SurvivorObjective = NewObjective; }

#pragma endregion


};
