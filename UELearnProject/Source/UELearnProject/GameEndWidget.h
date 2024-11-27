// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "MyGameState.h"

#include "GameEndWidget.generated.h"

/**
 * 
 */
UCLASS()
class UELEARNPROJECT_API UGameEndWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta= (BindWidget))
	class UScrollBox* ScoreBoard;// 需要在编辑器中创建一个ScrollBox控件，并将这个ScrollBox命名为ScoreBoard以进行绑定

	// 在Widget创建时初始化Widget中的控件
	virtual void NativeConstruct() override;

	// 在Widget销毁时释放控件
	virtual void NativeDestruct() override;

public:
	void UpdateScoreBoard();

private:
	class AMyGameState* GetGameState() const;
	
#if WITH_EDITOR
	// 只在编辑器中生效的函数，主要用来预览效果并调整样式
	virtual void NativePreConstruct() override;
#endif
};

