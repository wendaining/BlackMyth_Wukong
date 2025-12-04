// 黑神话 HUD 类实现

#include "BlackMythHUD.h"
#include "PlayerHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

ABlackMythHUD::ABlackMythHUD()
{
	// 默认不设置 Widget 类，需要在蓝图或 GameMode 中配置
}

void ABlackMythHUD::BeginPlay()
{
	Super::BeginPlay();

	CreatePlayerHUD();
}

void ABlackMythHUD::CreatePlayerHUD()
{
	if (!PlayerHUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABlackMythHUD: PlayerHUDWidgetClass is not set! Please set it in the HUD blueprint."));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABlackMythHUD: No PlayerController found!"));
		return;
	}

	// 创建 Widget
	PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(PC, PlayerHUDWidgetClass);
	if (!PlayerHUDWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("ABlackMythHUD: Failed to create PlayerHUDWidget!"));
		return;
	}

	// 添加到视口
	PlayerHUDWidget->AddToViewport(0);

	// 初始化 HUD（绑定到玩家角色）
	APawn* PlayerPawn = PC->GetPawn();
	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
	{
		PlayerHUDWidget->InitializeHUD(PlayerCharacter);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ABlackMythHUD: Player pawn is not a Character, HUD not initialized."));
	}

	UE_LOG(LogTemp, Log, TEXT("ABlackMythHUD: PlayerHUD created and added to viewport."));
}
