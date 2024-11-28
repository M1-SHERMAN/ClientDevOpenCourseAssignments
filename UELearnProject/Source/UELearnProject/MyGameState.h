#pragma once

#include "GameFramework/GameStateBase.h"

#include "MyGameState.generated.h"
// 用来存储玩家的Id、分数信息
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

	// 重写GetLifetimeReplicatedProps函数，用来设置需要同步的变量
	// 这一步是必须的，否则PlayerScores和RemainingGameTime变量不会在服务端和客户端之间同步
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	////////////////// 以下是游戏状态相关的变量和函数，通过Json配置文件进行设置 //////////////////
	
	//当RemainingGameTime变量在服务端发生变化时，会调用OnRep_RemainingTime函数
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_RemainingTime, Category="Game State")
	float RemainingGameTime;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_RemainingSpecialCube, Category="Game State")
	int RemainingSpecialCube;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Game State")
	int HitScore;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Game State")
	float ScaledSize;
	
	// 设置Cube生成的范围
	// 这里设置为(0, 1000, 1000)表示Cube将在X轴上的0到500的范围内生成
	UPROPERTY(BlueprintReadWrite, Category="Game State")
	FVector CubeSpawnRange = FVector(0.0f, 500.f, 500.f);
	
	UFUNCTION()
	void OnRep_RemainingTime() const;

	UFUNCTION()
	void OnRep_RemainingSpecialCube() const;


public:
	AMyGameState();

	const TArray<FPlayerScoreInfo>& GetPlayerScoreInfo() const {return PlayerScores;};

	int GetHitScore() const {return HitScore;};
	float GetScaledSize() const {return ScaledSize;};
	
	void AddPlayerScore(const APlayerState* PlayerState, int AddScore);
	int GetPlayerScore(const APlayerState* PlayerState) const;
	int GetTotalPlayerScore() const;

	void SetRemainingGameTime(float NewTime);
	float GetRemainingGameTime() const {return RemainingGameTime;};

	void SetRemainingSpecialCube(int NewSpecialCube);
	int GetRemainingSpecialCube() const {return RemainingSpecialCube;};

	FVector GetCubeSpawnRange() const {return CubeSpawnRange;};
	
	// 定义一个委托，当RemainingGameTime变量发生变化时，会调用该委托
	// 用于通知客户端游戏剩余时间的变化
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
	UPROPERTY(BlueprintAssignable, Category="Game State")
	FOnTimeUpdated OnTimeUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecialCubeUpdated, int, NewSpecialCube);
	UPROPERTY(BlueprintAssignable, Category="Game State")
	FOnSpecialCubeUpdated OnSpecialCubeUpdated;

private:
	void LoadConfig();

	// 使用PostInitializeComponents来加载JSON文件，避免在构造函数中加载
	// 因为构造函数可能在网络复制前就执行了，可能会进一步导致配置数据丢失
	// 而PostInitializeComponents可以确保在所有组件初始化之后、游戏正式开始之前才加载配置文件
	virtual void PostInitializeComponents() override;
};
