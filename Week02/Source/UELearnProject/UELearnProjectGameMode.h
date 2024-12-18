// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UELearnProjectGameMode.generated.h"

UCLASS(minimalapi)
class AUELearnProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AUELearnProjectGameMode();

	void AddScore(const AController* PlayerController, int NewScore) const;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="Game Rules")
	float GameDuration;

	UPROPERTY(BlueprintReadOnly, Category="Game Rules")
	float RemainingGameTime;
	
	UPROPERTY(BlueprintReadOnly, Category="Game Rules")
	int RemainingSpecialCubeNumber;

	UPROPERTY(BlueprintReadOnly, Category="Game Rules")
	FVector CubeSpawnRange;

	FTimerHandle GameTimerHandle;
	void UpdateGameTime();

	virtual void BeginPlay() override;
	

	UFUNCTION(BlueprintCallable, Category="Game Rules")
	void GenerateCubes();
	
	UPROPERTY(EditDefaultsOnly, Category="Cube Classes")
	TSubclassOf<class AShootingCubeNormal> NormalCubeClass;

	UPROPERTY(EditDefaultsOnly, Category="Cube Classes")
	TSubclassOf<class AShootingCubeSpecial> SpecialCubeClass;
};



