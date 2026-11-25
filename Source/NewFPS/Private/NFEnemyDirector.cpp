#include "NFEnemyDirector.h"
#include "NFEnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void UNFEnemyDirector::PruneTokens()
{
	Tokens.RemoveAll([](const FToken& Token)
	{
		const ANFEnemyBase* Enemy = Cast<ANFEnemyBase>(Token.Holder.Get());
		// A token dies with its holder, and with the player it was scoped to.
		return !Token.Holder.IsValid() || (Enemy && !Enemy->IsAlive()) || (!Token.bGlobalScope && !Token.Scope.IsValid());
	});
}

bool UNFEnemyDirector::RequestToken(FName Pool, const UObject* Scope, AActor* Holder, int32 MaxHolders)
{
	if (!Holder)
	{
		return false;
	}

	PruneTokens();

	int32 InUse = 0;
	for (const FToken& Token : Tokens)
	{
		if (Token.Pool != Pool || Token.Scope.Get() != Scope)
		{
			continue;
		}
		if (Token.Holder.Get() == Holder)
		{
			return true;
		}
		++InUse;
	}

	if (InUse >= MaxHolders)
	{
		return false;
	}

	FToken NewToken;
	NewToken.Pool = Pool;
	NewToken.Scope = Scope;
	NewToken.Holder = Holder;
	NewToken.bGlobalScope = (Scope == nullptr);
	Tokens.Add(NewToken);
	return true;
}

void UNFEnemyDirector::ReleaseTokens(const AActor* Holder)
{
	Tokens.RemoveAll([Holder](const FToken& Token) { return Token.Holder.Get() == Holder; });
}

void UNFEnemyDirector::RegisterEnemy(ANFEnemyBase* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(TWeakObjectPtr<ANFEnemyBase>(Enemy));
	}
}

void UNFEnemyDirector::UnregisterEnemy(ANFEnemyBase* Enemy)
{
	Enemies.Remove(TWeakObjectPtr<ANFEnemyBase>(Enemy));
	ReleaseTokens(Enemy);
}

int32 UNFEnemyDirector::CountAlive(TSubclassOf<ANFEnemyBase> EnemyClass) const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ANFEnemyBase>& WeakEnemy : Enemies)
	{
		const ANFEnemyBase* Enemy = WeakEnemy.Get();
		if (Enemy && Enemy->IsAlive() && (!EnemyClass || Enemy->IsA(EnemyClass)))
		{
			++Count;
		}
	}
	return Count;
}

int32 UNFEnemyDirector::GetPlayerCount() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? FMath::Max(GameState->PlayerArray.Num(), 1) : 1;
}

void UNFEnemyDirector::KnockbackEnemiesAround(const FVector& Origin, float Radius, float Distance)
{
	for (const TWeakObjectPtr<ANFEnemyBase>& WeakEnemy : Enemies)
	{
		ANFEnemyBase* Enemy = WeakEnemy.Get();
		if (Enemy && Enemy->IsAlive() && FVector::DistSquared(Enemy->GetActorLocation(), Origin) <= FMath::Square(Radius))
		{
			Enemy->ApplyKnockback(Origin, Distance);
		}
	}
}
