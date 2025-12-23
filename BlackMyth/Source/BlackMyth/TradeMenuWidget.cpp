#include "TradeMenuWidget.h"

void UTradeMenuWidget::OnBackClicked()
{
    // 关闭交易菜单
    RemoveFromParent();

    // 如果有父级菜单，则重新显示土地庙主菜单
    if (OwnerTempleWidget)
    {
        OwnerTempleWidget->AddToViewport();
    }
}