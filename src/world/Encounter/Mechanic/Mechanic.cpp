#include "Mechanic.h"
#include "MechanicState.h"

#include "Encounter/Encounter.h"

namespace Sapphire::World::Encounter::Mechanic
{
  void Mechanic::arm( EncounterPtr pEncounter, const std::vector< MechanicArg >& args )
  {
    m_state.arm( args );

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
}// namespace Sapphire::World::Encounter::Mechanic