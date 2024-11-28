// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "MyGameState.h"
#include "Blueprint/UserWidget.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 只在客户端上创建UI
	if (IsLocalController())
	{
		if (GameHUDClass != nullptr)
		{
			GameHUDWidget = CreateWidget<UUserWidget>(this, GameHUDClass);
			if (GameHUDWidget != nullptr)
			{
				GameHUDWidget->AddToViewport();
			}
		}
	}

	// 绑定GameState的OnGameEnd事件，以便于在游戏结束时处理UI
	if (AMyGameState* MyGameState = GetWorld()->GetGameState<AMyGameState>())
	{
		MyGameState->OnGameEnd.AddDynamic(this, &AMyPlayerController::OnGameEnd);
	}
}

void AMyPlayerController::OnGameEnd()
{
	if (!IsLocalController())
	{
		return;
	}

	
	if (GameHUDWidget != nullptr)
	{
		GameHUDWidget->RemoveFromParent();
	}

	if (GameOverUIClass != nullptr)
	{
		GameOverUIWidget = CreateWidget<UUserWidget>(this, GameOverUIClass);
		if (GameOverUIWidget != nullptr)
		{
			DisableInput(this);
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
			GameOverUIWidget->AddToViewport();
		}
	}

}