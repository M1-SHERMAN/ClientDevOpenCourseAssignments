// Copyright Epic Games, Inc. All Rights Reserved.

#include "UELearnProjectGameMode.h"
#include "MyGameState.h"
#include "ShootingCubeSpecial.h"
#include "ShootingCubeNormal.h"

#include "Engine/TargetPoint.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AUELearnProjectGameMode::AUELearnProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	
	GameStateClass = AMyGameState::StaticClass();


	if (NormalCubeClass == nullptr)
	{
		NormalCubeClass = AShootingCubeNormal::StaticClass();
	}
	if (SpecialCubeClass == nullptr)
	{
		SpecialCubeClass = AShootingCubeSpecial::StaticClass();
	}
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

void AUELearnProjectGameMode::GenerateCubes()
{
	TArray<AActor*> TargetPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), TargetPoints);

	for (AActor* TargetPoint : TargetPoints)
	{
		if (ATargetPoint* TargetPointCasted = Cast<ATargetPoint>(TargetPoint))
		{
			TSubclassOf<AShootingCubeBase> CubeClass = FMath::RandBool() ? NormalCubeClass : SpecialCubeClass;

			FVector Location = TargetPoint->GetActorLocation();
			FRotator Rotation = TargetPoint->GetActorRotation();
			
			if (auto* Cube = GetWorld()->SpawnActor<AShootingCubeBase>(CubeClass, Location, Rotation))
			{
				Cube->SetReplicates(true);
				Cube->SetReplicateMovement(true);
				Cube->bAlwaysRelevant = true;
			}
		}
	}
}

void AUELearnProjectGameMode::EndGame()
{
	GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);

	if (const AMyGameState* MyGameState = GetGameState<AMyGameState>())
	{
		check(GEngine);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Game Over! Final Scores: "));
		
		for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (const APlayerController* PlayerController = It->Get())
			{
				if (const APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
				{
					const int32 Score = MyGameState->GetPlayerScore(PlayerState);
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
		return;
	}

	if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
	{
		if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
		{
			MyGameState->AddPlayerScore(PlayerState, Score);
		}
	}
}
