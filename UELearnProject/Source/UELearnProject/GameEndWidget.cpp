// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEndWidget.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UGameEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始更新一次分数信息
	UpdateScoreBoard();
}

void UGameEndWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

AMyGameState* UGameEndWidget::GetGameState() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetGameState<AMyGameState>();
	}
	return nullptr;
}

void UGameEndWidget::UpdateScoreBoard()
{
	if (!ScoreBoard)
	{
		return;
	}

	if (AMyGameState* GameState = GetGameState())
	{
		const TArray<FPlayerScoreInfo>& PlayerScores = GameState->GetPlayerScoreInfo();

		for (const FPlayerScoreInfo& ScoreInfo : PlayerScores)
		{
			UTextBlock* ScoreText = NewObject<UTextBlock>(this);

			// FSlateFontInfo FontInfo;
			// FontInfo.Size = 32;
			// ScoreText->SetFont(FontInfo);

			FText ScoreFormat = FText::Format(
				NSLOCTEXT("ScoreBoard", "PlayerScore", "Player {0}: {1}"),
				FText::FromString(ScoreInfo.PlayerId),
				FText::AsNumber(ScoreInfo.Score)
				);
			ScoreText->SetText(ScoreFormat);

			ScoreBoard->AddChild(ScoreText);
		}
	}
}


#if WITH_EDITOR

void UGameEndWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!ScoreBoard)
	{
		return;
	}
	ScoreBoard->ClearChildren();
	
	// 创建模拟数据
	TArray<FPlayerScoreInfo> PreviewScores;
	PreviewScores.Add(FPlayerScoreInfo(TEXT("Player1")));
	PreviewScores.Add(FPlayerScoreInfo(TEXT("Player2")));
	PreviewScores[0].Score = 100;
	PreviewScores[1].Score = 200;

	// 为预览数据创建TextBlock
	for (const FPlayerScoreInfo& ScoreInfo : PreviewScores)
	{
		UTextBlock* ScoreText = NewObject<UTextBlock>(this);

		FText ScoreFormat = FText::Format(
			NSLOCTEXT("ScoreBoard", "PlayerScore", "{0}: {1}"),
			FText::FromString(ScoreInfo.PlayerId),
			FText::AsNumber(ScoreInfo.Score)
			);
		ScoreText->SetText(ScoreFormat);

		ScoreBoard->AddChild(ScoreText);
	}
}

#endif



