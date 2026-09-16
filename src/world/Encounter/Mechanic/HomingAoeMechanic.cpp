#include "HomingAoeMechanic.h"

#include <Encounter/Encounter.h>

#include <Actor/GameObject.h>
#include <Actor/Chara.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>

#include <Encounter/Mechanic/MechanicHelper.h>

#include <Util/Util.h>

#include <Logging/Logger.h>

namespace Sapphire::World::Encounter::Mechanic
{
  void HomingAoeArg::from_json( const nlohmann::json& json )
  {
    m_actorId = json.at( "actorId" ).get< uint32_t >();
    m_targetId = json.at( "targetId" ).get< uint32_t >();
    m_actionId = json.at( "actionId" ).get< uint32_t >();
    m_castIntervalMs = json.at( "castInterval" ).get< uint64_t >();
    m_onlyCastOnCollision = json.at( "onlyCastOnCollision" ).get< bool >();
  }

  void HomingAoeMechanic::arm( EncounterPtr pEncounter, MechanicArgPtr pArg )
  {
    m_pArg = pArg;
    m_state.arm();
  }

  void HomingAoeMechanic::interrupt( EncounterPtr pEncounter, bool overrideIfComplete )
  {
    // todo: properly handle interrupt
    auto status = m_state.getStatus();
    m_state.interrupt( overrideIfComplete );
  }

  void HomingAoeMechanic::complete( EncounterPtr pEncounter )
  {
    m_state.complete();
  }

  void HomingAoeMechanic::update( EncounterPtr pEncounter )
  {
    auto status = m_state.getStatus();
    if( status == MechanicStatus::Armed )
    {
      auto now = Common::Util::getTimeMs();
      auto lastTick = m_state.getLastTick();

      auto elapsed = now - lastTick;

      if( elapsed < 1500 )
        return;

      // todo: error
      auto pHomingArg = std::dynamic_pointer_cast< HomingAoeArg >( m_pArg );
      if( !pHomingArg )
        return;

      auto pCaster = MechanicHelper::resolveEntity( pEncounter, pHomingArg->m_actorId );
      if( !pCaster )
        return;

      auto pTarget = MechanicHelper::resolveEntity( pEncounter, pHomingArg->m_targetId );
      if( !pTarget )
        return;

      if( pTarget == pCaster )
        return;

      // todo: follow and cast

      m_state.setLastTick( now );
    }
  }

};