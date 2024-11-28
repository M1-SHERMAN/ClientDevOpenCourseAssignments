// Copyright Epic Games, Inc. All Rights Reserved.

#include "UELearnProjectProjectile.h"

#include "IScorable.h"
#include "UELearnProjectGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AUELearnProjectProjectile::AUELearnProjectProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(0.5f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AUELearnProjectProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 15000.f;
	ProjectileMovement->MaxSpeed = 15000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 1 seconds by default
	InitialLifeSpan = 1.0f;

	bReplicates = true;
}

void AUELearnProjectProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUELearnProjectProjectile, Projectile);
}

void AUELearnProjectProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || !HasAuthority()) return;

	// 如果碰撞到了带有IScorable接口的Actor，则调用HandleHitEvent函数，处理碰撞事件
	if (IScorable* Scorable = Cast<IScorable>(OtherActor))
	{
		Scorable->HandleHitEvent(OwningController);
		Destroy();
	}
}

void AUELearnProjectProjectile::SetOwningController(AController* Controller)
{
	OwningController = Controller;
}