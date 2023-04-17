// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Engine/GameInstance.h"
#include "RPGGameInstanceBase.generated.h"

class URPGItem;
class URPGSaveGame;

/**
 * - 一个游戏中只有一个，可用于存储全局游戏数据
 * - 如果定义了它的蓝图子类，需要在项目设置中修改
 */
/**
 * Base class for GameInstance, should be blueprinted
 * Most games will need to make a game-specific subclass of GameInstance
 * Once you make a blueprint subclass of your native subclass you will want to set it to be the default in project settings
 */
UCLASS()
class ACTIONRPG_API URPGGameInstanceBase : public UGameInstance
{
	GENERATED_BODY()

public:
	// Constructor
	URPGGameInstanceBase();

	/** 默认 inventory 中的物品，会添加到新的 player 的 inventory 中 */
	/** List of inventory items to add to new players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FPrimaryAssetId, FRPGItemData> DefaultInventory;

	/** 每个类型的道具的 slot 的最大数量 */
	/** Number of slots for each type of item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FPrimaryAssetType, int32> ItemSlotsPerType;

	/** 用于保存游戏的 slot 的名称，在构造函数中被初始化为 "SaveGame" ，这个和其他的 Item Slot 应该不是一个东西 */
	/** The slot name used for saving */
	UPROPERTY(BlueprintReadWrite, Category = Save)
	FString SaveSlot;

	/** 对于某些平台， UserIndex 可用来识别保存的用户，但在本项目中没有用到，在构造函数中被初始化为 0 */
	/** The platform-specific user index */
	UPROPERTY(BlueprintReadWrite, Category = Save)
	int32 SaveUserIndex;

	/** 当存档被加载 / 重置时的代理 */
	/** Delegate called when the save game has been loaded/reset */
	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnSaveGameLoaded OnSaveGameLoaded;

	/** 当存档被加载 / 重置时的原生代理 */
	/** Native delegate for save game load/reset */
	FOnSaveGameLoadedNative OnSaveGameLoadedNative;

	/**
	 * 把 DefaultInventory 补充到加载的存档中。
	 */
	/**
	 * Adds the default inventory to the SaveGame
	 * @param SaveGame
	 * @param bRemoveExtra If true, remove anything other than default inventory
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void AddDefaultInventory(URPGSaveGame* SaveGame, bool bRemoveExtra = false);

	/** 判断是否是有效的 ItemSlot */
	/** Returns true if this is a valid inventory slot */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool IsValidItemSlot(FRPGItemSlot ItemSlot) const;

	/** 返回当前保存的游戏，所以可以被用来初始化状态。直到 WriteSaveGame 被调用，改变才会写入。 */
	/** Returns the current save game, so it can be used to initialize state. Changes are not written until WriteSaveGame is called */
	UFUNCTION(BlueprintCallable, Category = Save)
	URPGSaveGame* GetCurrentSaveGame();

	/** 设置是否保存到硬盘 / 从硬盘中读取 */
	/** Sets rather save/load is enabled. If disabled it will always count as a new character */
	UFUNCTION(BlueprintCallable, Category = Save)
	void SetSavingEnabled(bool bEnabled);

	/**
	 * 同步加载（返回 true）或创建（返回 false）一个存档，创建的逻辑实际上是由 HandleSaveGameLoaded(USaveGame* SaveGameObject) 处理的
	 * 没有用到，蓝图中使用的是异步加载的方式：Async Load Game From Slot
	 */
	/** Synchronously loads a save game. If it fails, it will create a new one for you. Returns true if it loaded, false if it created one */
	UFUNCTION(BlueprintCallable, Category = Save)
	bool LoadOrCreateSaveGame();

	/** 返回加载的存档是否有效，无效的话会创建新的存档，最终将存档保存到 CurrentSaveGame 中 */
	/** Handle the final setup required after loading a USaveGame object using AsyncLoadGameFromSlot. Returns true if it loaded, false if it created one */
	UFUNCTION(BlueprintCallable, Category = Save)
	bool HandleSaveGameLoaded(USaveGame* SaveGameObject);

	/** 获得 SlotName 和 UserIndex 的 Helper 函数 */
	/** Gets the save game slot and user index used for inventory saving, ready to pass to GameplayStatics save functions */
	UFUNCTION(BlueprintCallable, Category = Save)
	void GetSaveSlotInfo(FString& SlotName, int32& UserIndex) const;

	/** 把当前保存的游戏写入硬盘，在后台线程中进行。 */
	/** Writes the current save game object to disk. The save to disk happens in a background thread*/
	UFUNCTION(BlueprintCallable, Category = Save)
	bool WriteSaveGame();

	/** 重置存档，只会重置 CurrentSaveGame ，不会直接写入硬盘 */
	/** Resets the current save game to it's default. This will erase player data! This won't save to disk until the next WriteSaveGame */
	UFUNCTION(BlueprintCallable, Category = Save)
	void ResetSaveGame();

protected:
	/** 当前的存档 */
	/** The current save game object */
	UPROPERTY()
	URPGSaveGame* CurrentSaveGame;

	/** 是否会保存到硬盘 / 从硬盘中加载 */
	/** Rather it will attempt to actually save to disk */
	UPROPERTY()
	bool bSavingEnabled;

	/** 是否正在进行保存操作 */
	/** True if we are in the middle of doing a save */
	UPROPERTY()
	bool bCurrentlySaving;

	/** 是否有由于当前正在进行保存而等待的保存操作 */
	/** True if another save was requested during a save */
	UPROPERTY()
	bool bPendingSaveRequested;

	/** 当发生异步保存时调用 */
	/** Called when the async save happens */
	virtual void HandleAsyncSave(const FString& SlotName, const int32 UserIndex, bool bSuccess);
};