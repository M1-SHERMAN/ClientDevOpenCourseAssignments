#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IScorable.generated.h"

UINTERFACE(MinimalAPI)
class UScorable : public UInterface
{
	GENERATED_BODY()
};

class UELEARNPROJECT_API IScorable
{
	GENERATED_BODY()

public:
	// 返回当前状态下的得分
	virtual int GetScoreValue() const = 0;
    
	// 处理被击中的逻辑
	virtual void HandleHitEvent(AController* InstigatorController) = 0;
};