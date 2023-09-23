// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "RPGAssetManager.h"
#include "RPGItem.generated.h"

class URPGGameplayAbility;

/** 所有 item 的基类，这个文件夹中的其他头文件是它的子类 */
/** Base class for all items, do not blueprint directly */
UCLASS(Abstract, BlueprintType)
class ACTIONRPG_API URPGItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Constructor */
	URPGItem()
		: Price(0)
		, MaxCount(1)
		, MaxLevel(1)
		, AbilityLevel(1)
	{}

	/** 道具的类型，在 C++ 的子类中设置，这个类型是在 URPGAssetManager 中定义的 */
	/** Type of this item, set in native parent class */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item)
	FPrimaryAssetType ItemType;

	/** 用户可见的道具名称 */
	/** User-visible short name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FText ItemName;

	/** 用户可见的描述 */
	/** User-visible long description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FText ItemDescription;

	/** 道具的图标 */
	/** Icon to display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FSlateBrush ItemIcon;

	/** 游戏中的价格 */
	/** Price in game */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	int32 Price;

	/** 可以在背包中保存的最大数量，0 代表无限 */
	/** Maximum number of instances that can be in inventory at once, <= 0 means infinite */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Max)
	int32 MaxCount;

	/** 返回道具是否是可消耗的，即 MaxCount <= 0 */
	/** Returns if the item is consumable (MaxCount <= 0)*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Max)
	bool IsConsumable() const;

	/** 道具的最大等级，0 代表无限 */
	/** Maximum level this item can be, <= 0 means infinite */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Max)
	int32 MaxLevel;

	/** 当这个物品被装备后赋予给角色的 Ability */
	/** Ability to grant if this item is slotted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
	TSubclassOf<URPGGameplayAbility> GrantedAbility;

	/** 能力的等级， <= 0 代表使用角色的等级 */
	/** Ability level this item grants. <= 0 means the character level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
	int32 AbilityLevel;

	/** 返回 PrimaryAssetId 的字符串，格式是 "ItemType: ItemName" */
	/** Returns the logical name, equivalent to the primary asset id */
	UFUNCTION(BlueprintCallable, Category = Item)
	FString GetIdentifierString() const;

	/** 重写这个函数使这个类型成为 Primary Asset */
	/** Overridden to use saved type */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// ASSET MANAGER EXAMPLE: Detailed preview texture for inventory menu
	// AssetBundles: [PropertyMetadata] Used for SoftObjectPtr/SoftObjectPath properties. Comma separated list of Bundle names used inside
	// PrimaryDataAssets to specify which bundles this reference is part of
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "Menu"))
	TSoftObjectPtr<UTexture> InventoryTexture;

	// ASSET MANAGER EXAMPLE: Sound that only plays in game
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AssetBundles = "Game"))
	TSoftObjectPtr<USoundBase> PickupSound;

	// ASSET MANAGER EXAMPLE: Item that is linked to this one somehow
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FPrimaryAssetId LinkedItem;
 
	// ASSET MANAGER EXAMPLE: Item that is linked to this one somehow
	// AssetRegistrySearchable: The AssetRegistrySearchable Specifier indicates that this property and its value will be automatically added to
	// the Asset Registry for any Asset class instances containing this as a member variable. It is not legal to use on struct properties or parameters.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, AssetRegistrySearchable)
	FName ExampleRegistryTag;
};
