#pragma once

#include "GameFramework/GameStateBase.h"

#include "MyGameState.generated.h"

USTRUCT()
struct FPlayerScoreInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerId;

	UPROPERTY()
	int Score;

	FPlayerScoreInfo()
		: Score(0)
	{}

	FPlayerScoreInfo(const FString& InPlayerId)
		: PlayerId(InPlayerId)
		, Score(0)
	{}
};

UCLASS()
class UELEARNPROJECT_API AMyGameState : public AGameStateBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(Replicated)
	TArray<FPlayerScoreInfo>PlayerScores;

	FPlayerScoreInfo& FindOrAddPlayerScore(const FString& PlayerId);
	
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	float RemainingGameTime;

	UFUNCTION()
	void OnRep_RemainingTime() const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	AMyGameState();

	void AddPlayerScore(const APlayerState* PlayerState, int Score);
	
	int GetPlayerScore(const APlayerState* PlayerState) const;

	int GetTotalPlayerScore() const;

	void SetRemainingGameTime(float NewTime);
	float GetRemainingGameTime() const {return RemainingGameTime;};
	
	UFUNCTION(BlueprintCallable, Category="Game State")
	float GetTimeRemaining() const {return RemainingGameTime;};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
	UPROPERTY(BlueprintAssignable, Category="Game State")
	FOnTimeUpdated OnTimeUpdated;
	
};
