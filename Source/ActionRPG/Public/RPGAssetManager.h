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
 * Asset Manager 是一个唯一的全局对象，可用于管理资产，提供了用于监听硬盘和内存使用的工具集。
 *
 * UE 中的资产分为两类：Primary Assets 和 Secondary Assets 。
 * Primary Assets 是能够被 Asset Manager 直接操作的资产，Secondary Assets 是被 Primary Assets 引用并自动加载 / 卸载的资产。默认只有
 * UWorld 资产（即 Level）是 Primary Assets ，其他的所有资产都是 Secondary Assets 。Primary Assets 必须实现 GetPrimaryAssetId() ，
 * （比如 RPGItem）。
 *
 * 资产还分为蓝图资产和非蓝图资产。
 * - 非蓝图资产只能创建资产的实例，不能像蓝图资产一样创建资产的类，使用 GetPrimaryAssetObject 从 C++ / 蓝图中访问。
 * - Blueprint Primary Asset 是继承了实现了 GetPrimaryAssetId() 的 C++ 类的蓝图类。可以使用 GetPrimaryAssetObjectClass 从蓝图 / C++
 *   中访问 Blueprint Primary Assets ，然后就可以像其他蓝图类一样使用：生成实例或访问 CDO 。
 * - 对于不需要实例化的蓝图类，可以将数据保存在继承自 UPrimaryDataAsset 的 Data-Only 的蓝图中（比如 RPGItem）。
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

    /** 这个接口是 UE 启动流程的一部分 */
    /** Starts initial load, gets called from InitializeObjectReferences */
    virtual void StartInitialLoading() override;

    /** FPrimaryAssetType 实际上就是一个 FName ，可以隐式转换，这个类型存在是为了让蓝图知道这不是一个普通的 FName ，在 .cpp 中定义 */
    /** Static types for items */
    static const FPrimaryAssetType PotionItemType; // 药剂
    static const FPrimaryAssetType SkillItemType; // 技能
    static const FPrimaryAssetType TokenItemType; // 代币
    static const FPrimaryAssetType WeaponItemType; // 武器

    /** 返回当前的 AssetManager 的单例，这个函数在 AssetManager 类中也有，但不是 virtual 的，实现基本上参照了 AssetManager 中的实现 */
    /** Returns the current AssetManager object */
    static URPGAssetManager& Get();

    /**
     * 同步加载尚未在内存中的道具以从 PrimaryAssetId 转换为 URPGItem
     * ARPG 一般都会有一个商店，所以需要在游戏启动时加载所有的道具
     */
    /**
     * Synchronously loads an RPGItem subclass, this can hitch but is useful when you cannot wait for an async load
     * This does not maintain a reference to the item so it will garbage collect if not loaded some other way
     *
     * @param PrimaryAssetId The asset identifier to load
     * @param bLogWarning If true, this will log a warning if the item failed to load
     */
    URPGItem* ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true) const;
};
