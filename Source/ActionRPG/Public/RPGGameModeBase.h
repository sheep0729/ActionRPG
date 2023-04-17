// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameModeBase.generated.h"

/**
 * 本项目的 GameMode 比较简单，因为大部分与地图相关的逻辑都放在蓝图中了
 * 更复杂的项目可能需要在 C++ 中有多个 GameMode
 */
/** Base class for GameMode, should be blueprinted */
UCLASS()
class ACTIONRPG_API ARPGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ARPGGameModeBase();

	/**
	 * 这个函数在本项目的正常情况下不会被调用，只是为了精简开发流程（比如自动测试）
	 */
	/** Overriding this function is not essential to this sample since this function 
	 *	is not being called in normal circumstances. Added just to streamline 
	 *	dev-time activities (like automated testing). The default ResetLevel 
	 *	implementation doesn't work all that well with how things are set up in 
	 *	ActionRPG (and that's ok, this is exactly why we override functions!). 
	 */
	virtual void ResetLevel() override;

	/** 返回游戏是否结束 */
	/** Returns true if GameOver() has been called, false otherwise */
	virtual bool HasMatchEnded() const override;

	/**
	 * 在游戏结束时调用
	 */
	/**
	 * Called when the game is over i.e. the player dies, the time runs out or the 
	 * game is finished
	 */
	UFUNCTION(BlueprintCallable, Category=Game)
	virtual void GameOver();

protected:
	/**
	 * BlueprintImplementableEvent 和 BlueprintNativeEvent 都是可以在蓝图中定义的函数，区别是 BlueprintImplementableEvent 没有
	 * Native 的实现，而 BlueprintNativeEvent 有 Native 的实现，但蓝图中可以重写，这种方式开销更大。
	 * 更普遍的命名方式是在 Native 的声明中加上 Receive 前缀（而不是 K2_）。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Game, meta=(DisplayName="DoRestart", ScriptName="DoRestart"))
	void K2_DoRestart();

	UFUNCTION(BlueprintImplementableEvent, Category=Game, meta=(DisplayName="OnGameOver", ScriptName="OnGameOver"))
	void K2_OnGameOver();

	UPROPERTY(BlueprintReadOnly, Category=Game)
	uint32 bGameOver : 1;
};

