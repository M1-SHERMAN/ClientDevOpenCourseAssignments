#include "MyGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AMyGameState::AMyGameState()
{
	RemainingGameTime = 0.f;
}

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, PlayerScores);
	DOREPLIFETIME(AMyGameState, RemainingGameTime);
}

FPlayerScoreInfo& AMyGameState::FindOrAddPlayerScore(const FString& PlayerId)
{
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerId == PlayerId)
		{
			return PlayerScores[i];
		}
	}
	int32 NewIndex = PlayerScores.Add(FPlayerScoreInfo(PlayerId));
	return PlayerScores[NewIndex];
}

void AMyGameState::AddPlayerScore(const APlayerState* PlayerState, int Score)
{
	if (!PlayerState || !HasAuthority())
	{
		return;
	}
	const FString PlayerId = FString::FromInt(PlayerState->GetPlayerId());

	FPlayerScoreInfo& PlayerScore = FindOrAddPlayerScore(PlayerId);
	int OldScore = PlayerScore.Score;
	PlayerScore.Score += Score;
	
	ForceNetUpdate();
}

int AMyGameState::GetPlayerScore(const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return 0;
	}
	
	const FString PlayerId = FString::FromInt(PlayerState->GetPlayerId());

	for (const FPlayerScoreInfo& PlayerScore : PlayerScores)
	{
		if (PlayerScore.PlayerId == PlayerId)
		{
			return PlayerScore.Score;
		}
	}

	return 0;
}

int AMyGameState::GetTotalPlayerScore() const
{
	int TotalScore = 0;
	for (const FPlayerScoreInfo& PlayerScore : PlayerScores)
	{
		TotalScore += PlayerScore.Score;
	}
	
	return TotalScore;
}

void AMyGameState::SetRemainingGameTime(float NewTime)
{
	if (HasAuthority())
	{
		RemainingGameTime = NewTime;
		OnTimeUpdated.Broadcast(RemainingGameTime);
	}
}

void AMyGameState::OnRep_RemainingTime() const
{
	OnTimeUpdated.Broadcast(RemainingGameTime);
}

