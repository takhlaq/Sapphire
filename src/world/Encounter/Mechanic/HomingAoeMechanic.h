#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Mechanic.h"

#include <Encounter/Forwards.h>

#include <nlohmann/json.hpp>

namespace Sapphire::World::Encounter::Mechanic
{
  struct HomingAoeArg : public MechanicArg
  {
  public:
    uint32_t m_actorId{ 0xE0000000 };
    uint32_t m_targetId{ 0xE0000000 };
    uint64_t m_castIntervalMs{ 0 };
    uint32_t m_actionId{ 0 };
    bool m_onlyCastOnCollision{ false };

    void from_json( const nlohmann::json& j ) override;
  };
  using HomingAoeArgPtr = std::shared_ptr< HomingAoeArg >;

  class HomingAoeMechanic : public Mechanic
  {
  public:
    HomingAoeMechanic() :
      Mechanic( MechanicId::HomingAoe )
    {
    }

    void arm( EncounterPtr pEncounter, MechanicArgPtr pArg );

    void interrupt( EncounterPtr pEncounter, bool overrideIfComplete = false );

    void complete( EncounterPtr pEncounter );

    void update( EncounterPtr pEncounter );
  };
};