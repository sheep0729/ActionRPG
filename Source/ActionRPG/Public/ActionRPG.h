// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// ----------------------------------------------------------------------------------------------------------------
// This header is included by all headers in the project so it's a good place to declare common includes
// We include EngineMinimal and the subset of engine headers used by most of our classes
// We don't want to include "Engine.h" as that pulls in too many classes we don't need and slows compile time
// ----------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------
// 模块的头文件，被所有其他的头文件第一个包含（除了 RPGTypes.h ，可能是因为概念上它应该是跨模块的）。
// - include 模块中共享的头文件
// - 声明模块中共享的符号
// ----------------------------------------------------------------------------------------------------------------

#include "EngineMinimal.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "RPGTypes.h"

ACTIONRPG_API DECLARE_LOG_CATEGORY_EXTERN(LogActionRPG, Log, All);

// DECLARE_STATS_GROUP(TEXT("ARPGCharacterBase"), STATGROUP_ARPGCharacterBase, STATCAT_Custom);
// DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("ARPGCharacterBase::HandleHealthChanged"), STAT_HandleHealthChanged, STATGROUP_ARPGCharacterBase, ACTIONRPG_API);