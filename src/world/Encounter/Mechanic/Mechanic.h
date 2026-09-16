#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <Util/Util.h>

#include <Encounter/Forwards.h>
#include <Encounter/Mechanic/MechanicState.h>

namespace Sapphire::World::Encounter::Mechanic
{
  enum class MechanicId : uint32_t
  {
    None,
    HomingAoe
  };

  class Mechanic :
    public std::enable_shared_from_this< Mechanic >
  {
  public:
    Mechanic( MechanicId id ) : m_id( id ) {}
    ~Mechanic() {}

    // todo: sanely pass snapshots
    virtual void arm( EncounterPtr pEncounter, const std::vector< MechanicArg >& args );

    virtual void interrupt( EncounterPtr pEncounter, bool overrideIfComplete = false );

    virtual void complete( EncounterPtr pEncounter );

    virtual void update( EncounterPtr pEncounter );

    const MechanicState& getState() const;

  protected:
    MechanicId m_id;
    MechanicState m_state;
    std::vector< std::shared_ptr< Mechanic > > m_subMechanics;
  };
}