#pragma once

#include "CoreMinimal.h"
#include "NFTypes.generated.h"

class AActor;
class AController;

/** Body region used by the damage pipeline. */
UENUM(BlueprintType)
enum class ENFHitZone : uint8
{
	Body,
	Head,
	Limb
};

/** How deep an actor stands in the tide. Ordered: a higher value is always wetter. */
UENUM(BlueprintType)
enum class ENFWetness : uint8
{
	Dry,
	/** Knee deep: movement slows down. */
	Knee,
	/**
	 * Waist deep (WaistDepth, 100 cm and up). Players: the Gaffer goes down and the Purser is hip-fire only.
	 * Chest is not a state but a depth inside Waist (UNFTideData::ChestDepth, 135 cm): from there no firearm
	 * works at all, only the Kedge. AI automatons short out at waist depth.
	 */
	Waist,
	/** Head under water: drowning. */
	Overhead
};

/** Label of a tide table entry. The water height itself comes from the entry's start/end levels. */
UENUM(BlueprintType)
enum class ENFTidePhase : uint8
{
	/** Slack at low water. */
	LowSlack,
	/** Water rising. */
	Flood,
	/** Slack at high water. */
	HighSlack,
	/** Water falling. */
	Ebb
};

/** Where a hit came from. Enemies use it to pick their reaction (stagger, knockback). */
UENUM(BlueprintType)
enum class ENFDamageType : uint8
{
	Bullet,
	/** Player melee (Kedge). */
	Melee,
	EnemyMelee,
	EnemyProjectile,
	Drown,
	/** Rules/script (short circuit, kill volume). */
	Script
};

/** Player life cycle in co-op (spec 6.2). */
UENUM(BlueprintType)
enum class ENFLifeState : uint8
{
	Alive,
	/** At 0 health: crawling, cannot fight, can be revived. */
	Downed,
	/** Bled out: pulled up by the lifeline, waiting to come back down. */
	Out
};

UENUM(BlueprintType)
enum class ENFMissionResult : uint8
{
	InProgress,
	Success,
	Failed
};

/** Everything the single damage pipeline (UNFHealthComponent::ApplyDamage) needs. */
USTRUCT(BlueprintType)
struct FNFDamageInfo
{
	GENERATED_BODY()

	/** Weapon base damage, before any multiplier. */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.f;

	/** Distance falloff multiplier (1 = no falloff). */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float DistanceMultiplier = 1.f;

	/** Region multiplier (head/limb) chosen by the weapon for Zone. */
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float ZoneMultiplier = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	ENFHitZone Zone = ENFHitZone::Body;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	ENFDamageType DamageType = ENFDamageType::Bullet;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	FVector ShotDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AController> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> Causer = nullptr;
};
