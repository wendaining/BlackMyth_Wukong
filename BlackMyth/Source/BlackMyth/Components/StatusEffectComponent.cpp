// 状态效果管理组件实现

#include "StatusEffectComponent.h"
#include "../StatusEffect/StatusEffectBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化动态材质
	SetupDynamicMaterials();
}

void UStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 移除所有效果
	RemoveAllEffects();

	Super::EndPlay(EndPlayReason);
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 没有激活效果时跳过
	if (ActiveEffects.Num() == 0)
	{
		return;
	}

	// 倒序遍历，方便移除过期效果
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		UStatusEffectBase* Effect = ActiveEffects[i];
		if (!Effect)
		{
			ActiveEffects.RemoveAt(i);
			continue;
		}

		// 更新效果
		Effect->OnTick(DeltaTime);

		// 广播更新事件
		OnEffectUpdated.Broadcast(Effect->GetEffectType(), Effect->GetRemainingTime());

		// 检查是否过期
		if (Effect->IsExpired())
		{
			// 调用效果的移除回调
			Effect->OnRemoved();

			// 获取类型用于广播
			EStatusEffectType EffectType = Effect->GetEffectType();

			// 从列表中移除
			ActiveEffects.RemoveAt(i);

			// 广播移除事件
			OnEffectRemoved.Broadcast(EffectType);

			UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: Effect [%s] expired and removed"),
				*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)));
		}
	}

	// 更新材质视觉效果
	UpdateMaterialEffects();
}

UStatusEffectBase* UStatusEffectComponent::ApplyEffect(TSubclassOf<UStatusEffectBase> EffectClass, AActor* InInstigator, float Duration)
{
	if (!EffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusEffectComponent::ApplyEffect - EffectClass is null!"));
		return nullptr;
	}

	// 获取效果类型（从 CDO 获取）
	UStatusEffectBase* CDO = EffectClass->GetDefaultObject<UStatusEffectBase>();
	if (!CDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusEffectComponent::ApplyEffect - Failed to get CDO!"));
		return nullptr;
	}

	EStatusEffectType EffectType = CDO->GetEffectType();

	// 检查是否已存在同类型效果
	UStatusEffectBase* ExistingEffect = GetEffect(EffectType);
	if (ExistingEffect)
	{
		// 已存在同类型效果，刷新持续时间
		ExistingEffect->RefreshDuration(Duration);

		// 广播事件（刷新也视为施加）
		OnEffectApplied.Broadcast(EffectType, Duration);

		UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: Effect [%s] refreshed to %.1f seconds"),
			*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
			Duration);

		return ExistingEffect;
	}

	// 创建新效果实例
	UStatusEffectBase* NewEffect = NewObject<UStatusEffectBase>(this, EffectClass);
	if (!NewEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("StatusEffectComponent::ApplyEffect - Failed to create effect instance!"));
		return nullptr;
	}

	// 初始化效果
	NewEffect->SetOwnerComponent(this);
	NewEffect->Initialize(GetOwner(), InInstigator, Duration);

	// 添加到列表
	ActiveEffects.Add(NewEffect);

	// 调用效果的施加回调
	NewEffect->OnApplied();

	// 更新材质视觉效果
	UpdateMaterialEffects();

	// 广播施加事件
	OnEffectApplied.Broadcast(EffectType, Duration);

	UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: Effect [%s] applied for %.1f seconds"),
		*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
		Duration);

	return NewEffect;
}

bool UStatusEffectComponent::RemoveEffect(EStatusEffectType EffectType)
{
	for (int32 i = 0; i < ActiveEffects.Num(); ++i)
	{
		if (ActiveEffects[i] && ActiveEffects[i]->GetEffectType() == EffectType)
		{
			// 调用效果的移除回调
			ActiveEffects[i]->OnRemoved();

			// 从列表中移除
			ActiveEffects.RemoveAt(i);

			// 更新材质视觉效果
			UpdateMaterialEffects();

			// 广播移除事件
			OnEffectRemoved.Broadcast(EffectType);

			UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: Effect [%s] manually removed"),
				*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)));

			return true;
		}
	}

	return false;
}

