// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// ----------------------------------------------------------------------------------------------------------------
// This header is included by all headers in the project so it's a good place to declare common includes
// We include EngineMinimal and the subset of engine headers used by most of our classes
// We don't want to include "Engine.h" as that pulls in too many classes we don't need and slows compile time
// ----------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------
// ģ���ͷ�ļ���������������ͷ�ļ���һ������������ RPGTypes.h ����������Ϊ��������Ӧ���ǿ�ģ��ģ���
// - include ģ���й����ͷ�ļ�
// - ����ģ���й���ķ���
// ----------------------------------------------------------------------------------------------------------------

#include "EngineMinimal.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "RPGTypes.h"

ACTIONRPG_API DECLARE_LOG_CATEGORY_EXTERN(LogActionRPG, Log, All);

DECLARE_STATS_GROUP(TEXT("ARPGCharacterBase"), STATGROUP_ARPGCharacterBase, STATCAT_Custom);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("ARPGCharacterBase::HandleHealthChanged"), STAT_HandleHealthChanged, STATGROUP_ARPGCharacterBase, ACTIONRPG_API);