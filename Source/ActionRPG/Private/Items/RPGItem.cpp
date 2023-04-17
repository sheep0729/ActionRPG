// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/RPGItem.h"

bool URPGItem::IsConsumable() const
{
	if (MaxCount <= 0)
	{
		return true;
	}
	return false;
}

FString URPGItem::GetIdentifierString() const
{
	return GetPrimaryAssetId().ToString();
}

FPrimaryAssetId URPGItem::GetPrimaryAssetId() const
{
	// 这是一个非蓝图的 DataAsset ，所以可以直接使用 FName ，如果是蓝图则需要去掉 _C 后缀
	// GetFName 会返回一个类的逻辑名称 TODO 是不是就是在 Asset Manager 中显示的名称？
	// This is a DataAsset and not a blueprint so we can just use the raw FName
	// For blueprints you need to handle stripping the _C suffix
	return FPrimaryAssetId(ItemType, GetFName());
}