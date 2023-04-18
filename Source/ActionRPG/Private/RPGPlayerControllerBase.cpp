// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGPlayerControllerBase.h"
#include "RPGCharacterBase.h"
#include "RPGGameInstanceBase.h"
#include "RPGSaveGame.h"
#include "Items/RPGItem.h"

bool ARPGPlayerControllerBase::AddInventoryItem(URPGItem* NewItem, int32 ItemCount, int32 ItemLevel, bool bAutoSlot)
{
	bool bChanged = false;
	if (!NewItem)
	{
		UE_LOG(LogActionRPG, Warning, TEXT("AddInventoryItem: Failed trying to add null item!"));
		return false;
	}

	if (ItemCount <= 0 || ItemLevel <= 0)
	{
		UE_LOG(LogActionRPG, Warning, TEXT("AddInventoryItem: Failed trying to add item %s with negative count or level!"), *NewItem->GetName());
		return false;
	}

	// 获得原来的道具数据
	// Find current item data, which may be empty
	FRPGItemData OldData;
	GetInventoryItemData(NewItem, OldData);

	// 从 OldData 更新到 NewData
	// Find modified data
	FRPGItemData NewData = OldData;
	NewData.UpdateItemData(FRPGItemData(ItemCount, ItemLevel), NewItem->MaxCount, NewItem->MaxLevel);

	// 判断是否真的更新了
	if (OldData != NewData)
	{
		// If data changed, need to update storage and call callback
		InventoryData.Add(NewItem, NewData);
		NotifyInventoryItemChanged(true, NewItem);
		bChanged = true;
	}

	// 是否需要装备
	if (bAutoSlot)
	{
		// Slot item if required
		bChanged |= FillEmptySlotWithItem(NewItem);
	}

	if (bChanged)
	{
		// If anything changed, write to save game
		SaveInventory();
		return true;
	}
	return false;
}

bool ARPGPlayerControllerBase::RemoveInventoryItem(URPGItem* RemovedItem, int32 RemoveCount)
{
	if (!RemovedItem)
	{
		UE_LOG(LogActionRPG, Warning, TEXT("RemoveInventoryItem: Failed trying to remove null item!"));
		return false;
	}

	// Find current item data, which may be empty
	FRPGItemData NewData;
	GetInventoryItemData(RemovedItem, NewData);

	if (!NewData.IsValid())
	{
		// Wasn't found
		return false;
	}

	// If RemoveCount <= 0, delete all
	if (RemoveCount <= 0)
	{
		NewData.ItemCount = 0;
	}
	else
	{
		NewData.ItemCount -= RemoveCount;
	}

	if (NewData.ItemCount > 0)
	{
		// Update data with new count
		InventoryData.Add(RemovedItem, NewData);
	}
	else
	{
		// 把道具从背包中移除
		// Remove item entirely, make sure it is unslotted
		InventoryData.Remove(RemovedItem);

		// 把道具从装备的插槽中移除
		for (TPair<FRPGItemSlot, URPGItem*>& Pair : SlottedItems)
		{
			if (Pair.Value == RemovedItem)
			{
				Pair.Value = nullptr;
				NotifySlottedItemChanged(Pair.Key, Pair.Value);
			}
		}
	}

	// If we got this far, there is a change so notify and save
	NotifyInventoryItemChanged(false, RemovedItem);

	SaveInventory();
	return true;
}

void ARPGPlayerControllerBase::GetInventoryItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType)
{
	for (const TPair<URPGItem*, FRPGItemData>& Pair : InventoryData)
	{
		if (Pair.Key)
		{
			FPrimaryAssetId AssetId = Pair.Key->GetPrimaryAssetId();

			// Filters based on item type
			if (AssetId.PrimaryAssetType == ItemType || !ItemType.IsValid())
			{
				Items.Add(Pair.Key);
			}
		}	
	}
}

bool ARPGPlayerControllerBase::SetSlottedItem(const FRPGItemSlot& ItemSlot, URPGItem* Item)
{
	// Iterate entire inventory because we need to remove from old slot
	bool bFound = false;
	for (TPair<FRPGItemSlot, URPGItem*>& Pair : SlottedItems)
	{
		// 找到了需要放入道具的插槽，需要更新
		if (Pair.Key == ItemSlot)
		{
			// Add to new slot
			bFound = true;
			Pair.Value = Item;
			NotifySlottedItemChanged(Pair.Key, Pair.Value);
		}
		// 这个道具之前放在了其他的插槽中，需要移除
		else if (Item != nullptr && Pair.Value == Item)
		{
			// If this item was found in another slot, remove it
			Pair.Value = nullptr;
			NotifySlottedItemChanged(Pair.Key, Pair.Value);
		}
	}

	if (bFound)
	{
		SaveInventory();
		return true;
	}

	return false;
}

