#include "MyGameState.h"

#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

AMyGameState::AMyGameState() = default;

void AMyGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		LoadConfig();
	}
}

void AMyGameState::LoadConfig()
{
	/* 1. 解析json文件的路径 */
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("GameConfig.json");
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Config file not found: %s"), *ConfigPath);
		return;
	}

	FString JsonString;

	/* 2. 将json文件读取为字符串 */
	if (FFileHelper::LoadFileToString(JsonString, *ConfigPath))
	{
		// 使用 TJsonReader 读取字符串
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);

		// 使用 FJsonObject 存储解析后的json对象
		TSharedPtr<FJsonObject> JsonObject;

		// 将 TJsonReader 反序列化成 FJsonObject，这个 FJsonObject 是 Json 文件的根
		// 注意判断FJsonObject是否有效
		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			// 从这里开始，按照树形结构，逐层提取json信息
			// 首先用 HasField() 判断是否存在某属性，如存在再用 GetXXXField() 提取该属性的值；
			if (JsonObject->HasField(TEXT("RemainingGameTime")))
			{
				RemainingGameTime = JsonObject->GetNumberField(TEXT("RemainingGameTime"));
			}
			UE_LOG(LogTemp, Warning, TEXT("RemainingGameTime is: %f"), RemainingGameTime);

			if (JsonObject->HasField(TEXT("RemainingSpecialCube")))
			{
				RemainingSpecialCube = JsonObject->GetNumberField(TEXT("RemainingSpecialCube"));
			}
			UE_LOG(LogTemp, Warning, TEXT("RemainingSpecialCube is: %d"), RemainingSpecialCube);

			if (JsonObject->HasField(TEXT("HitScore")))
			{
				HitScore = JsonObject->GetNumberField(TEXT("HitScore"));
			}
			UE_LOG(LogTemp, Warning, TEXT("HitScore is: %d"), HitScore);

			if (JsonObject->HasField(TEXT("ScaledSize")))
			{
				ScaledSize = JsonObject->GetNumberField(TEXT("ScaledSize"));
			}
			UE_LOG(LogTemp, Warning, TEXT("ScaledSize is: %f"), ScaledSize);

			if (JsonObject->HasField(TEXT("CubeSpawnRange")))
			{
				TArray<TSharedPtr<FJsonValue>> CubeSpawnRangeArray = JsonObject->GetArrayField(TEXT("CubeSpawnRange"));
				if (CubeSpawnRangeArray.Num() == 3)
				{
					CubeSpawnRange.X = CubeSpawnRangeArray[0]->AsNumber();
					UE_LOG(LogTemp, Warning, TEXT("CubeSpawnRange.X is: %f"), CubeSpawnRange.X);
					CubeSpawnRange.Y = CubeSpawnRangeArray[1]->AsNumber();
					UE_LOG(LogTemp, Warning, TEXT("CubeSpawnRange.Y is: %f"), CubeSpawnRange.Y);
					CubeSpawnRange.Z = CubeSpawnRangeArray[2]->AsNumber();
					UE_LOG(LogTemp, Warning, TEXT("CubeSpawnRange.Z is: %f"), CubeSpawnRange.Z);
				}
			}
		}
	}
}

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 在这里使用DOREPLIFETIME宏来设置需要同步的变量
	// 这里一定要注意，这里设置的变量必须是在.h文件中使用Replicated标记的变量
	DOREPLIFETIME(AMyGameState, PlayerScores);
	DOREPLIFETIME(AMyGameState, RemainingGameTime);
	DOREPLIFETIME(AMyGameState, RemainingSpecialCube);
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

