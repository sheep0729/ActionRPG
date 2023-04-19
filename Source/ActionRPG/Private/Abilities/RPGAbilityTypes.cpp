// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGAbilityTypes.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

bool FRPGGameplayEffectContainerSpec::HasValidEffects() const
{
	return TargetGameplayEffectSpecs.Num() > 0;
}

bool FRPGGameplayEffectContainerSpec::HasValidTargets() const
{
	return TargetData.Num() > 0;
}

void FRPGGameplayEffectContainerSpec::AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors)
{
	for (const FHitResult& HitResult : HitResults)
	{
		/** FGameplayAbilityTargetData_SingleTargetHit: Target data with a single hit result, data is packed into the hit result */
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetData.Add(NewData);
	}

	if (TargetActors.Num() > 0)
	{
		/** FGameplayAbilityTargetData_ActorArray: Target data with a source location and a list of targeted actors, makes sense for AOE attacks */
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->TargetActorArray.Append(TargetActors);
		TargetData.Add(NewData);
	}
}