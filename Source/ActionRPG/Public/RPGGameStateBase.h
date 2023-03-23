// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/GameStateBase.h"
#include "RPGGameStateBase.generated.h"

/** ����Ŀ�� GameState �Ƚϼ򵥣���Ϊ�󲿷����ͼ��ص��߼���������ͼ���� */
/** �����ӵ���Ŀ������Ҫ�� C++ ���ж�� GameState */
/** Base class for GameState, should be blueprinted */
UCLASS()
class ACTIONRPG_API ARPGGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Constructor */
	ARPGGameStateBase() {}
};