int32 ARPGPlayerControllerBase::GetInventoryItemCount(const URPGItem* Item) const
{
	const FRPGItemData* FoundItem = InventoryData.Find(Item);

	if (FoundItem)
	{
		return FoundItem->ItemCount;
	}
	return 0;
}

bool ARPGPlayerControllerBase::GetInventoryItemData(const URPGItem* Item, FRPGItemData& ItemData) const
{
	const FRPGItemData* FoundItem = InventoryData.Find(Item);

	if (FoundItem)
	{
		ItemData = *FoundItem;
		return true;
	}
	ItemData = FRPGItemData(0, 0);
	return false;
}

URPGItem* ARPGPlayerControllerBase::GetSlottedItem(const FRPGItemSlot& ItemSlot) const
{
	URPGItem* const* FoundItem = SlottedItems.Find(ItemSlot);

	if (FoundItem)
	{
		return *FoundItem;
	}
	return nullptr;
}

void ARPGPlayerControllerBase::GetSlottedItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType, bool bOutputEmptyIndexes)
{
	for (TPair<FRPGItemSlot, URPGItem*>& Pair : SlottedItems)
	{
		if (Pair.Key.ItemType == ItemType || !ItemType.IsValid())
		{
			Items.Add(Pair.Value);
		}
	}
}

void ARPGPlayerControllerBase::FillEmptySlots()
{
	bool bShouldSave = false;
	for (const TPair<URPGItem*, FRPGItemData>& Pair : InventoryData)
	{
		bShouldSave |= FillEmptySlotWithItem(Pair.Key);
	}

	if (bShouldSave)
	{
		SaveInventory();
	}
}

bool ARPGPlayerControllerBase::SaveInventory()
{
	// 获得 World 和 GameInstance
    const UWorld* World = GetWorld();
	URPGGameInstanceBase* GameInstance = World ? World->GetGameInstance<URPGGameInstanceBase>() : nullptr;

	if (!GameInstance)
	{
		return false;
	}

	URPGSaveGame* CurrentSaveGame = GameInstance->GetCurrentSaveGame();
	if (CurrentSaveGame)
	{
		// 清空 CurrentSaveGame 中缓存的数据
		// Reset cached data in save game before writing to it
		CurrentSaveGame->InventoryData.Reset();
		CurrentSaveGame->SlottedItems.Reset();

		for (const TPair<URPGItem*, FRPGItemData>& ItemPair : InventoryData)
		{
			if (ItemPair.Key)
			{
				FPrimaryAssetId AssetId;
				AssetId = ItemPair.Key->GetPrimaryAssetId();

				// 在 SaveGame 中的 URPGItem 由其 AssetId 代替
				CurrentSaveGame->InventoryData.Add(AssetId, ItemPair.Value);
			}
		}

		for (const TPair<FRPGItemSlot, URPGItem*>& SlotPair : SlottedItems)
		{
			FPrimaryAssetId AssetId;

			if (SlotPair.Value)
			{
				AssetId = SlotPair.Value->GetPrimaryAssetId();
			}

			// TODO 这行为什么不在 if 里？
			// 在 SaveGame 中的 URPGItem 由其 AssetId 代替
			CurrentSaveGame->SlottedItems.Add(SlotPair.Key, AssetId);
		}

		// 写入硬盘
		// Now that cache is updated, write to disk
		GameInstance->WriteSaveGame();
		return true;
	}
	return false;
}

bool ARPGPlayerControllerBase::LoadInventory()
{
	InventoryData.Reset();
	SlottedItems.Reset();

	// Fill in slots from game instance
	const UWorld* World = GetWorld();
	URPGGameInstanceBase* GameInstance = World ? World->GetGameInstance<URPGGameInstanceBase>() : nullptr;

	if (!GameInstance)
	{
		return false;
	}

	// 绑定回调
	// Bind to loaded callback if not already bound
	if (!GameInstance->OnSaveGameLoadedNative.IsBoundToObject(this))
	{
		GameInstance->OnSaveGameLoadedNative.AddUObject(this, &ARPGPlayerControllerBase::HandleSaveGameLoaded);
	}

	// 初始化 SlottedItems
	for (const TPair<FPrimaryAssetType, int32>& Pair : GameInstance->ItemSlotsPerType)
	{
		for (int32 SlotNumber = 0; SlotNumber < Pair.Value; SlotNumber++)
		{
			SlottedItems.Add(FRPGItemSlot(Pair.Key, SlotNumber), nullptr);
		}
	}

	URPGSaveGame* CurrentSaveGame = GameInstance->GetCurrentSaveGame();
	const URPGAssetManager& AssetManager = URPGAssetManager::Get();
	if (CurrentSaveGame)
	{
		// 从当前的存档中加载 InventoryData
		// Copy from save game into controller data
		bool bFoundAnySlots = false;
		for (const TPair<FPrimaryAssetId, FRPGItemData>& ItemPair : CurrentSaveGame->InventoryData)
		{
			// 根据 FPrimaryAssetId 加载 URPGItem
			URPGItem* LoadedItem = AssetManager.ForceLoadItem(ItemPair.Key);

			if (LoadedItem != nullptr)
			{
				InventoryData.Add(LoadedItem, ItemPair.Value);
			}
		}

		// 从当前的存档中加载 SlottedItems
		for (const TPair<FRPGItemSlot, FPrimaryAssetId>& SlotPair : CurrentSaveGame->SlottedItems)
		{
			if (SlotPair.Value.IsValid())
			{
				// 根据 FPrimaryAssetId 加载 URPGItem
				URPGItem* LoadedItem = AssetManager.ForceLoadItem(SlotPair.Value);
				
				if (GameInstance->IsValidItemSlot(SlotPair.Key) && LoadedItem)
				{
					SlottedItems.Add(SlotPair.Key, LoadedItem);
					bFoundAnySlots = true;
				}
			}
		}

		// 这里的逻辑是装备插槽应该尽可能装备道具
		if (!bFoundAnySlots)
		{
			// Auto slot items as no slots were saved
			FillEmptySlots();
		}

		NotifyInventoryLoaded();

		return true;
	}

	// 加载存档失败了，但是已经清空了 InventoryData 和 SlottedItems ，所以需要通知 UI
	// Load failed but we reset inventory, so need to notify UI
	NotifyInventoryLoaded();

	return false;
}

