// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGCharacterBase.h"
#include "Items/RPGItem.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/RPGGameplayAbility.h"

ARPGCharacterBase::ARPGCharacterBase()
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// QUICK_SCOPE_CYCLE_COUNTER(STAT_ARPGCharacterBase);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	// 这个 URPGAbilitySystemComponent 是继承自 UAbilitySystemComponent 的子类
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("AbilitySystemComponent")); // 创建 ASC
	AbilitySystemComponent->SetIsReplicated(true); // 设置复制

	// 直接将 AttributeSet 作为拥有 Ability System Component 的 Actor 的 subobject 。
	// 也可以通过 GetOrCreateAttributeSubobject 方法传递给 Ability System Component 。
	// Create the attribute set, this replicates by default
	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("AttributeSet"));

	// 角色的默认等级
	CharacterLevel = 1;
	bAbilitiesInitialized = false;
}

/**
 * @brief 实现 IAbilitySystemInterface
 */
UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
// DECLARE_CYCLE_STAT(TEXT("AddStartupGameplayAbilities"), STAT_AddStartupGameplayAbilities, STATGROUP_ARPGCharacterBase);
/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

/**
 * @brief 在创建 Character 或等级改变时执行
 */
void ARPGCharacterBase::AddStartupGameplayAbilities()
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// SCOPE_CYCLE_COUNTER(STAT_AddStartupGameplayAbilities);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	// 如果是 false 就终止运行，默认不在 shipping build 中运行
	check(AbilitySystemComponent);

	// 在服务器上且没有初始化
	if (GetLocalRole() == ROLE_Authority && !bAbilitiesInitialized)
	{
		// 在服务器上赋予 Ability
		// Grant abilities, but only on the server	
		for (TSubclassOf<URPGGameplayAbility>& StartupAbility : GameplayAbilities)
		{
			// FGameplayAbilitySpec 是一个可激活的 Ability 的 spec 。
			// 它内部有一个 Ability 的 CDO ，和这个 Ability 的所有实例。
			// 激活的 / 实例化的 Ability 还需要其他但不能保存在自身的信息， FGameplayAbilitySpec 也保存了这些信息。
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, GetCharacterLevel(), INDEX_NONE, this));
		}

		// 应用被动 Ability ，本项目中用来设置 AttributeSet 的初始值
		// Now apply passives
		for (const TSubclassOf<UGameplayEffect>& GameplayEffect : PassiveGameplayEffects)
		{
			// 创建一个 GameplayEffectContext 并返回 Handle 。
			// GameplayEffectContext 是 GE 在执行期间的上下文。
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			// 在 GE 的 Context 中设置这个 GE 是哪个 Actor 创建的，在这里是自己。
			EffectContext.AddSourceObject(this); 
			// 创建一个 FGameplayEffectSpec 并返回 Handle 。
			// FGameplayEffectSpec 是准备好被应用的 GE ，创建它需要
			//   GE 本身（需要它的 CDO）
			//   等级信息
			//   GE 的上下文
			FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, GetCharacterLevel(), EffectContext);
			if (NewHandle.IsValid())
			{
				// ApplyGameplayEffectSpecToTarget 最终会调用 ActiveGameplayEffects.ApplyGameplayEffectSpec()
				// ActiveGameplayEffects 是 AbilitySystemComponent 上的一个 FActiveGameplayEffectsContainer ，它是 FActiveGameplayEffect 的容器。
				// 返回的 FActiveGameplayEffectHandle 和上面的 Handle 不一样，它不是一个简单的 Wrapper ，而是用来在 Container 外部访问特定的 FActiveGameplayEffect。 
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent);
			}
		}

		// 添加已经装备的 Ability
		AddSlottedGameplayAbilities();

		bAbilitiesInitialized = true;
	}
}

