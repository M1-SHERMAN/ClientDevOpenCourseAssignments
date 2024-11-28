// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IScorable.h"
#include "GameFramework/Actor.h"

#include "ShootingCubeBase.generated.h"

class UBoxComponent;

UCLASS(Blueprintable, ClassGroup=(Cube))
class UELEARNPROJECT_API AShootingCubeBase : public AActor, public IScorable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShootingCubeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Collision Box")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, Category="Static Mesh")
	TObjectPtr<UStaticMeshComponent> CubeMesh;

protected:
	// 使用ReplicatedUsing标记，当HitCounter属性更新时，调用OnRep_HitCounter函数
	// 确保客户端和服务器的HitCounter属性同步
	UPROPERTY(ReplicatedUsing = OnRep_HitCounter, BlueprintReadOnly, Category="Scores")
	int HitCounter = 0;

	// HitScore和ScaledSize只会在GameMode中，当Cube被实际生成时才进行设置
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Scores")
	int HitScore;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Scores")
	float ScaledSize;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_HitCounter();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void HandleHitEvent(AController* PlayerController) override;
	
	UFUNCTION(Server, Reliable)
	void ServerHandleHitEvent(AController* InstigatorController);

	void SetHitScore(int NewHitScore) { HitScore = NewHitScore; }
	void SetScaledSize(float NewScaledSize) { ScaledSize = NewScaledSize; }
};
