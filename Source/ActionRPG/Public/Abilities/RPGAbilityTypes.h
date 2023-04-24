// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// ----------------------------------------------------------------------------------------------------------------
// 这个头文件用于在项目中共享特定于 Ability 的结构体和枚举
// 每个游戏都可能需要这样的文件来处理 GAS 扩展
// 
// ----------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
// This header is for Ability-specific structures and enums that are shared across a project
// Every game will probably need a file like this to handle their extensions to the system
// This file is a good place for subclasses of FGameplayEffectContext and FGameplayAbilityTargetData
// ----------------------------------------------------------------------------------------------------------------

#include "ActionRPG.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "RPGAbilityTypes.generated.h"

class URPGAbilitySystemComponent;
class UGameplayEffect;
class URPGTargetType;

/**
 * 定义 GE 、Tag 和目标信息的结构体
 * 这些结构体在蓝图或资产中静态定义，然后在运行时转变为 FRPGGameplayEffectContainerSpec
 */
/**
 * Struct defining a list of gameplay effects, a tag, and targeting info
 * These containers are defined statically in blueprints or assets and then turn into Specs at runtime
 */
USTRUCT(BlueprintType)
struct FRPGGameplayEffectContainer
{
	GENERATED_BODY()

public:
	FRPGGameplayEffectContainer() {}

	/** 设置查找目标的方式 */
	/** Sets the way that targeting happens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TSubclassOf<URPGTargetType> TargetType;

	/** 应用到目标上的 GE 列表 */
	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

/** 一个“处理过的” RPGGameplayEffectContainer，可以传递并最终应用 */
/** A "processed" version of RPGGameplayEffectContainer that can be passed around and eventually applied */
USTRUCT(BlueprintType)
struct FRPGGameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	FRPGGameplayEffectContainerSpec() {}


	/**
	 * FGameplayAbilityTargetDataHandle:
	 * 
	 *	Handle for Targeting Data. This servers two main purposes:
	 *		-Avoid us having to copy around the full targeting data structure in Blueprints
	 *		-Allows us to leverage polymorphism in the target data structure
	 *		-Allows us to implement NetSerialize and replicate by value between clients/server
	 *
	 *		-Avoid using UObjects could be used to give us polymorphism and by reference passing in blueprints.
	 *		-However we would still be screwed when it came to replication
	 *
	 *		-Replication by value
	 *		-Pass by reference in blueprints
	 *		-Polymorphism in TargetData structure
	 */
	/** Computed target data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayAbilityTargetDataHandle TargetData;

	/**
	 * FGameplayEffectSpecHandle:
	 * 允许蓝图生成一次 GameplayEffectSpec ，然后通过 Handle 引用，并多次应用 GE 。
	 */
	/**
	 * FGameplayEffectSpecHandle:
	 * Allows blueprints to generate a GameplayEffectSpec once and then reference it by handle, to apply it multiple times/multiple targets.
	 */
	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/**  */
	/** Returns true if this has any valid effect specs */
	bool HasValidEffects() const;

	/** Returns true if this has any valid targets */
	bool HasValidTargets() const;

	/** 把新的 Target 添加到 TargetData */
	/** Adds new targets to target data */
	void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
};
