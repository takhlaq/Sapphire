#include "StateFollowTarget.h"

#include <Common.h>
#include <Util/Util.h>
#include <Util/UtilMath.h>
#include <Navi/NaviProvider.h>

#include <Actor/GameObject.h>

#include <AI/Controller/Controller.h>

#include <Territory/Territory.h>
#include <Manager/TerritoryMgr.h>
#include <Service.h>

namespace Sapphire::World::AI::Fsm
{
  void StateFollowTarget::onEnter( Entity::GameObjectPtr& pEntity )
  {
    if( auto pController = pEntity->getController() )
    {
      pController->getPath().reset();
    }
  }

  void StateFollowTarget::onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tick )
  {
    auto elapsed = Common::Util::getTimeMs() - m_lastTick;

    // dont spam
    if( elapsed < 250 )
      return;

    auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
    auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );
    if( !pZone )
      return;

    auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() );
    if( !pTarget )
    {
      pEntity->setFollowTargetId( Common::INVALID_GAME_OBJECT_ID );
      return;
    }

    auto pNavi = pZone->getNaviProvider();
    if( !pNavi )
      return;

    auto posOffset = Common::Util::getOffsettedPosition( pTarget->getPos(), pTarget->getRot(), 0.f, 0.f, -1.f );

    if( auto pBNpc = pEntity->getAsBNpc() )
    {
      if( !pBNpc->isAlive() )
      {
        pBNpc->setFollowTargetId( Common::INVALID_GAME_OBJECT_ID );
        return;
      }

      if( pBNpc->moveTo( posOffset ) )
      {
        pBNpc->setNaviIsPathing( false );
        pBNpc->setRoamTargetReached( true );
        pBNpc->setRoamTargetPos( posOffset );
        return;
      }
      pNavi->setMoveTarget( pBNpc->getAgentId(), posOffset );
      pBNpc->setRoamTargetPos( posOffset );
      pBNpc->face( pTarget->getPos() );
    }


    m_lastTick = tick;
  }

  void StateFollowTarget::onExit( Entity::GameObjectPtr& pEntity )
  {
    if( auto pBNpc = pEntity->getAsBNpc() )
    {
      auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
      auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );
      if( !pZone )
        return;

      auto pNavi = pZone->getNaviProvider();
      if( !pNavi )
        return;

      auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() );
      if( !pTarget )
        return;

      pBNpc->face( pTarget->getPos() );
    }
  }
};