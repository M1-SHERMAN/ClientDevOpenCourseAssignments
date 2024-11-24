// Fill out your copyright notice in the Description page of Project Settings.

#include "ShootingCubeBase.h"
#include "UELearnProjectGameMode.h"
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

void AShootingCubeBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShootingCubeBase, HitCounter);
}

int AShootingCubeBase::GetScoreValue() const
{
	return HitScore;
}

void AShootingCubeBase::HandleHitEvent(AController* PlayerController)
{
	if (HasAuthority())
	{
		ServerHandleHitEvent(PlayerController);
	}
	else
	{
		ServerHandleHitEvent(PlayerController);
	}
}

void AShootingCubeBase::ServerHandleHitEvent_Implementation(AController* PlayerController)
{
	if (!HasAuthority())
	{
		return;
	}

	HitCounter++;

	UE_LOG(LogTemp, Warning, TEXT("ServerHandleHit with Controller: %s"), 
			PlayerController ? TEXT("Valid") : TEXT("Invalid"));
	
	if (HitCounter == 1)
	{
		SetActorScale3D(GetActorScale3D() * ScaledSize);
		
		if (!PlayerController)
		{
			UE_LOG(LogTemp, Error, TEXT("Hit with no valid InstigatorController!"));
		}
		
		if (AUELearnProjectGameMode* GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
		{
			const int ScoreToAdd = GetScoreValue();
			GameMode->AddScore(PlayerController, ScoreToAdd);
			
			UE_LOG(LogTemp, Warning, TEXT("Adding Score: %d, Controller: %s, GameMode: Valid"), 
						ScoreToAdd,
						PlayerController ? TEXT("Valid") : TEXT("Invalid"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Hit with no valid GameMode!"));
		}
	}
	else if (HitCounter == 2)
	{
		if (!PlayerController)
		{
			UE_LOG(LogTemp, Error, TEXT("Hit with no valid InstigatorController!"));
		}
		
		if (AUELearnProjectGameMode* GameMode = Cast<AUELearnProjectGameMode>(GetWorld()->GetAuthGameMode()))
		{
			const int ScoreToAdd = GetScoreValue();
			GameMode->AddScore(PlayerController, ScoreToAdd);
			UE_LOG(LogTemp, Warning, TEXT("Adding Score: %d, Controller: %s, GameMode: Valid"), 
			ScoreToAdd,
			PlayerController ? TEXT("Valid") : TEXT("Invalid"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Hit with no valid GameMode!"));
		}
		
		Destroy();
	}
}

void AShootingCubeBase::OnRep_HitCounter()
{
	if (HitCounter == 1)
	{
		SetActorScale3D(GetActorScale3D() * ScaledSize);
	}
}

