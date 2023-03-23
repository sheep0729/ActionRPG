// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * - �ڳ��μ��غ͵�ͼ����ʱ��ʾ�ļ��ؽ��棬��Ҫ����ģ�飨ActionRPG������֮ǰ��ʾ��������һ��������ģ�顣
 * - �ܶ���Ϸ������һ�������� Editor Module ����������ڱ༭����ʹ�õ� UI �͹��ߡ�
 */
/** Module interface for this game's loading screens */
class IActionRPGLoadingScreenModule : public IModuleInterface
{
public:
	/** Loads the module so it can be turned on */
	static inline IActionRPGLoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IActionRPGLoadingScreenModule>("ActionRPGLoadingScreen");
	}

	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen(bool bPlayUntilStopped, float PlayTime) = 0;

	/** Stops the loading screen */
	virtual void StopInGameLoadingScreen() = 0;
};
