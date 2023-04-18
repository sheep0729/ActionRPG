// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Items/RPGItem.h"
#include "GameFramework/SaveGame.h"
#include "RPGSaveGame.generated.h"

/** 保存游戏，用于在磁盘上存储物品栏 / 经验值信息 */

/** 游戏版本 */
/** List of versions, native code will handle fix-ups for any old versions */
namespace ERPGSaveGameVersion
{
	enum type
	{
		// Initial version
		Initial,
		// Added Inventory
		AddedInventory,
		// Added ItemData to store count/level
		AddedItemData,

		// -----<new versions must be added before this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
}

/** 用来保存游戏的对象，有版本信息 */
/** Object that is written to and read from the save game archive, with a data version */
UCLASS(BlueprintType)
class ACTIONRPG_API URPGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGSaveGame()
	{
		// Set to current version, this will get overwritten during serialization when loading
		SavedDataVersion = ERPGSaveGameVersion::LatestVersion;
	}

	/** 将 FPrimaryAssetId 作为道具的 Id ，并映射到背包中道具的数据 */
	/** Map of items to item data */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SaveGame)
	TMap<FPrimaryAssetId, FRPGItemData> InventoryData;

	/** 背包中道具的 slot 和道具 Id 的对应关系 */
	/** Map of slotted items */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SaveGame)
	TMap<FRPGItemSlot, FPrimaryAssetId> SlottedItems;

	/** 标识用户的 Id ，本项目中没用到 */
	/** User's unique id */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SaveGame)
	FString UserId;

protected:
	/** 这是一个示例，代表旧版本的游戏需要保存的数据，但现在已经不需要了 */
	/** Deprecated way of storing items, this is read in but not saved out */
	UPROPERTY()
	TArray<FPrimaryAssetId> InventoryItems_DEPRECATED;

	/** 上次保存的版本 */
	/** What LatestVersion was when the archive was saved */
	UPROPERTY()
	int32 SavedDataVersion;

	/** 重写这个函数来解决旧版本保存的游戏数据 */
	/** Overridden to allow version fix-ups */
	virtual void Serialize(FArchive& Ar) override;
};