void UStatusEffectComponent::RemoveAllEffects()
{
	for (UStatusEffectBase* Effect : ActiveEffects)
	{
		if (Effect)
		{
			Effect->OnRemoved();
			OnEffectRemoved.Broadcast(Effect->GetEffectType());
		}
	}

	ActiveEffects.Empty();

	// 重置材质
	ResetMaterialEffects();

	UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: All effects removed"));
}

bool UStatusEffectComponent::HasEffect(EStatusEffectType EffectType) const
{
	return GetEffect(EffectType) != nullptr;
}

UStatusEffectBase* UStatusEffectComponent::GetEffect(EStatusEffectType EffectType) const
{
	for (UStatusEffectBase* Effect : ActiveEffects)
	{
		if (Effect && Effect->GetEffectType() == EffectType)
		{
			return Effect;
		}
	}

	return nullptr;
}

void UStatusEffectComponent::SetupDynamicMaterials()
{
	if (bMaterialsInitialized)
	{
		return;
	}

	// 获取角色的骨骼网格体
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	CachedMesh = Mesh;

	// 为每个材质槽创建动态材质实例
	int32 NumMaterials = Mesh->GetNumMaterials();
	DynamicMaterials.Empty();
	OriginalMaterials.Empty();

	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* OriginalMaterial = Mesh->GetMaterial(i);
		if (OriginalMaterial)
		{
			// 备份原始材质
			OriginalMaterials.Add(i, OriginalMaterial);

			// 创建动态材质实例
			UMaterialInstanceDynamic* DynMat = Mesh->CreateDynamicMaterialInstance(i, OriginalMaterial);
			if (DynMat)
			{
				DynamicMaterials.Add(DynMat);
			}
		}
	}

	bMaterialsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("StatusEffectComponent: Dynamic materials initialized (%d materials)"), DynamicMaterials.Num());
}

void UStatusEffectComponent::UpdateMaterialEffects()
{
	if (!bMaterialsInitialized || DynamicMaterials.Num() == 0)
	{
		return;
	}

	// 如果没有激活效果，重置材质
	if (ActiveEffects.Num() == 0)
	{
		ResetMaterialEffects();
		return;
	}

	// 计算混合后的颜色（简单叠加/取最强效果）
	FLinearColor FinalTintColor = FLinearColor::White;
	FLinearColor FinalEmissiveColor = FLinearColor::Black;
	float FinalEmissiveIntensity = 0.0f;
	float MaxIntensity = 0.0f;

	for (UStatusEffectBase* Effect : ActiveEffects)
	{
		if (Effect && Effect->HasVisualEffect())
		{
			// 使用最强效果的颜色
			if (Effect->GetEmissiveIntensity() > MaxIntensity)
			{
				MaxIntensity = Effect->GetEmissiveIntensity();
				FinalTintColor = Effect->GetTintColor();
				FinalEmissiveColor = Effect->GetEmissiveColor();
				FinalEmissiveIntensity = Effect->GetEmissiveIntensity();
			}
		}
	}

	// 应用到所有动态材质
	for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
	{
		if (DynMat)
		{
			// 设置 Tint 颜色
			DynMat->SetVectorParameterValue(TEXT("TintColor"), FinalTintColor);
			DynMat->SetScalarParameterValue(TEXT("TintIntensity"), FinalEmissiveIntensity > 0.0f ? 0.3f : 0.0f);

			// 设置自发光
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), FinalEmissiveColor);
			DynMat->SetScalarParameterValue(TEXT("EmissiveIntensity"), FinalEmissiveIntensity);
		}
	}
}

void UStatusEffectComponent::ResetMaterialEffects()
{
	if (!bMaterialsInitialized)
	{
		return;
	}

	// 重置所有动态材质的参数
	for (UMaterialInstanceDynamic* DynMat : DynamicMaterials)
	{
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("TintColor"), FLinearColor::White);
			DynMat->SetScalarParameterValue(TEXT("TintIntensity"), 0.0f);
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			DynMat->SetScalarParameterValue(TEXT("EmissiveIntensity"), 0.0f);
		}
	}
}

void UStatusEffectComponent::RemoveEffectInternal(int32 Index)
{
	if (ActiveEffects.IsValidIndex(Index))
	{
		UStatusEffectBase* Effect = ActiveEffects[Index];
		if (Effect)
		{
			Effect->OnRemoved();
		}
		ActiveEffects.RemoveAt(Index);
	}
}
