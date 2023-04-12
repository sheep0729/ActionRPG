// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionRPG.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, ActionRPG, "UES_ActionRPG" );

/** Logging definitions */
DEFINE_LOG_CATEGORY(LogActionRPG);

// 在 ActionRPG.h 中声明的 STAT 需要在源文件中定义
DEFINE_STAT(STAT_HandleHealthChanged);