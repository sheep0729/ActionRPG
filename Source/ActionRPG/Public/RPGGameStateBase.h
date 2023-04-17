// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/GameStateBase.h"
#include "RPGGameStateBase.generated.h"

/** 本项目的 GameState 比较简单，因为大部分与地图相关的逻辑都放在蓝图中了 */
/** 更复杂的项目可能需要在 C++ 中有多个 GameState */
/** Base class for GameState, should be blueprinted */
UCLASS()
class ACTIONRPG_API ARPGGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ARPGGameStateBase() {}
};

