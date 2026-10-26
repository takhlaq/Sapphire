#include <ScriptObject.h>

#include <Actor/EventObject.h>
#include <Actor/BNpc.h>
#include <Encounter/Encounter.h>
#include <Encounter/TimelinePack.h>
#include <Logging/Logger.h>

#include <set>
#include <string>

using namespace Sapphire;
using namespace Sapphire::World::Encounter;

class SastashaUnnaturalRipples : public ScriptAPI::MechanicScript
{
public:
  void arm( const nlohmann::json& args, TimelinePack& pack, EncounterPtr pEncounter )
  {
    m_intervalMs = args.value( "intervalMs", uint64_t{ 1000 } );
    m_maxUpdates = args.value( "maxUpdates", uint32_t{ 3 } );
    m_lastTick = 0;
    m_updateCount = 0;
    m_lastUnnaturalRipplesStart = 0;
    m_armed = true;

    m_availableGuards = { 3978770, 3978761, 4064938, 4064937 };

    Logger::info( "SastashaUnnaturalRipples armed" );
  }

  void disarm( const nlohmann::json& args, TimelinePack& pack, EncounterPtr pEncounter )
  {
    m_armed = false;
    Logger::info( "SastashaUnnaturalRipples disarmed" );
  }

  void update( uint64_t tick, TimelinePack& pack, EncounterPtr pEncounter ) override
  {
    if( !m_armed )
      return;

    if( m_lastTick == 0 )
    {
      m_lastTick = tick;
      return;
    }

    if( tick - m_lastTick < m_intervalMs )
      return;

    if( !pEncounter )
      return;

    auto pDenn = pack.getBNpcByRef( "Denn The Orcatoothed", pEncounter );
    if( pDenn && pDenn->getState() == Entity::BNpcState::Combat )
    {
      auto dennHpp = pDenn->getHpPercent();
      bool sendLogMessage = false;

      if( m_lastUnnaturalRipplesStart == 0 )
      {
        Logger::info( "Unnatural ripples last start time 0" );
        m_lastUnnaturalRipplesStart = tick;
      }

      auto elapsed = tick - m_lastUnnaturalRipplesStart;
      if( elapsed > 12000 )
      {
        Logger::info( "Unnatural ripples elapsed" );
        Logger::info( "Denn HP {} MaxHP {} HPP {}", pDenn->getHp(), pDenn->getMaxHp(), dennHpp );

        if( dennHpp < 40 )
        {
          Logger::info( "HP Breakpoint 40" );
          if( !pack.isPhaseActive( "Unnaturalripples_3", "Bubbles" ) )
          {
            pack.requestPhaseTransition( "Unnaturalripples_3", "Bubbles" );
            sendLogMessage = true;
          }
        }

        if( dennHpp < 60 )
        {
          Logger::info( "HP Breakpoint 60" );
          if( !pack.isPhaseActive( "Unnaturalripples_2", "Bubbles" ) )
          {
            pack.requestPhaseTransition( "Unnaturalripples_2", "Bubbles" );
            sendLogMessage = true;
          }
        }

        if( dennHpp < 80 )
        {
          Logger::info( "HP Breakpoint 80" );

          if( !pack.isPhaseActive( "Unnaturalripples", "Bubbles" ) )
          {
            pack.requestPhaseTransition( "Unnaturalripples", "Bubbles" );
            sendLogMessage = true;
          }

          if( !pack.isPhaseActive( "Unnaturalripples_1", "Bubbles" ) )
          {
            pack.requestPhaseTransition( "Unnaturalripples_1", "Bubbles" );
            sendLogMessage = true;
          }
        }

        if( sendLogMessage )
        {
          for( auto& pPlayer : pEncounter->getPlayersInside() )
          {
            playerMgr().sendLogMessage( *pPlayer, 2040 );
          }
          Logger::info( "Unnatural ripples started bubbling" );
          m_lastUnnaturalRipplesStart = tick;
        }
      }

    }

    m_lastTick = tick;
    ++m_updateCount;

    if( m_maxUpdates != 0 && m_updateCount >= m_maxUpdates )
      m_armed = false;
  }

  void spawnGuardForEObj( const nlohmann::json& args, TimelinePack& pack, EncounterPtr pEncounter )
  {
    if( !m_armed )
      return;

    if( !pEncounter )
      return;

    auto& eobjName = args.at( "eobjName" );
    auto pEObj = pEncounter->getEObjByName( eobjName );
    if( pEObj )
    {
      auto pos = pEObj->getPos();
      auto rot = pEObj->getRot();

      pos = Common::Util::getOffsettedPosition( pos, rot, 0.f, 0.f, 1.5f );
      auto pTeri = pEncounter->getTeriPtr();

      auto tmpAvailable = m_availableGuards;
      for( auto guardId : tmpAvailable )
      {
        auto pGuard = pEncounter->getBNpc( guardId );
        bool createBNpc = false;
        if( pGuard )
        {
          if( !pGuard->isAlive() )
          {
            pTeri->removeActor( pGuard );
            createBNpc = true;
          }
        }
        else
        {
          createBNpc = true;
        }

        if( createBNpc )
        {
          // todo: use correct hp values
          pGuard = pTeri->createBNpcFromLayoutIdNoPush( guardId, 319, Common::BNpcType::Enemy, 0 );
          if( pGuard )
          {
            pEncounter->addBNpc( pGuard, pEncounter->getLockoutTime() != 0 );
            pGuard->setRot( rot );
            pGuard->setPos( pos );
            pGuard->init();
            pTeri->pushActor( pGuard );
            return;
          }
        }
      }
    }
  }

private:
  bool m_armed{ false };

  uint64_t m_intervalMs{ 1000 };
  uint64_t m_lastTick{ 0 };
  uint64_t m_lastUnnaturalRipplesStart{ 0 };
  uint32_t m_maxUpdates{ 0 };
  uint32_t m_updateCount{ 0 };
  std::set< uint32_t > m_availableGuards;
};

EXPOSE_MECHANIC_SCRIPT(
  SastashaUnnaturalRipples,
  MECHANIC_FUNCTION( arm ),
  MECHANIC_FUNCTION( disarm ),
  MECHANIC_FUNCTION( spawnGuardForEObj ),
);
