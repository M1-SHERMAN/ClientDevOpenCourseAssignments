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

	void AddScore(const AController* PlayerController, int Score) const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Rules")
	float GameDuration = 300.0f;

	FTimerHandle GameTimerHandle;

	virtual void BeginPlay() override;

	void EndGame();

	void UpdateGameTime();

	UFUNCTION(BlueprintCallable, Category="Game Rules")
	void GenerateCubes();
private:
	UPROPERTY(EditDefaultsOnly, Category="Cube Classes")
	TSubclassOf<class AShootingCubeNormal> NormalCubeClass;

	UPROPERTY(EditDefaultsOnly, Category="Cube Classes")
	TSubclassOf<class AShootingCubeSpecial> SpecialCubeClass;
};



