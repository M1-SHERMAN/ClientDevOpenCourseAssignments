// Fill out your copyright notice in the Description page of Project Settings.

#include "ShootingCubeBase.h"

#include "UELearnProjectCharacter.h"
#include "UELearnProjectGameMode.h"

#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"

// Sets default values
AShootingCubeBase::AShootingCubeBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	RootComponent = CollisionBox;
	CollisionBox->SetBoxExtent(FVector(50.f));

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube Mesh"));
	CubeMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		CubeMesh->SetStaticMesh(CubeMeshAsset.Object);
		CubeMesh->SetRelativeScale3D(FVector(0.95f));
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> CubeMaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial_Inst.BasicShapeMaterial_Inst"));
	if (CubeMaterialAsset.Succeeded())
	{
		CubeMesh->SetMaterial(0, CubeMaterialAsset.Object);
	}

	bReplicates = true;
}

// Called when the game starts or when spawned
void AShootingCubeBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShootingCubeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShootingCubeBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShootingCubeBase, HitCounter);
	DOREPLIFETIME(AShootingCubeBase, HitScore);
	DOREPLIFETIME(AShootingCubeBase, ScaledSize);
}

void AShootingCubeBase::HandleHitEvent(AController *InstigatorController)
{
	if (InstigatorController != nullptr)
	{
		if (APawn *Pawn = InstigatorController->GetPawn())
		{
			if (AUELearnProjectCharacter *Character = Cast<AUELearnProjectCharacter>(Pawn))
			{
				// 获取玩家角色后，让玩家角色来调用服务器的 RPC 函数以处理击中事件
				Character->ServerReportHit(this);
			}
		}
	}
}

void AShootingCubeBase::ServerHandleHitEvent_Implementation(AController *InstigatorController)
{
	HitCounter++;

	if (HitCounter == 1)
	{
		SetActorScale3D(GetActorScale3D() * ScaledSize);

		if (AUELearnProjectGameMode *GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
		{
			const int ScoreToAdd = HitScore;
			GameMode->AddScore(InstigatorController, ScoreToAdd);
		}
	}
	else if (HitCounter == 2)
	{

		if (AUELearnProjectGameMode *GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
		{
			const int ScoreToAdd = HitScore;
			GameMode->AddScore(InstigatorController, ScoreToAdd);
			Destroy();
		}
	}
}

void AShootingCubeBase::OnRep_HitCounter()
{
	if (HitCounter == 1)
	{
		SetActorScale3D(GetActorScale3D() * ScaledSize);
	}
}
