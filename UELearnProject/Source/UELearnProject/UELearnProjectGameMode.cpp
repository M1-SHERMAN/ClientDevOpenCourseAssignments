// Copyright Epic Games, Inc. All Rights Reserved.

#include "UELearnProjectGameMode.h"
#include "MyGameState.h"
#include "GameFramework/PlayerState.h"

#include "UObject/ConstructorHelpers.h"

AUELearnProjectGameMode::AUELearnProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	
	GameStateClass = AMyGameState::StaticClass();

}


void AUELearnProjectGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
		{
			MyGameState->SetRemainingGameTime(GameDuration);
			GetWorld()->GetTimerManager().SetTimer(
													GameTimerHandle,
													this,
													&AUELearnProjectGameMode::UpdateGameTime,
													1.0f,
													true);
		}
	}
}

void AUELearnProjectGameMode::UpdateGameTime()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (AMyGameState * MyGameState = GetGameState<AMyGameState>())
	{
		const float NewRemainingTime = MyGameState->GetRemainingGameTime() - 1.0f;
		MyGameState->SetRemainingGameTime(NewRemainingTime);
		check(GEngine);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%f"), NewRemainingTime));
		
		if (NewRemainingTime <= 0.0f)
		{
			EndGame();
		}
	}
}

void AUELearnProjectGameMode::EndGame()
{
	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	if (AMyGameState * MyGameState = GetGameState<AMyGameState>())
	{
		check(GEngine);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Game Over! Final Scores: "));

		UE_LOG(LogTemp, Log, TEXT("=== Game Over! Debug Scores ==="));
		UE_LOG(LogTemp, Log, TEXT("Number of Player Controllers: %d"), GetWorld()->GetNumPlayerControllers());
		
		for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PlayerController = It->Get())
			{
				if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
				{
					int32 Score = MyGameState->GetPlayerScore(PlayerState);
					check(GEngine);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Player: %s, Score: %d"), *PlayerState->GetPlayerName(), Score));
				}
			}
		}
		
		int TotalScore = MyGameState->GetTotalPlayerScore();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Total Score: ") + FString::FromInt(TotalScore));
		



	}
}

void AUELearnProjectGameMode::AddScore(const AController* PlayerController, int Score) const
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode AddScore: PlayerController is null"));
		return;
	}

	if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
	{
		if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
		{
			MyGameState->AddPlayerScore(PlayerState, Score);
			UE_LOG(LogTemp, Warning, TEXT("GameMode adding score %d to player %s"), Score, *PlayerState->GetPlayerName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode AddScore: PlayerState is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode AddScore: GameState is null"));
	}
}
