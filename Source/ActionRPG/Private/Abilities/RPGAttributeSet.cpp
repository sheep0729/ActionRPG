// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/RPGAttributeSet.h"
#include "Abilities/RPGAbilitySystemComponent.h"
#include "RPGCharacterBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

URPGAttributeSet::URPGAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, Mana(0.f)
	, MaxMana(0.f)
	, AttackPower(1.0f)
	, DefensePower(1.0f)
	, MoveSpeed(1.0f)
	, Damage(0.0f)
{
}

void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URPGAttributeSet, Health);
	DOREPLIFETIME(URPGAttributeSet, MaxHealth);
	DOREPLIFETIME(URPGAttributeSet, Mana);
	DOREPLIFETIME(URPGAttributeSet, MaxMana);
	DOREPLIFETIME(URPGAttributeSet, AttackPower);
	DOREPLIFETIME(URPGAttributeSet, DefensePower);
	DOREPLIFETIME(URPGAttributeSet, MoveSpeed);
}

void URPGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	/** This is a helper macro that can be used in RepNotify functions to handle attributes that will be predictively modified by clients. */
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Health, OldValue);
}

void URPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxHealth, OldValue);
}

void URPGAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Mana, OldValue);
}

void URPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxMana, OldValue);
}

void URPGAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, AttackPower, OldValue);
}

void URPGAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, DefensePower, OldValue);
}

void URPGAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MoveSpeed, OldValue);
}

void URPGAttributeSet::AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		const float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		// 不使用 GE ，直接改变属性的值。所以也不会调用下面的 Pre / Post 方法，不会检查 tag 和应用条件，没有  predict / roll back 功能。
		// This should only be used in cases where applying a real GameplayEffectSpec is too slow or not possible.
		// 和 ApplyModToAttribute 的区别是不检查 IsOwnerActorAuthoritative() ，因为这是一个单机游戏，显然不满足 IsOwnerActorAuthoritative 的条件。
		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void URPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
	}
}

// FGameplayEffectModCallbackData 是在 Mod 的回调中使用的数据结构
// EffectSpec;		// The spec that the mod came from
// EvaluatedData;	// The 'flat'/computed data to be applied to the target
// Target;		    // Target we intend to apply to
void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// FGameplayEffectContextHandle 内部属性只有一个类型为 FGameplayEffectContext 的 TSharedPtr ，这个 Handle 本身是一个简单的 Wrapper ，在执行所有方法前都检查了 TSharedPtr 是否 IsValid() 。
	// 还有一个自定义的序列化方法 NetSerialize 。
	// FGameplayEffectContext 是在 GE 执行期间传递的数据结构，保存了 GE 执行时需要的数据。
	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	// Instigator 是指发起这个 GE 的 Actor 
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	// CapturedSourceTags 的类型是 FTagContainerAggregator ，用来在 GE 执行的过程中合并来自不同来源的标签。
	// FGameplayTagContainer 是 Tag 的容器，内部用 TArray 保存 Tag 。
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	// Compute the delta between old and new, if it is available
	float DeltaValue = 0;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		// If this was additive, store the raw delta value to be passed along later
		DeltaValue = Data.EvaluatedData.Magnitude; // Magnitude 是指 GE 执行后计算出来的属性的值，但还没有 Clamp 。
	}

	// 从 Target.AbilityActorInfo 中获得一些 Actor 的信息
	// AbilityActorInfo 缓存了 GA 常用的角色信息：movement component, mesh component, anim instance 等，但很多都可能是 null 。
	// 这个函数是 GE 执行的后处理，所以应该调用的是 Target 的 AttributeSet 上的方法，也就是说 Target 是当前的 AttributeSet 的 Owner 。 Get the Target actor, which should be our owner
	ARPGCharacterBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		AActor* TargetActor = nullptr;
		AController* TargetController = nullptr;
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get(); // AvatarActor 是指用于表现身体的 Actor ，用于定位和动画
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<ARPGCharacterBase>(TargetActor);
	}

	// GE 修改的属性是 Damage 时的后处理
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// 根据下面 HandleDamage 的调用：
		//   SourceCharacter 是指发起这个伤害的 Character
		//   SourceActor 是指直接造成伤害的 Actor ，可能是武器或者发射物
		AActor* SourceActor = nullptr;
		ARPGCharacterBase* SourceCharacter = nullptr;
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			AController* SourceController = nullptr;
			SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
			SourceController = Source->AbilityActorInfo->PlayerController.Get();
			
			// 如果 SourceController 是 null ，可以用 SourceActor 来确定 SourceController 。
			if (SourceController == nullptr && SourceActor != nullptr) // 没有 PlayerController 但是有 AvatarActor
			{
				if (APawn* Pawn = Cast<APawn>(SourceActor)) // 从 Pawn 中获取 Controller
				{
					SourceController = Pawn->GetController();
				}
			}
			
			// TODO 这里不是很清楚在干什么，因为上面的 TargetCharacter 就是用 TargetActor 直接获得的，这里却要用 SourceController 来确定 SourceCharacter ，而 SourceController 很有可能是由 SourceActor 确定的。
			// Use the controller to find the source pawn
			if (SourceController)
			{
				SourceCharacter = Cast<ARPGCharacterBase>(SourceController->GetPawn());
			}
			else
			{
				SourceCharacter = Cast<ARPGCharacterBase>(SourceActor);
			}
			
			if (Context.GetEffectCauser()) // EffectCauser 是指实际上造成伤害的 Actor ，可能是 a weapon or projectile 。
			{
				// 根据下面对 HandleDamage 的调用，这里获得的 EffectCauser 应该是 SourceActor 优先级更高的值。
				SourceActor = Context.GetEffectCauser();
			}
		}

		// 取出 HitResult
		// Try to extract a hit result
		FHitResult HitResult;
		if (Context.GetHitResult())
		{
			HitResult = *Context.GetHitResult();
		}

		// 取出并清空 Damage 属性的值
		// Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);

		if (LocalDamageDone > 0)
		{
			// Apply the health change and then clamp it
			const float OldHealth = GetHealth();
			SetHealth(FMath::Clamp(OldHealth - LocalDamageDone, 0.0f, GetMaxHealth()));

			if (TargetCharacter)
			{
				// This is proper damage
				TargetCharacter->HandleDamage(LocalDamageDone, HitResult, SourceTags, SourceCharacter, SourceActor);

				// Call for all health changes
				TargetCharacter->HandleHealthChanged(-LocalDamageDone, SourceTags);
			}
		}
	}

	// GE 修改的属性是 Health 时的后处理
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes such as from healing or direct modifiers
		// First clamp it
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		if (TargetCharacter)
		{
			// Call for all health changes
			TargetCharacter->HandleHealthChanged(DeltaValue, SourceTags);
		}
	}

	// GE 修改的属性是 Mana 时的后处理
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Clamp mana
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));

		if (TargetCharacter)
		{
			// Call for all mana changes
			TargetCharacter->HandleManaChanged(DeltaValue, SourceTags);
		}
	}

	// GE 修改的属性是 MoveSpeed 时的后处理
	else if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		if (TargetCharacter)
		{
			// Call for all movespeed changes
			TargetCharacter->HandleMoveSpeedChanged(DeltaValue, SourceTags);
		}
	}
}
