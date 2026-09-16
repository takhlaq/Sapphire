#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <Util/Util.h>

#include <Encounter/Forwards.h>
#include <Encounter/Mechanic/MechanicState.h>

#include <nlohmann/json.hpp>

namespace Sapphire::World::Encounter::Mechanic
{
  enum class MechanicId : uint32_t
  {
    None,
    HomingAoe
  };

  struct MechanicArg : public std::enable_shared_from_this< MechanicArg >
  {
    virtual void from_json( const nlohmann::json& j ) {}
  };
  using MechanicArgPtr = std::shared_ptr< MechanicArg >;

  class Mechanic :
    public std::enable_shared_from_this< Mechanic >
  {
  public:
    Mechanic( MechanicId id ) : m_id( id ) {}
    ~Mechanic() {}

    // todo: sanely pass snapshots
    virtual void arm( EncounterPtr pEncounter, MechanicArgPtr pArg );

    virtual void interrupt( EncounterPtr pEncounter, bool overrideIfComplete = false );

    virtual void complete( EncounterPtr pEncounter );

    virtual void update( EncounterPtr pEncounter );

    const MechanicState& getState() const;

    static MechanicPtr from_json( const nlohmann::json& json );

  protected:
    MechanicId m_id;
    MechanicState m_state;
    MechanicArgPtr m_pArg;
    std::vector< std::shared_ptr< Mechanic > > m_subMechanics;
  };
}