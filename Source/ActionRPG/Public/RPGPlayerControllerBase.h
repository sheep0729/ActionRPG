// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/PlayerController.h"
#include "RPGInventoryInterface.h"
#include "RPGPlayerControllerBase.generated.h"

/** 几乎所有游戏都需要继承 PlayerController ，本项目中主要处理 inventory */
/** Base class for PlayerController, should be blueprinted */
UCLASS()
class ACTIONRPG_API ARPGPlayerControllerBase : public APlayerController, public IRPGInventoryInterface
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	ARPGPlayerControllerBase() {}
	virtual void BeginPlay() override;

	/** 从 item 定义到 item 数据的映射，保存 player 拥有的全部 item */
	/** Map of all items owned by this player, from definition to data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<URPGItem*, FRPGItemData> InventoryData;

	/**
	 * 从类型/数量到 Item 定义的映射，保存装备的道具
	 * 在 ARPGPlayerControllerBase::LoadInventory() 由 GameInstance::ItemSlotsPerType 初始化
	 */
	/** Map of slot, from type/num to item, initialized from ItemSlotsPerType on RPGGameInstanceBase */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FRPGItemSlot, URPGItem*> SlottedItems;

	/** 当背包中的道具改变时的 BP 委托，是蓝图中的 Event Dispatcher */
	/** Delegate called when an inventory item has been added or removed */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnInventoryItemChanged OnInventoryItemChanged;

	/** 当背包中的道具改变时的 Native 委托，本项目中没有绑定回调 */
	/** Native version above, called before BP delegate */
	FOnInventoryItemChangedNative OnInventoryItemChangedNative;

	/** 当装备的道具改变时的 BP 委托，是蓝图中的 Event Dispatcher ，在蓝图中没有用到 */
	/** Delegate called when an inventory slot has changed */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnSlottedItemChanged OnSlottedItemChanged;

	/** 当装备的道具改变时的 Native 委托，本项目中没有绑定回调 */
	/** Native version above, called before BP delegate */
	FOnSlottedItemChangedNative OnSlottedItemChangedNative;

	/** 当背包加载时的 BP 委托，是蓝图中的 Event Dispatcher ，在蓝图中没有用到 */
	/** Delegate called when the inventory has been loaded/reloaded */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnInventoryLoaded OnInventoryLoaded;

	/** 当背包加载时的 Native 委托，本项目中没有绑定回调 */
	/** Native version above, called before BP delegate */
	FOnInventoryLoadedNative OnInventoryLoadedNative;

	/** 在通知了所有的委托之后调用，在蓝图的实现中负责处理 UI 的逻辑，可能是为了先保证更新数据再更新 UI */
	/** Called after the inventory was changed and we notified all delegates */
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void InventoryItemChanged(bool bAdded, URPGItem* Item);

	/** 在通知了所有的委托之后调用 */
	/** Called after an item was equipped and we notified all delegates */
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void SlottedItemChanged(FRPGItemSlot ItemSlot, URPGItem* Item);

	/** 向背包中添加一个新道具，默认自动装备 */
	/** Adds a new inventory item, will add it to an empty slot if possible. If the item supports count you can add more than one count. It will also update the level when adding if required */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddInventoryItem(URPGItem* NewItem, int32 ItemCount = 1, int32 ItemLevel = 1, bool bAutoSlot = true);

	/** 移除背包中的一个物品，同时也会移除装备的道具 */
	/** Remove an inventory item, will also remove from slots. A remove count of <= 0 means to remove all copies */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveInventoryItem(URPGItem* RemovedItem, int32 RemoveCount = 1);

	/** 从 InventoryData 中查找所有 ItemType 的 URPGItem 并放到 Items 中，如果 ItemType 无效则会返回全部 */
	/** Returns all inventory items of a given type. If none is passed as type it will return all */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void GetInventoryItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType);

	/** 从 InventoryData 中查找 Item 的数量 */
	/** Returns number of instances of this item found in the inventory. This uses count from GetItemData */
	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInventoryItemCount(const URPGItem* Item) const;

	/** 根据 URPGItem 查找 FRPGItemData ，如果没找到返回 false */
	/** Returns the item data associated with an item. Returns false if none found */
	UFUNCTION(BlueprintPure, Category = Inventory)
	bool GetInventoryItemData(const URPGItem* Item, FRPGItemData& ItemData) const;

	/** 把 Item 放到 ItemSlot 中，会移除其他 ItemSlot 中的此 Item */
	/** Sets slot to item, will remove from other slots if necessary. If passing null this will empty the slot */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SetSlottedItem(const FRPGItemSlot& ItemSlot, URPGItem* Item);

	/** 从 SlottedItems 的指定 ItemSlot 中获得 Item */
	/** Returns item in slot, or null if empty */
	UFUNCTION(BlueprintPure, Category = Inventory)
	URPGItem* GetSlottedItem(const FRPGItemSlot& ItemSlot) const;

	/** 从 SlottedItems 获得全部具有指定 ItemType 的 Item ，如果 ItemType 无效则会返回全部，bOutputEmptyIndexes 没有用到 */
	/** Returns all slotted items of a given type. If none is passed as type it will return all */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void GetSlottedItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType, bool bOutputEmptyIndexes);

	/** 把背包中的道具尽可能装备上 */
	/** Fills in any empty slots with items in inventory */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void FillEmptySlots();

	/** 更新存档，会写入硬盘 */
	/** Manually save the inventory, this is called from add/remove functions automatically */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SaveInventory();

	/** 从存档中加载背包 */
	/** Loads inventory from save game on game instance, this will replace arrays */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool LoadInventory();

	// Implement IRPGInventoryInterface
	virtual const TMap<URPGItem*, FRPGItemData>& GetInventoryDataMap() const override
	{
		return InventoryData;
	}
	virtual const TMap<FRPGItemSlot, URPGItem*>& GetSlottedItemMap() const override
	{
		return SlottedItems;
	}
	virtual FOnInventoryItemChangedNative& GetInventoryItemChangedDelegate() override
	{
		return OnInventoryItemChangedNative;
	}
	virtual FOnSlottedItemChangedNative& GetSlottedItemChangedDelegate() override
	{
		return OnSlottedItemChangedNative;
	}
	virtual FOnInventoryLoadedNative& GetInventoryLoadedDelegate() override
	{
		return OnInventoryLoadedNative;
	}

protected:
	/** 自动装备一个道具，如果发生了改变则返回 true */
	/** Auto slots a specific item, returns true if anything changed */
	bool FillEmptySlotWithItem(URPGItem* NewItem);

	/** 背包更新后调用的回调 */
	/** Calls the inventory update callbacks */
	void NotifyInventoryItemChanged(bool bAdded, URPGItem* Item);
	void NotifySlottedItemChanged(FRPGItemSlot ItemSlot, URPGItem* Item);
	void NotifyInventoryLoaded() const;

	/** 加载存档后根据存档加载背包 */
	/** Called when a global save game as been loaded */
	void HandleSaveGameLoaded(URPGSaveGame* NewSaveGame);
};
