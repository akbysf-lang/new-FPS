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
	/** Waist/chest deep: weapons cannot be used (players), automatons short out (AI). */
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
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	FVector ShotDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AController> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> Causer = nullptr;
};