void ARPGCharacterBase::RemoveStartupGameplayAbilities()
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// DECLARE_SCOPE_CYCLE_COUNTER(TEXT("RemoveStartupGameplayAbilities"), STAT_RemoveStartupGameplayAbilities, STATGROUP_ARPGCharacterBase);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	check(AbilitySystemComponent);

	if (GetLocalRole() == ROLE_Authority && bAbilitiesInitialized)
	{
		// Remove any abilities added from a previous call
		TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
		// GetActivatableAbilities() 会返回 ActivatableAbilities.Items ，ActivatableAbilities 是一个 FGameplayAbilitySpecContainer
		// 所以 FGameplayAbilitySpec 就是 “Activatable” 的 Ability 。
		for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			// SourceObject 是指创建这个 Ability 的对象，这里通过 SourceObject 是否是自己来判断是否是 StartupGameplayAbility
			if (Spec.SourceObject == this && GameplayAbilities.Contains(Spec.Ability->GetClass()))
			{
				AbilitiesToRemove.Add(Spec.Handle);
			}
		}

		// 根据 FGameplayAbilitySpecHandle 通过 ClearAbility 来移除 Ability
		// Do in two passes so the removal happens after we have the full list
		for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
		{
			AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
		}

		// 移除所有的被动 GE ，这里的判断条件也是 GE 是否来自于自己
		// 但没有使用 FActiveGameplayEffectHandle ，而是使用 FGameplayEffectQuery
		// Remove all of the passive gameplay effects that were applied by this character
		FGameplayEffectQuery Query;
		Query.EffectSource = this;
		AbilitySystemComponent->RemoveActiveEffects(Query);

		// 移除已经装备的 Ability
		RemoveSlottedGameplayAbilities(true);

		bAbilitiesInitialized = false;
	}
}

void ARPGCharacterBase::OnItemSlotChanged(FRPGItemSlot ItemSlot, URPGItem* Item)
{
	RefreshSlottedGameplayAbilities();
}

void ARPGCharacterBase::RefreshSlottedGameplayAbilities()
{
	if (bAbilitiesInitialized)
	{
		// Refresh any invalid abilities and adds new ones
		RemoveSlottedGameplayAbilities(false);
		AddSlottedGameplayAbilities();
	}
}

void ARPGCharacterBase::FillSlottedAbilitySpecs(TMap<FRPGItemSlot, FGameplayAbilitySpec>& SlottedAbilitySpecs)
{
	// 先获取默认装备的 Ability
	// First add default ones
	for (const TPair<FRPGItemSlot, TSubclassOf<URPGGameplayAbility>>& DefaultPair : DefaultSlottedAbilities)
	{
		if (DefaultPair.Value.Get())
		{
			// DefaultSlottedAbilities 的 SourceObject 认为是自己
			SlottedAbilitySpecs.Add(DefaultPair.Key, FGameplayAbilitySpec(DefaultPair.Value, GetCharacterLevel(), INDEX_NONE, this));
		}
	}

	// 再用从已经装备的道具中获得的能力覆盖
	// Now potentially override with inventory
	if (InventorySource)
	{
		const TMap<FRPGItemSlot, URPGItem*>& SlottedItemMap = InventorySource->GetSlottedItemMap();

		for (const TPair<FRPGItemSlot, URPGItem*>& ItemPair : SlottedItemMap)
		{
			URPGItem* SlottedItem = ItemPair.Value;

			// 默认使用 Character 的 Level 作为 Ability 的 Level
			// Use the character level as default
			int32 AbilityLevel = GetCharacterLevel();

			if (SlottedItem && SlottedItem->ItemType.GetName() == FName(TEXT("Weapon")))
			{
				// 来自武器的 Ability 的 Level 保存在武器中
				// Override the ability level to use the data from the slotted item
				AbilityLevel = SlottedItem->AbilityLevel;
			}

			if (SlottedItem && SlottedItem->GrantedAbility)
			{
				// Source Object 是这个 Item
				// This will override anything from default
				SlottedAbilitySpecs.Add(ItemPair.Key, FGameplayAbilitySpec(SlottedItem->GrantedAbility, AbilityLevel, INDEX_NONE, SlottedItem));
			}
		}
	}
}

