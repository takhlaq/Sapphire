#include "RevealBNpcTask.h"

#include <Actor/BNpc.h>
#include <Manager/TerritoryMgr.h>
#include <Network/CommonActorControl.h>
#include <Network/Util/PacketUtil.h>
#include <Service.h>
#include <Territory/Territory.h>
#include <utility>

using namespace Sapphire::World;

RevealBNpcTask::RevealBNpcTask( uint64_t delayTime, Entity::BNpcPtr pBNpc ) :
  Task( delayTime ),
  m_pBNpc( std::move( pBNpc ) )
{
}

void RevealBNpcTask::onQueue()
{
  m_pBNpc->setAwaitingSpawnReveal( true );
}

void RevealBNpcTask::execute()
{
  m_pBNpc->setAwaitingSpawnReveal( false );
  auto pTeri = Common::Service< Manager::TerritoryMgr >::ref().getTerritoryByGuId( m_pBNpc->getTerritoryId() );
  if( pTeri && pTeri->getActiveBNpcByEntityId( m_pBNpc->getId() ) == m_pBNpc )
    Network::Util::Packet::sendBNpcSpawnControl( *m_pBNpc, Network::ActorControl::SpawnEffect, m_pBNpc->isAlive() ? 1 : 0 );
}

std::string RevealBNpcTask::toString()
{
  return fmt::format( "RevealBNpcTask: BNpc#{}", m_pBNpc->getId() );
}
