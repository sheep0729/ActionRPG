// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/RPGAbilityTypes.h"
#include "RPGTargetType.generated.h"

class ARPGCharacterBase;
class AActor;
struct FGameplayEventData;

/**
 * 这个类用来确定 Ability 的目标
 * 被设计为可以在蓝图中运行查找目标逻辑
 * 没有继承 GameplayAbilityTargetActor ，因为这个类不会在世界中实例化
 * 可以作为游戏特定的确定目标的蓝图的基础
 * 如果你的确定目标的逻辑更加复杂，你可能需要在世界中实例化一次，或者使用 Actor 池
 */
/**
 * Class that is used to determine targeting for abilities
 * It is meant to be blueprinted to run target logic
 * This does not subclass GameplayAbilityTargetActor because this class is never instanced into the world
 * This can be used as a basis for a game-specific targeting blueprint
 * If your targeting is more complicated you may need to instance into the world once or as a pooled actor
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class ACTIONRPG_API URPGTargetType : public UObject
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGTargetType() {}

	/** Called to determine targets to apply gameplay effects to */
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(ARPGCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/** Trivial target type that uses the owner */
UCLASS(NotBlueprintable)
class ACTIONRPG_API URPGTargetType_UseOwner : public URPGTargetType
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGTargetType_UseOwner() {}

	/** Uses the passed in event data */
	virtual void GetTargets_Implementation(ARPGCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

/** Trivial target type that pulls the target out of the event data */
UCLASS(NotBlueprintable)
class ACTIONRPG_API URPGTargetType_UseEventData : public URPGTargetType
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGTargetType_UseEventData() {}

	/** Uses the passed in event data */
	virtual void GetTargets_Implementation(ARPGCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
