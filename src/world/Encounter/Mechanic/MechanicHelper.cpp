#include "MechanicHelper.h"

#include <Encounter/Encounter.h>

using namespace Sapphire::World;

namespace Sapphire::World::Encounter::Mechanic
{
  Entity::GameObjectPtr MechanicHelper::resolveEntity( EncounterPtr pEncounter, uint32_t actorId )
  {
    for( auto& pActor : pEncounter->getActorsInside() )
    {
      if( pActor->getId() == actorId )
        return pActor;
    }
    return nullptr;
  }
}