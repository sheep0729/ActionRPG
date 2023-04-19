// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameplayEffectExecutionCalculation.h"
#include "RPGDamageExecution.generated.h"

/**
 * 在 GE_DamageBase->Executions->Calculation Class 中设置
 */
/**
 * A damage execution, which allows doing damage by combining a raw Damage number with AttackPower and DefensePower
 * Most games will want to implement multiple game-specific executions
 */
UCLASS()
class ACTIONRPG_API URPGDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	URPGDamageExecution();

	/**
	 * UFUNCTION(BlueprintNativeEvent, Category="Calculation")
	 * void Execute(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const;
	 *
	 * Called whenever the owning gameplay effect is executed. Allowed to do essentially whatever is desired, including generating new
	 * modifiers to instantly execute as well.
	 * 
	 * @note: Native subclasses should override the auto-generated Execute_Implementation function and NOT this one.
	 * 
	 * @param ExecutionParams		Parameters for the custom execution calculation
	 * @param OutExecutionOutput	[OUT] Output data populated by the execution detailing further behavior or results of the execution
	 */

	/**
	 * USTRUCT(BlueprintType)
     * struct GAMEPLAYABILITIES_API FGameplayEffectCustomExecutionParameters
     * 
	 * Struct representing parameters for a custom gameplay effect execution. Should not be held onto via reference, used just for the scope of the execution
	 */

	/**
	 * USTRUCT(BlueprintType)
	 * struct GAMEPLAYABILITIES_API FGameplayEffectCustomExecutionOutput
	 * 
	 * Struct representing the output of a custom gameplay effect execution.
	 */
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};