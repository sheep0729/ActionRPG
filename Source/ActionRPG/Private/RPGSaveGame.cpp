// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGSaveGame.h"
#include "RPGGameInstanceBase.h"

void URPGSaveGame::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && SavedDataVersion != ERPGSaveGameVersion::LatestVersion)
	{
		// 在这里处理之间版本的游戏数据
		if (SavedDataVersion < ERPGSaveGameVersion::AddedItemData)
		{
			// Convert from list to item data map
			for (const FPrimaryAssetId& ItemId : InventoryItems_DEPRECATED)
			{
				InventoryData.Add(ItemId, FRPGItemData(1, 1));
			}

			InventoryItems_DEPRECATED.Empty();
		}
		
		SavedDataVersion = ERPGSaveGameVersion::LatestVersion;
	}
}