/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
// DECLARE_CYCLE_STAT(TEXT("AddSlottedGameplayAbilities"), STAT_AddSlottedGameplayAbilities, STATGROUP_ARPGCharacterBase);
/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

void ARPGCharacterBase::AddSlottedGameplayAbilities()
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// const uint32 BeginTime = FPlatformTime::Cycles();
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	// 获得所有已经装备的 Ability
	TMap<FRPGItemSlot, FGameplayAbilitySpec> SlottedAbilitySpecs;
	FillSlottedAbilitySpecs(SlottedAbilitySpecs);

	// SlottedAbilitySpecs 补充到 SlottedAbilities 中
	// Now add abilities if needed
	for (const TPair<FRPGItemSlot, FGameplayAbilitySpec>& SpecPair : SlottedAbilitySpecs)
	{
		FGameplayAbilitySpecHandle& SpecHandle = SlottedAbilities.FindOrAdd(SpecPair.Key);

		if (!SpecHandle.IsValid())
		{
			SpecHandle = AbilitySystemComponent->GiveAbility(SpecPair.Value);
		}
	}

	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// const uint32 EndTime = FPlatformTime::Cycles();
	// SET_CYCLE_COUNTER(STAT_AddSlottedGameplayAbilities, BeginTime - EndTime);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/
}

void ARPGCharacterBase::RemoveSlottedGameplayAbilities(const bool bRemoveAll)
{
	TMap<FRPGItemSlot, FGameplayAbilitySpec> SlottedAbilitySpecs;

	// 如果不需要移除全部已经装备的能力，就需要获得最新的数据来更新 SlottedAbilities
	// 即 SlottedAbilitySpecs 代表最新的数据，SlottedAbilities 是可能过期的数据
	if (!bRemoveAll)
	{
		// 获得已经装备的全部能力
		// Fill in map so we can compare
		FillSlottedAbilitySpecs(SlottedAbilitySpecs);
	}

	// 这里假定了实际装备的 Ability 一定存在于 SlottedAbilities 中，即 SlottedAbilities 可能会多但一定不会少
	// 这很合理，因为这个函数是唯一会移除 SlottedAbilities 中的元素的地方

	for (TPair<FRPGItemSlot, FGameplayAbilitySpecHandle>& ExistingPair : SlottedAbilities)
	{
		const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(ExistingPair.Value);
		bool bShouldRemove = bRemoveAll || !FoundSpec;

		if (!bShouldRemove)
		{
			// 走到这里说明 !bRemoveAll && FoundSpec ，那么就需要判断数据是否过期
			// Need to check desired ability specs, if we got here FoundSpec is valid
			FGameplayAbilitySpec* DesiredSpec = SlottedAbilitySpecs.Find(ExistingPair.Key);

			// 最新的数据中没有 || 装备的能力不同 || 来自不同的 Item / 一个来自 Item 一个来自自己（DefaultSlottedAbilities）
			if (!DesiredSpec || DesiredSpec->Ability != FoundSpec->Ability || DesiredSpec->SourceObject != FoundSpec->SourceObject)
			{
				bShouldRemove = true;
			}
		}
		
		if (bShouldRemove)
		{	
			if (FoundSpec)
			{
				// Need to remove registered ability
				AbilitySystemComponent->ClearAbility(ExistingPair.Value);
			}

			// Make sure handle is cleared even if ability wasn't found
			ExistingPair.Value = FGameplayAbilitySpecHandle();
		}
	}
}

