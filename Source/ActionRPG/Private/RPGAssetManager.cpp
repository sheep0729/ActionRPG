// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGAssetManager.h"
#include "Items/RPGItem.h"
#include "AbilitySystemGlobals.h"

const FPrimaryAssetType	URPGAssetManager::PotionItemType = TEXT("Potion");
const FPrimaryAssetType	URPGAssetManager::SkillItemType = TEXT("Skill");
const FPrimaryAssetType	URPGAssetManager::TokenItemType = TEXT("Token");
const FPrimaryAssetType	URPGAssetManager::WeaponItemType = TEXT("Weapon");

URPGAssetManager& URPGAssetManager::Get()
{
	URPGAssetManager* This = Cast<URPGAssetManager>(GEngine->AssetManager);

	if (This)
	{
		return *This;
	}
	
	// UE_LOG 的 Verbosity 是 Fatal 时会输出日志并崩溃，所以下面的 return 从不会调用
	UE_LOG(LogActionRPG, Fatal, TEXT("Invalid AssetManager in DefaultEngine.ini, must be RPGAssetManager!"));
	return *NewObject<URPGAssetManager>(); // never calls this
}

/**
 * @brief 在这个函数中初始化 Gameplay Ability System 的全局数据
 */
void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// UAbilitySystemGlobals 是一个单例类，Get() 返回唯一的实例，保存了 Gameplay Ability System 的全局数据，可通过配置文件配置。
	// InitGlobalData() 是负责执行初始化的函数，Should be called once as part of project setup to load global data tables and tags
	UAbilitySystemGlobals::Get().InitGlobalData();
}


URPGItem* URPGAssetManager::ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning) const
{
	// FSoftObjectPath 内部有一个 FName 用来保存 top level asset （就是 Primary Assets） 的路径，还有一个 FString 用来保存 subobject 的路径
	// 软引用的资产可以按需加载
	const FSoftObjectPath ItemPath = GetPrimaryAssetPath(PrimaryAssetId);

	// This does a synchronous load and may hitch
	URPGItem* LoadedItem = Cast<URPGItem>(ItemPath.TryLoad());

	if (bLogWarning && LoadedItem == nullptr)
	{
		UE_LOG(LogActionRPG, Warning, TEXT("Failed to load item for identifier %s!"), *PrimaryAssetId.ToString());
	}

	return LoadedItem;
}
