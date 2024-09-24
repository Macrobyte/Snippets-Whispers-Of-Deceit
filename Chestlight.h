// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chestlight.generated.h"

UCLASS()
class BETRAYALGAME_API AChestlight : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AChestlight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
#pragma region Light
protected:
	UPROPERTY(Replicated, EditAnywhere)
	class USpotLightComponent* Light;

	UPROPERTY(Replicated, EditAnywhere, Category = "Chestlight")
	float Intensity = 1000.0f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Chestlight")
	bool bIsOn = false;

public:

	UFUNCTION()
	USpotLightComponent* GetLight() const { return Light; }
	
	UFUNCTION()
	void ToggleLight();

	UFUNCTION(Server, Reliable)
	void Server_ToggleLight();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ToggleLight();
	
	UFUNCTION(BlueprintCallable, Category = "Chestlight|Light")
	bool IsOn() const { return bIsOn; }

	UFUNCTION(BlueprintPure, Category = "Chestlight|Light")
	float GetIntensity() const { return Intensity; }

	UFUNCTION()
	void SetLightIntensity(float NewIntensity);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetLightIntensity(float NewIntensity);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetLightIntensity(float NewIntensity);
private:
#pragma endregion 
	
#pragma region Battery
protected:
	UPROPERTY(EditAnywhere, Category = "Chestlight|Battery")
	float MaxBatteryCharge = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Chestlight|Battery")
	float CurrentBatteryCharge = MaxBatteryCharge;

	UPROPERTY(VisibleAnywhere, Category = "Chestlight|Battery")
	bool bIsBatteryEmpty = false;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Chestlight|Battery")
	void DrainBattery(float DrainAmount);

	UFUNCTION(BlueprintCallable, Category = "Chestlight|Battery")
	void RechargeBattery(float RechargeAmount);

	UFUNCTION(BlueprintPure, Category = "Chestlight|Battery")
	float GetBatteryCharge() const { return CurrentBatteryCharge; }

	UFUNCTION(BlueprintPure, Category = "Chestlight|Battery")
	bool IsBatteryEmpty() const { return bIsBatteryEmpty; }
	
private:
#pragma endregion 
	


};
