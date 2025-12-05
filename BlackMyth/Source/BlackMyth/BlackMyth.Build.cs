// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlackMyth : ModuleRules
{
	public BlackMyth(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG",           // UserWidget, ProgressBar, TextBlock 等 UI 控件
			"Slate",         // Slate 基础框架
			"SlateCore",      // Slate 核心类型
			"AIModule", 	// AI 核心模块，包含AAIController、UPawnSensingComponent、BehaviorTree
			"NavigationSystem", // 寻路系统，负责管理NavMesh，即按下P键显示的绿色区域
			"GameplayTasks"		// 游戏任务模块，用于处理异步任务
		});
	}
}
