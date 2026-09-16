#include "Mechanic.h"
#include "MechanicState.h"

#include "Encounter/Encounter.h"

#include <Encounter/Mechanic/HomingAoeMechanic.h>

#include <map>

namespace Sapphire::World::Encounter::Mechanic
{
  void Mechanic::arm( EncounterPtr pEncounter, MechanicArgPtr pArg )
  {
    m_state.arm();
    m_pArg = pArg;
    // derived class should handle sub mechanics
  }

  void Mechanic::interrupt( EncounterPtr pEncounter, bool overrideIfComplete )
  {
    m_state.interrupt( overrideIfComplete );

    // derived class should handle sub mechanics
  }

  void Mechanic::complete( EncounterPtr pEncounter )
  {
    m_state.complete();

    // derived class should handle sub mechanics
  }

  void Mechanic::update( EncounterPtr pEncounter )
  {
    auto now = Common::Util::getTimeMs();
    m_state.setLastTick( now );

    // derived class should handle mechanic and sub mechanics
  }

  const MechanicState& Mechanic::getState() const
  {
    return m_state;
  }

  MechanicPtr Mechanic::from_json( const nlohmann::json& j )
  {
    MechanicPtr pRet{ nullptr };
    static std::unordered_map< std::string, MechanicId > mechanicIdMap =
    {
      { "HomingAoe", MechanicId::HomingAoe },
    };

    auto idStr = j.at( "id" ).get< std::string >();
    auto idIt = mechanicIdMap.find( idStr );
    if( idIt == mechanicIdMap.end() )
      return nullptr;

    auto id = idIt->second;
    switch( id )
    {
      case MechanicId::HomingAoe:
      {
        pRet = std::make_shared< HomingAoeMechanic >();
        break;
      }
      default:
        // todo: error
        break;
    }
    return pRet;
  }
}// namespace Sapphire::World::Encounter::Mechanic