#include <Encounter/Forwards.h>

#include <Actor/GameObject.h>

namespace Sapphire::World::Encounter::Mechanic
{
  class MechanicHelper
  {
  public:
    static Sapphire::Entity::GameObjectPtr resolveEntity( EncounterPtr pEncounter, uint32_t actorId );
  };
}