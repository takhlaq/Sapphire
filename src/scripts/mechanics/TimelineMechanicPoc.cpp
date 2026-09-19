#include <ScriptObject.h>

#include <Actor/BNpc.h>
#include <Encounter/Encounter.h>
#include <Encounter/TimelinePack.h>
#include <Logging/Logger.h>

#include <memory>
#include <string>
#include <string_view>

using namespace Sapphire;
using namespace Sapphire::World::Encounter;

class TimelineMechanicPoc : public ScriptAPI::MechanicScript
{
public:
  TimelineMechanicPoc() : MechanicScript( "TimelineMechanicPoc" )
  {
  }

  std::shared_ptr< MechanicScript > createInstance() const override
  {
    return std::make_shared< TimelineMechanicPoc >();
  }

  bool call( std::string_view function, const nlohmann::json& args,
             TimelinePack& pack, EncounterPtr pEncounter ) override
  {
    if( function == "arm" )
    {
      m_actorRef = args.value( "actor", std::string() );
      m_intervalMs = args.value( "intervalMs", uint64_t{ 1000 } );
      m_maxUpdates = args.value( "maxUpdates", uint32_t{ 3 } );
      m_lastTick = 0;
      m_updateCount = 0;
      m_armed = true;

      Logger::info( "TimelineMechanicPoc armed for actor '{}'", m_actorRef );
      return true;
    }

    if( function == "disarm" )
    {
      m_armed = false;
      Logger::info( "TimelineMechanicPoc disarmed" );
      return true;
    }

    return false;
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

    m_lastTick = tick;
    ++m_updateCount;

    auto pBNpc = pEncounter && !m_actorRef.empty() ? pack.getBNpcByRef( m_actorRef, pEncounter ) : nullptr;
    if( pBNpc )
      Logger::info( "TimelineMechanicPoc update {} for BNpc {}", m_updateCount, pBNpc->getId() );
    else
      Logger::info( "TimelineMechanicPoc update {}", m_updateCount );

    if( m_maxUpdates != 0 && m_updateCount >= m_maxUpdates )
      m_armed = false;
  }

private:
  bool m_armed{ false };
  std::string m_actorRef;
  uint64_t m_intervalMs{ 1000 };
  uint64_t m_lastTick{ 0 };
  uint32_t m_maxUpdates{ 3 };
  uint32_t m_updateCount{ 0 };
};

EXPOSE_SCRIPT( TimelineMechanicPoc );
