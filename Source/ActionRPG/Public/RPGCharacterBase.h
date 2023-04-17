// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionRPG.h"
#include "GameFramework/Character.h"
#include "UObject/ScriptInterface.h"
#include "RPGInventoryInterface.h"
#include "AbilitySystemInterface.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "Abilities/RPGAttributeSet.h"
#include "GenericTeamAgentInterface.h"
#include "RPGCharacterBase.generated.h"

class URPGGameplayAbility;
class UGameplayEffect;

/** 更复杂的游戏可能需要多个 C++ 角色类 */
/** Base class for Character, Designed to be blueprinted */
UCLASS()
/** 继承 IAbilitySystemInterface 接口，只有一个获取 ASC 的函数： GetAbilitySystemComponent() */
class ACTIONRPG_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	ARPGCharacterBase();
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 实现 IAbilitySystemInterface
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/*------------------------------------------------------ 属性 Getter ---------------------------------------------------*/

	/** Returns current health, will be 0 if dead */
	UFUNCTION(BlueprintCallable)
	virtual float GetHealth() const;

	/** Returns maximum health, health will never be greater than this */
	UFUNCTION(BlueprintCallable)
	virtual float GetMaxHealth() const;

	/** Returns current mana */
	UFUNCTION(BlueprintCallable)
	virtual float GetMana() const;

	/** Returns maximum mana, mana will never be greater than this */
	UFUNCTION(BlueprintCallable)
	virtual float GetMaxMana() const;

	/** Returns current movement speed */
	UFUNCTION(BlueprintCallable)
	virtual float GetMoveSpeed() const;

	/** Returns the character level that is passed to the ability system */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetCharacterLevel() const;

	/** Modifies the character level, this may change abilities. Returns true on success */
	UFUNCTION(BlueprintCallable)
	virtual bool SetCharacterLevel(int32 NewLevel);

	/**
	 * Attempts to activate any ability in the specified item slot. Will return false if no activatable ability found or activation fails
	 * Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 * If bAllowRemoteActivation is true, it will remotely activate local/server abilities, if false it will only try to locally activate the ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbilitiesWithItemSlot(FRPGItemSlot ItemSlot, bool bAllowRemoteActivation = true);

	/** Returns a list of active abilities bound to the item slot. This only returns if the ability is currently running */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GetActiveAbilitiesWithItemSlot(FRPGItemSlot ItemSlot, TArray<URPGGameplayAbility*>& ActiveAbilities);

	/**
	 * Attempts to activate all abilities that match the specified tags
	 * Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 * If bAllowRemoteActivation is true, it will remotely activate local/server abilities, if false it will only try to locally activate the ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbilitiesWithTags(FGameplayTagContainer AbilityTags, bool bAllowRemoteActivation = true);

	/** Returns a list of active abilities matching the specified tags. This only returns if the ability is currently running */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void GetActiveAbilitiesWithTags(FGameplayTagContainer AbilityTags, TArray<URPGGameplayAbility*>& ActiveAbilities);

	/** Returns total time and remaining time for cooldown tags. Returns false if no active cooldowns found */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool GetCooldownRemainingForTag(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration);

