#include "MyGameState.h"

#include "ShootingCubeSpecial.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AMyGameState::AMyGameState()
{
	RemainingGameTime = 0.f;
}


void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 在这里使用DOREPLIFETIME宏来设置需要同步的变量
	// 这里一定要注意，这里设置的变量必须是在.h文件中使用Replicated标记的变量
	DOREPLIFETIME(AMyGameState, PlayerScores);
	DOREPLIFETIME(AMyGameState, RemainingGameTime);
}

FPlayerScoreInfo& AMyGameState::FindOrAddPlayerScore(const FString& PlayerId)
{
	// 遍历PlayerScores数组，查找是否已经存在该玩家的分数信息
	for (int32 i = 0; i < PlayerScores.Num(); i++)
	{
		if (PlayerScores[i].PlayerId == PlayerId)
		{
			return PlayerScores[i];
		}
	}
	// 如果不存在，则添加一个新的FPlayerScoreInfo对象到PlayerScores数组中
	int32 NewIndex = PlayerScores.Add(FPlayerScoreInfo(PlayerId));
	return PlayerScores[NewIndex];
}

void AMyGameState::AddPlayerScore(const APlayerState* PlayerState, int AddScore)
{
	// 如果PlayerState为空或者不是服务端，则直接返回
	// 因为我们需要确保只有服务端才能修改PlayerScores数组
	if (!PlayerState || !HasAuthority())
	{
		return;
	}
	const FString PlayerId = FString::FromInt(PlayerState->GetPlayerId());

	FPlayerScoreInfo& PlayerScore = FindOrAddPlayerScore(PlayerId);
	PlayerScore.Score += AddScore;
	
	ForceNetUpdate();
}

int AMyGameState::GetPlayerScore(const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return 0;
	}

	// 使用PlayerState的PlayerId来查找PlayerScores数组中的分数信息
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

void AMyGameState::SetRemainingSpecialCube(int NewSpecialCube)
{
	if (HasAuthority())
	{
		RemainingSpecialCube = NewSpecialCube;
		OnSpecialCubeUpdated.Broadcast(RemainingSpecialCube);
	}
}

void AMyGameState::OnRep_RemainingTime() const
{
	OnTimeUpdated.Broadcast(RemainingGameTime);
}

void AMyGameState::OnRep_RemainingSpecialCube() const
{
	OnSpecialCubeUpdated.Broadcast(RemainingSpecialCube);
}

