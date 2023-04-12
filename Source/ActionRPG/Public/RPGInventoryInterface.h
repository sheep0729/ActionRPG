// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "RPGInventoryInterface.generated.h"

/**
 * 让 RPGCharacterBase 可以不需要 Cast 就能使用 RPGPlayerControllerBase 中 inventory 的相关功能
 * 只能在 C++ 中使用
 * 获得背包中的数据和相关代理
 */

/**
 * UINTERFACE 不是真正的 interface ，它是一个空的 class ，它的存在只是为了让 UE 的反射系统可见。
 *
 * Interface for actors that provide a set of RPGItems bound to ItemSlots
 * This exists so RPGCharacterBase can query inventory without doing hacky player controller casts
 * It is designed only for use by native classes
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class URPGInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/** 这个 I 开头的是真正需要被其他类继承的 interface */
class ACTIONRPG_API IRPGInventoryInterface
{
	GENERATED_BODY()

public:
	/** Returns the map of items to data */
	virtual const TMap<URPGItem*, FRPGItemData>& GetInventoryDataMap() const = 0;

	/** Returns the map of slots to items */
	virtual const TMap<FRPGItemSlot, URPGItem*>& GetSlottedItemMap() const = 0;

	/** Gets the delegate for inventory item changes */
	virtual FOnInventoryItemChangedNative& GetInventoryItemChangedDelegate() = 0;

	/** Gets the delegate for inventory slot changes */
	virtual FOnSlottedItemChangedNative& GetSlottedItemChangedDelegate() = 0;

	/** Gets the delegate for when the inventory loads */
	virtual FOnInventoryLoadedNative& GetInventoryLoadedDelegate() = 0;
};

