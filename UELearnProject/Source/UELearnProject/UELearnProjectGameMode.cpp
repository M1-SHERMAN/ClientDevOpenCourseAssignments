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
			CubeSpawnRange = MyGameState->GetCubeSpawnRange();
			RemainingSpecialCubeNumber = MyGameState->GetRemainingSpecialCube();
			
			MyGameState->SetRemainingGameTime(MyGameState->GetRemainingGameTime());

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
		RemainingGameTime = MyGameState->GetRemainingGameTime() - 1.0f;
		MyGameState->SetRemainingGameTime(RemainingGameTime);
		
		if (RemainingGameTime <= 0.0f)
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
		if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
		{
			int ScoreToAdd = MyGameState->GetHitScore();
			const float ScaledSize = MyGameState->GetScaledSize();
			if (const ATargetPoint* TargetPointCasted = Cast<ATargetPoint>(TargetPoint))
			{
				const int CurrentRemainingSpecialCubeNumber = MyGameState->GetRemainingSpecialCube();
				TSubclassOf<AShootingCubeBase> CubeClass;
				if (CurrentRemainingSpecialCubeNumber > 0)
				{
					CubeClass = FMath::RandBool() ? NormalCubeClass : SpecialCubeClass;
					if (CubeClass == SpecialCubeClass)
					{
						MyGameState->SetRemainingSpecialCube(CurrentRemainingSpecialCubeNumber - 1);
						ScoreToAdd *= 2;
					}
				}
				else
				{
					CubeClass = NormalCubeClass;
				}
				FVector Location;
				bool bLocationSpawned = false;
				while (!bLocationSpawned)
				{
					Location = TargetPointCasted->GetActorLocation();
					Location.Y += FMath::RandRange(-CubeSpawnRange.Y, CubeSpawnRange.Y);
					Location.Z += FMath::RandRange(-CubeSpawnRange.Z, CubeSpawnRange.Z);

					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(TargetPointCasted);

					bLocationSpawned = !GetWorld()->OverlapAnyTestByChannel(
						Location,
						FQuat::Identity,
						ECollisionChannel::ECC_WorldStatic,
						FCollisionShape::MakeBox(FVector(50.0f)),
							QueryParams);
				}
				
				FRotator Rotation = TargetPointCasted->GetActorRotation();
				if (AShootingCubeBase* SpawnedCube = GetWorld()->SpawnActor<AShootingCubeBase>(CubeClass, Location, Rotation))
				{
					SpawnedCube->SetHitScore(ScoreToAdd);
					SpawnedCube->SetScaledSize(ScaledSize);
				}
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

void AUELearnProjectGameMode::AddScore(const AController* PlayerController, int NewScore) const
{
	if (!PlayerController)
	{
		return;
	}

	if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
	{
		if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
		{
			MyGameState->AddPlayerScore(PlayerState, NewScore);
		}
	}
}
