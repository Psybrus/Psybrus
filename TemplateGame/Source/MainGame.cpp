#include "Psybrus.h"

#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// PsyGameInit
void PsyGameInit()
{

}

//////////////////////////////////////////////////////////////////////////
// PsyLaunchGame
void PsyLaunchGame()
{
	// Spawn entity called "MainEntity" from Dist/Content/default.pkg,
	// and name it "MainEntity_0", and place it in the root of the scene.
	ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			"MainEntity_0", "default", "MainEntity",
			MaMat4d(), nullptr ) );
}
