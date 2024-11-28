// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UELEARNPROJECT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	// UI类的指针，用来创建UI实例，需要在编辑器中手动指定
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UUserWidget> GameHUDClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UUserWidget> GameOverUIClass;

	// UI实例的指针，将在BeginPlay函数中初始化
	UPROPERTY()
	UUserWidget* GameHUDWidget;

	UPROPERTY()
	UUserWidget* GameOverUIWidget;

	UFUNCTION()
	void OnGameEnd();

public:
	virtual void BeginPlay() override;
};
