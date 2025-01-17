// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemActor/ASDroppedItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Item/ASItem.h"
#include "Item/ASWeapon.h"
#include "Item/ASArmor.h"
#include "Item/ASAmmo.h"
#include "Item/ASHealingKit.h"
#include "ASAssetManager.h"
#include "DataAssets/ItemDataAssets/ASWeaponDataAsset.h"
#include "DataAssets/ItemDataAssets/ASArmorDataAsset.h"
#include "DataAssets/ItemDataAssets/ASAmmoDataAsset.h"
#include "DataAssets/ItemDataAssets/ASHealingKitDataAsset.h"
#include "GameMode/ASItemFactoryComponent.h"

AASDroppedItemActor::AASDroppedItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetCanBeDamaged(false);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("DroppedItem"));
	Collision->SetGenerateOverlapEvents(true);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshComp->SetIsReplicated(true);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshComp->SetIsReplicated(true);

	RootComponent = Collision;
	SkeletalMeshComp->SetupAttachment(RootComponent);
	StaticMeshComp->SetupAttachment(RootComponent);
}

void AASDroppedItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASDroppedItemActor, ASItems);
}

void AASDroppedItemActor::SetSkeletalMesh(USkeletalMesh* InSkelMesh)
{
	if (SkeletalMeshComp != nullptr)
	{
		SkeletalMeshComp->SetSkeletalMesh(InSkelMesh);
	}
}

void AASDroppedItemActor::SetStaticMesh(UStaticMesh* InStaticMesh)
{
	if (StaticMeshComp != nullptr)
	{
		StaticMeshComp->SetStaticMesh(InStaticMesh);
	}
}

TArray<TWeakObjectPtr<UASItem>> AASDroppedItemActor::GetItems() const
{
	TArray<TWeakObjectPtr<UASItem>> Result;

	for (auto& ASItem : ASItems)
	{
		if (IsValid(ASItem))
		{
			Result.Emplace(ASItem);
		}
	}

	return Result;
}

void AASDroppedItemActor::AddItem(UASItem* InItem)
{
	if (InItem == nullptr)
	{
		AS_LOG_S(Error);
		return;
	}

	InItem->SetOwner(this);

	if (GetLifeSpan() > 0.0f)
	{
		SetSelfDestroy(0.0f);		// Cancel self destory
	}

	ASItems.Emplace(InItem);
}

bool AASDroppedItemActor::RemoveItem(UASItem* InItem)
{
	if (InItem == nullptr)
	{
		AS_LOG_S(Error);
		return false;
	}

	if (ASItems.Remove(InItem) == 0)
	{
		AS_LOG_S(Error);
		return false;
	}

	InItem->SetOwner(nullptr);

	if (ASItems.Num() == 0)
	{
		SetSelfDestroy(3.0f);
	}

	return true;
}

void AASDroppedItemActor::SetSelfDestroy(float InLifeSpan)
{
	SetActorHiddenInGame(InLifeSpan > 0.0f);
	SetLifeSpan(InLifeSpan);
}

void AASDroppedItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		for (auto& ItemDataAssetPair : DropItemDataAssetMap)
		{
			FPrimaryAssetId& ItemDataAssetId = ItemDataAssetPair.Key;
			int32 Count = ItemDataAssetPair.Value;

			if (!ItemDataAssetId.IsValid())
				continue;

			UASItemDataAsset* ItemDataAsset = UASAssetManager::Get().GetDataAsset<UASItemDataAsset>(ItemDataAssetId);
			if (ItemDataAsset == nullptr)
				continue;

			ASItems.Emplace(UASItemFactoryComponent::NewASItem(GetWorld(), this, ItemDataAsset, Count));
		}
	}	
}

void AASDroppedItemActor::OnRep_ASItems(TArray<UASItem*>& OldItems)
{
	if (OnRemoveItemEvent.IsBound())
	{
		for (auto& Item : OldItems)
		{
			if (!ASItems.Contains(Item))
			{
				TWeakObjectPtr<UASItem> ItemPtr = MakeWeakObjectPtr(Item);
				OnRemoveItemEvent.Broadcast(ItemPtr);
			}
		}
	}
}