void ARPGCharacterBase::PossessedBy(AController* NewController)
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// double ThisTime = 0;
	// {
	// 	SCOPE_SECONDS_COUNTER(ThisTime);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

		Super::PossessedBy(NewController);

		// 设置 Inventory Source ，对于 AIController 会失败，因为没有实现 IRPGInventoryInterface
		// Try setting the inventory source, this will fail for AI
		InventorySource = NewController;

		if (InventorySource)
		{
			InventoryUpdateHandle = InventorySource->GetSlottedItemChangedDelegate().AddUObject(this, &ARPGCharacterBase::OnItemSlotChanged);
			InventoryLoadedHandle = InventorySource->GetInventoryLoadedDelegate().AddUObject(this, &ARPGCharacterBase::RefreshSlottedGameplayAbilities);
		}

		// Initialize our abilities
		if (AbilitySystemComponent)
		{
			// 设置 ACS 的 Actor
			// OwnerActor ：逻辑上拥有这个 ASC ，可以是 PlayerState
			// AvatarActor ：ASC 作用于这个 Actor ，一般是 Pawn
			AbilitySystemComponent->InitAbilityActorInfo(this, this);
			AddStartupGameplayAbilities();
		}
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// }
	// UE_LOG(LogTemp, Log, TEXT("ARPGCharacterBase::PossessedBy %.2f"), ThisTime);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/
}

void ARPGCharacterBase::UnPossessed()
{
	// Unmap from inventory source
	if (InventorySource && InventoryUpdateHandle.IsValid())
	{
		/** Removes a delegate instance from this multi-cast delegate's invocation list (performance is O(N)) */
		InventorySource->GetSlottedItemChangedDelegate().Remove(InventoryUpdateHandle);
		/** Clear handle to indicate it is no longer bound */
		InventoryUpdateHandle.Reset();

		InventorySource->GetInventoryLoadedDelegate().Remove(InventoryLoadedHandle);
		InventoryLoadedHandle.Reset();
	}

	InventorySource = nullptr;
}

/**
 * @brief 在 APawn 中声明的 Controller 属性设置了这个 RepNotify
 */
void ARPGCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Our controller changed, must update ActorInfo on AbilitySystemComponent
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RefreshAbilityActorInfo();
	}
}

void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPGCharacterBase, CharacterLevel);
}

float ARPGCharacterBase::GetHealth() const
{
	if (!AttributeSet)
	return 1.f;

	return AttributeSet->GetHealth();
}

float ARPGCharacterBase::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

float ARPGCharacterBase::GetMana() const
{
	return AttributeSet->GetMana();
}

float ARPGCharacterBase::GetMaxMana() const
{
	return AttributeSet->GetMaxMana();
}

float ARPGCharacterBase::GetMoveSpeed() const
{
	return AttributeSet->GetMoveSpeed();
}

int32 ARPGCharacterBase::GetCharacterLevel() const
{
	return CharacterLevel;
}

bool ARPGCharacterBase::SetCharacterLevel(int32 NewLevel)
{
	if (CharacterLevel != NewLevel && NewLevel > 0)
	{
		// Our level changed so we need to refresh abilities
		RemoveStartupGameplayAbilities();
		CharacterLevel = NewLevel;
		AddStartupGameplayAbilities();

		return true;
	}
	return false;
}

bool ARPGCharacterBase::ActivateAbilitiesWithItemSlot(const FRPGItemSlot& ItemSlot, const bool bAllowRemoteActivation)
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// SCOPE_LOG_TIME_FUNC();
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	const FGameplayAbilitySpecHandle* FoundHandle = SlottedAbilities.Find(ItemSlot);

	if (FoundHandle && AbilitySystemComponent)
	{
		return AbilitySystemComponent->TryActivateAbility(*FoundHandle, bAllowRemoteActivation);
	}

	return false;
}

