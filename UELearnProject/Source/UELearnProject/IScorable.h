#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IScorable.generated.h"

UINTERFACE(MinimalAPI)
class UScorable : public UInterface
{
	GENERATED_BODY()
};

// 这个接口主要用来处理得分相关的逻辑
// 当OnHit事件发生时，我们检测是否击中了一个实现了IScorable接口的Actor
// 如果是，我们就调用HandleHitEvent方法来处理得分逻辑
class UELEARNPROJECT_API IScorable
{
	GENERATED_BODY()

public:
	// 返回当前状态下的得分
	virtual int GetScoreValue() const = 0;
    
	// 处理被击中的逻辑
	virtual void HandleHitEvent(AController* InstigatorController) = 0;
};
