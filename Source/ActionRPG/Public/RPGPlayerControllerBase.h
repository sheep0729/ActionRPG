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

	/** 当背包中的道具改变时的 BP 委托 */
	/** Delegate called when an inventory item has been added or removed */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnInventoryItemChanged OnInventoryItemChanged;

	/** 当背包中的道具改变时的 Native 委托 */
	/** Native version above, called before BP delegate */
	FOnInventoryItemChangedNative OnInventoryItemChangedNative;

	/** 当装备的道具改变时的 BP 委托 */
	/** Delegate called when an inventory slot has changed */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnSlottedItemChanged OnSlottedItemChanged;

	/** 当装备的道具改变时的 Native 委托 */
	/** Native version above, called before BP delegate */
	FOnSlottedItemChangedNative OnSlottedItemChangedNative;

	/** 当背包加载时的 BP 委托 */
	/** Delegate called when the inventory has been loaded/reloaded */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnInventoryLoaded OnInventoryLoaded;

	/** 当背包加载时的 Native 委托 */
	/** Native version above, called before BP delegate */
	FOnInventoryLoadedNative OnInventoryLoadedNative;
	
	/** Called after the inventory was changed and we notified all delegates */
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void InventoryItemChanged(bool bAdded, URPGItem* Item);

	/** Called after an item was equipped and we notified all delegates */
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void SlottedItemChanged(FRPGItemSlot ItemSlot, URPGItem* Item);

	/** 向背包中添加一个新道具 */
	/** Adds a new inventory item, will add it to an empty slot if possible. If the item supports count you can add more than one count. It will also update the level when adding if required */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddInventoryItem(URPGItem* NewItem, int32 ItemCount = 1, int32 ItemLevel = 1, bool bAutoSlot = true);

	/** Remove an inventory item, will also remove from slots. A remove count of <= 0 means to remove all copies */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveInventoryItem(URPGItem* RemovedItem, int32 RemoveCount = 1);

	/** Returns all inventory items of a given type. If none is passed as type it will return all */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void GetInventoryItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType);

	/** Returns number of instances of this item found in the inventory. This uses count from GetItemData */
	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInventoryItemCount(URPGItem* Item) const;

	/** Returns the item data associated with an item. Returns false if none found */
	UFUNCTION(BlueprintPure, Category = Inventory)
	bool GetInventoryItemData(URPGItem* Item, FRPGItemData& ItemData) const;

	/** Sets slot to item, will remove from other slots if necessary. If passing null this will empty the slot */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SetSlottedItem(FRPGItemSlot ItemSlot, URPGItem* Item);

	/** Returns item in slot, or null if empty */
	UFUNCTION(BlueprintPure, Category = Inventory)
	URPGItem* GetSlottedItem(const FRPGItemSlot& ItemSlot) const;

	/** Returns all slotted items of a given type. If none is passed as type it will return all */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void GetSlottedItems(TArray<URPGItem*>& Items, FPrimaryAssetType ItemType, bool bOutputEmptyIndexes);

	/** Fills in any empty slots with items in inventory */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void FillEmptySlots();

	/** Manually save the inventory, this is called from add/remove functions automatically */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool SaveInventory();

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
	/** Auto slots a specific item, returns true if anything changed */
	bool FillEmptySlotWithItem(URPGItem* NewItem);

	/** Calls the inventory update callbacks */
	void NotifyInventoryItemChanged(bool bAdded, URPGItem* Item);
	void NotifySlottedItemChanged(FRPGItemSlot ItemSlot, URPGItem* Item);
	void NotifyInventoryLoaded();

	/** Called when a global save game as been loaded */
	void HandleSaveGameLoaded(URPGSaveGame* NewSaveGame);
};