void ARPGCharacterBase::GetActiveAbilitiesWithItemSlot(const FRPGItemSlot& ItemSlot, TArray<URPGGameplayAbility*>& ActiveAbilities)
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// static FTotalTimeAndCount GetActiveAbilitiesWithItemSlotTime;
	// SCOPE_LOG_TIME_FUNC_WITH_GLOBAL(&GetActiveAbilitiesWithItemSlotTime);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	const FGameplayAbilitySpecHandle* FoundHandle = SlottedAbilities.Find(ItemSlot);

	if (FoundHandle && AbilitySystemComponent)
	{
		const FGameplayAbilitySpec* FoundSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(*FoundHandle);

		if (FoundSpec)
		{
			TArray<UGameplayAbility*> AbilityInstances = FoundSpec->GetAbilityInstances();

			// Find all ability instances executed from this slot
			for (UGameplayAbility* ActiveAbility : AbilityInstances)
			{
				// IMPORTANT 这说明 FGameplayAbilitySpec 中保存的所有 AbilityInstance 就是 ActiveAbilities ，类型是 UGameplayAbility
				ActiveAbilities.Add(Cast<URPGGameplayAbility>(ActiveAbility));
			}
		}
	}
}

bool ARPGCharacterBase::ActivateAbilitiesWithTags(FGameplayTagContainer AbilityTags, bool bAllowRemoteActivation)
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags, bAllowRemoteActivation);
	}

	return false;
}

void ARPGCharacterBase::GetActiveAbilitiesWithTags(FGameplayTagContainer AbilityTags, TArray<URPGGameplayAbility*>& ActiveAbilities)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetActiveAbilitiesWithTags(AbilityTags, ActiveAbilities);
	}
}

bool ARPGCharacterBase::GetCooldownRemainingForTag(FGameplayTagContainer CooldownTags, float& TimeRemaining, float& CooldownDuration)
{
	/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
	// static FTotalTimeAndCount GetCooldownRemainingForTagTime;
	// SCOPE_LOG_TIME("ARPGCharacterBase::GetCooldownRemainingForTag", &GetCooldownRemainingForTagTime);
	/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

	if (AbilitySystemComponent && CooldownTags.Num() > 0)
	{
		TimeRemaining = 0.f;
		CooldownDuration = 0.f;

		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
		TArray< TPair<float, float> > DurationAndTimeRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);
		if (DurationAndTimeRemaining.Num() > 0)
		{
			int32 BestIdx = 0;
			float LongestTime = DurationAndTimeRemaining[0].Key;
			for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
			{
				// 找到最长的剩余 CD 时间
				if (DurationAndTimeRemaining[Idx].Key > LongestTime)
				{
					LongestTime = DurationAndTimeRemaining[Idx].Key;
					BestIdx = Idx;
				}
			}

			TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

			return true;
		}
	}
	return false;
}

void ARPGCharacterBase::HandleDamage(float DamageAmount, const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags, ARPGCharacterBase* InstigatorPawn, AActor* DamageCauser)
{
	OnDamaged(DamageAmount, HitInfo, DamageTags, InstigatorPawn, DamageCauser);	
}

void ARPGCharacterBase::HandleHealthChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// We only call the BP callback if this is not the initial ability setup
	if (bAbilitiesInitialized)
	{
		/*---------------------------------------------------------- STAT BEGIN --------------------------------------------------------------*/
		// INC_DWORD_STAT(STAT_HandleHealthChanged)
		/*----------------------------------------------------------- STAT END ---------------------------------------------------------------*/

		OnHealthChanged(DeltaValue, EventTags);
	}
}

void ARPGCharacterBase::HandleManaChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	if (bAbilitiesInitialized)
	{
		OnManaChanged(DeltaValue, EventTags);
	}
}

void ARPGCharacterBase::HandleMoveSpeedChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// Update the character movement's walk speed
	GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();

	if (bAbilitiesInitialized)
	{
		OnMoveSpeedChanged(DeltaValue, EventTags);
	}
}

FGenericTeamId ARPGCharacterBase::GetGenericTeamId() const
{
	static const FGenericTeamId PlayerTeam(0);
	static const FGenericTeamId AITeam(1);
	return Cast<APlayerController>(GetController()) ? PlayerTeam : AITeam;
}