bool ARPGPlayerControllerBase::FillEmptySlotWithItem(URPGItem* NewItem)
{
	// Look for an empty item slot to fill with this item
	const FPrimaryAssetType NewItemType = NewItem->GetPrimaryAssetId().PrimaryAssetType;
	FRPGItemSlot EmptySlot;
	for (TPair<FRPGItemSlot, URPGItem*>& Pair : SlottedItems)
	{
		if (Pair.Key.ItemType == NewItemType)
		{
			if (Pair.Value == NewItem)
			{
				// Item is already slotted
				return false;
			}

			// 找到空的且 SlotNumber 尽量小的 slot
			// 如果 Pair.Key 无效，逻辑会有问题（存在可装备的 slot 但最后没有装备） ，所以这里应该是认为 SlottedItems 中的 Key 都是 Valid 的
			// （这很合理因为 SlottedItems 只会在 ARPGPlayerControllerBase::LoadInventory() 中 Add ，且 Key 保证有效）
			// 那么 EmptySlot.IsValid() 就只会在 EmptySlot 从未被赋值时返回 false （因为默认构造的 SlotNumber = -1）
			if (Pair.Value == nullptr && (!EmptySlot.IsValid() || EmptySlot.SlotNumber > Pair.Key.SlotNumber))
			{
				// We found an empty slot worth filling
				EmptySlot = Pair.Key;
			}
		}
	}

	if (EmptySlot.IsValid())
	{
		// 这里实际修改 SlottedItems
		SlottedItems[EmptySlot] = NewItem;
		NotifySlottedItemChanged(EmptySlot, NewItem);
		return true;
	}

	return false;
}

void ARPGPlayerControllerBase::NotifyInventoryItemChanged(bool bAdded, URPGItem* Item)
{
	// Notify native before blueprint
	OnInventoryItemChangedNative.Broadcast(bAdded, Item);
	OnInventoryItemChanged.Broadcast(bAdded, Item);
	
	// Call BP update event
	InventoryItemChanged(bAdded, Item);
}

void ARPGPlayerControllerBase::NotifySlottedItemChanged(FRPGItemSlot ItemSlot, URPGItem* Item)
{
	// Notify native before blueprint
	OnSlottedItemChangedNative.Broadcast(ItemSlot, Item);
	OnSlottedItemChanged.Broadcast(ItemSlot, Item);

	// Call BP update event
	SlottedItemChanged(ItemSlot, Item);
}

void ARPGPlayerControllerBase::NotifyInventoryLoaded() const
{
	// Notify native before blueprint
	OnInventoryLoadedNative.Broadcast();
	OnInventoryLoaded.Broadcast();
	
}

void ARPGPlayerControllerBase::HandleSaveGameLoaded(URPGSaveGame* NewSaveGame)
{
	LoadInventory();
}

void ARPGPlayerControllerBase::BeginPlay()
{
	// 虽然 GameInstance 的 Init 函数一定在所有的 BeginPlay 之前执行（https://docs.unrealengine.com/4.26/en-US/InteractiveExperiences/Framework/GameFlow/）
	// 但是加载存档使用了异步的方法，所以在这里调用 LoadInventory() 不一定能成功加载背包，但一定能够绑定回调 HandleSaveGameLoaded(URPGSaveGame* NewSaveGame)
	// 这个回调会再次调用 LoadInventory() 。
	// Load inventory off save game before starting play
	LoadInventory();

	Super::BeginPlay();
}