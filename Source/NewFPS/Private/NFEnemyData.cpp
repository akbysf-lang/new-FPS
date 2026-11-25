#include "NFEnemyData.h"
#include "NFEnemyProjectile.h"

UNFAttendantData::UNFAttendantData()
{
	ProjectileClass = ANFEnemyProjectile::StaticClass();
	// 1 player: 2 - 2 players: 3 - 3 and 4 players: 4.
	FireTokensByPlayerCount = { 2, 3, 4, 4 };
}