protected:
	/** 角色的等级，在角色出生后不应该直接修改 */
	/** The level of this character, should not be modified directly once it has already spawned */
	UPROPERTY(EditAnywhere, Replicated, Category = Abilities)
	int32 CharacterLevel;

	/** 在创建 Character 时赋予的 Ability ，这些 Ability 会被 tag 或事件激活，不和输入绑定 */
	/** Abilities to grant to this character on creation. These will be activated by tag or event and are not bound to specific inputs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
	TArray<TSubclassOf<URPGGameplayAbility>> GameplayAbilities;

	/** 默认装备的 Ability ，在 inventory 中的 Ability 之前添加 */
	/** Map of item slot to gameplay ability class, these are bound before any abilities added by the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Abilities)
	TMap<FRPGItemSlot, TSubclassOf<URPGGameplayAbility>> DefaultSlottedAbilities;

	/** 用来设置 Character 的初始属性的 GE 会在 Character 创建和 Character 的等级改变时应用（AddStartupGameplayAbilities()），
	 * BP_PlayerCharacter 中添加的 GE 是 GE_PlayerStats
	 * NPC_GoblinBP 中添加的 GE 是 GE_GoblinStats
	 * NPC_SpiderBoss 中添加的 GE 是 GE_SpiderStats
	 * 他们使用的数据保存在 StartingStats 中 */
	/** Passive gameplay effects applied on creation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Abilities)
	TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

	/** AS 组件 */
	/** The component used to handle ability system interactions */
	UPROPERTY()
	URPGAbilitySystemComponent* AbilitySystemComponent;

	/** Gameplay Ability System 使用的属性集 */
	/** List of attributes modified by the ability system */
	UPROPERTY()
	URPGAttributeSet* AttributeSet;

	/** 一个指向 inventory 源的指针，可能是 null */
	/** Cached pointer to the inventory source for this character, can be null */
	UPROPERTY()
	TScriptInterface<IRPGInventoryInterface> InventorySource;

	/** 记录角色的能力是否已经被初始化 */
	/** If true we have initialized our abilities */
	UPROPERTY()
	int32 bAbilitiesInitialized;

	/** 从 slot 到其赋予的 Ability 的映射 */
	/** Map of slot to ability granted by that slot. I may refactor this later */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	TMap<FRPGItemSlot, FGameplayAbilitySpecHandle> SlottedAbilities;

	/** Delegate handles */
	FDelegateHandle InventoryUpdateHandle;
	FDelegateHandle InventoryLoadedHandle;

	/**
	 * Called when character takes damage, which may have killed them
	 *
	 * @param DamageAmount Amount of damage that was done, not clamped based on current health
	 * @param HitInfo The hit info that generated this damage
	 * @param DamageTags The gameplay tags of the event that did the damage
	 * @param InstigatorCharacter The character that initiated this damage
	 * @param DamageCauser The actual actor that did the damage, might be a weapon or projectile
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnDamaged(float DamageAmount, const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags, ARPGCharacterBase* InstigatorCharacter, AActor* DamageCauser);

	/**
	 * Called when health is changed, either from healing or from being damaged
	 * For damage this is called in addition to OnDamaged/OnKilled
	 *
	 * @param DeltaValue Change in health value, positive for heal, negative for cost. If 0 the delta is unknown
	 * @param EventTags The gameplay tags of the event that changed mana
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);

	/**
	 * Called when mana is changed, either from healing or from being used as a cost
	 *
	 * @param DeltaValue Change in mana value, positive for heal, negative for cost. If 0 the delta is unknown
	 * @param EventTags The gameplay tags of the event that changed mana
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnManaChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);

	/**
	 * Called when movement speed is changed
	 *
	 * @param DeltaValue Change in move speed
	 * @param EventTags The gameplay tags of the event that changed mana
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnMoveSpeedChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);

	/** 在装备的道具改变时调用，绑定到委托或接口 */
	/** Called when slotted items change, bound to delegate on interface */
	void OnItemSlotChanged(FRPGItemSlot ItemSlot, URPGItem* Item);
	void RefreshSlottedGameplayAbilities();

	/** Apply the startup gameplay abilities and effects */
	void AddStartupGameplayAbilities();

	/** Attempts to remove any startup gameplay abilities */
	void RemoveStartupGameplayAbilities();

	/** Adds slotted item abilities if needed */
	void AddSlottedGameplayAbilities();

	/** 获取来自装备的 Ability ，放到 SlottedAbilitySpecs 中 */
	/** Fills in with ability specs, based on defaults and inventory */
	void FillSlottedAbilitySpecs(TMap<FRPGItemSlot, FGameplayAbilitySpec>& SlottedAbilitySpecs);

	/** Remove slotted gameplay abilities, if force is false it only removes invalid ones */
	void RemoveSlottedGameplayAbilities(bool bRemoveAll);

	// Called from RPGAttributeSet, these call BP events above
	virtual void HandleDamage(float DamageAmount, const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags, ARPGCharacterBase* InstigatorCharacter, AActor* DamageCauser);
	virtual void HandleHealthChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);
	virtual void HandleManaChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);
	virtual void HandleMoveSpeedChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags);

	/** Required to support AIPerceptionSystem */
	virtual FGenericTeamId GetGenericTeamId() const override;

	// Friended to allow access to handle functions above
	friend URPGAttributeSet;
};
