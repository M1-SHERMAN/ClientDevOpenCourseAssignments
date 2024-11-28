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
			MyGameState->MulticastEndGame();
			GetWorld()->GetTimerManager().ClearTimer(GameTimerHandle);
		}
	}
}

void AUELearnProjectGameMode::GenerateCubes()
{
	TArray<AActor*> TargetPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), TargetPoints);
	
	// 遍历所有放置在场景中的TargetPoint，之后需要根据TargetPoint的位置在关卡中动态生成Cube
	for (AActor* TargetPoint : TargetPoints)
	{
		// 获取GameState，在之后生成Cube时需要用到GameState中存储的HitScore和ScaledSize来设置Cube中的相关属性
		if (AMyGameState* MyGameState = GetGameState<AMyGameState>())
		{
			int ScoreToAdd = MyGameState->GetHitScore();
			const float ScaledSize = MyGameState->GetScaledSize();
			if (const ATargetPoint* TargetPointCasted = Cast<ATargetPoint>(TargetPoint))
			{
				/* 1. 计算Cube生成的位置和旋转 */
				FVector Location;
				// 需要确保生成的Cube不会与其他物体重叠，如果重叠则需要重新计算位置
				bool bLocationSpawned = false;
				
				// 设置最大尝试次数，避免当找不到Cube可以生成的位置时导致死循环
				constexpr int MaxAttempts = 50;
				int CurrentAttempt = 0;
				while (!bLocationSpawned && CurrentAttempt <= MaxAttempts)
				{
					
					Location = TargetPointCasted->GetActorLocation();

					// 在X轴上生成Cube，Y轴和Z轴的位置随机
					Location.Y += FMath::RandRange(-CubeSpawnRange.Y, CubeSpawnRange.Y);
					Location.Z += FMath::RandRange(-CubeSpawnRange.Z, CubeSpawnRange.Z);

					// 忽略TargetPoint的碰撞
					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(TargetPointCasted);

					// 使用OverlapAnyTestByChannel函数来检测生成的Cube是否与其他物体重叠
					bLocationSpawned = !GetWorld()->OverlapAnyTestByChannel(
						Location,										// 检测的位置
						FQuat::Identity,								// 检测的旋转
						ECollisionChannel::ECC_WorldDynamic,			// 检测的碰撞频道
						FCollisionShape::MakeBox(FVector(50.0f)),	// 检测的形状
						QueryParams);									// 检测的参数，这里忽略TargetPoint的碰撞
	
					CurrentAttempt++;
				}
				// 如果当前TargetPoint没有找到合适的位置生成Cube，则继续下一个TargetPoint
				if (!bLocationSpawned)
				{
					continue;
				}

				
				FRotator Rotation = TargetPointCasted->GetActorRotation();

				/* 2. 判断Cube的类型 */
				// 获取特殊方块的数量，如果还有特殊方块剩余，则有一半的概率生成特殊方块
				const int CurrentRemainingSpecialCubeNumber = MyGameState->GetRemainingSpecialCube();
				TSubclassOf<AShootingCubeBase> CubeClass;
				if (CurrentRemainingSpecialCubeNumber > 0)
				{
					CubeClass = FMath::RandBool() ? NormalCubeClass : SpecialCubeClass;
					if (CubeClass == SpecialCubeClass)
					{
						// 如果生成了特殊方块，则需要减少剩余的特殊方块数量，并且将分数翻倍
						MyGameState->SetRemainingSpecialCube(CurrentRemainingSpecialCubeNumber - 1);
						ScoreToAdd *= 2;
					}
				}
				else
				{
					CubeClass = NormalCubeClass;
				}
				
				// 如果成功生成Cube，则将Cube的HitScore和ScaledSize设置为GameState中的值
				if (AShootingCubeBase* SpawnedCube = GetWorld()->SpawnActor<AShootingCubeBase>(CubeClass, Location, Rotation))
				{
					SpawnedCube->SetHitScore(ScoreToAdd);
					SpawnedCube->SetScaledSize(ScaledSize);
				}
				
			}
		}
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
