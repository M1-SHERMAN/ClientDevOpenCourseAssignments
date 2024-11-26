// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootingCubeBase.h"

#include "ShootingCubeSpecial.generated.h"

/**
 * 
 */
UCLASS()
class UELEARNPROJECT_API AShootingCubeSpecial : public AShootingCubeBase
{
	GENERATED_BODY()
public:
	AShootingCubeSpecial();
protected:
	virtual int GetScoreValue() const override
	{
		return HitScore * 2;
	}
};
