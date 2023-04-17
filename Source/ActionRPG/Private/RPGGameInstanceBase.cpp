// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGGameInstanceBase.h"
#include "RPGAssetManager.h"
#include "RPGSaveGame.h"
#include "Items/RPGItem.h"
#include "Kismet/GameplayStatics.h"

URPGGameInstanceBase::URPGGameInstanceBase()
	: SaveSlot(TEXT("SaveGame"))
	, SaveUserIndex(0)
{}

void URPGGameInstanceBase::AddDefaultInventory(URPGSaveGame* SaveGame, bool bRemoveExtra)
{
	// 相当于把 inventory 重置为 default inventory
	// If we want to remove extra, clear out the existing inventory
	if (bRemoveExtra)
	{
		SaveGame->InventoryData.Reset();
	}

	// 如果不设置 bRemoveExtra ，相当于把 default inventory 补充到 inventory 中
	// Now add the default inventory, this only adds if not already in the inventory
	for (const TPair<FPrimaryAssetId, FRPGItemData>& Pair : DefaultInventory)
	{
		if (!SaveGame->InventoryData.Contains(Pair.Key))
		{
			SaveGame->InventoryData.Add(Pair.Key, Pair.Value);
		}
	}
}

bool URPGGameInstanceBase::IsValidItemSlot(FRPGItemSlot ItemSlot) const
{
	if (ItemSlot.IsValid())
	{
		const int32* FoundCount = ItemSlotsPerType.Find(ItemSlot.ItemType);

		if (FoundCount)
		{
			// TODO 这里为什么是小于？
			return ItemSlot.SlotNumber < *FoundCount;
		}
	}
	return false;
}

URPGSaveGame* URPGGameInstanceBase::GetCurrentSaveGame()
{
	return CurrentSaveGame;
}

void URPGGameInstanceBase::SetSavingEnabled(bool bEnabled)
{
	bSavingEnabled = bEnabled;
}

bool URPGGameInstanceBase::LoadOrCreateSaveGame()
{
	URPGSaveGame* LoadedSave = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SaveSlot, SaveUserIndex) && bSavingEnabled)
	{
		LoadedSave = Cast<URPGSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, SaveUserIndex));
	}

	return HandleSaveGameLoaded(LoadedSave);
}

bool URPGGameInstanceBase::HandleSaveGameLoaded(USaveGame* SaveGameObject)
{
	bool bLoaded = false;

	// 如果没有启用保存到硬盘，就创建新的存档
	if (!bSavingEnabled)
	{
		// If saving is disabled, ignore passed in object
		SaveGameObject = nullptr;
	}

	// 替换当前的存档
	// Replace current save, old object will GC out
	CurrentSaveGame = Cast<URPGSaveGame>(SaveGameObject);

	if (CurrentSaveGame)
	{
		// 保证存档中包含 default inventory
		// Make sure it has any newly added default inventory
		AddDefaultInventory(CurrentSaveGame, false);
		bLoaded = true;
	}
	else
	{
		// 没有加载到存档，创建了一个新的
		// This creates it on demand
		CurrentSaveGame = Cast<URPGSaveGame>(UGameplayStatics::CreateSaveGameObject(URPGSaveGame::StaticClass()));

		AddDefaultInventory(CurrentSaveGame, true);
	}

	OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	OnSaveGameLoadedNative.Broadcast(CurrentSaveGame);

	return bLoaded;
}

void URPGGameInstanceBase::GetSaveSlotInfo(FString& SlotName, int32& UserIndex) const
{
	SlotName = SaveSlot;
	UserIndex = SaveUserIndex;
}

bool URPGGameInstanceBase::WriteSaveGame()
{
	if (bSavingEnabled) // 检查是否启用了保存
	{
		if (bCurrentlySaving) // 检查当前是否在保存
		{
			// 在当前保存操作结束后再处理，这里直接返回，用一个变量记录，相当于等待保存的队列长度为 1 。
			// Schedule another save to happen after current one finishes. We only queue one save
			bPendingSaveRequested = true;
			return true;
		}

		// 表明正在执行一个异步的保存操作
		// Indicate that we're currently doing an async save
		bCurrentlySaving = true;

		// 安排一个异步的保存操作，这里的 Slot 是一个字符串，相当于 Key 。
		// 这个函数也有蓝图的版本。
		// 在游戏线程上序列化，在平台相关的工作线程上进行实际的写入操作。
		// 完成时会在游戏线程上调用委托
		// 传入的委托会被拷贝到工作线程，所以要保证 Payload 在 copy by value 时是线程安全的（这里没有 Payload）。
		// This goes off in the background
		UGameplayStatics::AsyncSaveGameToSlot(GetCurrentSaveGame(), SaveSlot, SaveUserIndex, FAsyncSaveGameToSlotDelegate::CreateUObject(this, &URPGGameInstanceBase::HandleAsyncSave));
		return true;
	}
	return false;
}

void URPGGameInstanceBase::ResetSaveGame()
{
	// 给 HandleSaveGameLoaded 传入一个 nullptr ，它就会创建一个新的存档
	// Call handle function with no loaded save, this will reset the data
	HandleSaveGameLoaded(nullptr);
}

void URPGGameInstanceBase::HandleAsyncSave(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	// 调用到这个函数时 bCurrentlySaving 应该被设为 true 。
	ensure(bCurrentlySaving);
	bCurrentlySaving = false;

	// 在这里处理等待的保存操作
	if (bPendingSaveRequested)
	{
		// Start another save as we got a request while saving
		bPendingSaveRequested = false;
		WriteSaveGame();
	}
}
