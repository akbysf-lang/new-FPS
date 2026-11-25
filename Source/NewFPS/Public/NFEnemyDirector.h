#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubclassOf.h"
#include "NFEnemyDirector.generated.h"

class AActor;
class ANFEnemyBase;
class UObject;

/**
 * Server-side bookkeeping shared by all enemies: who is alive, and attack tokens.
 *
 * Tokens keep fights readable (prototype-spec.md 5): only a limited number of enemies may attack at once, the
 * rest circle or wait. A token belongs to a Pool (e.g. "ScuttlerMelee") and a Scope: the target player for
 * per-player limits, or null for a global limit. The caller passes the limit, so it stays in the enemy's data.
 */
UCLASS()
class NEWFPS_API UNFEnemyDirector : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * True when Holder holds (or now gets) a token of Pool/Scope. Asking again while holding is free.
	 * At most MaxHolders enemies hold a token of the same Pool/Scope at a time.
	 */
	bool RequestToken(FName Pool, const UObject* Scope, AActor* Holder, int32 MaxHolders);

	/** Give back every token Holder has. */
	void ReleaseTokens(const AActor* Holder);

	void RegisterEnemy(ANFEnemyBase* Enemy);
	void UnregisterEnemy(ANFEnemyBase* Enemy);

	/** Living enemies of EnemyClass (or a subclass). */
	int32 CountAlive(TSubclassOf<ANFEnemyBase> EnemyClass) const;

	/** Players in the session (at least 1). */
	int32 GetPlayerCount() const;

	/** Push every living enemy within Radius of Origin away from it by about Distance cm (solo spare line). */
	void KnockbackEnemiesAround(const FVector& Origin, float Radius, float Distance);

private:
	struct FToken
	{
		FName Pool;
		TWeakObjectPtr<const UObject> Scope;
		TWeakObjectPtr<AActor> Holder;
		bool bGlobalScope = false;
	};

	void PruneTokens();

	TArray<FToken> Tokens;
	TArray<TWeakObjectPtr<ANFEnemyBase>> Enemies;
};
