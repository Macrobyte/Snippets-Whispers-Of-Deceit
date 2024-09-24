// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawnLocation.generated.h"

UCLASS()
class BETRAYALGAME_API AItemSpawnLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemSpawnLocation();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PlaceholderActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> ItemFilter = AActor::StaticClass();

	UFUNCTION(BlueprintCallable)
	void UpdatePlaceholderActor();
	
public:
	UChildActorComponent* GetPlaceholderActor() const { return PlaceholderActor; }
	TSubclassOf<AActor> GetItemFilter() const { return ItemFilter; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


#if WITH_EDITOR

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
#endif

};
