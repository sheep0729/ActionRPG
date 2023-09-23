// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGAssetManager.h"
#include "Items/RPGItem.h"
#include "AbilitySystemGlobals.h"
#include "AssetRegistry/AssetData.h"
#include "Items/RPGWeaponItem.h"

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

void URPGAssetManager::AssetManagerSample()
{
	// Get the global Asset Manager 获取资产管理器
	URPGAssetManager& AssetManager = URPGAssetManager::Get();

	// Get a list of all weapons that can be loaded 获取所有可以加载的武器
	TArray<FPrimaryAssetId> WeaponIdList;
	AssetManager.GetPrimaryAssetIdList(WeaponItemType, WeaponIdList);

	for (const FPrimaryAssetId& WeaponId : WeaponIdList)
	{
		// Get tag / value for an unloaded weapon 获取资产中的数据
		FAssetData AssetDataToParse;
		AssetManager.GetPrimaryAssetData(WeaponId, AssetDataToParse);

		FName QueryExample;
		AssetDataToParse.GetTagValue(GET_MEMBER_NAME_CHECKED(URPGItem, ExampleRegistryTag), QueryExample);

		UE_LOG(LogTemp, Log, TEXT("Read ExampleRegistryTag %s from Weapon %s"), *QueryExample.ToString(), *AssetDataToParse.AssetName.ToString());
	}

	// Permanently load a single item
	TArray<FName> CurrentLoadState;
	CurrentLoadState.Add(FName("Game"));

	FName WeaponName = FName("Weapon_Hammer_3");
	FPrimaryAssetId WeaponId = FPrimaryAssetId(WeaponItemType, WeaponName);
	AssetManager.LoadPrimaryAsset(WeaponId, CurrentLoadState, FStreamableDelegate::CreateUObject(&AssetManager, &URPGAssetManager::CallbackFunction, WeaponId));

	TArray<FPrimaryAssetId> ListOfPrimaryAssetIds;
	AssetManager.GetPrimaryAssetIdList(SkillItemType, ListOfPrimaryAssetIds);

	// Load a list of items as long as Handle is alive
	TSharedPtr<FStreamableHandle> Handle = AssetManager.PreloadPrimaryAssets(ListOfPrimaryAssetIds, CurrentLoadState, false);

	// Should store Handle somewhere
}

void URPGAssetManager::CallbackFunction(FPrimaryAssetId WeaponId)
{
	if (const URPGWeaponItem* Weapon = GetPrimaryAssetObject<URPGWeaponItem>(WeaponId))
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded Weapon %s"), *Weapon->GetName());
	}

	// Release Previously loaded item
	UnloadPrimaryAsset(WeaponId);
}
