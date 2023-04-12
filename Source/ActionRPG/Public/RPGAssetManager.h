// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Engine/AssetManager.h"
#include "RPGAssetManager.generated.h"

class URPGItem;

/**
 * AssetManager 的子类。
 * 资源管理器最初设计用于管理在各种不同情况下和整个游戏中可用的资源，通常应用于所有物品栏项目。
 * 需要在 DefaultEngine.ini 中设置 AssetManagerClassName
 */

/**
 * Game implementation of asset manager, overrides functionality and stores game-specific types
 * It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic
 * This is used by setting AssetManagerClassName in DefaultEngine.ini
 */
UCLASS()
class ACTIONRPG_API URPGAssetManager : public UAssetManager
{
    GENERATED_BODY()

public:
    // Constructor and overrides
    URPGAssetManager()
    {
    }

    virtual void StartInitialLoading() override;

    /** Static types for items */
    static const FPrimaryAssetType PotionItemType; // 药剂
    static const FPrimaryAssetType SkillItemType; // 技能
    static const FPrimaryAssetType TokenItemType; // 代币
    static const FPrimaryAssetType WeaponItemType; // 武器

    /** Returns the current AssetManager object */
    static URPGAssetManager& Get();

    /**
     * Synchronously loads an RPGItem subclass, this can hitch but is useful when you cannot wait for an async load
     * This does not maintain a reference to the item so it will garbage collect if not loaded some other way
     *
     * ForceLoadItem 可以同步加载尚未在内存中的项目以从 PrimaryAssetId 转换为 URPGItem
     * （由于上面提到的存储预载，在ARPG中通常需要这样操作）
     *
     * @param PrimaryAssetId The asset identifier to load
     * @param bDisplayWarning If true, this will log a warning if the item failed to load
     */
    URPGItem* ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true);
};
